#include "sdk_config.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"

#define RNG_DELAY_MS 100
#define RX_TIMEOUT_UUS 5000
static uint8 rx_poll_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'W', 'A', 'V', 'E', 0xE0, 0, 0};
static uint8 tx_resp_msg[] = {0x41, 0x88, 0, 0xCA, 0xDE, 'V', 'E', 'W', 'A', 0xE1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ALL_MSG_COMMON_LEN 10
#define ALL_MSG_SN_IDX 2
#define RESP_MSG_POLL_RX_TS_IDX 10
#define RESP_MSG_RESP_TX_TS_IDX 14
#define RESP_MSG_TS_LEN 4

static uint8 frame_seq_nb = 0;

#define RX_BUF_LEN 24
static uint8 rx_buffer[RX_BUF_LEN];

static uint32 status_reg = 0;

#define UUS_TO_DWT_TIME 65536
#define POLL_RX_TO_RESP_TX_DLY_UUS 1500
//#define POLL_RX_TO_RESP_TX_DLY_UUS 1100
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500

typedef unsigned long long uint64;
static uint64 poll_rx_ts;
static uint64 resp_tx_ts;

static uint64 get_rx_timestamp_u64(void);
static void resp_msg_set_ts(uint8 *ts_field, const uint64 ts);

//int ss_resp_run(void)
//{
//    // 1. Bật RX
//    dwt_rxenable(DWT_START_RX_IMMEDIATE);

//    // 2. Chờ sự kiện RX (thành công / timeout / lỗi)
//    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
//             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
//    {}

//    // 3. ĐỌC FRAME TRƯỚC KHI CLEAR FLAG
//    uint32 frame_len = 0;
//    int rx_success = 0;

//    if (status_reg & SYS_STATUS_RXFCG)
//    {
//        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
//        if (frame_len <= RX_BUF_LEN)
//        {
//            dwt_readrxdata(rx_buffer, frame_len, 0);
//            rx_success = 1;
//        }
//    }

//    // 4. BÂY GIỜ MỚI CLEAR TẤT CẢ CÁC FLAG RX
//    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

//    // 5. Xử lý nếu nhận thành công
//    if (rx_success)
//    {
//        rx_buffer[ALL_MSG_SN_IDX] = 0;
//        if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
//        {
//            printf("[Anchor] Poll received\r\n");

//            // --- Tính toán và gửi RESP ---
//            poll_rx_ts = get_rx_timestamp_u64();
//            uint32 resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
//            dwt_setdelayedtrxtime(resp_tx_time);
//            resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;

//            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
//            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);

//            tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
//            dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
//            dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

//            int ret = dwt_starttx(DWT_START_TX_DELAYED);
//            if (ret == DWT_SUCCESS)
//            {
//                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
//                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
//                frame_seq_nb++;
//                printf("[Anchor] -> RESP sent\r\n");
//            }
//            else
//            {
//                printf("[Anchor] Failed to send RESP (missed time)\r\n");
//            }
//        }
//        else
//        {
//            printf("[Anchor] Frame received but NOT Poll\r\n");
//        }
//    }
//    else
//    {
//        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
//        dwt_rxreset();
//        printf("[Anchor] RX timeout or error\r\n");
//    }

//    dwt_rxenable(DWT_START_RX_IMMEDIATE);
//    return 1;
//}


int ss_resp_run(void)
{
    printf("[Anchor] === Enter ss_resp_run() ===\r\n");

    // 1. Bật RX
    
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    printf("[Anchor] -> Enabling RX (listening...)\r\n");
    uint32 wait_count = 0;
    // 2. Chờ sự kiện RX (frame nhận thành công / timeout / lỗi)
    while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
             (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
    {
    wait_count++;
    if (wait_count % 1000000 == 0) {
        printf("[Anchor] ...still waiting (no frame yet)\r\n");
    }
    }

    printf("[Anchor] -> Event detected, SYS_STATUS = 0x%08X\r\n", status_reg);

    // 3. Đọc frame trước khi clear flag
    uint32 frame_len = 0;
    int rx_success = 0;

    if (status_reg & SYS_STATUS_RXFCG)
    {
        frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
        printf("[Anchor] -> Frame received OK, len = %lu bytes\r\n", frame_len);

        if (frame_len <= RX_BUF_LEN)
        {
            dwt_readrxdata(rx_buffer, frame_len, 0);
            rx_success = 1;

            // In ra 8 byte đầu để xem có đúng Poll không
            printf("[Anchor] -> RX Data (first 8 bytes): ");
            for (int k = 0; k < 8; k++)
                printf("%02X ", rx_buffer[k]);
            printf("\r\n");
        }
    }

    // 4. Clear tất cả cờ RX
    dwt_write32bitreg(SYS_STATUS_ID,
                      SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);

    // 5. Nếu nhận thành công
    if (rx_success)
    {
        // Xóa số sequence cũ trước khi so sánh
        rx_buffer[ALL_MSG_SN_IDX] = 0;

        if (memcmp(rx_buffer, rx_poll_msg, ALL_MSG_COMMON_LEN) == 0)
        {
            printf("[Anchor] Poll message detected\r\n");

            // === Tính toán timestamp & chuẩn bị gửi RESP ===
            poll_rx_ts = get_rx_timestamp_u64();
            printf("[Anchor] Poll RX Timestamp = %llu\r\n", poll_rx_ts);

            uint32 resp_tx_time = (poll_rx_ts +
                    (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

            dwt_setdelayedtrxtime(resp_tx_time);
            resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
            printf("[Anchor] RESP TX scheduled @ %llu (DWT time)\r\n", resp_tx_ts);

            // Gắn timestamp vào frame RESP
            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_POLL_RX_TS_IDX], poll_rx_ts);
            resp_msg_set_ts(&tx_resp_msg[RESP_MSG_RESP_TX_TS_IDX], resp_tx_ts);
            tx_resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;

            // Ghi vào buffer TX và gửi
            dwt_writetxdata(sizeof(tx_resp_msg), tx_resp_msg, 0);
            dwt_writetxfctrl(sizeof(tx_resp_msg), 0, 1);

            printf("[Anchor] -> Sending RESP...\r\n");
            int ret = dwt_starttx(DWT_START_TX_DELAYED);

            if (ret == DWT_SUCCESS)
            {
                // Chờ TX hoàn thành
                while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {}
                dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
                frame_seq_nb++;
                printf("[Anchor] RESP sent successfully\r\n");
            }
            else
            {
                printf("[Anchor] Failed to send RESP (missed time)\r\n");
            }
        }
        else
        {
            printf("[Anchor] Received frame but it is NOT Poll\r\n");
        }
    }
    else
    {
        // Nhận thất bại hoặc timeout
        printf("[Anchor] ❌ RX timeout or error (no valid Poll)\r\n");
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
        dwt_rxreset();
    }

    dwt_rxenable(DWT_START_RX_IMMEDIATE);
    printf("[Anchor] -> Listening again...\r\n\n");
    deca_sleep(5);
    return 1;
}

static uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

static void resp_msg_set_ts(uint8 *ts_field, const uint64 ts)
{
    int i;
    for (i = 0; i < RESP_MSG_TS_LEN; i++)
    {
        ts_field[i] = (ts >> (i * 8)) & 0xFF;
    }
}

void ss_responder_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    dwt_setleds(DWT_LEDS_ENABLE);
    while (true)
    {
        ss_resp_run();
        vTaskDelay(RNG_DELAY_MS);
    }
}