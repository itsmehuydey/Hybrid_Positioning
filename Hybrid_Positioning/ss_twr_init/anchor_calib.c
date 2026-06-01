#include "anchor_calib.h"
#include "utils.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port_platform.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ---------------------------------------------------------------
   Hằng số TWR – giống ss_init_main.c
   --------------------------------------------------------------- */
#define SPEED_OF_LIGHT   299702547.0
#define FREQ_OFFSET_MULT (998.4e6 / 2.0 / 1024.0 / 131072.0)
#define HERTZ_PPM_CHAN5  (-1.0e6 / 6489.6e6)

/* Số lần thử lại nếu 1 lần TWR thất bại */
#define CALIB_RETRIES    5

/* Timeout chờ gói phản hồi UWB (~10 ms) */
#define CALIB_RX_TIMEOUT 10000

/* ---------------------------------------------------------------
   Biến nội bộ
   --------------------------------------------------------------- */
static uint8_t calib_seq = 0;
static uint8_t calib_rx_buf[64];

/* Gói poll anchor-to-anchor.
   Dùng cùng function code 0xE0 với tag poll → anchor đích
   dùng code responder hiện có, KHÔNG cần sửa ss_resp_main.c
   thêm nhánh mới.                                              */
static uint8_t calib_poll_msg[] = {
    0x41, 0x88, 0,           /* Frame ctrl, seq# (byte 2 cập nhật lúc gửi) */
    0xCA, 0xDE,              /* PAN ID                                       */
    'W', 'A', 'V', 'E',     /* Magic header                                 */
    0xE0,                    /* Function code – poll (giống tag)             */
    0,                       /* Byte 10: ID anchor đích (cập nhật lúc gửi)  */
    0, 0                     /* FCS placeholder                              */
};

/* ---------------------------------------------------------------
   calib_do_twr()
   Thực hiện 1 phép đo TWR đến anchor đích, trả về:
     - dist   : khoảng cách (m)
     - out_x/y: tọa độ anchor đích (đọc từ byte 18-25 gói phản hồi)
   Trả về 1 nếu thành công, 0 nếu thất bại.
   --------------------------------------------------------------- */
