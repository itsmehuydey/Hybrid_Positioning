#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h"
#include "ble_scanner.h" 
#include "uwb_calib.h"
#include "utils.h"

extern uint8_t g_current_role;
extern uint8_t g_current_node_id;
extern uint16_t g_my_mac;
extern float g_my_pos_x;
extern float g_my_pos_y;
extern void flash_config_write(uint8_t role, uint8_t id, float x, float y);

float my_pos_x = 0.0f, my_pos_y = 0.0f;
static bool g_is_calibrating = false;

static uint8_t tx_resp_msg[31] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0           
};

#define POLL_RX_TO_RESP_TX_DLY_UUS 1500 
#define UUS_TO_DWT_TIME            65536
#define TX_ANT_DLY                 16436 

static uint8_t rx_buffer[64];

// ======================================================================
// HÀM MỚI: Lấy 20 mẫu UWB rồi tính trung bình
// ======================================================================
static float average_measure_tof(uint8_t target, uint8_t my_id) {
    float sum = 0.0f;
    int count = 0;
    int max_tries = 60; // Thử tối đa 60 lần để vớt đủ 20 mẫu
    
    printf("[A%d] Sampling 20 dists to A%d...\r\n", my_id, target);
    for (int i = 0; i < max_tries && count < 20; i++) { 
        float d = measure_inter_anchor_tof(target);
        if (d > 0.05f && d < 100.0f) {
            sum += d;
            count++;
        }
        vTaskDelay(pdMS_TO_TICKS(5)); // Delay nhỏ để nhường RTOS
    }
    
    if (count > 0) {
        float avg = sum / count;
        printf("[A%d] Avg to A%d: %.3f (from %d valid samples)\r\n", my_id, target, avg, count);
        return avg; 
    }
    
    printf("[A%d] FAILED to measure A%d\r\n", my_id, target);
    return 0.0f; 
}

