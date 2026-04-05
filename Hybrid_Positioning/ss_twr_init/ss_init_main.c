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

// Đo 4 Anchor
#define MAX_ANCHORS 4
#define SPEED_OF_LIGHT 299702547.0

#define FREQ_OFFSET_MULTIPLIER          (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5  (-1.0e6 / 6489.6e6)

#define TAG_ID 1

// TỌA ĐỘ ANCHOR CỐ ĐỊNH 
static const float HARDCODED_ANCHOR_X[MAX_ANCHORS] = {0.0f, 1.0f, 0.0f, 1.0f}; 
static const float HARDCODED_ANCHOR_Y[MAX_ANCHORS] = {0.0f, 0.0f, 2.0f, 2.0f};

// CẤU HÌNH AUTO-CALIBRATION ẨN
#define CALIB_SAMPLES 10
#define CALIB_TRUE_DIST 0.7071f  // Khoảng cách thực tế từ tâm (0.5, 0.5) đến các góc

// Các biến lưu trạng thái tự động calib cho từng anchor riêng biệt
static float dynamic_anchor_offset[MAX_ANCHORS] = {0.0f, 0.0f, 0.0f, 0.0f};
static int calib_count[MAX_ANCHORS] = {0, 0, 0, 0};
static float raw_dist_sum[MAX_ANCHORS] = {0.0f, 0.0f, 0.0f, 0.0f};
static bool is_calib_done[MAX_ANCHORS] = {false, false, false, false};

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0, 0};
static uint8 rx_buffer[32];
static uint16 g_cycle_id = 0;

static vec2 g_tag_pos_est = {0.0, 0.0};

typedef struct {
    float x;
    float y;
    float dist;
    uint32_t last_update_tick;
    bool is_valid;
} anchor_data_t;

static anchor_data_t anchors_info[MAX_ANCHORS];

void reset_uwb_state(void) {
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
}

bool calculate_tag_position(float *out_x, float *out_y) {
    vec2 valid_anchors[MAX_ANCHORS];
    double valid_distances[MAX_ANCHORS];
    int count = 0;

    uint32_t current_tick = xTaskGetTickCount();
    for (int i = 0; i < MAX_ANCHORS; i++) {
        if (anchors_info[i].is_valid && (current_tick - anchors_info[i].last_update_tick < pdMS_TO_TICKS(2000))) {
            valid_anchors[count].x = (double)anchors_info[i].x;
            valid_anchors[count].y = (double)anchors_info[i].y;
            valid_distances[count] = (double)anchors_info[i].dist;
            count++;
        }
    }

    if (count < 3) return false; 

    int iterations = tof_2d_localize(valid_anchors, count, valid_distances, &g_tag_pos_est);

    if (iterations > 0) {
        *out_x = (float)g_tag_pos_est.x;
        *out_y = (float)g_tag_pos_est.y;
        return true;
    }

    return false;
}

