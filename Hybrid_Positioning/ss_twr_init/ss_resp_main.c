// ============================ ss_resp_main.c ============================
#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_hybrid.h"

#ifndef NODE_ID
#define NODE_ID 0
#endif
#define MY_ANCHOR_ID  NODE_ID

#define POLL_MSG_DEST_ID_IDX  10
#define RNG_DELAY_MS 5
#define RX_TIMEOUT_UUS 5000

// ===== MẪU POLL (GIỮ NGUYÊN) =====
static uint8 rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0xFF, 0, 0};
static uint8 tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ===== ĐỊNH NGHĨA BLINK (TDOA) – TAG GỬI BROADCAST =====
#define BLINK_FUNC_CODE           0xE2
#define BLINK_MSG_FUNC_IDX        9
#define BLINK_MSG_CYCLE_LSB_IDX   11
#define BLINK_MSG_CYCLE_MSB_IDX   12

// ===== TDOA REPORT (SLAVE -> MASTER) =====
#define TDOA_REP_FUNC_CODE        0xE3
#define TDOA_REP_MSG_FUNC_IDX     9
#define TDOA_REP_MSG_DEST_IDX     10
#define TDOA_REP_MSG_SRC_IDX      11
#define TDOA_REP_MSG_CYCLE_LSB    12
#define TDOA_REP_MSG_CYCLE_MSB    13
#define TDOA_REP_MSG_TS_IDX       14   // 5 bytes: [14..18]

#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4

static uint8 frame_seq_nb = 0;

#define RX_BUF_LEN 24
static uint8 rx_buffer[RX_BUF_LEN];

static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME 65536
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

static uint64 poll_rx_ts;
static uint64 resp_tx_ts;

static uint64 get_rx_timestamp_u64(void);
static void   resp_msg_set_ts(uint8 *ts_field, const uint64 ts);

static void send_tdoa_report_to_master(uint16 cycle_id, uint64 ts)
{
    // 10 common bytes + dest + src + cycle(2) + ts(5) = 19 bytes
    static uint8 tx_tdoa_rep_msg[19] = {
        0x41, 0x88, 0, 0xCA, 0xDE, 'W','A','V','E',
        TDOA_REP_FUNC_CODE,
        0x00, // DEST (master=0)
        0x00, // SRC
        0x00, 0x00, // cycle lsb/msb
        0,0,0,0,0   // ts[5]
    };

    tx_tdoa_rep_msg[ALL_MSG_SN_IDX]        = frame_seq_nb;
    tx_tdoa_rep_msg[TDOA_REP_MSG_DEST_IDX] = 0;                 // master = 0
    tx_tdoa_rep_msg[TDOA_REP_MSG_SRC_IDX]  = (uint8)MY_ANCHOR_ID;

    tx_tdoa_rep_msg[TDOA_REP_MSG_CYCLE_LSB] = (uint8)(cycle_id & 0xFF);
    tx_tdoa_rep_msg[TDOA_REP_MSG_CYCLE_MSB] = (uint8)((cycle_id >> 8) & 0xFF);

    for (int i = 0; i < 5; i++)
        tx_tdoa_rep_msg[TDOA_REP_MSG_TS_IDX + i] = (uint8)((ts >> (8 * i)) & 0xFF);

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    dwt_writetxdata(sizeof(tx_tdoa_rep_msg), tx_tdoa_rep_msg, 0);
    dwt_writetxfctrl(sizeof(tx_tdoa_rep_msg), 0, 1);

    (void)dwt_starttx(DWT_START_TX_IMMEDIATE);
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    frame_seq_nb++;
}

static uint32 total_received = 0;
static uint32 for_me = 0;
static uint32 replied = 0;
static uint32 ignored = 0;

