/*
 * ss_init_main.c  —  TAG / Initiator
 * TWR Single-Sided TOF, 4 anchors, dest_id per poll
 *
 * FIX: khi nhận RESP, check rx_buffer[MSG_SRC_IDX] == anchor_id
 *      (byte 10 trong RESP là src_anchor_id, không phải dest)
 *
 * Build flag: -DTAG_ID=<0..3>
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"

/* ─────────────────────────── tunable ─────────────────────────── */
#ifndef TAG_ID
#define TAG_ID 0
#endif

#define MAX_ANCHORS             4
#define TAG_RX_TIMEOUT_UUS      12000   /* > RESP_TX_DELAY_UUS + propagation */
#define INTER_ANCHOR_DELAY_MS   100
#define WAIT_RESP_TIMEOUT_MS    60      /* watchdog SW */

/* ────────────────────────── constants ────────────────────────── */
#define SPEED_OF_LIGHT          299702547.0
/* DWT_TIME_UNITS defined in deca_device_api.h */

/* ────────────────────────── frame layout ─────────────────────── */
/*
 *  POLL (TAG→ANCHOR):
 *    [0..4] header  [5..8]='WAVE'  [9]=E0  [10]=dest_anchor_id
 *
 *  RESP (ANCHOR→TAG):
 *    [0..4] header  [5..8]='VEWE'  [9]=E1  [10]=src_anchor_id
 *    [11..14]=poll_rx_ts  [15..18]=resp_tx_ts
 */
#define ALL_MSG_SN_IDX          2
#define ALL_MSG_COMMON_LEN      10
#define POLL_FUNC_CODE          0xE0
#define RESP_FUNC_CODE          0xE1
#define MSG_DEST_IDX            10   /* POLL: anchor đích */
#define MSG_SRC_IDX             10   /* RESP: anchor nguồn — cùng offset, khác ý nghĩa */
#define RESP_MSG_POLL_RX_TS_IDX 11
#define RESP_MSG_RESP_TX_TS_IDX 15
#define RESP_MSG_TS_LEN         4
#define RESP_FRAME_MIN_LEN      (RESP_MSG_RESP_TX_TS_IDX + RESP_MSG_TS_LEN)  /* =19 */

static uint8_t tx_poll_msg[] = {
    0x41, 0x88, 0, 0xCA, 0xDE,
    'W', 'A', 'V', 'E',
    POLL_FUNC_CODE,
    0x00,        /* dest anchor id — điền trước khi gửi */
    0x00, 0x00   /* padding */
};

/* Chỉ so sánh 10 byte header, không so sánh byte 10 (src_id) ở đây */
static uint8_t rx_resp_hdr[] = {
    0x41, 0x88, 0, 0xCA, 0xDE,
    'V', 'E', 'W', 'A',
    RESP_FUNC_CODE
};

/* ──────────────────────── runtime state ──────────────────────── */
#define RX_BUF_LEN  24
static uint8_t  rx_buffer[RX_BUF_LEN];
static uint8_t  frame_seq_nb = 0;

typedef struct {
    double  distance;
    int     valid;
} AnchorMeas;

static AnchorMeas meas[MAX_ANCHORS];

