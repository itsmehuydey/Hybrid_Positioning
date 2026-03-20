#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_hybrid.h"
#include "ble_tdoa_report.h"
#include "ble_beacon.h"
#include "ble_scanner.h"

#ifndef NODE_ID
#define NODE_ID 0
#endif
#define MY_ANCHOR_ID  NODE_ID

#define RNG_DELAY_MS              5
#define BLINK_MSG_DEST_IDX        10

static uint8 rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0xFF, 0, 0};
static uint8 tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define BLINK_FUNC_CODE           0xE2
#define BLINK_MSG_FUNC_IDX        9
#define POLL_MSG_DEST_ID_IDX      10
#define BLINK_MSG_CYCLE_LSB_IDX   11
#define BLINK_MSG_CYCLE_MSB_IDX   12

#define ALL_MSG_COMMON_LEN        10
#define ALL_MSG_SN_IDX            2
#define RESP_MSG_POLL_RX_TS_IDX   10
#define RESP_MSG_RESP_TX_TS_IDX   14
#define RESP_MSG_TS_LEN           4

static uint8 frame_seq_nb = 0;

#define RX_BUF_LEN  64
static uint8 rx_buffer[RX_BUF_LEN];
static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME 65536
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

static uint64 poll_rx_ts;
static uint64 resp_tx_ts;

static uint64 get_rx_timestamp_u64(void);
static void   resp_msg_set_ts(uint8 *ts_field, const uint64 ts);

/* ------------------------------------------------------------------
   [1] Trạng thái sync CCP của anchor này
       Master gửi BLINK(k) rồi BLINK(k+1) – anchor ghi lại 2 ts
   ------------------------------------------------------------------ */
typedef struct {
    uint16_t cycle_id;
    uint64_t syn_k;       /* ts nhận BLINK(k)     */
    uint64_t syn_k1;      /* ts nhận BLINK(k+1)   */
    uint64_t T_i;         /* ts nhận packet tag   */
    uint8_t  has_syn_k;
    uint8_t  has_syn_k1;
    uint8_t  has_Ti;
} anchor_sync_state_t;

static anchor_sync_state_t g_sync = {0};

static inline void uwb_clear_rx_events(void)
{
    dwt_write32bitreg(SYS_STATUS_ID,
                      SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
}

static inline void uwb_force_rx_on(void)
{
    uwb_clear_rx_events();
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

/* ------------------------------------------------------------------
   [3] Gửi BLE report về Master
       Chỉ gửi khi đã có đủ syn_k + syn_k1 + T_i
   ------------------------------------------------------------------ */
static void send_tdoa_report_ble(uint16_t cycle_id)
{
    if (!g_sync.has_syn_k || !g_sync.has_syn_k1 || !g_sync.has_Ti) return;

    ble_tdoa_report_t pkt;
    pkt.msg_type  = 'S';
    pkt.anchor_id = (uint8_t)MY_ANCHOR_ID;
    pkt.cycle_id  = cycle_id;
    pkt.syn_k     = g_sync.syn_k;
    pkt.syn_k1    = g_sync.syn_k1;
    pkt.T_i       = g_sync.T_i;

    ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));

    printf("[A%d] BLE SEND 'S' | cycle=%u syn_k=%llu syn_k1=%llu Ti=%llu\r\n",
           MY_ANCHOR_ID, cycle_id,
           (unsigned long long)g_sync.syn_k,
           (unsigned long long)g_sync.syn_k1,
           (unsigned long long)g_sync.T_i);
}