int ss_resp_run(void)
{
    dwt_rxenable(DWT_START_RX_IMMEDIATE);

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
        vTaskDelay(1);
    }

    uint32 frame_len = 0;
    int rx_success = 0;

    if (status_reg & SYS_STATUS_RXFCG)
    {
        total_received++;

        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;

        if (frame_len <= RX_BUF_LEN)
        {
            dwt_readrxdata(rx_buffer, frame_len, 0);
            rx_success = 1;
        }
    }

    dwt_write32bitreg(SYS_STATUS_ID,
                      SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

    if (rx_success)
    {
        uint8 func_code = rx_buffer[BLINK_MSG_FUNC_IDX];

        // ---------- CASE 0: TDOA REPORT (SLAVE -> MASTER) ----------
        if (func_code == TDOA_REP_FUNC_CODE)
        {
            if (MY_ANCHOR_ID != 0) return 1;
            if (frame_len < 19) return 1;

            uint8  src = rx_buffer[TDOA_REP_MSG_SRC_IDX];
            uint16 cycle_id = ((uint16)rx_buffer[TDOA_REP_MSG_CYCLE_MSB] << 8) |
                              (uint16)rx_buffer[TDOA_REP_MSG_CYCLE_LSB];

            uint64 ts = 0;
            for (int i = 0; i < 5; i++)
                ts |= ((uint64)rx_buffer[TDOA_REP_MSG_TS_IDX + i]) << (8 * i);

            master_hybrid_handle_uwb_tdoa(src, cycle_id, ts);
if (src != 0)
{
    uint8 ts_tab[5];
    uint64 ref_ts = 0;

    dwt_readsystime(ts_tab);   

    for (int i = 4; i >= 0; i--)
    {
        ref_ts <<= 8;
        ref_ts |= ts_tab[i];
    }

    master_hybrid_handle_uwb_tdoa(0, cycle_id, ref_ts);
}


            return 1;
        }

        // ---------- CASE 1: BLINK (TDOA) ----------
        if (func_code == BLINK_FUNC_CODE)
        {
            uint16 cycle_id = ((uint16)rx_buffer[BLINK_MSG_CYCLE_MSB_IDX] << 8) |
                              (uint16)rx_buffer[BLINK_MSG_CYCLE_LSB_IDX];

            uint64 blink_rx_ts = get_rx_timestamp_u64();

            printf("[A%d] BLINK: cycle=%u ts=%llu\r\n",
                   MY_ANCHOR_ID, cycle_id, (unsigned long long)blink_rx_ts);

            if (MY_ANCHOR_ID == 0)
            {
                // master lưu ref timestamp của chính nó
                master_hybrid_handle_uwb_tdoa(0, cycle_id, blink_rx_ts);
            }
            else
            {
                // slave report timestamp về master qua UWB
                send_tdoa_report_to_master(cycle_id, blink_rx_ts);
            }
            return 1;
        }

        // ---------- CASE 2: POLL (TWR) – GIỮ NGUYÊN ----------
        uint8 dest_id = rx_buffer[POLL_MSG_DEST_ID_IDX];

        printf("[A%d] RX: dest=%d | len=%lu | ", MY_ANCHOR_ID, dest_id, frame_len);
        for (int i=0; i<8 && i<(int)frame_len; i++) printf("%02X ", rx_buffer[i]);
        printf("\r\n");

        if (dest_id != MY_ANCHOR_ID)
        {
            ignored++;
            printf("[A%d] SKIP (not for me) [Stats: rx=%lu, for_me=%lu, replied=%lu, ignored=%lu]\r\n",
                   MY_ANCHOR_ID, total_received, for_me, replied, ignored);
            return 1;
        }

        for_me++;

        uint8 rx_buffer_check[RX_BUF_LEN];
        memcpy(rx_buffer_check, rx_buffer, frame_len);
        rx_buffer_check[ALL_MSG_SN_IDX]        = 0;
        rx_buffer_check[POLL_MSG_DEST_ID_IDX]  = 0xFF;

        if (memcmp(rx_buffer_check, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            printf("[A%d] VALID POLL for me!\r\n", MY_ANCHOR_ID);

            poll_rx_ts = get_rx_timestamp_u64();

            uint32 resp_tx_time = (poll_rx_ts +
                    (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            dwt_setdelayedtrxtime(resp_tx_time);
            resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);
            tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;

            dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
            dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

            printf("[A%d] → Sending RESP...\r\n", MY_ANCHOR_ID);
            int ret = dwt_starttx(DWT_START_TX_DELAYED);

            if (ret == DWT_SUCCESS)
            {
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                frame_seq_nb++;
                replied++;
                printf("[A%d]  RESP sent OK [Stats: rx=%lu, for_me=%lu, replied=%lu, ignored=%lu]\r\n",
                       MY_ANCHOR_ID, total_received, for_me, replied, ignored);
            }
            else
            {
                printf("[A%d]  RESP send FAILED (late)\r\n", MY_ANCHOR_ID);
            }
        }
        else
        {
            printf("[A%d]  Wrong POLL format\r\n", MY_ANCHOR_ID);
        }
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
    }

    return 1;
}

static uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

static void resp_msg_set_ts(uint8 *ts_field, const uint64 ts)
{
    for (int i = 0; i < RESP_MSG_TS_LEN; i++)
    {
        ts_field[i] = (ts >> (i * 8)) & 0xFF;
    }
}

void ss_responder_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    dwt_setleds(DWT_LEDS_ENABLE);

    printf("[A%d]  ANCHOR RESPONDER STARTED\r\n", MY_ANCHOR_ID);

    if (MY_ANCHOR_ID == 0)
    {
        master_hybrid_init();
        master_hybrid_reset();
    }

    while (true)
    {
        ss_resp_run();
        vTaskDelay(RNG_DELAY_MS);
    }
}
