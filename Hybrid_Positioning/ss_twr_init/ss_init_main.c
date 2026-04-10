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
#include "ble_scanner.h"
#include "utils.h"

#define MAX_ANCHORS 4 
#define SPEED_OF_LIGHT 299702547.0
#define FREQ_OFFSET_MULTIPLIER          (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5  (-1.0e6 / 6489.6e6)

extern uint8_t g_current_node_id;
extern uint16_t g_my_mac;
extern uint8_t g_current_role; 
// Cập nhật tham số truyền vào của hàm Flash
extern void flash_config_write(uint8_t role, uint8_t id, float x, float y, float z);

static const float HARDCODED_ANCHOR_X[MAX_ANCHORS] = {0.0f, 1.0f, 0.0f, 0.0f}; 
static const float HARDCODED_ANCHOR_Y[MAX_ANCHORS] = {0.0f, 0.0f, 1.0f, 0.0f};
static const float HARDCODED_ANCHOR_Z[MAX_ANCHORS] = {0.0f, 0.0f, 0.0f, 1.0f}; 

#define CALIB_SAMPLES 10                  
#define CALIB_TRUE_DISTANCE 1.0f          

static int calib_count[MAX_ANCHORS] = {0};
static float calib_sum[MAX_ANCHORS] = {0};
static float anchor_offset[MAX_ANCHORS] = {0}; 
static bool is_calibrated[MAX_ANCHORS] = {false};
static bool g_is_calibrating = false;

static uint8_t tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0, 0};
static uint8_t rx_buffer[32]; 
static uint16_t g_cycle_id = 0;
static vec3 g_tag_pos_est = {0.0, 0.0, 0.0};

typedef struct {
    float x; float y; float z; float dist;
    uint32_t last_update_tick; bool is_valid;
} anchor_data_t;

static anchor_data_t anchors_info[MAX_ANCHORS];

void reset_uwb_state(void) {
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
}

bool calculate_tag_position(float *out_x, float *out_y, float *out_z) {
    vec3 valid_anchors[MAX_ANCHORS];
    double valid_distances[MAX_ANCHORS];
    int count = 0;

    uint32_t current_tick = xTaskGetTickCount();
    for (int i = 0; i < MAX_ANCHORS; i++) {
        if (anchors_info[i].is_valid && (current_tick - anchors_info[i].last_update_tick < pdMS_TO_TICKS(2000))) {
            valid_anchors[count].x = (double)anchors_info[i].x;
            valid_anchors[count].y = (double)anchors_info[i].y;
            valid_anchors[count].z = (double)anchors_info[i].z;
            valid_distances[count] = (double)anchors_info[i].dist;
            count++;
        }
    }
    if (count < 4) return false; 
    if (tof_3d_localize(valid_anchors, count, valid_distances, &g_tag_pos_est) > 0) {
        *out_x = (float)g_tag_pos_est.x; *out_y = (float)g_tag_pos_est.y; *out_z = (float)g_tag_pos_est.z;
        return true;
    }
    return false;
}

