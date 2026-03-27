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

// THÊM THƯ VIỆN SOLVER MỚI VÀO ĐÂY
#include "utils.h"

#define MAX_ANCHORS 4   // Số anchor thực tế trong hệ thống
#define SPEED_OF_LIGHT 299702547.0

#define FREQ_OFFSET_MULTIPLIER          (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5  (-1.0e6 / 6489.6e6)

// --- ID CỦA TAG HIỆN TẠI ---
#define TAG_ID 1

// --- TỌA ĐỘ ANCHOR ---
// KHÔNG hardcode ở đây nữa.
// Tag sẽ đọc tọa độ (x, y) trực tiếp từ gói phản hồi UWB của từng Anchor
// (byte 18-21 = X float, byte 22-25 = Y float, byte 26 = Anchor ID).
// Điều này đảm bảo tọa độ Tag và Anchor luôn nhất quán.

// --- CẤU HÌNH TỰ ĐỘNG CALIBRATE NGẦM ---
#define CALIB_SAMPLES       20   // Số mẫu lấy trung bình để tính offset
#define CALIB_TRUE_DISTANCE 1.0f // Đặt Tag cách mỗi Anchor đúng 1.0m lúc khởi động

static int   calib_count[MAX_ANCHORS]  = {0};
static float calib_sum[MAX_ANCHORS]    = {0};
static float anchor_offset[MAX_ANCHORS]= {0};
static bool  is_calibrated[MAX_ANCHORS]= {false};
// ---------------------------------------

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0, 0};
static uint8 rx_buffer[32];
static uint16 g_cycle_id = 0;

static vec2 g_tag_pos_est = {0.0, 0.0};

typedef struct {
    float    x;                 // Tọa độ X anchor (đọc từ gói UWB, đơn vị: m)
    float    y;                 // Tọa độ Y anchor (đọc từ gói UWB, đơn vị: m)
    float    dist;              // Khoảng cách đo được sau khi bù offset (m)
    uint32_t last_update_tick;  // Tick FreeRTOS của lần cập nhật gần nhất
    bool     is_valid;          // true nếu đã có ít nhất 1 phép đo hợp lệ
    bool     pos_known;         // true nếu đã nhận được tọa độ anchor từ gói UWB
} anchor_data_t;

static anchor_data_t anchors_info[MAX_ANCHORS];

void reset_uwb_state(void) {
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
}

