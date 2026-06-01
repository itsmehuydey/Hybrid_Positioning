#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h"
#include "anchor_calib.h"

#define MY_ANCHOR_ID NODE_ID

/* Tọa độ sẽ được điền bởi anchor_self_calibrate() */
static float my_pos_x = 0.0f;
static float my_pos_y = 0.0f;

/* ---------------------------------------------------------------
   Gói phản hồi UWB (27 byte)
     [0-9]   Header
     [10-13] T1: Poll RX timestamp
     [14-17] T2: Resp TX timestamp  (pre-computed từ DELAYED time)
     [18-21] Anchor pos X (float)
     [22-25] Anchor pos Y (float)
     [26]    Anchor ID
   --------------------------------------------------------------- */
static uint8 tx_resp_msg[27] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1,
    0,0,0,0,  /* T1 */
    0,0,0,0,  /* T2 */
    0,0,0,0,  /* X  */
    0,0,0,0,  /* Y  */
    0         /* ID */
};
/* FCS 2 byte được DW1000 TỰ APPEND khi TX, KHÔNG đưa vào buffer.
   dwt_writetxfctrl(27, ...) báo cho chip gửi 27 byte user data
   + 2 byte FCS tỰ động ⇒ trên không là 29 byte. Tag nhận 27 byte. */
/* [FIX 1] Tăng delay lên 3000µs (từ 1500µs).
   Lý do: sau khi nhận poll, code phải đọc data + chuẩn bị response.
   1500µs đôi khi không đủ khi FreeRTOS có task switch. 3000µs an toàn hơn
   và vẫn nằm trong RX timeout 5ms của tag. */
#define POLL_RX_TO_RESP_TX_DLY_UUS  3000
#define UUS_TO_DWT_TIME             65536
#define TX_ANT_DLY                  16436

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

void ss_responder_task_function(void *pvParameter) {

    /* === BƯỚC 1: SELF-CALIBRATION === */
    bool calib_ok = anchor_self_calibrate(MY_ANCHOR_ID, &my_pos_x, &my_pos_y);
    if (!calib_ok) {
        printf("[A%d] WARNING: calibration failed, using (0,0)\r\n", MY_ANCHOR_ID);
    }

    /* Reset DW1000 hoàn toàn sau calibration */
    dwt_forcetrxoff();
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Nhúng tọa độ và ID vào template một lần duy nhất */
    memcpy(&tx_resp_msg[18], &my_pos_x, sizeof(float));
    memcpy(&tx_resp_msg[22], &my_pos_y, sizeof(float));
    tx_resp_msg[26] = (uint8)MY_ANCHOR_ID;

    /* Khởi tạo BLE */
    ble_raw_beacon_init(MY_ANCHOR_ID);

    printf("[A%d] READY. pos=(%.3f, %.3f)\r\n",
           MY_ANCHOR_ID, my_pos_x, my_pos_y);

    TickType_t last_ble_tx       = 0;
    TickType_t last_heartbeat_tx = 0;
    uint8_t    anchor_seq        = 0;

    /* === BƯỚC 2: VÒNG LẶP CHÍNH === */
    uint32_t rx_count = 0;
    while (1) {
        rx_count++;
        dwt_setrxtimeout(65000);
        dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);  /* clear all */
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        while (!(dwt_read32bitreg(SYS_STATUS_ID) &
                 (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32 status = dwt_read32bitreg(SYS_STATUS_ID);

        if (status & SYS_STATUS_RXFCG) {
            uint32 flen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (flen <= sizeof(rx_buffer))
                dwt_readrxdata(rx_buffer, flen, 0);

            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == (uint8)MY_ANCHOR_ID) {

                /* === TIMING-CRITICAL: không printf trong đoạn này === */
                uint64 poll_rx_ts = get_rx_timestamp_u64();

                uint32 resp_tx_time = (uint32)(
                    (poll_rx_ts + (uint64)POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME) >> 8
                );
                dwt_setdelayedtrxtime(resp_tx_time);

                uint64 resp_tx_ts = ((uint64)(resp_tx_time & 0xFFFFFFFEUL) << 8) + TX_ANT_DLY;

                resp_msg_set_ts(&tx_resp_msg[10], poll_rx_ts);
                resp_msg_set_ts(&tx_resp_msg[14], resp_tx_ts);

                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

                int tx_ret = dwt_starttx(DWT_START_TX_DELAYED);
                if (tx_ret == DWT_SUCCESS) {
                    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                    /* printf SAU TX -- không ảnh hưởng timing */
                    printf("[A%d] responded (delayed) rx#%lu\r\n",
                           MY_ANCHOR_ID, (unsigned long)rx_count);
                } else {
                    /* Fallback IMMEDIATE khi DELAYED "time in past" */
                    dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
                    dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);
                    if (dwt_starttx(DWT_START_TX_IMMEDIATE) == DWT_SUCCESS) {
                        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                        printf("[A%d] responded (immediate fallback) rx#%lu\r\n",
                               MY_ANCHOR_ID, (unsigned long)rx_count);
                    } else {
                        printf("[A%d] TX FAIL rx#%lu\r\n",
                               MY_ANCHOR_ID, (unsigned long)rx_count);
                        dwt_rxreset();
                    }
                }

            } else {
                /* Frame không dành cho anchor này */
                printf("[A%d] ignore: b9=0x%02X b10=%d myID=%d\r\n",
                       MY_ANCHOR_ID, rx_buffer[9], rx_buffer[10], (uint8)MY_ANCHOR_ID);
                dwt_rxreset();
            }

        } else {
            dwt_rxreset();
        }

        /* Heartbeat / BLE section đã tạm tắt để debug.
           Chỉ in alive mỗi 10 giây qua timer đơn giản. */
        TickType_t now2 = xTaskGetTickCount();
        if (now2 - last_heartbeat_tx > pdMS_TO_TICKS(10000)) {
            printf("[A%d] --- heartbeat, rx_count=%lu ---\r\n",
                   MY_ANCHOR_ID, (unsigned long)rx_count);
            last_heartbeat_tx = now2;
        }
    }
}