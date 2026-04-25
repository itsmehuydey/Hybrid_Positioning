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
#include "uwb_calib.h"
#include "utils.h"

// =============================================================================
// Extern globals từ main.c
// =============================================================================
extern uint8_t  g_current_role;
extern uint8_t  g_current_node_id;
extern uint16_t g_my_mac;
extern float    g_my_pos_x;
extern float    g_my_pos_y;
extern void flash_config_write(uint8_t role, uint8_t id, float x, float y);

// =============================================================================
// Cấu hình hệ thống
// =============================================================================
#define MAX_ANCHORS 8   // Phải khớp với ss_init_main.c

// Tọa độ thực tế của anchor — được load từ Flash (do config_tool.py gửi qua BLE)
float my_pos_x = 0.0f, my_pos_y = 0.0f;

static bool g_is_calibrating = false;

// =============================================================================
// UWB response frame
// Bố cục byte quan trọng:
//   byte[9]    = msg type (0xE1 = normal response, 0xED = calib done)
//   byte[10..13] = poll_rx_ts (4 byte)
//   byte[14..17] = resp_tx_ts (4 byte)
//   byte[18..21] = my_pos_x (float) <- TAG đọc tọa độ anchor từ đây
//   byte[22..25] = my_pos_y (float)
//   byte[26..29] = (reserved)
//   byte[30]   = anchor node_id
// =============================================================================
static uint8_t tx_resp_msg[31] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1,
    0, 0, 0, 0,   // [10..13] poll_rx_ts
    0, 0, 0, 0,   // [14..17] resp_tx_ts
    0, 0, 0, 0,   // [18..21] pos_x
    0, 0, 0, 0,   // [22..25] pos_y
    0, 0, 0, 0,   // [26..29] reserved
    0             // [30] anchor_id
};

#define POLL_RX_TO_RESP_TX_DLY_UUS  1500
#define UUS_TO_DWT_TIME             65536
//#define TX_ANT_DLY                  16436

static uint8_t rx_buffer[64];

