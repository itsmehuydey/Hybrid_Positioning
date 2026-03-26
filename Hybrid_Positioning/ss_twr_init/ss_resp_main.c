/*
 * ss_resp_main.c  —  ANCHOR / Responder
 * TWR Single-Sided TOF, lọc theo dest_id
 *
 * FIX: resp_tx_ts phải được tính và ghi vào payload TRƯỚC khi
 *      dwt_writetxdata(), dùng 40-bit timestamps để tính đúng.
 *
 * Build flag: -DNODE_ID=<0..3>
 */

#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"

/* ─────────────────────────── tunable ─────────────────────────── */
#ifndef NODE_ID
#define NODE_ID 1
#endif
#define MY_ANCHOR_ID NODE_ID

/*
 * Khoảng thời gian từ lúc nhận xong POLL đến lúc anchor BẮT ĐẦU TX RESP.
 * Phải đủ lớn để code kịp tính resp_tx_ts và gọi dwt_setdelayedtrxtime().
 * Khuyến nghị: >= 500 µs. Tăng nếu thấy "DELAYED_TX LATE".
 * TAG cần RX timeout > delay này + propagation.
 */
#define RESP_TX_DELAY_UUS   1000

#define UUS_TO_DWT_TIME     65536   /* 1 µs = 65536 DWT ticks (499.2 MHz × 128) */

/* ────────────────────────── frame layout ─────────────────────── */
/*
 *  POLL (TAG→ANCHOR):
 *    [0..4] header  [5..8]='WAVE'  [9]=E0  [10]=dest_anchor_id  [11..12]=pad
 *
 *  RESP (ANCHOR→TAG):
 *    [0..4] header  [5..8]='VEWE'  [9]=E1  [10]=src_anchor_id
 *    [11..14]=poll_rx_ts (32-bit lo)
 *    [15..18]=resp_tx_ts (32-bit lo)
 *
 *  Cả poll_rx_ts và resp_tx_ts đều là 32-bit thấp của timestamp 40-bit.
 *  Đủ chính xác cho SS-TWR vì chênh lệch nhỏ, không cần 40-bit đầy đủ.
 */
#define ALL_MSG_SN_IDX           2
#define ALL_MSG_COMMON_LEN       10
#define POLL_FUNC_CODE           0xE0
#define RESP_FUNC_CODE           0xE1
#define MSG_DEST_IDX             10   /* byte 10 trong POLL: anchor đích */
#define MSG_SRC_IDX              10   /* byte 10 trong RESP: anchor nguồn */
#define RESP_MSG_POLL_RX_TS_IDX  11
#define RESP_MSG_RESP_TX_TS_IDX  15
#define RESP_MSG_TS_LEN          4

static uint8_t tx_resp_msg[] = {
    0x41, 0x88, 0, 0xCA, 0xDE,
    'V', 'E', 'W', 'A',
    RESP_FUNC_CODE,
    (uint8_t)MY_ANCHOR_ID,  /* src anchor id */
    0, 0, 0, 0,             /* poll_rx_ts [11..14] */
    0, 0, 0, 0              /* resp_tx_ts [15..18] */
};

static uint8_t rx_poll_hdr[] = {
    0x41, 0x88, 0, 0xCA, 0xDE,
    'W', 'A', 'V', 'E',
    POLL_FUNC_CODE
};

/* ──────────────────────── runtime state ──────────────────────── */
#define RX_BUF_LEN  20
static uint8_t  rx_buffer[RX_BUF_LEN];
static uint8_t  frame_seq_nb = 0;

/* ─────────────────────────── helpers ─────────────────────────── */
static void ts_set_u32(uint8_t *field, uint32_t ts)
{
    for (int i = 0; i < RESP_MSG_TS_LEN; i++)
        field[i] = (uint8_t)(ts >> (i * 8));
}

static uint64_t get_rx_ts_u40(void)
{
    uint8_t tab[5]; uint64_t ts = 0;
    dwt_readrxtimestamp(tab);
    for (int i = 4; i >= 0; i--) { ts <<= 8; ts |= tab[i]; }
    return ts;
}

