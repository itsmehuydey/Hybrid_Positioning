#include "uwb_calib.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "ble_beacon.h"

#define POLL_RX_TO_RESP_TX_DLY_UUS 1500 
#define UUS_TO_DWT_TIME            65536
#define TX_ANT_DLY                 16436 
#define SPEED_OF_LIGHT             299702547.0
#define FREQ_OFFSET_MULTIPLIER     (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_TO_PPM_MULTIPLIER_CHAN_5 (-1.0e6 / 6489.6e6)

// Thêm biến bù trừ phần mềm (Antenna Delay Offset). Giá trị 0.52 tương đương 52cm sai số.
#define UWB_HARDWARE_OFFSET        0.52f 

static uint8_t rx_buffer[64];
static uint8_t tx_resp_msg[31] = {
    0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0           
};

uint64_t get_rx_timestamp_u64(void) {
    uint8_t ts_tab[5]; uint64_t ts = 0;
    dwt_readrxtimestamp(ts_tab);
    for (int i = 4; i >= 0; i--) { ts <<= 8; ts |= ts_tab[i]; }
    return ts;
}

void resp_msg_set_ts(uint8_t *ts_field, const uint64_t ts) {
    for (int i = 0; i < 4; i++) ts_field[i] = (ts >> (i * 8)) & 0xFF;
}

void uwb_calib_init(void) {
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
}

float measure_inter_anchor_tof(uint8_t target_id) {
    uint8_t tx_poll[13] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, target_id, 0, 0};
    uint8_t rx_buf[32];
    
    uwb_calib_init();
    dwt_setrxtimeout(5000);
    dwt_writetxdata(sizeof(tx_poll), tx_poll, 0);
    dwt_writetxfctrl(sizeof(tx_poll), 0, 1);
    
    if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) == DWT_SUCCESS) {
        while (!((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));
        if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
            dwt_readrxdata(rx_buf, 32, 0);
            if (rx_buf[9] == 0xE1) {
                uint32_t t_round_t = dwt_readtxtimestamplo32();
                uint32_t t_round_r = dwt_readrxtimestamplo32();
                uint32_t t_reply_r = *(uint32_t*)&rx_buf[10];
                uint32_t t_reply_t = *(uint32_t*)&rx_buf[14];
                int32_t rtd_init = (int32_t)(t_round_r - t_round_t);
                int32_t rtd_resp = (int32_t)(t_reply_t - t_reply_r);
                float clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
                double tof = ((double)rtd_init - (double)rtd_resp * (1.0f - clockOffsetRatio)) / 2.0;
                
                // Tính khoảng cách thô
                float raw_dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);
                
                // Trừ đi sai số phần cứng để tối ưu hiệu chuẩn (Optimization)
                float final_dist = raw_dist - UWB_HARDWARE_OFFSET;
                
                return (final_dist > 0.01f) ? final_dist : 0.01f;
            }
        }
    }
    return 0.0f;
}

void serve_as_inter_anchor_responder(uint32_t timeout_ms, uint8_t my_id) {
    TickType_t start = xTaskGetTickCount();
    while (xTaskGetTickCount() - start < pdMS_TO_TICKS(timeout_ms)) {
        uwb_calib_init();
        dwt_setrxtimeout(65000);
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR))) {
            if (xTaskGetTickCount() - start >= pdMS_TO_TICKS(timeout_ms)) return;
        }
        if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
            uint32_t frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
            if (frame_len <= sizeof(rx_buffer)) dwt_readrxdata(rx_buffer, frame_len, 0);
            
            if (rx_buffer[9] == 0xE0 && rx_buffer[10] == my_id) {
                uint64_t poll_rx_ts = get_rx_timestamp_u64();
                uint32_t resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
                dwt_setdelayedtrxtime(resp_tx_time);
                uint64_t resp_tx_ts = (((uint64_t)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
                
                resp_msg_set_ts(&tx_resp_msg[10], poll_rx_ts);
                resp_msg_set_ts(&tx_resp_msg[14], resp_tx_ts);
                tx_resp_msg[30] = my_id;
                
                dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
                dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);
                if (dwt_starttx(DWT_START_TX_DELAYED) == DWT_SUCCESS) {
                    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
                    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                }
            }
        }
    }
}

void send_ble_distance(uint8_t src, uint8_t dst, float dist) {
    uint8_t payload[7];
    payload[0] = 'D'; payload[1] = src; payload[2] = dst;
    memcpy(&payload[3], &dist, 4);
    for(int i = 0; i < 20; i++) {
        ble_raw_beacon_send_payload(payload, 7);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}