#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "ble_hybrid.h"  

#define APP_NAME "INIT v1.3"
#define RNG_DELAY_MS 100
#define POLL_MSG_DEST_ID_IDX  10

// ===== GÓI POLL (TWR) GIỮ NGUYÊN =====
static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0xFF, 0, 0};
static uint8 rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// ===== GÓI BLINK (TDOA) – BROADCAST =====
#define BLINK_FUNC_CODE           0xE2
#define BLINK_MSG_FUNC_IDX        9
#define BLINK_MSG_DEST_IDX        10
#define BLINK_MSG_CYCLE_LSB_IDX   11
#define BLINK_MSG_CYCLE_MSB_IDX   12

static uint8 tx_blink_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E',
                               BLINK_FUNC_CODE, 0xFF, 0, 0};

#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4

static uint8 frame_seq_nb = 0;

// ID chu kỳ đo – dùng chung cho BLINK + TOF
static uint16 g_cycle_id = 0;

#define RX_BUF_LEN 20
static uint8 rx_buffer[RX_BUF_LEN];

static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME 65536
#define SPEED_OF_LIGHT 299702547

static double tof;
static double distance;

static void resp_msg_get_ts(uint8 *ts_field, uint32 *ts);
static void send_blink_broadcast(uint16 cycle_id);

static volatile int tx_count = 0;
static volatile int rx_count = 0;

#define MAX_ANCHORS 8

typedef struct {
    double toa;       
    double distance;  
    int valid;
} AnchorMeas;

static AnchorMeas meas[MAX_ANCHORS];

// ===== RESET DW1000 (GIỮ NHƯ CŨ) =====
static void reset_dw1000_state(void)
{
    dwt_write32bitreg(SYS_STATUS_ID,
                      SYS_STATUS_ALL_TX | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    dwt_rxreset();
    dwt_setrxtimeout(0);  // Clear timeout
}

int ss_init_run(int anchor_id)
{
    if (anchor_id < 0 || anchor_id >= MAX_ANCHORS) return 0;

    // ===== 1. RESET TRẠNG THÁI TRƯỚC KHI GỬI =====
    reset_dw1000_state();
    vTaskDelay(5);  // Cho ổn định

    // ===== 2. GỬI POLL (TWR) =====
    tx_poll_msg[ALL_MSG_SN_IDX]       = frame_seq_nb;
    tx_poll_msg[POLL_MSG_DEST_ID_IDX] = (uint8)anchor_id;
    
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);  
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
    dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
    
    int ret = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    if (ret != DWT_SUCCESS) {
        reset_dw1000_state();
        return 0;
    }
    
    tx_count++;
    dwt_setrxtimeout(2500); 
    
    uint32 wait_start = xTaskGetTickCount();
    uint32 max_wait   = pdMS_TO_TICKS(50);  // Tối đa 50ms
    
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
        if ((xTaskGetTickCount() - wait_start) > max_wait) {
            reset_dw1000_state();
            return 0;
        }
        vTaskDelay(1);
    }

    frame_seq_nb++;

    // ===== 4. XỬ LÝ KẾT QUẢ =====
    if (status_reg & SYS_STATUS_RXFCG)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

        int frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        
        if (frame_len <= RX_BUF_LEN) {
            dwt_readrxdata(rx_buffer, frame_len, 0);
        } else {
            reset_dw1000_state();
            return 0;
        }

        rx_buffer[ALL_MSG_SN_IDX] = 0;

        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            // ===== TÍNH TOF & DISTANCE (GIỮ NGUYÊN) =====
            uint32 poll_tx_ts = dwt_readtxtimestamplo32();
            uint32 resp_rx_ts = dwt_readrxtimestamplo32();
            uint32 poll_rx_ts, resp_tx_ts;
            resp_msg_get_ts(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
            resp_msg_get_ts(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

            float clockOffsetRatio = dwt_readcarrierintegrator() *
                (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);
            int32 rtd_init = resp_rx_ts - poll_tx_ts;
            int32 rtd_resp = resp_tx_ts - poll_rx_ts;

            tof = ((rtd_init - rtd_resp * (1.0f - clockOffsetRatio)) / 2.0f) * DWT_TIME_UNITS;
            distance = tof * SPEED_OF_LIGHT;

            meas[anchor_id].toa      = tof;
            meas[anchor_id].distance = distance;
            meas[anchor_id].valid    = 1;
            
            printf("[Tag->A%d]  Dist = %.2f m\r\n", anchor_id, distance);
            
            // ===== GỬI TOF QUA BLE ĐẾN MASTER =====
            mh_ble_tof_packet_t ble_pkt;
            ble_pkt.msg_type   = 'T';
            ble_pkt.anchor_id  = (uint8_t)anchor_id;
            ble_pkt.cycle_id   = g_cycle_id;
            ble_pkt.tof        = (float)tof;
            ble_pkt.distance   = (float)distance;
            
            // Gọi hàm gửi BLE (bạn cần implement hàm này)
            // ble_send_to_master((uint8_t*)&ble_pkt, sizeof(ble_pkt));
            
            reset_dw1000_state();  // Clean cho lần sau
            return 1;
        }
        else
        {
            printf("[Tag->A%d]  Wrong RESP header\r\n", anchor_id);
        }
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID,
                          SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
    }

    meas[anchor_id].valid = 0;
    reset_dw1000_state();
    return 0;
}

static void resp_msg_get_ts(uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < RESP_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}

// ===== GỬI BLINK BROADCAST (TDOA) TRƯỚC KHI ĐO TOF =====
static void send_blink_broadcast(uint16 cycle_id)
{
    tx_blink_msg[ALL_MSG_SN_IDX]          = frame_seq_nb;
    tx_blink_msg[BLINK_MSG_DEST_IDX]      = 0xFF;  // broadcast
    tx_blink_msg[BLINK_MSG_CYCLE_LSB_IDX] = (uint8)(cycle_id & 0xFF);
    tx_blink_msg[BLINK_MSG_CYCLE_MSB_IDX] = (uint8)((cycle_id >> 8) & 0xFF);

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    dwt_writetxdata(sizeof(tx_blink_msg), tx_blink_msg, 0);
    dwt_writetxfctrl(sizeof(tx_blink_msg), 0, 1);

    dwt_starttx(DWT_START_TX_IMMEDIATE);

    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) { }

    frame_seq_nb++;
}

void ss_initiator_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    dwt_setleds(DWT_LEDS_ENABLE);

    printf("\n[Tag]  INITIATOR TASK STARTED \r\n");
    printf("[Tag] Will measure %d anchors continuously\r\n\n", MAX_ANCHORS);

    while (1)
    {
        // ===== MỖI CYCLE: GỬI 1 BLINK CHUNG ĐỂ LẤY TDOA =====
        g_cycle_id++;
        send_blink_broadcast(g_cycle_id);
        vTaskDelay(pdMS_TO_TICKS(10));  // cho anchors ổn định 1 chút

        int success_count = 0;

        // ===== ĐO TUẦN TỰ TỪNG ANCHOR (TWR GIỮ NGUYÊN) =====
        for (int i = 0; i < MAX_ANCHORS; i++)
        {
            if (ss_init_run(i)) {
                success_count++;
            }
            vTaskDelay(pdMS_TO_TICKS(1000));  // 1s giữa mỗi anchor
        }

        printf(" Success: %d/%d anchors\n", success_count, MAX_ANCHORS);
        int ok = 1;
        for (int i = 0; i < 4; i++)
            if (!meas[i].valid) ok = 0;

        if (!ok) {
            printf("\n Not enough valid measurements\n");
        }
    }
}