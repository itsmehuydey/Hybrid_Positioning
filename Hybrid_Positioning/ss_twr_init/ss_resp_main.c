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
#define MY_ANCHOR_ID NODE_ID

static uint8 rx_poll_msg[] = {0x41,0x88,0,0xCA,0xDE,'W','A','V','E',0xE0,0xFF,0,0};
static uint8 tx_resp_msg[] = {0x41,0x88,0,0xCA,0xDE,'V','E','W','A',0xE1,0,0,0,0,0,0,0,0,0,0};

#define BLINK_FUNC_CODE         0xE2
#define BLINK_MSG_FUNC_IDX      9
#define BLINK_MSG_DEST_IDX      10
#define BLINK_MSG_CYCLE_LSB_IDX 11
#define BLINK_MSG_CYCLE_MSB_IDX 12
#define POLL_MSG_DEST_ID_IDX    10

#define ALL_MSG_COMMON_LEN      10
#define ALL_MSG_SN_IDX          2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN         4

static uint8 frame_seq_nb = 0;

#define RX_BUF_LEN 64
static uint8  rx_buffer[RX_BUF_LEN];
static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME            65536
#define POLL_RX_TO_RESP_TX_DLY_UUS 2500

static uint64 poll_rx_ts;
static uint64 resp_tx_ts;

static uint64 get_rx_timestamp_u64(void);
static void   resp_msg_set_ts(uint8 *ts_field, const uint64 ts);

/* ====================================================================
   SYNC STATE – sliding window, 2 BLINK liên tiếp bất kể cycle_id
   ====================================================================
   - cycle_k  / syn_k  : BLINK đầu
   - cycle_k1 / syn_k1 : BLINK kế tiếp (cycle_id khác cycle_k)
   - T_i               : POLL nhận được (CHỈ dùng cho slave, A0 không cần)

   A0 (master/ref):
     - Dùng syn_k1 làm T_ref (không cần POLL)
     - Self-feed ngay sau khi có syn_k + syn_k1
     - cycle_id = cycle_k1

   Slave:
     - Cần đủ syn_k + syn_k1 + T_i
     - Gửi BLE 'S', cycle_id = cycle_k1
   ==================================================================== */
typedef struct {
    uint16_t cycle_k;
    uint64_t syn_k;
    uint8_t  has_syn_k;

    uint16_t cycle_k1;
    uint64_t syn_k1;
    uint8_t  has_syn_k1;

    uint64_t T_i;
    uint8_t  has_Ti;

    uint32_t created_tick;
} anchor_sync_state_t;

static anchor_sync_state_t g_sync = {0};

#define SYNC_TIMEOUT_TICKS pdMS_TO_TICKS(5000)

/* ====================================================================
   BLE TX QUEUE (chỉ dùng cho slave)
   ==================================================================== */
static ble_tdoa_report_t g_ble_pending_pkt;
static volatile uint8_t  g_ble_pending    = 0;
static volatile uint8_t  g_ble_send_count = 0;
#define BLE_SEND_REPEAT      6
#define BLE_SEND_INTERVAL_MS 10

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

/* ====================================================================
   Commit sync report
   A0: self-feed trực tiếp dùng syn_k1 làm T_i (không cần POLL)
   Slave: queue BLE
   ==================================================================== */
static void commit_sync_report(void)
{
    if (!g_sync.has_syn_k || !g_sync.has_syn_k1) return;

    /* Slave cần thêm T_i */
    if (MY_ANCHOR_ID != 0 && !g_sync.has_Ti) return;

    ble_tdoa_report_t rpt;
    rpt.msg_type  = 'S';
    rpt.anchor_id = (uint8_t)MY_ANCHOR_ID;
    rpt.cycle_id  = g_sync.cycle_k1;
    rpt.syn_k     = g_sync.syn_k;
    rpt.syn_k1    = g_sync.syn_k1;
    /* A0: T_i = syn_k1 (thời điểm nhận BLINK cuối = ref baseline)
       Slave: T_i = timestamp nhận POLL                              */
    rpt.T_i       = (MY_ANCHOR_ID == 0) ? g_sync.syn_k1 : g_sync.T_i;

    if (MY_ANCHOR_ID == 0)
    {
        master_hybrid_handle_ble_tdoa_report(&rpt);
        printf("[A0] SYNC ck=%u ck1=%u\r\n", g_sync.cycle_k, g_sync.cycle_k1);
    }
    else
    {
        g_ble_pending_pkt = rpt;
        g_ble_send_count  = 0;
        g_ble_pending     = 1;
        printf("[A%d] SYNC queued ck=%u ck1=%u\r\n",
               MY_ANCHOR_ID, g_sync.cycle_k, g_sync.cycle_k1);
    }

    /* Slide: syn_k1 → syn_k mới */
    g_sync.cycle_k      = g_sync.cycle_k1;
    g_sync.syn_k        = g_sync.syn_k1;
    g_sync.has_syn_k    = 1;
    g_sync.has_syn_k1   = 0;
    g_sync.has_Ti       = 0;
    g_sync.created_tick = xTaskGetTickCount();
}