static int calib_do_twr(uint8_t target_id,
                        float  *out_dist,
                        float  *out_x,
                        float  *out_y)
{
    for (int retry = 0; retry < CALIB_RETRIES; retry++) {

        /* Reset chip trước mỗi lần đo */
        dwt_write32bitreg(SYS_STATUS_ID, 0xFFFFFFFF);
        dwt_rxreset();
        vTaskDelay(pdMS_TO_TICKS(5));

        /* Cập nhật seq# và ID đích */
        calib_poll_msg[2]  = ++calib_seq;
        calib_poll_msg[10] = target_id;

        /* Gửi poll, chờ phản hồi */
        dwt_writetxdata(sizeof(calib_poll_msg), calib_poll_msg, 0);
        dwt_writetxfctrl(sizeof(calib_poll_msg), 0, 1);

        /* [FIX] setrxtimeout phải gọi TRƯỚC starttx */
        dwt_setrxtimeout(CALIB_RX_TIMEOUT);

        if (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_SUCCESS)
            continue;

        /* Chờ sự kiện */
        while (!((dwt_read32bitreg(SYS_STATUS_ID)) &
                 (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

        if (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_RXFCG))
            continue; /* timeout hoặc lỗi → thử lại */

        uint32_t flen = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
        if (flen > sizeof(calib_rx_buf)) continue;
        dwt_readrxdata(calib_rx_buf, flen, 0);

        /* Kiểm tra: function code 0xE1 (response) và Anchor ID khớp
           (byte 26 = Anchor ID nhúng sẵn bởi ss_resp_main.c)          */
        if (calib_rx_buf[9] != 0xE1 || calib_rx_buf[26] != target_id)
            continue;

        /* === Đọc timestamp 40-bit – tránh wrap-around sau ~17s === */
        uint8_t  tx_ts[5], rx_ts[5];
        uint64_t t_tx = 0, t_rx = 0;

        dwt_readtxtimestamp(tx_ts);
        dwt_readrxtimestamp(rx_ts);

        for (int b = 4; b >= 0; b--) {
            t_tx = (t_tx << 8) | tx_ts[b];
            t_rx = (t_rx << 8) | rx_ts[b];
        }

        uint32_t t_round_t = (uint32_t)(t_tx & 0xFFFFFFFFUL);
        uint32_t t_round_r = (uint32_t)(t_rx & 0xFFFFFFFFUL);
        /* T1, T2 do anchor đích nhúng vào byte 10-17 */
        uint32_t t_reply_r = *(uint32_t*)&calib_rx_buf[10];
        uint32_t t_reply_t = *(uint32_t*)&calib_rx_buf[14];

        int32_t rtd_init = (int32_t)(t_round_r - t_round_t);
        int32_t rtd_resp = (int32_t)(t_reply_t - t_reply_r);

        /* Bù lệch clock (SS-TWR chuẩn APS013) */
        float cor = dwt_readcarrierintegrator() *
                    (float)(FREQ_OFFSET_MULT * HERTZ_PPM_CHAN5 / 1e6);
        double tof = ((double)rtd_init - (double)rtd_resp * (1.0 - cor)) / 2.0;
        float  dist = (float)(tof * DWT_TIME_UNITS * SPEED_OF_LIGHT);

        if (dist < 0.05f || dist > 100.0f)
            continue; /* Khoảng cách không hợp lý → thử lại */

        /* Đọc tọa độ anchor đích từ byte 18-25 gói phản hồi */
        memcpy(out_x, &calib_rx_buf[18], sizeof(float));
        memcpy(out_y, &calib_rx_buf[22], sizeof(float));
        *out_dist = dist;
        return 1; /* thành công */
    }

    return 0; /* hết retry, thất bại */
}

/* ---------------------------------------------------------------
   anchor_self_calibrate()  – hàm công khai
   --------------------------------------------------------------- */
bool anchor_self_calibrate(uint8_t my_node_id, float *out_x, float *out_y)
{
    /* --- Anchor gốc: tọa độ cố định (0, 0), không cần đo --- */
    if (my_node_id == ANCHOR_REF_ID) {
        *out_x = 0.0f;
        *out_y = 0.0f;
        printf("[CALIB] A%u = REFERENCE (0.000, 0.000)\r\n", my_node_id);
        return true;
    }

    /* --- Chờ các anchor ID thấp hơn calibrate xong trước ---
       Anchor ID = ANCHOR_REF_ID + k → chờ k * CALIB_SLOT_MS
       Ví dụ: ID=3 chờ 5s, ID=4 chờ 10s, ID=5 chờ 15s          */
    uint32_t wait_ms = (uint32_t)(my_node_id - ANCHOR_REF_ID) * CALIB_SLOT_MS;
    printf("[CALIB] A%u: waiting %lums for lower-ID anchors...\r\n",
           my_node_id, (unsigned long)wait_ms);
    vTaskDelay(pdMS_TO_TICKS(wait_ms));

    /* --- Đo TWR đến từng anchor có ID thấp hơn --- */
    vec2   known_pos[8];   /* tọa độ các anchor tham chiếu */
    double known_dist[8];  /* khoảng cách đến chúng        */
    int    valid = 0;

    for (uint8_t t = ANCHOR_REF_ID; t < my_node_id; t++) {
        float dist, ax, ay;
        printf("[CALIB] A%u → A%u...\r\n", my_node_id, t);

        if (calib_do_twr(t, &dist, &ax, &ay)) {
            known_pos[valid].x = (double)ax;
            known_pos[valid].y = (double)ay;
            known_dist[valid]  = (double)dist;
            valid++;
            printf("[CALIB]   OK d=%.3fm  A%u@(%.3f,%.3f)\r\n",
                   dist, t, ax, ay);
        } else {
            printf("[CALIB]   FAIL\r\n");
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }

    /* --- Trường hợp đặc biệt: Anchor đầu tiên (ID = REF+1) ---
       Chỉ có 1 tham chiếu (A0 tại gốc) → không giải được 2D.
       Theo quy ước: đặt anchor này dọc trục X dương → (d, 0).
       Yêu cầu vật lý: A0 và A1 phải thẳng hàng (cùng 1 tường). */
    if (my_node_id == ANCHOR_REF_ID + 1u) {
        if (valid == 1) {
            *out_x = (float)known_dist[0]; /* = khoảng cách đến A0 */
            *out_y = 0.0f;
            printf("[CALIB] A%u DONE (X-axis): (%.3f, 0.000)\r\n",
                   my_node_id, *out_x);
            return true;
        }
        printf("[CALIB] A%u: ranging to A0 failed\r\n", my_node_id);
        *out_x = 0.0f; *out_y = 0.0f;
        return false;
    }

    /* --- Cần ít nhất 2 tham chiếu để giải 2D --- */
    if (valid < 2) {
        printf("[CALIB] A%u: only %d ref(s) → fallback (0,0)\r\n",
               my_node_id, valid);
        *out_x = 0.0f; *out_y = 0.0f;
        return false;
    }

    /* --- Giải trilateration 2D bằng Levenberg-Marquardt ---
       Initial guess = trung bình tọa độ các anchor tham chiếu.
       Anchor mới thường gần trung tâm nhóm anchor cũ nên
       init guess này hội tụ nhanh.                           */
    vec2 guess = {0.0, 0.0};
    for (int i = 0; i < valid; i++) {
        guess.x += known_pos[i].x;
        guess.y += known_pos[i].y;
    }
    guess.x /= valid;
    guess.y /= valid;

    int ok = tof_2d_localize(known_pos, valid, known_dist, &guess);

    if (ok > 0) {
        *out_x = (float)guess.x;
        *out_y = (float)guess.y;
        printf("[CALIB] A%u DONE: (%.3f, %.3f)\r\n",
               my_node_id, *out_x, *out_y);
        return true;
    }

    printf("[CALIB] A%u: solver failed → fallback (0,0)\r\n", my_node_id);
    *out_x = 0.0f;
    *out_y = 0.0f;
    return false;
}
