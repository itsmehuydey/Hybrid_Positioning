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

static uint8 tx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
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
    double toa;       // TOA tại anchor (giây)
    double distance;  // TOF (mét)
    int valid;
} AnchorMeas;

static AnchorMeas meas[MAX_ANCHORS];
extern vec2 anc[];
extern double phi[];
extern vec2 pos_est;
extern Kalman2D kf;

int ss_init_run(int anchor_id)
{
    if (anchor_id < 0 || anchor_id >= MAX_ANCHORS) return 0;

    tx_poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    dwt_writetxdata(sizeof(tx_poll_msg), tx_poll_msg, 0);
    dwt_writetxfctrl(sizeof(tx_poll_msg), 0, 1);
    dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
    tx_count++;

    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {};

    frame_seq_nb++;

    if (status_reg & SYS_STATUS_RXFCG)
    {
        uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

        if (frame_len <= RX_BUF_LEN)
            dwt_readrxdata(rx_buffer, frame_len, 0);

        rx_buffer[ALL_MSG_SN_IDX] = 0;
        if (memcmp(rx_buffer, rx_resp_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            rx_count++;

            uint32 poll_tx_ts = dwt_readtxtimestamplo32();
            uint32 resp_rx_ts = dwt_readrxtimestamplo32();
            uint32 poll_rx_ts, resp_tx_ts;
            float clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_5 / 1.0e6);

            resp_msg_get_ts(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
            resp_msg_get_ts(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

            int32 rtd_init = resp_rx_ts - poll_tx_ts;
            int32 rtd_resp = resp_tx_ts - poll_rx_ts;

            tof = ((rtd_init - rtd_resp * (1.0f - clockOffsetRatio)) / 2.0f) * DWT_TIME_UNITS;
            distance = tof * SPEED_OF_LIGHT;

            // LƯU TOA: thời gian poll đến anchor
            double ti = poll_rx_ts * DWT_TIME_UNITS;

            meas[anchor_id].toa = ti;
            meas[anchor_id].distance = distance;
            meas[anchor_id].valid = 1;

            printf("A%d: TOA=%.3e s, D=%.3f m\n", anchor_id, ti, distance);
            return 1;
        }
    }
    else
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
    }

    meas[anchor_id].valid = 0;
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

    while (1)
    {
        printf("\n=== [HYBRID] Measuring %d Anchors ===\n", N_ANCHORS);

        // 1. Đo tuần tự từng anchor
        for (int i = 0; i < N_ANCHORS; i++)
        {
            printf("-> Measure Anchor %d...\n", i);
            if (ss_init_run(i))
                printf("   OK\n");
            else
                printf("   FAIL\n");
            vTaskDelay(80);
        }

        // 2. Lấy dữ liệu
        if (!meas[0].valid) { vTaskDelay(100); continue; }
        t0 = meas[0].toa;

        for (int i = 1; i < N_ANCHORS; i++)
            ti[i-1] = meas[i].valid ? meas[i].toa : t0;  // fallback

        if (meas[a1].valid) d1 = meas[a1].distance;
        if (meas[a2].valid) d2 = meas[a2].distance;

        // 3. GỌI THUẬT TOÁN
        int it = hybrid_localize(anc, N_ANCHORS, t0, ti, phi, d1, d2, a1, a2, &pos_est, &kf);

        printf("=> POS: (%.3f, %.3f) | GN: %d it\n", pos_est.x, pos_est.y, it);

        vTaskDelay(500);  // ~2 lần/giây
    }
}