
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
#define NODE_ID 1
#endif
#define MY_ANCHOR_ID NODE_ID

#define CCP_FUNC_CODE       0xE2   // Master sync BLINK
#define TAG_FUNC_CODE       0xE3   // Tag packet broadcast

#define MSG_FUNC_IDX        9
#define MSG_DEST_IDX        10
#define MSG_CYCLE_LSB_IDX   11
#define MSG_CYCLE_MSB_IDX   12

static uint8 rx_buffer[64];
static uint32 status_reg = 0;

static uint64 get_rx_timestamp_u64(void);

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

static ble_tdoa_report_t g_ble_pending_pkt;
static volatile uint8_t  g_ble_pending    = 0;
static volatile uint8_t  g_ble_send_count = 0;
static uint32_t ble_send_start_tick = 0;

#define BLE_SEND_REPEAT      3
#define BLE_SEND_INTERVAL_MS 20

static inline void uwb_clear_rx_events(void) {
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
}

static inline void uwb_force_rx_on(void) {
    uwb_clear_rx_events();
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

static void commit_sync_report(void) {
    if (!g_sync.has_syn_k || !g_sync.has_syn_k1 || !g_sync.has_Ti) return;

    ble_tdoa_report_t rpt;
    rpt.msg_type  = 'S';
    rpt.anchor_id = (uint8_t)MY_ANCHOR_ID;
    rpt.cycle_id  = g_sync.cycle_k;
    rpt.syn_k     = g_sync.syn_k;
    rpt.syn_k1    = g_sync.syn_k1;
    rpt.T_i       = g_sync.T_i;

    if (MY_ANCHOR_ID == 0) {
        master_hybrid_handle_ble_tdoa_report(&rpt);
        printf("[A0] SYNC COMMIT cyc=%u T_i=%llu\r\n", g_sync.cycle_k, g_sync.T_i);
    } else {
        g_ble_pending_pkt = rpt;
        g_ble_send_count  = 0;
        g_ble_pending     = 1;
        ble_send_start_tick = xTaskGetTickCount() + pdMS_TO_TICKS(MY_ANCHOR_ID * (BLE_SEND_REPEAT * BLE_SEND_INTERVAL_MS + 50));
        printf("[A%d] SYNC queued cyc=%u T_i=%llu\r\n", MY_ANCHOR_ID, g_sync.cycle_k, g_sync.T_i);
    }

    g_sync.cycle_k    = g_sync.cycle_k1;
    g_sync.syn_k      = g_sync.syn_k1;
    g_sync.has_syn_k  = 1;
    g_sync.has_syn_k1 = 0;
    g_sync.has_Ti     = 0;
    g_sync.created_tick = xTaskGetTickCount();
}

static uint32_t last_ble_send_tick = 0;

static void flush_ble_pending(void) {
    if (!g_ble_pending) return;
    uint32_t now = xTaskGetTickCount();
    if (ble_send_start_tick > now) return;
    if ((now - last_ble_send_tick) < pdMS_TO_TICKS(BLE_SEND_INTERVAL_MS)) return;

    last_ble_send_tick = now;
    ble_raw_beacon_send_payload((uint8_t *)&g_ble_pending_pkt, sizeof(ble_tdoa_report_t));
    g_ble_send_count++;
    if (g_ble_send_count >= BLE_SEND_REPEAT) g_ble_pending = 0;
}

/* ================== MASTER GỬI CCP (2 BLINK) ================== */
static uint16_t g_master_cycle = 0;
static void master_send_ccp(void) {
    if (MY_ANCHOR_ID != 0) return;

    uint8 tx_ccp[14] = {0x41,0x88,0,0xCA,0xDE,'W','A','V','E', CCP_FUNC_CODE, 0xFF, 0,0,0};

    // BLINK(k)
    g_master_cycle = (g_master_cycle + 1) & 0xFFFF;
    tx_ccp[MSG_CYCLE_LSB_IDX] = (uint8_t)(g_master_cycle & 0xFF);
    tx_ccp[MSG_CYCLE_MSB_IDX] = (uint8_t)(g_master_cycle >> 8);
    dwt_writetxdata(sizeof(tx_ccp), tx_ccp, 0);
    dwt_writetxfctrl(sizeof(tx_ccp), 0, 1);
    dwt_starttx(DWT_START_TX_IMMEDIATE);
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    vTaskDelay(pdMS_TO_TICKS(20));

    // BLINK(k+1)
    g_master_cycle = (g_master_cycle + 1) & 0xFFFF;
    tx_ccp[MSG_CYCLE_LSB_IDX] = (uint8_t)(g_master_cycle & 0xFF);
    tx_ccp[MSG_CYCLE_MSB_IDX] = (uint8_t)(g_master_cycle >> 8);
    dwt_writetxdata(sizeof(tx_ccp), tx_ccp, 0);
    dwt_writetxfctrl(sizeof(tx_ccp), 0, 1);
    dwt_starttx(DWT_START_TX_IMMEDIATE);
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
}

int ss_resp_run(void) {
    status_reg = dwt_read32bitreg(SYS_STATUS_ID);
    if (!(status_reg & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) return 0;

    uint32 frame_len = 0;
    int rx_ok = 0;
    if (status_reg & SYS_STATUS_RXFCG) {
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (frame_len <= sizeof(rx_buffer)) {
            dwt_readrxdata(rx_buffer, frame_len, 0);
            rx_ok = 1;
        }
    }
    uwb_clear_rx_events();
    if (!rx_ok) { dwt_rxreset(); uwb_force_rx_on(); return 1; }
    if (frame_len < 11) { uwb_force_rx_on(); return 1; }

    uint8 func = rx_buffer[MSG_FUNC_IDX];
    if (rx_buffer[MSG_DEST_IDX] != 0xFF) { uwb_force_rx_on(); return 1; }

    uint16_t cycle_id = ((uint16_t)rx_buffer[MSG_CYCLE_MSB_IDX] << 8) | rx_buffer[MSG_CYCLE_LSB_IDX];
    uint64_t ts = get_rx_timestamp_u64();

    if (g_sync.has_syn_k && (xTaskGetTickCount() - g_sync.created_tick) > SYNC_TIMEOUT_TICKS)
        memset(&g_sync, 0, sizeof(g_sync));

    if (func == CCP_FUNC_CODE) {   // Master sync
        if (!g_sync.has_syn_k) {
            g_sync.cycle_k = cycle_id;
            g_sync.syn_k = ts;
            g_sync.has_syn_k = 1;
            printf("[A%d] SYN_K (CCP) cyc=%u\r\n", MY_ANCHOR_ID, cycle_id);
        } else if (cycle_id != g_sync.cycle_k) {
            if (!g_sync.has_syn_k1) {
                g_sync.cycle_k1 = cycle_id;
                g_sync.syn_k1 = ts;
                g_sync.has_syn_k1 = 1;
                if (g_sync.has_Ti) commit_sync_report();
            } else {
                if (g_sync.has_Ti) commit_sync_report();
                g_sync.cycle_k = cycle_id;
                g_sync.syn_k = ts;
                g_sync.has_syn_k = 1;
                g_sync.has_syn_k1 = 0;
                g_sync.has_Ti = 0;
            }
        }
    }
    else if (func == TAG_FUNC_CODE) {   // Tag packet
        if (g_sync.has_syn_k) {
            g_sync.T_i = ts;
            g_sync.has_Ti = 1;
            printf("[A%d] TAG PACKET T_i(raw)=%llu\r\n", MY_ANCHOR_ID, ts);
            if (g_sync.has_syn_k1) commit_sync_report();
        }
    }

    uwb_force_rx_on();
    return 1;
}

static uint64 get_rx_timestamp_u64(void) {
    uint8 ts_tab[5]; uint64 ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) { ts <<= 8; ts |= ts_tab[i]; }
    return ts;
}

void ss_responder_task_function(void *pvParameter) {
    UNUSED_PARAMETER(pvParameter);
    printf("[A%d] STARTED (CCP mode)\r\n", MY_ANCHOR_ID);

    dwt_setleds(DWT_LEDS_ENABLE);
    dwt_enableframefilter(0);

    if (MY_ANCHOR_ID == 0) {
        master_hybrid_init();
        master_hybrid_reset();
    }

    uint8_t ble_buf[64]; uint16_t ble_len;
    uwb_force_rx_on();

    while (1) {
        ss_resp_run();
        flush_ble_pending();

        if (MY_ANCHOR_ID == 0) {
            master_send_ccp();               // Master gửi 2 BLINK mỗi vòng
            for (int i = 0; i < 8; i++) {
                if (ble_scan_packet(ble_buf, &ble_len)) {
                    if (ble_len >= 1 && ble_buf[0] == 'S') {
                        master_hybrid_handle_ble_tdoa_report((ble_tdoa_report_t *)ble_buf);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
