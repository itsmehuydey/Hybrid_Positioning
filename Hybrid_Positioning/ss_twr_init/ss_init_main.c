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

#define MAX_ANCHORS 3
#define SPEED_OF_LIGHT 299702547.0

#define FREQ_OFFSET_MULTIPLIER          (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5  (-1.0e6 / 6489.6e6)

// --- TỌA ĐỘ ANCHOR CỐ ĐỊNH ---
// Ví dụ khoảng cách thực tế trên bàn
static const float HARDCODED_ANCHOR_X[MAX_ANCHORS] = {0.0f, 1.0f, 0.0f}; 
static const float HARDCODED_ANCHOR_Y[MAX_ANCHORS] = {0.0f, 0.0f, 0.8f};

// --- CẤU HÌNH TỰ ĐỘNG CALIBRATE NGẦM ---
#define CALIB_SAMPLES 20                  // Lấy 20 mẫu để tính trung bình
#define CALIB_TRUE_DISTANCE 1.0f          // Cần đặt Tag cách các Anchor 1 mét lúc vừa bật nguồn

// Các biến lưu trạng thái calibrate ngầm
static int calib_count[MAX_ANCHORS] = {0};
static float calib_sum[MAX_ANCHORS] = {0};
static float anchor_offset[MAX_ANCHORS] = {0}; // Ban đầu offset = 0
static bool is_calibrated[MAX_ANCHORS] = {false};
// ---------------------------------------

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0, 0};
static uint8 rx_buffer[32];
static uint16 g_cycle_id = 0;

static vec2 g_tag_pos_est = {0.0, 0.0};

typedef struct __attribute__((packed)) {
    uint8_t  msg_type;   
    uint8_t  anchor_id;  
    uint16_t cycle_id;   
    float    distance;   
    float    tag_x;      
    float    tag_y;      
} ble_pos_report_t;

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
    ble_raw_beacon_init(1);
    printf("[TAG] SYSTEM STARTING - BACKGROUND CALIBRATION ENABLED\r\n");
    printf("[INFO] Place Tag %.2fm from Anchors for the first %d samples.\r\n\n", CALIB_TRUE_DISTANCE, CALIB_SAMPLES);

    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid = false;
        anchors_info[i].last_update_tick = 0;
    }

    while (1) {
        g_cycle_id++;
        
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
                            
                            // --- THU THẬP MẪU CALIBRATE NGẦM ---
                            if (!is_calibrated[a]) {
                                calib_sum[a] += raw_dist;
                                calib_count[a]++;
                                
                                // Nếu đủ mẫu, tự động chốt Offset
                                if (calib_count[a] == CALIB_SAMPLES) {
                                    float avg_dist = calib_sum[a] / CALIB_SAMPLES;
                                    anchor_offset[a] = avg_dist - CALIB_TRUE_DISTANCE;
                                    is_calibrated[a] = true;
                                    
                                    printf("\r\n[>>>] A%d AUTO-CALIBRATED! Avg: %.2fm | Offset applied: %.2fm\r\n\r\n", a, avg_dist, anchor_offset[a]);
                                }
                            }

                            // --- ÁP DỤNG OFFSET ---
                            // Trước khi calib xong, anchor_offset = 0.0 nên dist = raw_dist (chưa bù trừ).
                            // Sau khi calib xong, nó tự động trừ đi giá trị đã chốt.
                            float dist = raw_dist - anchor_offset[a];
                            if (dist <= 0.0f) dist = 0.01f;

                            anchors_info[a].x = HARDCODED_ANCHOR_X[a];
                            anchors_info[a].y = HARDCODED_ANCHOR_Y[a];
                            
                            anchors_info[a].dist = dist;
                            anchors_info[a].last_update_tick = xTaskGetTickCount();
                            anchors_info[a].is_valid = true;
                            
                            // In ra để dễ quan sát: Nếu có dấu * là đã calibrate
                            printf("[TOF] A%d: %.2fm %s\r\n", a, dist, is_calibrated[a] ? "(*)" : "");
                        }
                    }
                }
            }
        } 

        // Luôn luôn chạy định vị, không cần quan tâm đã calib xong hay chưa
        float tag_x = 0.0f, tag_y = 0.0f;
        
        if (calculate_tag_position(&tag_x, &tag_y)) {
            printf(">>> TAG: %.2f %.2f\r\n", tag_x, tag_y);

            ble_pos_report_t ble_pkt;
            ble_pkt.msg_type  = 'T';
            ble_pkt.anchor_id = 0xFF; 
            ble_pkt.cycle_id  = g_cycle_id;
            ble_pkt.distance  = 0;
            ble_pkt.tag_x     = tag_x;
            ble_pkt.tag_y     = tag_y;

            ble_raw_beacon_send_payload((uint8_t *)&ble_pkt, sizeof(ble_pkt));
        } else {
            printf("[!] Waiting for valid TOF data...\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}