#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h"
#include "anchor_calib.h"   /* anchor_self_calibrate() */

/* ---------------------------------------------------------------
   NODE_ID convention (xem anchor_calib.h để hiểu đầy đủ):
     NODE_ID = 1       → Tag
     NODE_ID = 2       → Anchor gốc (Reference), tự động = (0,0)
     NODE_ID = 3,4,5…  → Anchor slave, tự tính tọa độ khi boot
   --------------------------------------------------------------- */
#ifndef NODE_ID
#define NODE_ID 2   /* Mặc định: anchor gốc */
#endif
#define MY_ANCHOR_ID NODE_ID

/* ---------------------------------------------------------------
   Tọa độ của anchor này – KHÔNG hardcode ở đây nữa.
   Sẽ được điền bởi anchor_self_calibrate() lúc khởi động.
   --------------------------------------------------------------- */
static float my_pos_x = 0.0f;
static float my_pos_y = 0.0f;

/* ---------------------------------------------------------------
   Format gói phản hồi UWB (27 byte):
     [0-9]   Header
     [10-13] T1: Poll RX timestamp  → Tag dùng để tính TOF
     [14-17] T2: Resp TX timestamp  → Tag dùng để tính TOF
     [18-21] Tọa độ X (float, LE)   → Tag đọc để biết vị trí anchor
     [22-25] Tọa độ Y (float, LE)   → Tag đọc để biết vị trí anchor
     [26]    Anchor ID               → Tag dùng để lưu đúng slot
   --------------------------------------------------------------- */
static uint8 tx_resp_msg[27] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, /* [0-9]  Header  */
    0, 0, 0, 0,                                            /* [10-13] T1      */
    0, 0, 0, 0,                                            /* [14-17] T2      */
    0, 0, 0, 0,                                            /* [18-21] pos X   */
    0, 0, 0, 0,                                            /* [22-25] pos Y   */
    0                                                       /* [26]    ID      */
};

/* 1500 us delay giữa nhận poll và gửi response */
#define POLL_RX_TO_RESP_TX_DLY_UUS 1500
#define UUS_TO_DWT_TIME            65536
#define TX_ANT_DLY                 16436

static uint8 rx_buffer[64];

static uint64 get_rx_timestamp_u64(void) {
    uint8 ts[5]; uint64 t = 0;
    dwt_readrxtimestamp(ts);
    for (int i = 4; i >= 0; i--) { t <<= 8; t |= ts[i]; }
    return t;
}

static void resp_msg_set_ts(uint8 *field, uint64 ts) {
    for (int i = 0; i < 4; i++) field[i] = (ts >> (i * 8)) & 0xFF;
}

/* ---------------------------------------------------------------
   Task chính của Anchor
   --------------------------------------------------------------- */
