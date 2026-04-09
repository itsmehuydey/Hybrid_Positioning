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
#include "ble_scanner.h" // THÊM HEADER BLE SCANNER

// THÊM THƯ VIỆN SOLVER MỚI VÀO ĐÂY
#include "utils.h"

#define MAX_ANCHORS 3
#define SPEED_OF_LIGHT 299702547.0

#define FREQ_OFFSET_MULTIPLIER          (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5  (-1.0e6 / 6489.6e6)

// --- BIẾN TOÀN CỤC ĐỂ ĐỌC FLASH VÀ CẤU HÌNH ---
extern uint8_t g_current_node_id;
extern uint16_t g_my_mac;
extern void flash_config_write(uint8_t role, uint8_t id);

// --- TỌA ĐỘ ANCHOR CỐ ĐỊNH ---
static const float HARDCODED_ANCHOR_X[MAX_ANCHORS] = {0.0f, 1.0f, 0.0f}; 
static const float HARDCODED_ANCHOR_Y[MAX_ANCHORS] = {0.0f, 0.0f, 1.0f};

#define CALIB_SAMPLES 10                  
#define CALIB_TRUE_DISTANCE 1.0f          

static int calib_count[MAX_ANCHORS] = {0};
static float calib_sum[MAX_ANCHORS] = {0};
static float anchor_offset[MAX_ANCHORS] = {0}; 
static bool is_calibrated[MAX_ANCHORS] = {false};
// ---------------------------------------

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
    // Sửa để truyền ID động vào BLE
    ble_raw_beacon_init(g_current_node_id);
    printf("[TAG] SYSTEM STARTING - BACKGROUND CALIBRATION ENABLED\r\n");
    printf("[INFO] Place Tag %.2fm from Anchors for the first %d samples.\r\n\n", CALIB_TRUE_DISTANCE, CALIB_SAMPLES);

    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid = false;
        anchors_info[i].last_update_tick = 0;
    }

    static uint8_t tag_seq = 0; 

    while (1) {
        g_cycle_id++;
        
        // 1. QUÁ TRÌNH UWB ĐO KHOẢNG CÁCH
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
                            if (!is_calibrated[a]) {
                                calib_sum[a] += raw_dist;
                                calib_count[a]++;
                                
                                if (calib_count[a] == CALIB_SAMPLES) {
                                    float avg_dist = calib_sum[a] / CALIB_SAMPLES;
                                    anchor_offset[a] = avg_dist - CALIB_TRUE_DISTANCE;
                                    is_calibrated[a] = true;
                                }
                            }

                            float dist = raw_dist - anchor_offset[a];
                            if (dist <= 0.0f) dist = 0.01f;

                            anchors_info[a].x = HARDCODED_ANCHOR_X[a];
                            anchors_info[a].y = HARDCODED_ANCHOR_Y[a];
                            anchors_info[a].dist = dist;
                            anchors_info[a].last_update_tick = xTaskGetTickCount();
                            anchors_info[a].is_valid = true;
                        }
                    }
                }
            }
        } 

        // 2. TÍNH TỌA ĐỘ VÀ ĐÓNG GÓI GỬI BLE CÓ LẶP
        float tag_x = 0.0f, tag_y = 0.0f;
        
        if (calculate_tag_position(&tag_x, &tag_y)) {
            float d0 = anchors_info[0].is_valid ? anchors_info[0].dist : 0.0f;
            float d1 = anchors_info[1].is_valid ? anchors_info[1].dist : 0.0f;
            float d2 = anchors_info[2].is_valid ? anchors_info[2].dist : 0.0f;

            // Sử dụng ID động g_current_node_id cho JSON
            printf("{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"d\":[%.2f,%.2f,%.2f]}\r\n", 
                   g_current_node_id, tag_x, tag_y, d0, d1, d2);

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
            pkt.start_byte = '{'; 
            pkt.id = g_current_node_id; // Sử dụng ID động
            pkt.x = tag_x;
            pkt.y = tag_y;
            pkt.d[0] = d0; 
            pkt.d[1] = d1; 
            pkt.d[2] = d2;

            for(int i = 0; i < 10; i++) {
                pkt.seq = tag_seq++;
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }

        } else {
            printf("[!] Waiting for valid TOF data...\r\n");
        }

        // ========================================================
        // TÍNH NĂNG ĐỔI ROLE CÁCH 2: "LIẾC" BLE ~45ms XEM CÓ LỆNH KHÔNG
        // ========================================================
        web_config_t cfg;
        if (ble_scan_for_config(&cfg)) {
            if (cfg.target_mac == g_my_mac || cfg.target_mac == 0xFFFF) {
                printf("\r\n[TAG] => NHAN LENH DOI ROLE! Role moi: %d, ID: %d\r\n", cfg.role, cfg.node_id);
                flash_config_write(cfg.role, cfg.node_id);
                vTaskDelay(pdMS_TO_TICKS(100)); // Đợi tí cho Flash ghi xong
                NVIC_SystemReset(); // Tự khởi động lại sang Role mới
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}