bool calculate_tag_position(float *out_x, float *out_y) {
    vec2   valid_anchors[MAX_ANCHORS];
    double valid_distances[MAX_ANCHORS];
    int    count = 0;

    uint32_t current_tick = xTaskGetTickCount();
    for (int i = 0; i < MAX_ANCHORS; i++) {
        // Anchor hợp lệ khi:
        //   1. is_valid : đã có ít nhất 1 khoảng cách đo được
        //   2. Chưa quá 2 giây kể từ lần đo cuối
        // Bỏ yêu cầu pos_known tạm thời - anchor mới (self-calib) mới cần
        bool fresh = (current_tick - anchors_info[i].last_update_tick < pdMS_TO_TICKS(2000));
        if (anchors_info[i].is_valid && fresh) {
            valid_anchors[count].x  = (double)anchors_info[i].x;
            valid_anchors[count].y  = (double)anchors_info[i].y;
            valid_distances[count]  = (double)anchors_info[i].dist;
            count++;
        }
    }

    // Cần ít nhất 3 anchor để giải multilateration 2D
    // (Hạ xuống 2 khi test nếu chưa đủ anchor)
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
    printf("[TAG] SYSTEM STARTING - BACKGROUND CALIBRATION ENABLED\r\n");
    printf("[INFO] Place Tag %.2fm from Anchors for the first %d samples.\r\n\n", CALIB_TRUE_DISTANCE, CALIB_SAMPLES);

    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid = false;
        anchors_info[i].last_update_tick = 0;
    }

    static uint8_t tag_seq = 0; // Bộ đếm BLE cho Tag

    while (1) {
        g_cycle_id++;
        
        // ==========================================
        // 1. QUÁ TRÌNH UWB ĐO KHOẢNG CÁCH
        // ==========================================
        for (int a = 0; a < MAX_ANCHORS; a++) {
            reset_uwb_state(); 
            vTaskDelay(pdMS_TO_TICKS(5)); 

            tx_poll_msg[10] = (uint8)a; 
            tx_poll_msg[2]++;          

            dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
            dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
            
            if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) == DWT_SUCCESS) {
                dwt_setrxtimeout(5000); 

                while (!((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

                if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
                    dwt_readrxdata(rx_buffer, 30, 0);

                    if (rx_buffer[9] == 0xE1) {

                        // === [FIX 1] ĐỌC TIMESTAMP 40-BIT ĐẦY ĐỦ ===
                        // dwt_readtxtimestamplo32() chỉ lấy 32-bit thấp → bị wrap-around
                        // sau ~17s. Phải dùng hàm đọc 40-bit rồi lấy phần thấp 32-bit.
                        uint8  tx_ts_raw[5], rx_ts_raw[5];
                        uint64 t_round_t_40, t_round_r_40;

                        dwt_readtxtimestamp(tx_ts_raw);  // Đọc TX timestamp 40-bit
                        dwt_readrxtimestamp(rx_ts_raw);  // Đọc RX timestamp 40-bit

                        t_round_t_40 = 0;
                        t_round_r_40 = 0;
                        for (int b = 4; b >= 0; b--) {
                            t_round_t_40 = (t_round_t_40 << 8) | tx_ts_raw[b];
                            t_round_r_40 = (t_round_r_40 << 8) | rx_ts_raw[b];
                        }

                        // Lấy 32-bit thấp để tính RTD (timestamp 40-bit đảm bảo
                        // phép trừ không bị lỗi do wrap-around trong 17s)
                        uint32 t_round_t = (uint32)(t_round_t_40 & 0xFFFFFFFFUL);
                        uint32 t_round_r = (uint32)(t_round_r_40 & 0xFFFFFFFFUL);

                        // T_reply_r và T_reply_t: Anchor gửi qua gói phản hồi UWB
                        uint32 t_reply_r = *(uint32*)&rx_buffer[10]; // Poll RX timestamp
                        uint32 t_reply_t = *(uint32*)&rx_buffer[14]; // Resp TX timestamp

                        int32 rtd_init = (int32)(t_round_r - t_round_t);
                        int32 rtd_resp = (int32)(t_reply_t - t_reply_r);

                        // Bù lệch tần số clock giữa Tag và Anchor (SS-TWR chuẩn APS013)
                        float clockOffsetRatio = dwt_readcarrierintegrator() *
                            (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
                        double tof = ((double)rtd_init - (double)rtd_resp *
                            (1.0 - clockOffsetRatio)) / 2.0;
                        float raw_dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

                        // === [FIX 2] ĐỌC TỌA ĐỘ ANCHOR TỪ GÓI UWB ===
                        // Anchor nhúng tọa độ của chính nó vào gói phản hồi:
                        //   byte 18-21: tọa độ X (float, little-endian)
                        //   byte 22-25: tọa độ Y (float, little-endian)
                        //   byte 26:    Anchor ID
                        // → KHÔNG cần hardcode tọa độ ở phía Tag nữa.
                        float anc_x, anc_y;
                        uint8 anc_id_from_pkt;
                        memcpy(&anc_x, &rx_buffer[18], sizeof(float));
                        memcpy(&anc_y, &rx_buffer[22], sizeof(float));
                        anc_id_from_pkt = rx_buffer[26];

                        // Lưu tọa độ anchor vào slot theo anchor index (a),
                        // không dùng anc_id_from_pkt để tránh mismatch khi anchor
                        // chưa tự calibrate (gửi ID khác hoặc 0).
                        anchors_info[a].x = anc_x;
                        anchors_info[a].y = anc_y;
                        anchors_info[a].pos_known = true;

                        if (raw_dist > 0.05f && raw_dist < 100.0f) {

                            // Calibrate offset ngầm (không gate - vẫn dùng dist
                            // ngay cả khi chưa có đủ mẫu, chỉ bù offset sau)
                            if (!is_calibrated[a]) {
                                calib_sum[a]   += raw_dist;
                                calib_count[a]++;
                                if (calib_count[a] == CALIB_SAMPLES) {
                                    float avg_dist   = calib_sum[a] / CALIB_SAMPLES;
                                    anchor_offset[a] = avg_dist - CALIB_TRUE_DISTANCE;
                                    is_calibrated[a] = true;
                                    printf("[CAL] A%d offset=%.3fm\r\n", a, anchor_offset[a]);
                                }
                                // Không có gate: vẫn dùng raw_dist (offset=0)
                            }

                            float dist = raw_dist - anchor_offset[a];
                            if (dist <= 0.0f) dist = 0.01f;

                            printf("[TOF] A%d=%.2fm raw=%.2fm\r\n", a, dist, raw_dist);

                            anchors_info[a].dist             = dist;
                            anchors_info[a].last_update_tick = xTaskGetTickCount();
                            anchors_info[a].is_valid         = true;
                        }
                    }
                }
            }
        }

        // ==========================================
        // 2. TÍNH TỌA ĐỘ VÀ ĐÓNG GÓI GỬI BLE CÓ LẶP
        // ==========================================
        float tag_x = 0.0f, tag_y = 0.0f;
        
        if (calculate_tag_position(&tag_x, &tag_y)) {
            // Lấy khoảng cách của 3 anchor đầu tiên (nếu có)
            float d0 = anchors_info[0].is_valid ? anchors_info[0].dist : 0.0f;
            float d1 = anchors_info[1].is_valid ? anchors_info[1].dist : 0.0f;
            float d2 = anchors_info[2].is_valid ? anchors_info[2].dist : 0.0f;

            // In JSON ra màn hình Serial (UART) để bạn theo dõi trực tiếp qua cáp
            printf("{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"d\":[%.2f,%.2f,%.2f]}\r\n", 
                   TAG_ID, tag_x, tag_y, d0, d1, d2);

            // Cấu trúc ép sát bộ nhớ (Packed Struct) để gửi BLE siêu nhẹ (23 Bytes)
            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte; 
                uint8_t id;
                uint8_t seq;
                float x;
                float y;
                float d[3];
            } ble_packed_data_t;
            #pragma pack(pop)

            ble_packed_data_t pkt;
            pkt.start_byte = '{';  // Ký hiệu nhận diện cho code Python
            pkt.id = TAG_ID;
            pkt.x = tag_x;
            pkt.y = tag_y;
            pkt.d[0] = d0; 
            pkt.d[1] = d1; 
            pkt.d[2] = d2;

            // Bắn lặp lại 10 lần để chắc chắn máy tính bắt được
            for(int i = 0; i < 10; i++) {
                pkt.seq = tag_seq++;
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }

        } else {
            printf("[!] Waiting for valid TOF data...\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}