void ss_initiator_task_function(void *pvParameter) {
    ble_raw_beacon_init(TAG_ID);
    
    // Khởi tạo trạng thái UWB
    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid = false;
        anchors_info[i].last_update_tick = 0;
    }

    static uint8_t tag_seq = 0; 

    // Các biến dùng để cộng dồn 5 mẫu
    static int batch_count = 0;
    static float sum_x = 0.0f, sum_y = 0.0f;
    static float sum_d0 = 0.0f, sum_d1 = 0.0f, sum_d2 = 0.0f, sum_d3 = 0.0f;

    while (1) {
        g_cycle_id++;
        
        // 1. UWB ĐO KHOẢNG CÁCH TUẦN TỰ TỪ 0->3
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
                        uint32 t_round_t = dwt_readtxtimestamplo32();
                        uint32 t_round_r = dwt_readrxtimestamplo32();
                        uint32 t_reply_r = *(uint32*)&rx_buffer[10];
                        uint32 t_reply_t = *(uint32*)&rx_buffer[14];

                        int32 rtd_init = (int32)(t_round_r - t_round_t);
                        int32 rtd_resp = (int32)(t_reply_t - t_reply_r);

                        float clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
                        double tof = ((double)rtd_init - (double)rtd_resp * (1.0f - clockOffsetRatio)) / 2.0;
                        float raw_dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

                        if (raw_dist > 0.05f && raw_dist < 100.0f) {
                            
                            // --- AUTO-CALIBRATION ẨN TRONG NỀN ---
                            if (!is_calib_done[a]) {
                                raw_dist_sum[a] += raw_dist;
                                calib_count[a]++;
                                
                                if (calib_count[a] >= CALIB_SAMPLES) {
                                    dynamic_anchor_offset[a] = (raw_dist_sum[a] / (float)CALIB_SAMPLES) - CALIB_TRUE_DIST;
                                    is_calib_done[a] = true;
                                }
                            }
                            
                            float dist = raw_dist - dynamic_anchor_offset[a];
                            if (dist <= 0.0f) dist = 0.01f;

                            anchors_info[a].x = HARDCODED_ANCHOR_X[a];
                            anchors_info[a].y = HARDCODED_ANCHOR_Y[a];
                            
                            anchors_info[a].dist = (anchors_info[a].is_valid) ? 
                                                   (anchors_info[a].dist * 0.7f + dist * 0.3f) : dist;
                                                   
                            anchors_info[a].last_update_tick = xTaskGetTickCount();
                            anchors_info[a].is_valid = true;
                        }
                    }
                }
            }
        } 

        // 2. TÍNH TỌA ĐỘ VÀ CỘNG DỒN DỮ LIỆU
        float tag_x = 0.0f, tag_y = 0.0f;
        
        if (calculate_tag_position(&tag_x, &tag_y)) {
            float d0 = anchors_info[0].is_valid ? anchors_info[0].dist : 0.0f;
            float d1 = anchors_info[1].is_valid ? anchors_info[1].dist : 0.0f;
            float d2 = anchors_info[2].is_valid ? anchors_info[2].dist : 0.0f;
            float d3 = anchors_info[3].is_valid ? anchors_info[3].dist : 0.0f;

            // Cộng dồn các giá trị
            sum_x += tag_x;
            sum_y += tag_y;
            sum_d0 += d0;
            sum_d1 += d1;
            sum_d2 += d2;
            sum_d3 += d3;
            batch_count++;

            // 3. ĐỦ 5 MẪU -> CHIA TRUNG BÌNH -> IN LOG & GỬI BLE
            if (batch_count >= 5) {
                float avg_x = sum_x / 5.0f;
                float avg_y = sum_y / 5.0f;
                float avg_d0 = sum_d0 / 5.0f;
                float avg_d1 = sum_d1 / 5.0f;
                float avg_d2 = sum_d2 / 5.0f;
                float avg_d3 = sum_d3 / 5.0f;

                // Log ra format JSON với dữ liệu đã được làm mượt
                printf("{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"d\":[%.2f,%.2f,%.2f,%.2f]}\r\n", 
                       TAG_ID, avg_x, avg_y, avg_d0, avg_d1, avg_d2, avg_d3);

                // Gửi BLE
                #pragma pack(push, 1)
                typedef struct {
                    uint8_t start_byte; 
                    uint8_t id;
                    uint8_t seq;
                    float x;
                    float y;
                } ble_light_data_t;
                #pragma pack(pop)

                ble_light_data_t pkt;
                pkt.start_byte = '{';  
                pkt.id = TAG_ID;
                pkt.x = avg_x;
                pkt.y = avg_y;

                for(int i = 0; i < 5; i++) {
                    pkt.seq = tag_seq++;
                    ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                    vTaskDelay(pdMS_TO_TICKS(15));
                }

                // Reset lại các biến đếm để bắt đầu chu kỳ 5 mẫu mới
                batch_count = 0;
                sum_x = 0.0f; 
                sum_y = 0.0f;
                sum_d0 = 0.0f; 
                sum_d1 = 0.0f; 
                sum_d2 = 0.0f; 
                sum_d3 = 0.0f;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(200)); 
    }
}