// =============================================================================
// Lấy 50 mẫu UWB, lọc trimmed mean (loại 20% trên/dưới)
// =============================================================================
static float average_measure_tof(uint8_t target, uint8_t my_id) {
    float samples[50];
    int   count     = 0;
    int   max_tries = 150;

    printf("[A%d] Sampling 50 dists to A%d...\r\n", my_id, target);
    for (int i = 0; i < max_tries && count < 50; i++) {
        float d = measure_inter_anchor_tof(target);
        if (d > 0.05f && d < 100.0f) {
            samples[count++] = d;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    if (count == 0) {
        printf("[A%d] FAILED to measure A%d\r\n", my_id, target);
        return 0.0f;
    }

    // Bubble sort
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (samples[j] > samples[j + 1]) {
                float tmp = samples[j]; samples[j] = samples[j + 1]; samples[j + 1] = tmp;
            }

    // Trimmed mean: loại 20% dưới và 20% trên
    int   trim        = count / 5;
    float sum         = 0.0f;
    int   valid_count = 0;
    for (int i = trim; i < count - trim; i++) { sum += samples[i]; valid_count++; }

    float avg = (valid_count > 0) ? (sum / valid_count) : samples[count / 2];
    printf("[A%d] Avg to A%d: %.3f (raw=%d, valid=%d)\r\n", my_id, target, avg, count, valid_count);
    return avg;
}

// =============================================================================
// Blast khoảng cách tới anchor khác qua BLE (giúp A0 thu thập)
// =============================================================================
static void robust_ble_blast_distances(uint8_t my_id, uint8_t t1, float d1, uint8_t t2, float d2) {
    printf("[A%d] Blasting BLE dists for 2.5s (d%d%d=%.2f, d%d%d=%.2f)...\r\n",
           my_id, my_id, t1, d1, my_id, t2, d2);

    TickType_t start   = xTaskGetTickCount();
    uint8_t    payload[7];
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
        if (d1 <= 0.05f && d2 <= 0.05f) vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("[A%d] Done blasting BLE.\r\n", my_id);
}

// =============================================================================
// Kiểm tra xem tọa độ Flash có hợp lệ không
// Tọa độ hợp lệ nếu đã được ghi (không phải 0,0 với tất cả anchor trừ A0),
// hoặc với A0 thì luôn hợp lệ (A0 ở gốc tọa độ)
// =============================================================================
static bool flash_coords_valid(void) {
    // A0 luôn ở gốc tọa độ (0,0) — hợp lệ
    if (g_current_node_id == 0) return true;
    // Các anchor khác: tọa độ hợp lệ nếu x != 0 hoặc y != 0
    return (g_my_pos_x != 0.0f || g_my_pos_y != 0.0f);
}

// =============================================================================
// Main ANCHOR (responder) task
// =============================================================================
void ss_responder_task_function(void *pvParameter) {
    // Load tọa độ từ Flash (đã được ghi bởi config_tool.py qua BLE lệnh 'A')
    my_pos_x = g_my_pos_x;
    my_pos_y = g_my_pos_y;

    ble_raw_beacon_init(g_current_node_id);
    printf("[A%d] READY. POS: (%.2f, %.2f). Flash valid: %s\r\n",
           g_current_node_id, my_pos_x, my_pos_y,
           flash_coords_valid() ? "YES" : "NO");

    // Nhúng tọa độ vào frame UWB response (TAG sẽ đọc từ đây)
    memcpy(&tx_resp_msg[18], &my_pos_x, sizeof(float));
    memcpy(&tx_resp_msg[22], &my_pos_y, sizeof(float));
    tx_resp_msg[30] = g_current_node_id;

    TickType_t last_ble_tx   = 0;
    TickType_t last_ble_scan = 0;
    TickType_t last_status   = 0;
    static uint8_t anchor_seq = 0;

    while (1) {

        // =====================================================================
        // Chế độ Calib tự động (Role 99 hoặc nhận lệnh 0xEC)
        // SKIP nếu tọa độ Flash đã hợp lệ (config thủ công đã xong)
        // =====================================================================
        if (g_is_calibrating) {
            if (flash_coords_valid()) {
                // Đã có tọa độ thủ công → không cần calib tự động
                printf("[A%d] Flash coords valid (%.2f, %.2f) -> SKIP AUTO CALIB\r\n",
                       g_current_node_id, my_pos_x, my_pos_y);
                g_is_calibrating = false;

                // Vẫn gửi 0xED nếu là A0 để TAG thoát khỏi calib mode
                if (g_current_node_id == 0) {
                    printf("[A0] Sending 0xED to unblock TAG...\r\n");
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
                }
                continue;
            }

            // --- Calib tự động (chỉ khi chưa có tọa độ thủ công) ---
            printf("\r\n[A%d] >>> ENTER AUTO CALIBRATION MODE\r\n", g_current_node_id);

            if (g_current_node_id == 0) {
                // ===========================================================
                // A0 = Master Calib
                // Đo khoảng cách tới A1..A(N-1), thu BLE distances từ peer anchors,
                // giải hình học rồi broadcast tọa độ
                // ===========================================================
                int n_anchors = MAX_ANCHORS; // Có thể ít hơn nếu không phát hiện đủ
                float d0x[MAX_ANCHORS] = {0}; // d(A0, Ax)

                printf("[A0] Waiting 2.5s for Tag to trigger other anchors...\r\n");
                vTaskDelay(pdMS_TO_TICKS(2500));

                // Đo tới A1..A(N-1)
                int  found = 0;
                bool anchor_reachable[MAX_ANCHORS] = {false};
                for (int a = 1; a < n_anchors; a++) {
                    d0x[a] = average_measure_tof(a, 0);
                    if (d0x[a] > 0.05f) {
                        anchor_reachable[a] = true;
                        found++;
                    }
                }

                if (found < 2) {
                    printf("[A0] Only %d anchor(s) reachable, need >= 2. CALIB ABORTED.\r\n", found);
                    g_is_calibrating = false;
                    continue;
                }

                // Thu distances giữa các peer anchors qua BLE
                // dist_peer[i][j] = khoảng cách giữa Ai và Aj (i<j, chỉ lưu upper triangle)
                float dist_peer[MAX_ANCHORS][MAX_ANCHORS];
                memset(dist_peer, 0, sizeof(dist_peer));

                printf("[A0] Collecting BLE distances from peer anchors (max 15s)...\r\n");
                TickType_t s = xTaskGetTickCount();
                TickType_t last_log = s;
                int needed = found * (found - 1) / 2; // số cặp peer cần
                int got    = 0;

                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(15000) && got < needed) {
                    if (xTaskGetTickCount() - last_log > pdMS_TO_TICKS(2000)) {
                        printf("[A0] Still collecting BLE... got %d/%d pairs\r\n", got, needed);
                        last_log = xTaskGetTickCount();
                    }
                    uint8_t b[32]; uint16_t l;
                    if (ble_scan_packet(b, &l) && l >= 7 && b[0] == 'D') {
                        float d; memcpy(&d, &b[3], 4);
                        uint8_t src = b[1], dst = b[2];
                        if (d > 0.05f && src >= 1 && src < MAX_ANCHORS && dst >= 1 && dst < MAX_ANCHORS && src != dst) {
                            uint8_t lo = (src < dst) ? src : dst;
                            uint8_t hi = (src < dst) ? dst : src;
                            if (dist_peer[lo][hi] == 0.0f) {
                                dist_peer[lo][hi] = d;
                                got++;
                                printf("[A0] + Got d%d%d: %.2f\r\n", lo, hi, d);
                            }
                        }
                    }
                    vTaskDelay(pdMS_TO_TICKS(5));
                }

                // Trường hợp đơn giản nhất: 3 anchor (sử dụng calculate_anchor_geometry)
                // Mở rộng cho nhiều anchor hơn: dùng trilateration tổng quát
                // Ở đây để đơn giản, giải cho tối đa 3 anchor đầu tiên
                int a1_idx = -1, a2_idx = -1, a3_idx = -1;
                for (int a = 1; a < MAX_ANCHORS; a++) {
                    if (!anchor_reachable[a]) continue;
                    if (a1_idx < 0)      a1_idx = a;
                    else if (a2_idx < 0) a2_idx = a;
                    else if (a3_idx < 0) a3_idx = a;
                    else break;
                }

                if (a1_idx < 0 || a2_idx < 0) {
                    printf("[A0] Not enough anchors for geometry. CALIB ABORTED.\r\n");
                    g_is_calibrating = false;
                    continue;
                }

                float d01 = d0x[a1_idx];
                float d02 = d0x[a2_idx];
                float d03 = (a3_idx >= 0) ? d0x[a3_idx] : 0.0f;
                float d12 = (a1_idx < a2_idx) ? dist_peer[a1_idx][a2_idx] : dist_peer[a2_idx][a1_idx];
                float d13 = (a3_idx >= 0) ? ((a1_idx < a3_idx) ? dist_peer[a1_idx][a3_idx] : dist_peer[a3_idx][a1_idx]) : 0.0f;
                float d23 = (a3_idx >= 0) ? ((a2_idx < a3_idx) ? dist_peer[a2_idx][a3_idx] : dist_peer[a3_idx][a2_idx]) : 0.0f;

                vec2 calc_a1, calc_a2, calc_a3;
                bool geo_ok = false;

                if (a3_idx >= 0 && d12 > 0.05f && d13 > 0.05f && d23 > 0.05f) {
                    geo_ok = calculate_anchor_geometry(d01, d02, d03, d12, d13, d23,
                                                       &calc_a1, &calc_a2, &calc_a3);
                } else if (d12 > 0.05f) {
                    // Chỉ có 2 anchor phụ (A1, A2): giải đơn giản hơn
                    calc_a1.x = d01; calc_a1.y = 0.0;
                    double x2 = (d02*d02 + d01*d01 - d12*d12) / (2.0*d01);
                    double y2_sq = d02*d02 - x2*x2;
                    calc_a2.x = x2; calc_a2.y = (y2_sq > 0) ? sqrt(y2_sq) : 0.0;
                    geo_ok = true;
                }

                if (geo_ok) {
                    printf("[A0] Geometry OK. Broadcasting coords via BLE for 6s...\r\n");
                    TickType_t blast_start = xTaskGetTickCount();
                    while (xTaskGetTickCount() - blast_start < pdMS_TO_TICKS(6000)) {
                        ble_beacon_send_geometry(a1_idx, (float)calc_a1.x, (float)calc_a1.y);
                        vTaskDelay(pdMS_TO_TICKS(15));
                        ble_beacon_send_geometry(a2_idx, (float)calc_a2.x, (float)calc_a2.y);
                        vTaskDelay(pdMS_TO_TICKS(15));
                        if (a3_idx >= 0) {
                            ble_beacon_send_geometry(a3_idx, (float)calc_a3.x, (float)calc_a3.y);
                            vTaskDelay(pdMS_TO_TICKS(15));
                        }
                    }

                    // A0 luôn ở gốc (0,0)
                    my_pos_x = 0.0f; my_pos_y = 0.0f;
                    g_my_pos_x = 0.0f; g_my_pos_y = 0.0f;
                    flash_config_write(g_current_role, g_current_node_id, 0.0f, 0.0f);
                    memcpy(&tx_resp_msg[18], &my_pos_x, 4);
                    memcpy(&tx_resp_msg[22], &my_pos_y, 4);

                    // Gửi 0xED để TAG thoát calib mode
                    printf("[A0] Sending 0xED to TAG...\r\n");
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
                } else {
                    printf("[A0] Geometry calculation FAILED. CALIB ABORTED.\r\n");
                }

            } else {
                // ===========================================================
                // Ax (x > 0) = Peer anchor trong calib
                // Thời gian serve tăng dần theo ID để A0 có đủ thời gian đo
                // ===========================================================
                uint32_t serve_ms = 3000 + (uint32_t)g_current_node_id * 2000;
                printf("[A%d] Serving as responder for %dms...\r\n", g_current_node_id, (int)serve_ms);
                serve_as_inter_anchor_responder(serve_ms, g_current_node_id);

                // Đo tới các anchor có ID cao hơn mình (tránh đo 2 lần)
                float d_next1 = 0.0f, d_next2 = 0.0f;
                int   next_id1 = -1,  next_id2 = -1;

                // Chỉ đo tới anchor ngay tiếp theo (ID+1) để tiết kiệm thời gian
                // Các anchor khác sẽ được đo bởi A0 trực tiếp
                for (int a = g_current_node_id + 1; a < MAX_ANCHORS; a++) {
                    float d = average_measure_tof(a, g_current_node_id);
                    if (d > 0.05f) {
                        if (next_id1 < 0) { d_next1 = d; next_id1 = a; }
                        else if (next_id2 < 0) { d_next2 = d; next_id2 = a; break; }
                    }
                }

                // Blast qua BLE cho A0
                if (next_id1 >= 0) {
                    robust_ble_blast_distances(g_current_node_id,
                                               next_id1, d_next1,
                                               (next_id2 >= 0) ? next_id2 : 0,
                                               d_next2);
                }

                // Chờ geometry từ A0
                printf("[A%d] Waiting for geometry from A0 (max 10s)...\r\n", g_current_node_id);
                TickType_t s = xTaskGetTickCount();
                while (xTaskGetTickCount() - s < pdMS_TO_TICKS(10000)) {
                    if (ble_scan_for_geometry(g_current_node_id, &my_pos_x, &my_pos_y)) {
                        printf("\r\n[A%d] <<< GEOMETRY UPDATED: (%.2f, %.2f) >>>\r\n",
                               g_current_node_id, my_pos_x, my_pos_y);
                        g_my_pos_x = my_pos_x;
                        g_my_pos_y = my_pos_y;
                        flash_config_write(g_current_role, g_current_node_id, my_pos_x, my_pos_y);
                        printf("[A%d] Saved to Flash.\r\n", g_current_node_id);
                        break;
                    }
                }
                // Cập nhật frame response với tọa độ mới
                memcpy(&tx_resp_msg[18], &my_pos_x, 4);
                memcpy(&tx_resp_msg[22], &my_pos_y, 4);
            }

            g_is_calibrating = false;
            printf("[A%d] EXIT CALIBRATION -> RESUMING NORMAL MODE\r\n", g_current_node_id);
            continue;
        } // end if calibrating

        // =====================================================================
        // Chế độ bình thường: Lắng nghe UWB & BLE
        // =====================================================================
        TickType_t now = xTaskGetTickCount();
        if (now - last_status > pdMS_TO_TICKS(3000)) {
            printf("[A%d] STATUS: LISTENING (pos=%.2f,%.2f)...\r\n",
                   g_current_node_id, my_pos_x, my_pos_y);
            last_status = now;
        }

        // --- UWB RX ---
        dwt_setrxtimeout(65000);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32_t status = dwt_read32bitreg(SYS_STATUS_ID);
        if (status & SYS_STATUS_RXFCG) {
            uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer)) dwt_readrxdata(rx_buffer, frame_len, 0);

            // 0xEC = trigger calib
            if (rx_buffer[9] == 0xEC && (rx_buffer[10] == g_current_node_id || rx_buffer[10] == 0xFF)) {
                g_is_calibrating = true;
                continue;
            }

            // 0xE0 = normal poll (unicast by anchor ID)
            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == g_current_node_id) {
                uint64_t poll_rx_ts   = get_rx_timestamp_u64();
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
            } else {
                dwt_rxreset();
            }
        } else {
            dwt_rxreset();
        }

        // --- BLE scan (mỗi 2s) ---
        if (now - last_ble_scan > pdMS_TO_TICKS(2000)) {
            // Quét gói 'A' mới (có tọa độ)
            anchor_config_t acfg;
            if (ble_scan_for_anchor_config(&acfg)) {
                if (acfg.target_mac == g_my_mac || acfg.target_mac == 0xFFFF) {
                    if (acfg.role == 99) {
                        g_is_calibrating = true;
                    } else {
                        printf("[A%d] <<< NEW CONFIG: role=%d id=%d x=%.2f y=%.2f\r\n",
                               g_current_node_id, acfg.role, acfg.node_id, acfg.x, acfg.y);
                        flash_config_write(acfg.role, acfg.node_id, acfg.x, acfg.y);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        NVIC_SystemReset();
                    }
                }
            }

            // Quét gói 'C' cũ (tương thích ngược)
            web_config_t cfg;
            if (ble_scan_for_config(&cfg)) {
                if (cfg.target_mac == g_my_mac || cfg.target_mac == 0xFFFF) {
                    if (cfg.role == 99 && cfg.node_id == 0) {
                        g_is_calibrating = true;
                    } else {
                        flash_config_write(cfg.role, cfg.node_id, g_my_pos_x, g_my_pos_y);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        NVIC_SystemReset();
                    }
                }
            }
            last_ble_scan = now;
        }

        // --- BLE beacon (mỗi 5s): anchor broadcast vị trí của mình ---
        if (now - last_ble_tx > pdMS_TO_TICKS(5000)) {
            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte;
                uint8_t id;
                uint8_t seq;
                float   x;
                float   y;
            } ble_anchor_packed_t;
            #pragma pack(pop)

            ble_anchor_packed_t pkt;
            pkt.start_byte = '[';
            pkt.id         = g_current_node_id;
            pkt.x          = my_pos_x;
            pkt.y          = my_pos_y;

            for (int i = 0; i < 3; i++) {
                pkt.seq = anchor_seq++;
                ble_raw_beacon_send_payload((uint8_t*)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }
            last_ble_tx = now;
        }
    }
}