#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h"
#include "utils.h"
#include "anchor_calib.h"

/* ---------------------------------------------------------------
   MAX_ANCHORS = số anchor THỰC TẾ trong hệ thống.
   [FIX 4] Phải khớp với số mạch anchor bạn đã nạp firmware:
     - 3 anchor  (NODE_ID 2, 3, 4) → MAX_ANCHORS = 3
     - 4 anchor  (NODE_ID 2, 3, 4, 5) → MAX_ANCHORS = 4
   Nếu để MAX_ANCHORS = 4 nhưng chỉ có 3 anchor thực tế, tag sẽ
   poll 1 anchor không tồn tại → luôn có 1 timeout/cycle.
   --------------------------------------------------------------- */
#define MAX_ANCHORS 3   /* <<< SỬA CHỖ NÀY khớp với số anchor thực tế */

/* [FIX 5] Thời gian tag chờ trước khi bắt đầu poll.
   Anchor tự calibrate mất: CALIB_SLOT_MS * (N-1) + TWR time
   Với CALIB_SLOT_MS=5000, 3 anchor: anchor cuối cần ~10s + vài giây TWR.
   Tag chờ 12s = an toàn cho tất cả anchor calibrate xong.            */
#define TAG_STARTUP_DELAY_MS   15000u  /* 15s chờ anchor calib */

#define TAG_ID         1
#define SPEED_OF_LIGHT 299702547.0
#define FREQ_OFFSET_MULTIPLIER         (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5 (-1.0e6 / 6489.6e6)

/* Background calibrate offset khoảng cách */
#define CALIB_SAMPLES       20
#define CALIB_TRUE_DISTANCE 1.0f

static int   calib_count[MAX_ANCHORS]   = {0};
static float calib_sum[MAX_ANCHORS]     = {0};
static float anchor_offset[MAX_ANCHORS] = {0};
static bool  is_calibrated[MAX_ANCHORS] = {false};

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0, 0};
static uint8 rx_buffer[32];

static vec2 g_tag_pos_est = {0.0, 0.0};

typedef struct {
    float    x;
    float    y;
    float    dist;
    uint32_t last_update_tick;
    bool     is_valid;
    bool     pos_known;
} anchor_data_t;

static anchor_data_t anchors_info[MAX_ANCHORS];

void reset_uwb_state(void) {
    dwt_forcetrxoff();
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
}

bool calculate_tag_position(float *out_x, float *out_y) {
    vec2   valid_anchors[MAX_ANCHORS];
    double valid_distances[MAX_ANCHORS];
    int    count = 0;

    uint32_t now = xTaskGetTickCount();
    for (int i = 0; i < MAX_ANCHORS; i++) {
        bool fresh = (now - anchors_info[i].last_update_tick < pdMS_TO_TICKS(2000));
        if (anchors_info[i].is_valid && anchors_info[i].pos_known && fresh) {
            valid_anchors[count].x  = (double)anchors_info[i].x;
            valid_anchors[count].y  = (double)anchors_info[i].y;
            valid_distances[count]  = (double)anchors_info[i].dist;
            count++;
        }
    }

    if (count < 3) {
        printf("[DBG] valid anchors: %d/3 needed\r\n", count);
        return false;
    }

    int ok = tof_2d_localize(valid_anchors, count, valid_distances, &g_tag_pos_est);
    if (ok > 0) {
        *out_x = (float)g_tag_pos_est.x;
        *out_y = (float)g_tag_pos_est.y;
        return true;
    }
    return false;
}