static void flush_ble_pending(void)
{
    if (!g_ble_pending) return;
    if (g_ble_send_count >= BLE_SEND_REPEAT) { g_ble_pending = 0; return; }

    ble_raw_beacon_send_payload(
        (uint8_t *)&g_ble_pending_pkt, sizeof(ble_tdoa_report_t));
    g_ble_send_count++;

    if (g_ble_send_count == 1)
        printf("[A%d] BLE S ck1=%u\r\n",
               MY_ANCHOR_ID, g_ble_pending_pkt.cycle_id);

    if (g_ble_send_count >= BLE_SEND_REPEAT) g_ble_pending = 0;
}

/* ====================================================================
   UWB RX
   ==================================================================== */
int ss_resp_run(void)
{
    uwb_force_rx_on();

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
        vTaskDelay(1);

    uint32 frame_len = 0;
    int    rx_ok     = 0;

    if (status_reg & SYS_STATUS_RXFCG)
    {
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (frame_len <= RX_BUF_LEN) { dwt_readrxdata(rx_buffer, frame_len, 0); rx_ok = 1; }
    }

    uwb_clear_rx_events();

    if (!rx_ok) {
        dwt_rxreset(); uwb_force_rx_on(); return 1;
    }

    if (frame_len <= BLINK_MSG_FUNC_IDX) { uwb_force_rx_on(); return 1; }

    uint8 func_code = rx_buffer[BLINK_MSG_FUNC_IDX];

    /* ── BLINK ──────────────────────────────────────────────────────── */
    if (func_code == BLINK_FUNC_CODE)
    {
        if (frame_len <= BLINK_MSG_CYCLE_MSB_IDX ||
            rx_buffer[BLINK_MSG_DEST_IDX] != 0xFF)
        { uwb_force_rx_on(); return 1; }

        uint16_t cycle_id =
            ((uint16_t)rx_buffer[BLINK_MSG_CYCLE_MSB_IDX] << 8) |
             (uint16_t)rx_buffer[BLINK_MSG_CYCLE_LSB_IDX];

        uint64_t ts = get_rx_timestamp_u64();

        /* Timeout reset */
        if (g_sync.has_syn_k &&
            (xTaskGetTickCount() - g_sync.created_tick) > SYNC_TIMEOUT_TICKS)
        {
            memset(&g_sync, 0, sizeof(g_sync));
            printf("[A%d] SYNC timeout\r\n", MY_ANCHOR_ID);
        }

        if (!g_sync.has_syn_k)
        {
            /* (a) syn_k */
            g_sync.cycle_k      = cycle_id;
            g_sync.syn_k        = ts;
            g_sync.has_syn_k    = 1;
            g_sync.has_syn_k1   = 0;
            g_sync.has_Ti       = 0;
            g_sync.created_tick = xTaskGetTickCount();
            printf("[A%d] SYN_K ck=%u\r\n", MY_ANCHOR_ID, cycle_id);
        }
        else if (cycle_id == g_sync.cycle_k)
        {
            /* (b) duplicate – bỏ qua */
        }
        else if (!g_sync.has_syn_k1)
        {
            /* (c) syn_k1 */
            g_sync.cycle_k1   = cycle_id;
            g_sync.syn_k1     = ts;
            g_sync.has_syn_k1 = 1;
            printf("[A%d] SYN_K1 ck=%u ck1=%u\r\n",
                   MY_ANCHOR_ID, g_sync.cycle_k, cycle_id);

            /* A0 commit ngay (không cần T_i) */
            if (MY_ANCHOR_ID == 0) commit_sync_report();
            /* Slave commit nếu đã có T_i */
            else if (g_sync.has_Ti) commit_sync_report();
        }
        else
        {
            /* (d) cả 2 đã có – BLINK mới = slide */
            if (MY_ANCHOR_ID != 0 && g_sync.has_Ti) commit_sync_report();

            g_sync.cycle_k      = cycle_id;
            g_sync.syn_k        = ts;
            g_sync.has_syn_k    = 1;
            g_sync.has_syn_k1   = 0;
            g_sync.has_Ti       = 0;
            g_sync.created_tick = xTaskGetTickCount();
            printf("[A%d] SYN_K ck=%u\r\n", MY_ANCHOR_ID, cycle_id);
        }

        uwb_force_rx_on();
        return 1;
    }

    /* ── POLL ───────────────────────────────────────────────────────── */
    {
        uint8 dest_id = rx_buffer[POLL_MSG_DEST_ID_IDX];
        if (dest_id != (uint8)MY_ANCHOR_ID) { uwb_force_rx_on(); return 1; }

        uint8 rx_check[RX_BUF_LEN];
        memcpy(rx_check, rx_buffer, frame_len);
        rx_check[ALL_MSG_SN_IDX]       = 0;
        rx_check[POLL_MSG_DEST_ID_IDX] = 0xFF;

        if (memcmp(rx_check, rx_poll_msg, ALL_MSG_COMMON_LEN) != 0)
        { uwb_force_rx_on(); return 1; }

        poll_rx_ts = get_rx_timestamp_u64();

        /* Slave: lưu T_i */
        if (MY_ANCHOR_ID != 0 && g_sync.has_syn_k)
        {
            g_sync.T_i    = poll_rx_ts;
            g_sync.has_Ti = 1;
            printf("[A%d] Ti ck=%u ck1=%u k1=%d\r\n",
                   MY_ANCHOR_ID, g_sync.cycle_k,
                   g_sync.cycle_k1, g_sync.has_syn_k1);
            if (g_sync.has_syn_k1) commit_sync_report();
        }

        /* Trả RESP */
        uint32 resp_tx_time =
            (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
        dwt_setdelayedtrxtime(resp_tx_time);
        resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

        resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
        resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);
        tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;

        dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
        dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

        if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS) {
            /* Fix busy-wait: nhường CPU cho Softdevice */
            uint32_t t0 = xTaskGetTickCount();
            while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
                if ((xTaskGetTickCount() - t0) > pdMS_TO_TICKS(5)) break;
                vTaskDelay(1);
            }
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
            frame_seq_nb++;
        } else {
            printf("[A%d] RESP late\r\n", MY_ANCHOR_ID);
        }

        uwb_force_rx_on();
    }

    return 1;
}

static uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5]; uint64 ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) { ts <<= 8; ts |= ts_tab[i]; }
    return ts;
}

static void resp_msg_set_ts(uint8 *ts_field, const uint64 ts)
{
    for (int i = 0; i < RESP_MSG_TS_LEN; i++)
        ts_field[i] = (ts >> (i * 8)) & 0xFF;
}

void ss_responder_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    printf("[A%d] STARTED\r\n", MY_ANCHOR_ID);

    dwt_setleds(DWT_LEDS_ENABLE);
    dwt_enableframefilter(0);

    if (MY_ANCHOR_ID == 0) {
        master_hybrid_init();
        master_hybrid_reset();
    }

    uint8_t  ble_buf[64];
    uint16_t ble_len;
    uwb_force_rx_on();

    while (1)
    {
        ss_resp_run();

        if (g_ble_pending) {
            flush_ble_pending();
            vTaskDelay(pdMS_TO_TICKS(BLE_SEND_INTERVAL_MS));
        }

        if (MY_ANCHOR_ID == 0) {
            if (ble_scan_packet(ble_buf, &ble_len))
                master_hybrid_handle_ble_data(ble_buf, ble_len);
        }

        vTaskDelay(1);
    }
}