int ss_resp_run(void)
{
    uwb_force_rx_on();

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
        vTaskDelay(1);
    }

    uint32 frame_len = 0;
    int rx_success = 0;

    if (status_reg & SYS_STATUS_RXFCG)
    {
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;

        if (frame_len <= RX_BUF_LEN)
        {
            dwt_readrxdata(rx_buffer, frame_len, 0);
            rx_success = 1;
        }
    }

    uwb_clear_rx_events();

    if (rx_success)
    {
        if (frame_len <= BLINK_MSG_FUNC_IDX)
        {
            uwb_force_rx_on();
            return 1;
        }

        uint8 func_code = rx_buffer[BLINK_MSG_FUNC_IDX];

        /* --------------------------------------------------------------
           [1] Nhận BLINK từ Master
           Master gửi 2 BLINK liên tiếp với cùng cycle_id.
           Lần đầu → ghi syn_k; lần hai → ghi syn_k1.
           -------------------------------------------------------------- */
        if (func_code == BLINK_FUNC_CODE)
        {
            if (frame_len <= BLINK_MSG_CYCLE_MSB_IDX) {
                uwb_force_rx_on();
                return 1;
            }

            if (rx_buffer[BLINK_MSG_DEST_IDX] != 0xFF) {
                uwb_force_rx_on();
                return 1;
            }

            uint16_t cycle_id =
                ((uint16_t)rx_buffer[BLINK_MSG_CYCLE_MSB_IDX] << 8) |
                 (uint16_t)rx_buffer[BLINK_MSG_CYCLE_LSB_IDX];

            uint64_t blink_rx_ts = get_rx_timestamp_u64();

            printf("[A%d] RX BLINK | cycle=%u\r\n", MY_ANCHOR_ID, cycle_id);

            /* Phân biệt BLINK(k) và BLINK(k+1) theo thứ tự nhận
               trong cùng cycle_id                                       */
            if (g_sync.cycle_id != cycle_id ||
                (!g_sync.has_syn_k && !g_sync.has_syn_k1))
            {
                /* BLINK(k) – lần đầu của cycle mới */
                memset(&g_sync, 0, sizeof(g_sync));
                g_sync.cycle_id  = cycle_id;
                g_sync.syn_k     = blink_rx_ts;
                g_sync.has_syn_k = 1;
                printf("[A%d] SYN_K saved | cycle=%u\r\n", MY_ANCHOR_ID, cycle_id);
            }
            else if (g_sync.has_syn_k && !g_sync.has_syn_k1)
            {
                /* BLINK(k+1) – lần thứ hai cùng cycle */
                g_sync.syn_k1     = blink_rx_ts;
                g_sync.has_syn_k1 = 1;
                printf("[A%d] SYN_K1 saved | cycle=%u\r\n", MY_ANCHOR_ID, cycle_id);

                /* Nếu đã có T_i trước đó thì gửi ngay */
                if (g_sync.has_Ti) {
                    send_tdoa_report_ble(cycle_id);
                }
            }
            /* else: BLINK dư, bỏ qua */

            uwb_force_rx_on();
            return 1;
        }

        /* --------------------------------------------------------------
           Nhận POLL từ TAG (Two-Way Ranging)
           -------------------------------------------------------------- */
        uint8 dest_id = rx_buffer[POLL_MSG_DEST_ID_IDX];

        if (dest_id != (uint8)MY_ANCHOR_ID)
        {
            uwb_force_rx_on();
            return 1;
        }

        uint8 rx_buffer_check[RX_BUF_LEN];
        memcpy(rx_buffer_check, rx_buffer, frame_len);
        rx_buffer_check[ALL_MSG_SN_IDX]       = 0;
        rx_buffer_check[POLL_MSG_DEST_ID_IDX] = 0xFF;

        if (memcmp(rx_buffer_check, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            poll_rx_ts = get_rx_timestamp_u64();

            /* [2] Lưu T_i – timestamp nhận packet tag */
            if (g_sync.cycle_id != 0 && g_sync.has_syn_k)
            {
                g_sync.T_i    = poll_rx_ts;
                g_sync.has_Ti = 1;
                printf("[A%d] T_i saved | cycle=%u Ti=%llu\r\n",
                       MY_ANCHOR_ID, g_sync.cycle_id,
                       (unsigned long long)poll_rx_ts);

                /* Nếu đã có cả syn_k + syn_k1 thì gửi BLE ngay */
                if (g_sync.has_syn_k1) {
                    send_tdoa_report_ble(g_sync.cycle_id);
                }
            }

            /* Trả RESP để TWR vẫn hoạt động (TOF đo từ TAG) */
            uint32 resp_tx_time = (poll_rx_ts +
                    (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            dwt_setdelayedtrxtime(resp_tx_time);
            resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);
            tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;

            dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
            dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

            int ret = dwt_starttx(DWT_START_TX_DELAYED);

            if (ret == DWT_SUCCESS)
            {
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                frame_seq_nb++;
            }
            else
            {
                printf("[A%d] RESP send FAILED (late)\r\n", MY_ANCHOR_ID);
            }

            uwb_force_rx_on();
        }
        else
        {
            printf("[A%d] Wrong POLL format\r\n", MY_ANCHOR_ID);
            uwb_force_rx_on();
        }
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
        uwb_force_rx_on();
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

    printf("[A%d] ANCHOR RESPONDER STARTED (NODE_ID=%d)\r\n",
           MY_ANCHOR_ID, NODE_ID);

    dwt_setleds(DWT_LEDS_ENABLE);
    dwt_enableframefilter(0);

    if (MY_ANCHOR_ID == 0)
    {
        master_hybrid_init();
        master_hybrid_reset();
    }

    uint8_t ble_buf[64];
    uint16_t ble_len;

    uwb_force_rx_on();

    while (1)
    {
        ss_resp_run();

        if (MY_ANCHOR_ID == 0)
        {
            if (ble_scan_packet(ble_buf, &ble_len))
            {
                printf("[MASTER][BLE RX] len=%u\r\n", ble_len);
                master_hybrid_handle_ble_data(ble_buf, ble_len);
            }
        }

        vTaskDelay(1);
    }
}