void ss_initiator_task_function(void *pvParameter) {
    ble_raw_beacon_init(TAG_ID);

    printf("[TAG] Waiting %ds for anchors...\r\n", TAG_STARTUP_DELAY_MS / 1000);
    vTaskDelay(pdMS_TO_TICKS(TAG_STARTUP_DELAY_MS));
    printf("[TAG] Starting UWB polling.\r\n\n");


    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid  = false;
        anchors_info[i].pos_known = false;
        anchors_info[i].last_update_tick = 0;
    }

    static uint8_t tag_seq = 0;

    while (1) {

        /* === 1. ĐO KHOẢNG CÁCH ĐẾN TỪNG ANCHOR === */
        for (int a = 0; a < MAX_ANCHORS; a++) {
            reset_uwb_state();
            vTaskDelay(pdMS_TO_TICKS(1000));  /* 1s giữa mỗi anchor — debug mode */

            /* [FIX 4] Poll đúng NODE_ID của anchor: ANCHOR_REF_ID + index.
               Ví dụ: a=0 → ID=2, a=1 → ID=3, a=2 → ID=4.
               Đảm bảo không poll vượt quá số anchor thực tế.          */
            uint8_t target_anchor_id = (uint8_t)(ANCHOR_REF_ID + a);
            tx_poll_msg[10] = target_anchor_id;
            tx_poll_msg[2]++;

            dwt_setrxtimeout(20000);  /* 20ms — đủ cho anchor xử lý + DELAYED/IMMEDIATE TX */

            dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
            dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);

            if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS) {
                printf("[DBG] A%d: TX FAIL\r\n", target_anchor_id);
                continue;
            }

            /* Chờ RX event */
            while (!((dwt_read32bitreg(SYS_STATUS_ID)) &
                     (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

            uint32_t st = dwt_read32bitreg(SYS_STATUS_ID);

            if (!(st & SYS_STATUS_RXFCG)) {
                if (st & SYS_STATUS_ALL_RX_TO)
                    printf("[DBG] A%d: TIMEOUT\r\n", target_anchor_id);
                else
                    printf("[DBG] A%d: RX ERR 0x%08X\r\n", target_anchor_id, (unsigned)st);
                continue;
            }

            /* Nhận được gói response */
            dwt_readrxdata(rx_buffer, 30, 0);

            if (rx_buffer[9] != 0xE1) {
                printf("[DBG] A%d: wrong func code 0x%02X\r\n", target_anchor_id, rx_buffer[9]);
                continue;
            }

            /* Đọc timestamps 40-bit đầy đủ để tránh wrap-around */
            uint8  tx_ts_raw[5], rx_ts_raw[5];
            uint64 t_tx = 0, t_rx = 0;
            dwt_readtxtimestamp(tx_ts_raw);
            dwt_readrxtimestamp(rx_ts_raw);
            for (int b = 4; b >= 0; b--) {
                t_tx = (t_tx << 8) | tx_ts_raw[b];
                t_rx = (t_rx << 8) | rx_ts_raw[b];
            }

            uint32 t_round_t = (uint32)(t_tx & 0xFFFFFFFFUL);
            uint32 t_round_r = (uint32)(t_rx & 0xFFFFFFFFUL);
            uint32 t_reply_r = *(uint32*)&rx_buffer[10];  /* T1: poll rx  */
            uint32 t_reply_t = *(uint32*)&rx_buffer[14];  /* T2: resp tx  */

            int32 rtd_init = (int32)(t_round_r - t_round_t);
            int32 rtd_resp = (int32)(t_reply_t - t_reply_r);

            float cor = (float)(dwt_readcarrierintegrator() *
                        (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6));
            double tof = ((double)rtd_init - (double)rtd_resp * (1.0 - cor)) / 2.0;
            float raw_dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

            /* Đọc tọa độ anchor từ gói UWB */
            float anc_x, anc_y;
            memcpy(&anc_x, &rx_buffer[18], sizeof(float));
            memcpy(&anc_y, &rx_buffer[22], sizeof(float));

            anchors_info[a].x         = anc_x;
            anchors_info[a].y         = anc_y;
            anchors_info[a].pos_known = true;

            if (raw_dist > 0.05f && raw_dist < 100.0f) {
                /* Background offset calibration */
                if (!is_calibrated[a]) {
                    calib_sum[a]   += raw_dist;
                    calib_count[a]++;
                    if (calib_count[a] == CALIB_SAMPLES) {
                        anchor_offset[a] = calib_sum[a] / CALIB_SAMPLES - CALIB_TRUE_DISTANCE;
                        is_calibrated[a] = true;
                        printf("[CAL] A%d offset=%.3fm\r\n", a, anchor_offset[a]);
                    }
                }

                float dist = raw_dist - anchor_offset[a];
                if (dist <= 0.0f) dist = 0.01f;

                printf("[TOF] A%d(id=%d) %.2fm raw=%.2fm pos=(%.2f,%.2f)\r\n",
                       a, target_anchor_id, dist, raw_dist, anc_x, anc_y);

                anchors_info[a].dist             = dist;
                anchors_info[a].last_update_tick = xTaskGetTickCount();
                anchors_info[a].is_valid         = true;
            } else {
                printf("[DBG] A%d: dist out of range %.2fm\r\n", target_anchor_id, raw_dist);
            }
        }

        /* === 2. TÍNH TỌA ĐỘ VÀ GỬI BLE === */
        float tag_x = 0.0f, tag_y = 0.0f;

        if (calculate_tag_position(&tag_x, &tag_y)) {
            float d0 = anchors_info[0].is_valid ? anchors_info[0].dist : 0.0f;
            float d1 = anchors_info[1].is_valid ? anchors_info[1].dist : 0.0f;
            float d2 = (MAX_ANCHORS > 2 && anchors_info[2].is_valid) ? anchors_info[2].dist : 0.0f;

            printf("{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"d\":[%.2f,%.2f,%.2f]}\r\n",
                   TAG_ID, tag_x, tag_y, d0, d1, d2);

            /* [DEBUG] BLE send tạm tắt để tránh block vòng lặp poll.
               Bật lại sau khi xác nhận TOF hoạt động đúng. */
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}