static inline void uwb_clear_events(void)
{
    dwt_write32bitreg(SYS_STATUS_ID,
        SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
}

static inline void uwb_rx_on(void)
{
    uwb_clear_events();
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

/* ──────────────────────── one iteration ─────────────────────── */
int ss_resp_run(void)
{
    uint32_t status_reg = dwt_read32bitreg(SYS_STATUS_ID);

    if (!(status_reg & (SYS_STATUS_RXFCG |
                        SYS_STATUS_ALL_RX_TO |
                        SYS_STATUS_ALL_RX_ERR)))
        return 0;

    /* ── đọc frame ── */
    uint32_t frame_len = 0;
    int rx_ok = 0;

    if (status_reg & SYS_STATUS_RXFCG) {
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (frame_len > 0 && frame_len <= RX_BUF_LEN) {
            dwt_readrxdata(rx_buffer, frame_len, 0);
            rx_ok = 1;
        }
    }

    uwb_clear_events();

    if (!rx_ok || frame_len < (ALL_MSG_COMMON_LEN + 1)) {
        dwt_rxreset();
        uwb_rx_on();
        return 1;
    }

    rx_buffer[ALL_MSG_SN_IDX] = 0;  /* bỏ qua SN khi so sánh header */

    /* ── lọc: chỉ xử lý POLL gửi đến mình ── */
    if (memcmp(rx_buffer, rx_poll_hdr, ALL_MSG_COMMON_LEN) != 0 ||
        rx_buffer[MSG_DEST_IDX] != (uint8_t)MY_ANCHOR_ID)
    {
        uwb_rx_on();
        return 1;
    }

    /* ═══════════════════════════════════════════════════════════
     * QUAN TRỌNG: đọc poll_rx_ts (40-bit) NGAY SAU KHI nhận xong.
     * Dùng 40-bit để tính resp_tx_time chính xác, tránh overflow
     * khi cộng thêm RESP_TX_DELAY_UUS * UUS_TO_DWT_TIME.
     * Ghi 32-bit thấp vào payload (đủ cho TAG tính rtd_resp).
     * ══════════════════════════════════════════════════════════ */
    uint64_t poll_rx_ts_40 = get_rx_ts_u40();
    uint32_t poll_rx_ts_lo = (uint32_t)poll_rx_ts_40;

    /* ── tính thời điểm TX response (40-bit, shift >>8 cho DWT) ── */
    uint64_t resp_tx_ts_40 = (poll_rx_ts_40
                              + (uint64_t)(RESP_TX_DELAY_UUS * UUS_TO_DWT_TIME))
                             & 0xFFFFFFFFFFULL;       /* giữ đúng 40-bit */
    uint32_t resp_tx_time  = (uint32_t)(resp_tx_ts_40 >> 8); /* DWT register */
    uint32_t resp_tx_ts_lo = (uint32_t)(resp_tx_ts_40 & 0xFFFFFFFF);

    /* ═══════════════════════════════════════════════════════════
     * GHI CẢ HAI TIMESTAMPS VÀO PAYLOAD TRƯỚC KHI writetxdata().
     * Đây là bước bị thiếu trong version cũ — resp_tx_ts = 0.
     * ══════════════════════════════════════════════════════════ */
    ts_set_u32(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts_lo);
    ts_set_u32(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts_lo);

    tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb++;
    tx_resp_msg[MSG_SRC_IDX]    = (uint8_t)MY_ANCHOR_ID;

    /* ── nạp frame và set delayed TX ── */
    dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
    dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);
    dwt_setdelayedtrxtime(resp_tx_time);

    int tx_ret = dwt_starttx(DWT_START_TX_DELAYED);

    if (tx_ret != DWT_SUCCESS) {
        /*
         * Trễ quá: RESP_TX_DELAY_UUS quá nhỏ hoặc code chạy chậm.
         * Tăng RESP_TX_DELAY_UUS. Fallback immediate để không mất packet,
         * nhưng resp_tx_ts sẽ lệch một chút (vẫn tốt hơn = 0).
         */
        printf("[A%d] DELAYED_TX LATE! Tang RESP_TX_DELAY_UUS (hien=%d)\r\n",
               MY_ANCHOR_ID, RESP_TX_DELAY_UUS);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX);
        dwt_starttx(DWT_START_TX_IMMEDIATE);
    }

    /* ── chờ TX xong ── */
    uint32_t t0 = xTaskGetTickCount();
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
        if ((xTaskGetTickCount() - t0) > pdMS_TO_TICKS(5)) {
            printf("[A%d] TX TIMEOUT\r\n", MY_ANCHOR_ID);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    printf("[A%d] POLL->RESP  poll_rx=0x%08lX  resp_tx=0x%08lX\r\n",
           MY_ANCHOR_ID, poll_rx_ts_lo, resp_tx_ts_lo);

    uwb_rx_on();
    return 1;
}

/* ─────────────────────── FreeRTOS task entry ─────────────────── */
void ss_responder_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    printf("[A%d] STARTED (TOF responder, dest_id=%d)\r\n",
           MY_ANCHOR_ID, MY_ANCHOR_ID);

    dwt_setleds(DWT_LEDS_ENABLE);
    dwt_enableframefilter(0);   /* lọc dest_id bằng software */

    uwb_rx_on();

    while (1) {
        ss_resp_run();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}