void ss_initiator_task_function(void *pvParameter) {
    ble_raw_beacon_init(g_current_node_id);
    printf("[TAG] SYSTEM STARTING\r\n");

    if (g_current_role == 99) {
        g_is_calibrating = true;
        printf("[TAG] BOOTED WITH ROLE 99 -> AUTO-START CALIBRATION\r\n");
    }

    for (int i = 0; i < MAX_ANCHORS; i++) {
        anchors_info[i].is_valid = false;
        anchors_info[i].last_update_tick = 0;
    }

    static uint8_t tag_seq = 0; 
    TickType_t last_status_log = xTaskGetTickCount();

    while (1) {
        if (g_is_calibrating) {
            printf("\r\n==========================================\r\n");
            printf("[TAG] TRIGGER CALIBRATION -> SENDING 0xEC\r\n");
            printf("==========================================\r\n");
            
            for (int a = 0; a < MAX_ANCHORS; a++) {
                tx_poll_msg[9] = 0xEC;
                tx_poll_msg[10] = (uint8_t)a; 
                
                printf("[TAG] ---> Firing rapid Unicast 0xEC to Anchor %d...\r\n", a);
                for(int retry = 0; retry < 15; retry++) {
                    reset_uwb_state();
                    tx_poll_msg[2]++; 
                    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
                    dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
                    
                    if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_SUCCESS) {
                         while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                         dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                    }
                    vTaskDelay(pdMS_TO_TICKS(15)); 
                }
                printf("[TAG]   + Done sending to A%d\r\n", a);
            }
            
            printf("[TAG] DONE SENDING 0xEC. WAITING FOR 0xED FROM A0 (Max 20s)...\r\n");
            TickType_t s = xTaskGetTickCount();
            TickType_t last_wait_log = s;
            
            while (xTaskGetTickCount() - s < pdMS_TO_TICKS(20000)) { 
                if(xTaskGetTickCount() - last_wait_log > pdMS_TO_TICKS(4000)) {
                    printf("[TAG] ... still waiting for 0xED ...\r\n");
                    last_wait_log = xTaskGetTickCount();
                }

                reset_uwb_state(); 
                dwt_setrxtimeout(65000);
                dwt_rxenable(DWT_START_RX_IMMEDIATE);
                
                while (!((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
                    if (xTaskGetTickCount() - s >= pdMS_TO_TICKS(20000)) break;
                }
                
                if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
                    dwt_readrxdata(rx_buffer, 32, 0);
                    if (rx_buffer[9] == 0xED) {
                        printf("\r\n[TAG] <<< RECEIVED 0xED FROM A0! CALIBRATION DONE!\r\n");
                        break;
                    }
                }
            }
            
            g_is_calibrating = false;
            tx_poll_msg[9] = 0xE0; 
            printf("[TAG] RESUMING NORMAL MODE\r\n==========================================\r\n");

            if (g_current_role == 99) {
                printf("[TAG] CALIBRATION FINISHED. REVERTING FLASH TO ROLE 1 AND RESETTING...\r\n");
                // Ghi đè Role về 1, tọa độ của Tag thì set mặc định 0,0,0
                flash_config_write(1, g_current_node_id, 0.0f, 0.0f, 0.0f);
                vTaskDelay(pdMS_TO_TICKS(100));
                NVIC_SystemReset();
            }

            continue;
        }

        if (xTaskGetTickCount() - last_status_log > pdMS_TO_TICKS(3000)) {
            printf("[TAG] STATUS: NORMAL LOCALIZATION\r\n");
            last_status_log = xTaskGetTickCount();
        }

        g_cycle_id++;
        
        for (int a = 0; a < MAX_ANCHORS; a++) {
            reset_uwb_state(); 
            vTaskDelay(pdMS_TO_TICKS(5)); 
            tx_poll_msg[10] = (uint8_t)a; 
            tx_poll_msg[2]++;          

            dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
            dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
            
            if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) == DWT_SUCCESS) {
                dwt_setrxtimeout(5000); 
                while (!((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

                if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
                    dwt_readrxdata(rx_buffer, 32, 0); 
                    if (rx_buffer[9] == 0xE1) {
                        uint32_t t_round_t = dwt_readtxtimestamplo32();
                        uint32_t t_round_r = dwt_readrxtimestamplo32();
                        uint32_t t_reply_r = *(uint32_t*)&rx_buffer[10];
                        uint32_t t_reply_t = *(uint32_t*)&rx_buffer[14];

                        int32_t rtd_init = (int32_t)(t_round_r - t_round_t);
                        int32_t rtd_resp = (int32_t)(t_reply_t - t_reply_r);

                        float clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
                        double tof = ((double)rtd_init - (double)rtd_resp * (1.0f - clockOffsetRatio)) / 2.0;
                        float raw_dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

                        if (raw_dist > 0.05f && raw_dist < 100.0f) {
                            if (!is_calibrated[a]) {
                                calib_sum[a] += raw_dist;
                                calib_count[a]++;
                                if (calib_count[a] == CALIB_SAMPLES) {
                                    anchor_offset[a] = (calib_sum[a] / CALIB_SAMPLES) - CALIB_TRUE_DISTANCE;
                                    is_calibrated[a] = true;
                                }
                            }
                            float dist = raw_dist - anchor_offset[a];
                            if (dist <= 0.0f) dist = 0.01f;
                            anchors_info[a].x = HARDCODED_ANCHOR_X[a];
                            anchors_info[a].y = HARDCODED_ANCHOR_Y[a];
                            anchors_info[a].z = HARDCODED_ANCHOR_Z[a];
                            anchors_info[a].dist = dist;
                            anchors_info[a].last_update_tick = xTaskGetTickCount();
                            anchors_info[a].is_valid = true;
                        }
                    } else if (rx_buffer[9] == 0xEC) {
                        if (!g_is_calibrating) {
                            printf("[TAG] <<< RECEIVED 0xEC VIA UWB -> SWITCH TO CALIB MODE\r\n");
                        }
                        g_is_calibrating = true;
                    }
                }
            }
        } 

        float tag_x = 0.0f, tag_y = 0.0f, tag_z = 0.0f;
        if (calculate_tag_position(&tag_x, &tag_y, &tag_z)) {
            float d0 = anchors_info[0].is_valid ? anchors_info[0].dist : 0.0f;
            float d1 = anchors_info[1].is_valid ? anchors_info[1].dist : 0.0f;
            float d2 = anchors_info[2].is_valid ? anchors_info[2].dist : 0.0f;
            float d3 = anchors_info[3].is_valid ? anchors_info[3].dist : 0.0f;

            printf("{\"id\":%d,\"type\":\"tag\",\"status\":\"measuring\",\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"d\":[%.2f,%.2f,%.2f,%.2f]}\r\n", 
                   g_current_node_id, tag_x, tag_y, tag_z, d0, d1, d2, d3);

            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte; 
                uint8_t id; 
                uint8_t seq;
                float x; 
                float y; 
                float z; 
            } ble_packed_data_t;
            #pragma pack(pop)

            ble_packed_data_t pkt;
            pkt.start_byte = '{'; 
            pkt.id = g_current_node_id;
            pkt.x = tag_x; 
            pkt.y = tag_y; 
            pkt.z = tag_z;

            for(int i = 0; i < 10; i++) {
                pkt.seq = tag_seq++;
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }
        } else {
            printf("{\"id\":%d,\"type\":\"tag\",\"status\":\"waiting_for_anchors\"}\r\n", g_current_node_id);
        }

        web_config_t cfg;
        if (ble_scan_for_config(&cfg)) {
            if (cfg.target_mac == g_my_mac || cfg.target_mac == 0xFFFF) {
                if (cfg.role == 99 && cfg.node_id == 0) {
                    if (!g_is_calibrating) {
                        printf("[TAG] <<< RECEIVED ROLE 99 VIA BLE -> SWITCH TO CALIB MODE\r\n");
                    }
                    g_is_calibrating = true;
                } else {
                    flash_config_write(cfg.role, cfg.node_id, 0.0f, 0.0f, 0.0f);
                    vTaskDelay(pdMS_TO_TICKS(100)); 
                    NVIC_SystemReset();
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}