/* ─────────────────────────── helpers ─────────────────────────── */
static void reset_dw1000(void)
{
    dwt_write32bitreg(SYS_STATUS_ID,
        SYS_STATUS_ALL_TX | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    dwt_rxreset();
    dwt_setrxtimeout(0);
}

static void ts_get_u32(const uint8_t *field, uint32_t *ts)
{
    *ts = 0;
    for (int i = 0; i < RESP_MSG_TS_LEN; i++)
        *ts += (uint32_t)field[i] << (i * 8);
}

/* ─────────────────────── single TWR exchange ─────────────────── */
int ss_init_run(int anchor_id)
{
    if (anchor_id < 0 || anchor_id >= MAX_ANCHORS) return 0;

    reset_dw1000();
    vTaskDelay(pdMS_TO_TICKS(2));

    tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    tx_poll_msg[MSG_DEST_IDX]   = (uint8_t)anchor_id;

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
    dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);

    dwt_setrxtimeout(TAG_RX_TIMEOUT_UUS);
    int ret = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    if (ret != DWT_SUCCESS) {
        printf("[TAG] TX FAIL A%d\r\n", anchor_id);
        reset_dw1000();
        meas[anchor_id].valid = 0;
        return 0;
    }

    /* ── chờ RX hoặc timeout ── */
    uint32_t status_reg;
    uint32_t t0 = xTaskGetTickCount();

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
        if ((xTaskGetTickCount() - t0) > pdMS_TO_TICKS(WAIT_RESP_TIMEOUT_MS)) {
            printf("[TAG] WATCHDOG A%d\r\n", anchor_id);
            reset_dw1000();
            meas[anchor_id].valid = 0;
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    frame_seq_nb++;

    if (!(status_reg & SYS_STATUS_RXFCG)) {
        dwt_write32bitreg(SYS_STATUS_ID,
            SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
        printf("[TAG] RX_FAIL A%d\r\n", anchor_id);
        meas[anchor_id].valid = 0;
        reset_dw1000();
        return 0;
    }

    /* ── đọc frame ── */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
    uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;

    if (frame_len > RX_BUF_LEN || frame_len < RESP_FRAME_MIN_LEN) {
        printf("[TAG] FRAME_LEN A%d len=%lu\r\n", anchor_id, frame_len);
        reset_dw1000();
        meas[anchor_id].valid = 0;
        return 0;
    }

    dwt_readrxdata(rx_buffer, frame_len, 0);
    rx_buffer[ALL_MSG_SN_IDX] = 0;

    /* ── kiểm tra header ── */
    if (memcmp(rx_buffer, rx_resp_hdr, ALL_MSG_COMMON_LEN) != 0) {
        printf("[TAG] HDR_MISMATCH A%d\r\n", anchor_id);
        reset_dw1000();
        meas[anchor_id].valid = 0;
        return 0;
    }

    /* ── kiểm tra src_id: anchor trả lời đúng là anchor mình poll ── */
    if (rx_buffer[MSG_SRC_IDX] != (uint8_t)anchor_id) {
        printf("[TAG] SRC_MISMATCH A%d got=%d\r\n",
               anchor_id, rx_buffer[MSG_SRC_IDX]);
        reset_dw1000();
        meas[anchor_id].valid = 0;
        return 0;
    }

    /* ── lấy timestamps ── */
    uint32_t poll_tx_ts  = dwt_readtxtimestamplo32();
    uint32_t resp_rx_ts  = dwt_readrxtimestamplo32();
    uint32_t poll_rx_ts_v, resp_tx_ts_v;
    ts_get_u32(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts_v);
    ts_get_u32(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts_v);

    /* ── tính khoảng cách (SS-TWR + clock offset correction) ── */
    float clockOffsetRatio =
        (float)dwt_readcarrierintegrator() *
        (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6f);

    int32_t rtd_init = (int32_t)(resp_rx_ts   - poll_tx_ts);
    int32_t rtd_resp = (int32_t)(resp_tx_ts_v - poll_rx_ts_v);

    double tof      = ((rtd_init - rtd_resp * (1.0 - clockOffsetRatio)) / 2.0)
                      * DWT_TIME_UNITS;
    double distance = tof * SPEED_OF_LIGHT;

    meas[anchor_id].distance = distance;
    meas[anchor_id].valid    = 1;

    printf("[TOF] A%d  d=%.3f m  (rtd_i=%ld  rtd_r=%ld  tof=%.1f ns)\r\n",
           anchor_id, distance,
           (long)rtd_init, (long)rtd_resp,
           tof * 1e9);

    reset_dw1000();
    return 1;
}

/* ─────────────────────── FreeRTOS task entry ─────────────────── */
void ss_initiator_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    dwt_setleds(DWT_LEDS_ENABLE);
    frame_seq_nb = 0;
    memset(meas, 0, sizeof(meas));

    printf("[TAG] START  TAG_ID=%d  anchors=%d\r\n", TAG_ID, MAX_ANCHORS);

    uint32_t cycle = 0;

    while (1)
    {
        cycle++;
        int ok = 0;

        for (int i = 0; i < MAX_ANCHORS; i++)
        {
            if (ss_init_run(i)) ok++;
            vTaskDelay(pdMS_TO_TICKS(INTER_ANCHOR_DELAY_MS));
        }

        printf("[TAG] cyc=%lu  ok=%d/%d  |", cycle, ok, MAX_ANCHORS);
        for (int i = 0; i < MAX_ANCHORS; i++) {
            if (meas[i].valid)
                printf("  A%d=%.2fm", i, meas[i].distance);
            else
                printf("  A%d=---", i);
        }
        printf("\r\n");
    }
}