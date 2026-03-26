#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"

#ifndef NODE_ID
#define NODE_ID 3
#endif
#define MY_ANCHOR_ID NODE_ID

static uint8 tx_resp_msg[] = {0x41,0x88,0,0xCA,0xDE,'V','E','W','A',0xE1,0,0,0,0,0,0,0,0,0,0};

// Giảm delay xuống 1.5ms để Tag không phải đợi lâu
#define POLL_RX_TO_RESP_TX_DLY_UUS 1500 
#define UUS_TO_DWT_TIME            65536

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
    printf("[A%d] READY FOR TOF\r\n", MY_ANCHOR_ID);
    
    while (1) {
        // Luôn dọn sạch sự kiện cũ trước khi bật RX
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // Chờ sự kiện (Có gói tin hoặc Lỗi/Timeout)
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        uint32 status = dwt_read32bitreg(SYS_STATUS_ID);
        if (status & SYS_STATUS_RXFCG) {
            uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            dwt_readrxdata(rx_buffer, frame_len, 0);

            // Kiểm tra Function Code (0xE0) và Destination ID
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
                // Nếu không phải gói cho mình, reset ngay để nhận gói tiếp theo
                dwt_rxreset();
            }
        }
        // Tránh chiếm dụng CPU quá mức
        vTaskDelay(1);
    }
}