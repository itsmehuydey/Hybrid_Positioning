#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h" // Kéo thư viện BLE vào

// Thay đổi NODE_ID (0, 1, 2, 3) tương ứng với từng mạch Anchor khi nạp code
#ifndef NODE_ID
#define NODE_ID 2
#endif
#define MY_ANCHOR_ID NODE_ID

// =======================================================
// TỌA ĐỘ ANCHOR (đơn vị: mét, gốc tọa độ tại Anchor 0)
// Khi nạp firmware cho mỗi mạch Anchor, thay NODE_ID
// đúng với vị trí vật lý rồi đo thực tế để điền số này.
//
// Ví dụ bố trí phòng 5m x 5m:
//   A0 (0,0)  ─────  A1 (5,0)
//     │                  │
//   A2 (0,5)  ─────  A3 (5,5)
//
// QUAN TRỌNG: Tag sẽ ĐỌC tọa độ này từ gói phản hồi UWB
// (byte 18-25), nên KHÔNG cần sửa ở phía Tag.
// =======================================================
#if MY_ANCHOR_ID == 0
    float my_pos_x = 0.0f, my_pos_y = 0.0f;  // Góc dưới-trái
#elif MY_ANCHOR_ID == 1
    float my_pos_x = 5.0f, my_pos_y = 0.0f;  // Góc dưới-phải
#elif MY_ANCHOR_ID == 2
    float my_pos_x = 0.0f, my_pos_y = 5.0f;  // Góc trên-trái
#else
    float my_pos_x = 5.0f, my_pos_y = 5.0f;  // Góc trên-phải (A3)
#endif

// -------------------------------------------------------
// Format gói phản hồi UWB (27 byte), Tag đọc để biết:
//   - Timestamp Poll RX (T1): byte 10-13 → tính TOF
//   - Timestamp Resp TX (T2): byte 14-17 → tính TOF
//   - Tọa độ X của Anchor   : byte 18-21 (float, LE)
//   - Tọa độ Y của Anchor   : byte 22-25 (float, LE)
//   - Anchor ID             : byte 26
//
// Nhờ đó Tag tự biết tọa độ từng Anchor mà KHÔNG cần
// hardcode ở phía Tag → tránh mâu thuẫn cấu hình.
// -------------------------------------------------------
static uint8 tx_resp_msg[27] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, // [ 0- 9] Header
    0, 0, 0, 0,                                            // [10-13] TS1: Poll RX timestamp
    0, 0, 0, 0,                                            // [14-17] TS2: Resp TX timestamp
    0, 0, 0, 0,                                            // [18-21] Anchor pos X (float)
    0, 0, 0, 0,                                            // [22-25] Anchor pos Y (float)
    0                                                       // [26]    Anchor ID
};

#define POLL_RX_TO_RESP_TX_DLY_UUS 1500 
#define UUS_TO_DWT_TIME            65536
#define TX_ANT_DLY                 16436 

static uint8 rx_buffer[64];

static uint64 get_rx_timestamp_u64(void) {
    uint8 ts_tab[5]; uint64 ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) { ts <<= 8; ts |= ts_tab[i]; }
    return ts;
}

static void resp_msg_set_ts(uint8 *ts_field, const uint64 ts) {
    for (int i = 0; i < 4; i++) ts_field[i] = (ts >> (i * 8)) & 0xFF;
}

void ss_responder_task_function(void *pvParameter) {
    // Khởi tạo BLE cho Anchor
    ble_raw_beacon_init(MY_ANCHOR_ID);
    printf("[A%d] READY. POS: (%.2f, %.2f)\r\n", MY_ANCHOR_ID, my_pos_x, my_pos_y);
    
    // Nhúng sẵn tọa độ X, Y và ID vào mảng gửi qua UWB
    memcpy(&tx_resp_msg[18], &my_pos_x, sizeof(float));
    memcpy(&tx_resp_msg[22], &my_pos_y, sizeof(float));
    tx_resp_msg[26] = (uint8)MY_ANCHOR_ID;

    TickType_t last_ble_tx = 0;
    static uint8_t anchor_seq = 0; // Bộ đếm BLE cho Anchor

    while (1) {
        // Đặt timeout 65000 symbols (~65ms) để vòng lặp không bị kẹt chết ở UWB RX
        dwt_setrxtimeout(65000);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // Chờ nhận được khung UWB hoặc hết thời gian timeout (65ms)
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32 status = dwt_read32bitreg(SYS_STATUS_ID);
        if (status & SYS_STATUS_RXFCG) {
            uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer)) {
                dwt_readrxdata(rx_buffer, frame_len, 0);
            }

            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == (uint8)MY_ANCHOR_ID) {
                uint64 poll_rx_ts = get_rx_timestamp_u64();
                uint32 resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                dwt_setdelayedtrxtime(resp_tx_time);

                uint64 resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

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

        // ==========================================
        // GỬI BLE ĐỊNH KỲ (MỖI 2 GIÂY) - CÓ SPAM LẶP
        // ==========================================
        TickType_t now = xTaskGetTickCount();
        if (now - last_ble_tx > pdMS_TO_TICKS(5000)) {
            // In ra UART dạng JSON
            printf("{\"id\":%d,\"x\":%.2f,\"y\":%.2f,\"role\":\"anchor\"}\r\n", 
                   MY_ANCHOR_ID, my_pos_x, my_pos_y);

            // Đóng gói nhị phân (11 byte)
            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte; 
                uint8_t id;
                uint8_t seq;
                float x;
                float y;
            } ble_anchor_packed_t;
            #pragma pack(pop)

            ble_anchor_packed_t pkt;
            pkt.start_byte = '[';  // Mã ASCII '[' để nhận diện là Anchor
            pkt.id = MY_ANCHOR_ID;
            pkt.x = my_pos_x;
            pkt.y = my_pos_y;

            // Bắn lặp lại 10 lần để chắc chắn máy tính bắt được
            for(int i = 0; i < 10; i++) {
                pkt.seq = anchor_seq++;
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }

            last_ble_tx = now;
        }
    }
}