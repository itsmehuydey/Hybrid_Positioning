#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"        
#include "hybrid_scalable.h"

#define APP_NAME "SS TWR INIT v1.3"
#define RNG_DELAY_MS 100
#define POLL_MSG_DEST_ID_IDX  10

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0xFF, 0, 0};
static uint8 rx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4

static uint8 frame_seq_nb = 0;

#define RX_BUF_LEN 20
static uint8 rx_buffer[RX_BUF_LEN];

static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME 65536
#define SPEED_OF_LIGHT 299702547

static double tof;
static double distance;

static void resp_msg_get_ts(uint8 *ts_field, uint32 *ts);

static volatile int tx_count = 0;
static volatile int rx_count = 0;

#define MAX_ANCHORS 8

typedef struct {
    double toa;       
    double distance;  
    int valid;
} AnchorMeas;

static AnchorMeas meas[MAX_ANCHORS];
extern vec2 anc[];
static double phi[N_ANCHORS - 1];
extern vec2 pos_est;

// ===== THÊM HÀM RESET DW1000 =====
static void reset_dw1000_state(void)
{
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    dwt_rxreset();
    dwt_setrxtimeout(0);  // Clear timeout
}

int ss_init_run(int anchor_id)
{
    if (anchor_id < 0 || anchor_id >= MAX_ANCHORS) return 0;

    printf("\n[Tag->A%d] START\r\n", anchor_id);

    // ===== 1. RESET TRẠNG THÁI TRƯỚC KHI GỬI =====
    reset_dw1000_state();
    vTaskDelay(5);  // Cho ổn định

    // ===== 2. GỬI POLL =====
    tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    tx_poll_msg[POLL_MSG_DEST_ID_IDX] = (uint8)anchor_id;
    
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);  
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
    dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);

    printf("[Tag->A%d] → POLL (seq %d)\r\n", anchor_id, frame_seq_nb);
    
    int ret = dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    if (ret != DWT_SUCCESS) {
        printf("[Tag->A%d]  TX FAILED\r\n", anchor_id);
        reset_dw1000_state();
        return 0;
    }
    
    tx_count++;
    dwt_setrxtimeout(2500); 
    
    uint32 wait_start = xTaskGetTickCount();
    uint32 max_wait = pdMS_TO_TICKS(50);  // Tối đa 50ms
    
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
        if ((xTaskGetTickCount() - wait_start) > max_wait) {
            printf("[Tag->A%d]  SW TIMEOUT\r\n", anchor_id);
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
            printf("[Tag->A%d]  Frame too long (%d)\r\n", anchor_id, frame_len);
            reset_dw1000_state();
            return 0;
        }

        rx_buffer[ALL_MSG_SN_IDX] = 0;

        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            printf("[Tag->A%d]  RESP OK\r\n", anchor_id);

            // ===== TÍNH TOF & DISTANCE =====
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

            meas[anchor_id].toa = tof;
            meas[anchor_id].distance = distance;
            meas[anchor_id].valid = 1;
            
            printf("[Tag->A%d]  Dist = %.2f m\r\n", anchor_id, distance);
            
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
        printf("[Tag->A%d]  RX FAIL (timeout/error)\r\n", anchor_id);
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
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

void ss_initiator_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    dwt_setleds(DWT_LEDS_ENABLE);

    double t0, ti[N_ANCHORS-1], d1 = -1, d2 = -1;
    int a1 = TOF_A1, a2 = TOF_A2;

    printf("\n[Tag]  INITIATOR TASK STARTED \r\n");
    printf("[Tag] Will measure %d anchors continuously\r\n\n", N_ANCHORS);

    while (1)
    {
        printf("    [CYCLE] Measuring %d Anchors\n", N_ANCHORS);

        int success_count = 0;

        // ===== ĐO TUẦN TỰ TỪNG ANCHOR =====
        for (int i = 0; i < N_ANCHORS; i++)
        {
            printf("\n--- [%d/%d] Anchor %d ---\n", i+1, N_ANCHORS, i);
            
            if (ss_init_run(i)) {
                success_count++;
            }
            
            // ===== DELAY GIỮA CÁC LẦN GỬI (QUAN TRỌNG!) =====
            vTaskDelay(pdMS_TO_TICKS(100));  // 150ms giữa mỗi anchor
        }

        printf(" Success: %d/%d anchors\n", success_count, N_ANCHORS);

        // ===== TÍNH TOÁN VỊ TRÍ NẾU ĐỦ DỮ LIỆU =====
        if (meas[0].valid && meas[1].valid && meas[2].valid && meas[3].valid)
        {
            t0 = meas[0].toa;
            double d0 = meas[0].distance;

            printf("\n🔧 Calculating clock offsets...\n");
            for (int i = 1; i < N_ANCHORS; i++)
            {
                double ti_val = meas[i].toa;
                double di = meas[i].distance;
                phi[i-1] = (ti_val - t0) - (di - d0) / C0;
                ti[i-1] = meas[i].toa;
                printf("   phi[%d] = %+.3e s\n", i-1, phi[i-1]);
            }

            // Gán distance cho hybrid
            if (a1 >= 0 && a1 < N_ANCHORS) d1 = meas[a1].distance;
            if (a2 >= 0 && a2 < N_ANCHORS) d2 = meas[a2].distance;

            int it = hybrid_localize(anc, N_ANCHORS, t0, ti, phi, d1, d2, a1, a2, &pos_est);

            printf("\n POSITION: (%.3f, %.3f) m | GN: %d iter\n", 
                   pos_est.x, pos_est.y, it);
        }
        else
        {
            printf("\n Not enough valid measurements for positioning\n");
        }

        // ===== DELAY GIỮA CÁC CHU KỲ ĐO =====
        vTaskDelay(pdMS_TO_TICKS(100));  // 500ms giữa các cycle
    }
}