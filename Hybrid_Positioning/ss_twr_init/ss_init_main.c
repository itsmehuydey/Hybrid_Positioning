#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_beacon.h"

#define MAX_ANCHORS 4
#define SPEED_OF_LIGHT 299702547

static uint8 tx_poll_msg[] = {0x41,0x88,0,0xCA,0xDE,'W','A','V','E',0xE0,0,0,0};
static uint8 rx_buffer[24];
static uint16 g_cycle_id = 0;

// Hàm quan trọng: Xóa sạch trạng thái cũ để đo Anchor mới
void reset_uwb_state(void) {
    dwt_setrxtimeout(0);
    dwt_rxreset();
    dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF); // Clear all flags
}

void ss_initiator_task_function(void *pvParameter) {
    //ble_raw_beacon_init(TAG_ID);
    printf("[TAG] STARTING SEQUENTIAL TOF\r\n");

    while (1) {
        g_cycle_id++;
        for (int a = 0; a < MAX_ANCHORS; a++) {
            
            reset_uwb_state(); // Reset trước khi đổi ID
            vTaskDelay(2);     // Nghỉ ngắn để chip ổn định

            tx_poll_msg[10] = (uint8)a; // Gán ID Anchor (0, 1, 2, 3)
            tx_poll_msg[2]++;           // Tăng Sequence Number

            dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
            dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
            
            // Gửi POLL và chờ phản hồi ngay lập tức
            if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) == DWT_SUCCESS) {
                dwt_setrxtimeout(5000); // 5ms là đủ cho phản hồi TOF

                // Chờ sự kiện
                while (!((dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

                if (dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG) {
                    dwt_readrxdata(rx_buffer, 24, 0);
                    if (rx_buffer[9] == 0xE1) {
                        uint32 t_round_t = dwt_readtxtimestamplo32();
                        uint32 t_round_r = dwt_readrxtimestamplo32();
                        uint32 t_reply_r = *(uint32*)&rx_buffer[10];
                        uint32 t_reply_t = *(uint32*)&rx_buffer[14];

                        int32 rtd_init = (int32)(t_round_r - t_round_t);
                        int32 rtd_resp = (int32)(t_reply_t - t_reply_r);

                        float clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
                        double tof = ((double)rtd_init - (double)rtd_resp * (1.0f - clockOffsetRatio)) / 2.0;
                        float dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

                        if (dist > 0 && dist < 150) {
                            // Gửi BLE
                            //mh_ble_tof_packet_t ble_pkt = {'T', (uint8_t)a, g_cycle_id, dist};
                            //ble_raw_beacon_send_payload((uint8_t *)&ble_pkt, sizeof(ble_pkt));
                            printf("[TOF] A%d: %.2fm\r\n", a, dist);
                        }
                    }
                }
            }
            // Khe hở thời gian để Anchor tiếp theo không bị nhiễu bởi gói tin cũ
            vTaskDelay(pdMS_TO_TICKS(1000)); 
        }
    }
}