void ss_responder_task_function(void *pvParameter) {

    /* === BƯỚC 1: SELF-CALIBRATION ===
       Tự động tính ra tọa độ (my_pos_x, my_pos_y) bằng cách đo
       TWR đến các anchor có ID thấp hơn. Anchor gốc (ID=2) sẽ
       trả về (0,0) ngay lập tức. Anchor slave chờ theo slot.    */
    bool calib_ok = anchor_self_calibrate(MY_ANCHOR_ID, &my_pos_x, &my_pos_y);
    if (!calib_ok) {
        /* Fallback: dùng (0,0) nếu calibrate thất bại, hệ thống
           vẫn chạy nhưng tọa độ của anchor này sẽ không chính xác */
        printf("[A%d] WARNING: calibration failed, using (0,0)\r\n", MY_ANCHOR_ID);
    }

    /* === BƯỚC 2: KHỞI TẠO BLE VÀ NHÚNG TỌA ĐỘ VÀO GÓI UWB === */
    ble_raw_beacon_init(MY_ANCHOR_ID);

    /* Nhúng tọa độ và ID vào template gói phản hồi UWB một lần duy nhất.
       Mỗi lần tag poll, anchor chỉ cần cập nhật timestamp (T1, T2),
       tọa độ và ID giữ nguyên.                                    */
    memcpy(&tx_resp_msg[18], &my_pos_x, sizeof(float));
    memcpy(&tx_resp_msg[22], &my_pos_y, sizeof(float));
    tx_resp_msg[26] = (uint8)MY_ANCHOR_ID;

    printf("[A%d] READY. pos=(%.3f, %.3f)\r\n",
           MY_ANCHOR_ID, my_pos_x, my_pos_y);

    /* BLE beacon định kỳ và bộ đếm seq */
    TickType_t last_ble_tx  = 0;
    uint8_t    anchor_seq   = 0;

    /* === BƯỚC 3: VÒNG LẶP CHÍNH – PHẢN HỒI TAG ===              */
    while (1) {

        /* Đặt timeout 65ms để không bị kẹt nếu không có gói nào  */
        dwt_setrxtimeout(65000);
        dwt_write32bitreg(SYS_STATUS_ID,
            SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Chờ nhận gói UWB                                        */
        while (!(dwt_read32bitreg(SYS_STATUS_ID) &
                 (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32 status = dwt_read32bitreg(SYS_STATUS_ID);

        if (status & SYS_STATUS_RXFCG) {
            uint32 flen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (flen <= sizeof(rx_buffer))
                dwt_readrxdata(rx_buffer, flen, 0);

            /* Chỉ xử lý gói poll (0xE0) gửi đúng anchor này.
               Cùng function code với calibration poll của anchor
               khác → responder phục vụ cả tag lẫn anchor peer.   */
            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == (uint8)MY_ANCHOR_ID) {

                /* Tính thời điểm phát response                    */
                uint64 poll_rx_ts   = get_rx_timestamp_u64();
                uint32 resp_tx_time =
                    (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                dwt_setdelayedtrxtime(resp_tx_time);

                /* Timestamp chính xác (bao gồm antenna delay)     */
                uint64 resp_tx_ts =
                    (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

                /* Cập nhật T1 và T2 vào gói phản hồi              */
                resp_msg_set_ts(&tx_resp_msg[10], poll_rx_ts);
                resp_msg_set_ts(&tx_resp_msg[14], resp_tx_ts);

                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

                if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS) {
                    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                }

            } else {
                /* Gói không dành cho anchor này → reset RX         */
                dwt_rxreset();
            }
        } else {
            dwt_rxreset();
        }

        /* === BLE BEACON ĐỊNH KỲ (5 giây) ===
           Phát vị trí anchor qua BLE để PC/RPi có thể
           hiển thị sơ đồ anchor khi debug.                        */
        TickType_t now = xTaskGetTickCount();
        if (now - last_ble_tx > pdMS_TO_TICKS(5000)) {
            printf("{\"id\":%d,\"x\":%.3f,\"y\":%.3f,\"role\":\"anchor\"}\r\n",
                   MY_ANCHOR_ID, my_pos_x, my_pos_y);

            /* Gói BLE compact (11 byte): start='[', id, seq, x, y */
            #pragma pack(push, 1)
            typedef struct {
                uint8_t start_byte;  /* '[' = anchor packet       */
                uint8_t id;
                uint8_t seq;
                float   x;
                float   y;
            } ble_anchor_pkt_t;
            #pragma pack(pop)

            ble_anchor_pkt_t pkt;
            pkt.start_byte = '[';
            pkt.id         = MY_ANCHOR_ID;
            pkt.x          = my_pos_x;
            pkt.y          = my_pos_y;

            /* Phát lặp 10 lần để đảm bảo bắt được */
            for (int i = 0; i < 10; i++) {
                pkt.seq = anchor_seq++;
                ble_raw_beacon_send_payload((uint8_t*)&pkt, sizeof(pkt));
                vTaskDelay(pdMS_TO_TICKS(15));
            }

            last_ble_tx = now;
        }
    }
}