static void robust_ble_blast_distances(uint8_t my_id, uint8_t t1, float d1, uint8_t t2, float d2) {
    printf("[A%d] Blasting BLE dists (d%d%d=%.2f, d%d%d=%.2f) for 2.5s...\r\n", 
           my_id, my_id, t1, d1, my_id, t2, d2);
    
    TickType_t start = xTaskGetTickCount();
    uint8_t payload[7];
    payload[0] = 'D'; 
    payload[1] = my_id;
    
    while (xTaskGetTickCount() - start < pdMS_TO_TICKS(2500)) {
        if (d1 > 0.05f) {
            payload[2] = t1; memcpy(&payload[3], &d1, 4);
            ble_raw_beacon_send_payload(payload, 7);
            vTaskDelay(pdMS_TO_TICKS(15));
        }
        if (d2 > 0.05f) {
            payload[2] = t2; memcpy(&payload[3], &d2, 4);
            ble_raw_beacon_send_payload(payload, 7);
            vTaskDelay(pdMS_TO_TICKS(15));
        }
        if (d1 <= 0.05f && d2 <= 0.05f) { 
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    printf("[A%d] Done blasting BLE.\r\n", my_id);
}

void ss_responder_task_function(void *pvParameter) {
    // Kéo tọa độ từ Flash. Nếu Flash trắng (0,0) thì gán tọa độ ảo mặc định cho 2D
    if (g_my_pos_x == 0.0f && g_my_pos_y == 0.0f && g_current_node_id != 0) {
        if (g_current_node_id == 1) my_pos_x = 1.0f;
        else if (g_current_node_id == 2) my_pos_y = 1.0f;
        else if (g_current_node_id == 3) { my_pos_x = 1.0f; my_pos_y = 1.0f; }
    } else {
        my_pos_x = g_my_pos_x;
        my_pos_y = g_my_pos_y;
    }

    ble_raw_beacon_init(g_current_node_id);
    printf("[A%d] READY. BOOT POS: (%.2f, %.2f)\r\n", g_current_node_id, my_pos_x, my_pos_y);
    
    memcpy(&tx_resp_msg[18], &my_pos_x, sizeof(float));
    memcpy(&tx_resp_msg[22], &my_pos_y, sizeof(float));
    tx_resp_msg[30] = g_current_node_id;

    TickType_t last_ble_tx = 0;
    TickType_t last_ble_scan = 0; 
    TickType_t last_status_log = 0;
    static uint8_t anchor_seq = 0; 

    while (1) {
        if (g_is_calibrating) {
            printf("\r\n[A%d] >>> ENTER CALIBRATION MODE\r\n", g_current_node_id);
            
            if (g_current_node_id == 0) {
                float d01 = 0, d02 = 0, d03 = 0, d12 = 0, d13 = 0, d23 = 0;
                
                printf("[A0] Waiting 2.5s for Tag to trigger A1, A2, A3...\r\n");
                vTaskDelay(pdMS_TO_TICKS(2500));
                
                printf("[A0] START MEASURING TO A1, A2, A3 (UWB AVERAGING)...\r\n");
                d01 = average_measure_tof(1, 0); 
                d02 = average_measure_tof(2, 0); 
                d03 = average_measure_tof(3, 0); 
                
                TickType_t s = xTaskGetTickCount();
                TickType_t last_wait_log = s;
                uint8_t b[32]; uint16_t l;
                bool got_all_data = false;

                printf("[A0] WAITING FOR BLE DISTANCES FROM A1, A2 (Max 12s)...\r\n");
                
                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(12000)) {
                    if(xTaskGetTickCount() - last_wait_log > pdMS_TO_TICKS(2000)) {
                        printf("[A0] Still waiting BLE... Current: d12=%.2f, d13=%.2f, d23=%.2f\r\n", d12, d13, d23);
                        last_wait_log = xTaskGetTickCount();
                    }

                    if (ble_scan_packet(b, &l) && l >= 7 && b[0] == 'D') {
                        float d; memcpy(&d, &b[3], 4);
                        if (d > 0.05f) {
                            if (b[1] == 1 && b[2] == 2 && d12 == 0.0f) { d12 = d; printf("[A0] + CAUGHT d12: %.2f\r\n", d); }
                            if (b[1] == 1 && b[2] == 3 && d13 == 0.0f) { d13 = d; printf("[A0] + CAUGHT d13: %.2f\r\n", d); }
                            if (b[1] == 2 && b[2] == 3 && d23 == 0.0f) { d23 = d; printf("[A0] + CAUGHT d23: %.2f\r\n", d); }
                        }
                    }
                    
                    if (d12 > 0.0f && d13 > 0.0f && d23 > 0.0f) {
                        got_all_data = true;
                        break; 
                    }
                    vTaskDelay(pdMS_TO_TICKS(5)); 
                }
                
                if (got_all_data) {
                    printf("[A0] ALL DATA RECEIVED. CALCULATING GEOMETRY...\r\n");
                    vec2 a1, a2, a3;
                    if (calculate_anchor_geometry(d01, d02, d03, d12, d13, d23, &a1, &a2, &a3)) {
                        
                        printf("[A0] GEOMETRY CALCULATED. Blasting via BLE for 6s...\r\n");
                        TickType_t blast_start = xTaskGetTickCount();
                        while (xTaskGetTickCount() - blast_start < pdMS_TO_TICKS(6000)) {
                            ble_beacon_send_geometry(1, (float)a1.x, (float)a1.y); vTaskDelay(pdMS_TO_TICKS(15));
                            ble_beacon_send_geometry(2, (float)a2.x, (float)a2.y); vTaskDelay(pdMS_TO_TICKS(15));
                            ble_beacon_send_geometry(3, (float)a3.x, (float)a3.y); vTaskDelay(pdMS_TO_TICKS(15));
                        }
                        
                        // LƯU TỌA ĐỘ VÀO FLASH CHO A0 (Gốc 0,0)
                        my_pos_x = 0; my_pos_y = 0;
                        g_my_pos_x = 0; g_my_pos_y = 0;
                        printf("[A0] Saving pos to Flash: 0, 0\r\n");
                        flash_config_write(g_current_role, g_current_node_id, my_pos_x, my_pos_y);
                        
                        memcpy(&tx_resp_msg[18], &my_pos_x, 4);
                        memcpy(&tx_resp_msg[22], &my_pos_y, 4);
                        
                        printf("[A0] SENDING 0xED TO TAG...\r\n");
                        uint8_t cmd[13] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xED, 0xFF, 0, 0};
                        
                        for (int i = 0; i < 15; i++) {
                            dwt_forcetrxoff(); 
                            dwt_writetxdata(13, cmd, 0); dwt_writetxfctrl(13, 0, 1);
                            if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_SUCCESS) {
                                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS); 
                            }
                            vTaskDelay(pdMS_TO_TICKS(15));
                        }
                        printf("[A0] 0xED SENT.\r\n");
                    } else {
                        printf("[A0] MATH ERROR: calculate_anchor_geometry FAILED!\r\n");
                    }
                } else {
                    printf("[A0] TIMEOUT! MISSING DISTANCES. CALIB ABORTED.\r\n");
                }
                
            } else if (g_current_node_id == 1) {
                printf("[A1] Responding to A0 UWB requests (4s window)...\r\n");
                serve_as_inter_anchor_responder(4000, 1); 
                
                printf("[A1] Measuring to A2, A3 (AVERAGING)...\r\n");
                float d12 = average_measure_tof(2, 1); 
                float d13 = average_measure_tof(3, 1); 
                
                robust_ble_blast_distances(1, 2, d12, 3, d13);
                
                printf("[A1] WAITING FOR GEOMETRY FROM A0 (Max 8s)...\r\n");
                TickType_t s = xTaskGetTickCount();
                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(8000)) {
                    if (ble_scan_for_geometry(1, &my_pos_x, &my_pos_y)) {
                        printf("\r\n[A1] <<< GEOMETRY UPDATED: (%.2f, %.2f) >>>\r\n", my_pos_x, my_pos_y);
                        // LƯU FLASH
                        g_my_pos_x = my_pos_x; g_my_pos_y = my_pos_y;
                        flash_config_write(g_current_role, g_current_node_id, my_pos_x, my_pos_y);
                        printf("[A1] Saved to Flash permanently.\r\n");
                        break;
                    }
                }
                memcpy(&tx_resp_msg[18], &my_pos_x, 4); memcpy(&tx_resp_msg[22], &my_pos_y, 4);
                
            } else if (g_current_node_id == 2) {
                printf("[A2] Responding to A0, A1 UWB requests (6s window)...\r\n");
                serve_as_inter_anchor_responder(6000, 2);
                
                printf("[A2] Measuring to A3 (AVERAGING)...\r\n");
                float d23 = average_measure_tof(3, 2); 
                
                robust_ble_blast_distances(2, 3, d23, 0, 0.0f);
                
                printf("[A2] WAITING FOR GEOMETRY FROM A0 (Max 8s)...\r\n");
                TickType_t s = xTaskGetTickCount();
                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(8000)) {
                    if (ble_scan_for_geometry(2, &my_pos_x, &my_pos_y)) {
                        printf("\r\n[A2] <<< GEOMETRY UPDATED: (%.2f, %.2f) >>>\r\n", my_pos_x, my_pos_y);
                        // LƯU FLASH
                        g_my_pos_x = my_pos_x; g_my_pos_y = my_pos_y;
                        flash_config_write(g_current_role, g_current_node_id, my_pos_x, my_pos_y);
                        printf("[A2] Saved to Flash permanently.\r\n");
                        break;
                    }
                }
                memcpy(&tx_resp_msg[18], &my_pos_x, 4); memcpy(&tx_resp_msg[22], &my_pos_y, 4);
                
            } else if (g_current_node_id == 3) {
                printf("[A3] Responding to A0, A1, A2 UWB requests (8s window)...\r\n");
                serve_as_inter_anchor_responder(8000, 3);
                
                printf("[A3] WAITING FOR GEOMETRY FROM A0 (Max 8s)...\r\n");
                TickType_t s = xTaskGetTickCount();
                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(8000)) {
                    if (ble_scan_for_geometry(3, &my_pos_x, &my_pos_y)) {
                        printf("\r\n[A3] <<< GEOMETRY UPDATED: (%.2f, %.2f) >>>\r\n", my_pos_x, my_pos_y);
                        // LƯU FLASH
                        g_my_pos_x = my_pos_x; g_my_pos_y = my_pos_y;
                        flash_config_write(g_current_role, g_current_node_id, my_pos_x, my_pos_y);
                        printf("[A3] Saved to Flash permanently.\r\n");
                        break;
                    }
                }
                memcpy(&tx_resp_msg[18], &my_pos_x, 4); memcpy(&tx_resp_msg[22], &my_pos_y, 4);
            }
            
            g_is_calibrating = false;
            printf("[A%d] EXIT CALIBRATION -> RESUMING NORMAL MODE\r\n", g_current_node_id);
            continue;
        }

        TickType_t now = xTaskGetTickCount();
        if (now - last_status_log > pdMS_TO_TICKS(3000)) {
            printf("[A%d] STATUS: LISTENING FOR UWB & BLE...\r\n", g_current_node_id);
            last_status_log = now;
        }

        dwt_setrxtimeout(65000);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32_t status = dwt_read32bitreg(SYS_STATUS_ID);
        if (status & SYS_STATUS_RXFCG) {
            uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer)) dwt_readrxdata(rx_buffer, frame_len, 0);

            if (rx_buffer[9] == 0xEC) {
                printf("[A%d] <<< RX Raw UWB 0xEC - Target byte in packet: %d\r\n", g_current_node_id, rx_buffer[10]);
            }

            if (rx_buffer[9] == 0xEC && (rx_buffer[10] == g_current_node_id || rx_buffer[10] == 0xFF)) {
                if (!g_is_calibrating) {
                    printf("[A%d] MATCHED TARGET ID %d -> SWITCH TO CALIB MODE\r\n", g_current_node_id, rx_buffer[10]);
                }
                g_is_calibrating = true;
                continue;
            }

            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == g_current_node_id) {
                uint64_t poll_rx_ts = get_rx_timestamp_u64();
                uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                dwt_setdelayedtrxtime(resp_tx_time);
                uint64_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

                resp_msg_set_ts(&tx_resp_msg[10], poll_rx_ts);
                resp_msg_set_ts(&tx_resp_msg[14], resp_tx_ts);

                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);
                if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS) {
                    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                }
            } else dwt_rxreset();
        } else dwt_rxreset();

        if (now - last_ble_scan > pdMS_TO_TICKS(2000)) {
            web_config_t cfg;
            if (ble_scan_for_config(&cfg)) {
                if (cfg.target_mac == g_my_mac || cfg.target_mac == 0xFFFF) {
                    if (cfg.role == 99 && cfg.node_id == 0) {
                        if (!g_is_calibrating) {
                            printf("[A%d] <<< RECEIVED ROLE 99 VIA BLE -> SWITCH TO CALIB MODE\r\n", g_current_node_id);
                        }
                        g_is_calibrating = true;
                    } else {
                        // Nhận lệnh đổi Role, lưu Flash cả role, id và tọa độ
                        flash_config_write(cfg.role, cfg.node_id, g_my_pos_x, g_my_pos_y);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        NVIC_SystemReset();
                    }
                }
            }
            last_ble_scan = now;
        }

        if (now - last_ble_tx > pdMS_TO_TICKS(5000)) {
            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte; uint8_t id; uint8_t seq;
                float x; float y;
            } ble_anchor_packed_t;
            #pragma pack(pop)

            ble_anchor_packed_t pkt;
            pkt.start_byte = '['; pkt.id = g_current_node_id;
            pkt.x = my_pos_x; pkt.y = my_pos_y;

            for(int i = 0; i < 10; i++) {
                pkt.seq = anchor_seq++;
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }
            last_ble_tx = now;
        }
    }
}