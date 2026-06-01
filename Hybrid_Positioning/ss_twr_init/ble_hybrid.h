#ifndef MASTER_HYBRID_H
#define MASTER_HYBRID_H

#include <stdint.h>
#include <stdbool.h>
#include "ble_tdoa_report.h"   /* ble_tdoa_report_t */

#ifdef __cplusplus
extern "C" {
#endif

/* =========================
   CẤU HÌNH HỆ THỐNG
   ========================= */

#define MH_MAX_ANCHORS    4
#define MH_MAX_CYCLE_BUF  16

#ifndef TAG_ID
#define TAG_ID  1
#endif

#ifndef ANCHOR_ID_LIST
#define ANCHOR_ID_LIST {0, 1, 2, 3}
#endif


/* ================= BLE PACKET TỪ TAG → MASTER =================
   TAG gửi TOF qua BLE với format này (GIỮ NGUYÊN)              */
#pragma pack(push, 1)
typedef struct
{
    uint8_t  msg_type;   /* 'T' = TOF packet */
    uint8_t  anchor_id;
    uint16_t cycle_id;
    float    distance;   /* khoảng cách (m) */
} mh_ble_tof_packet_t;


/* ================= HYBRID → GATEWAY =================
   MASTER gửi lên Gateway sau khi gom đủ dữ liệu       */
typedef struct
{
    uint8_t  msg_type;                    /* 'H' */
    uint16_t cycle_id;

    uint8_t  tag_id;

    uint8_t  anchor_ids[MH_MAX_ANCHORS];  /* mapping index -> ID thực tế */
    uint8_t  ref_idx;                     /* index anchor reference       */

    uint8_t  tdoa_mask;                   /* bit i = 1 → có Δd_i         */
    uint8_t  tof_mask;                    /* bit i = 1 → có d_i (TOF)    */

    float    delta_d[MH_MAX_ANCHORS];     /* Δd_i (m) sau khi bù drift    */
    float    dist[MH_MAX_ANCHORS];        /* d_i (m) từ TOF               */
} mh_hybrid_packet_t;

#pragma pack(pop)


/* ================== API CHO MASTER ================== */

/* Khởi tạo / reset */
void master_hybrid_init(void);
void master_hybrid_reset(void);

/* RX BLE từ TAG (TOF packet 'T') – GIỮ NGUYÊN */
void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len);

/* RX BLE TDOA report từ Anchor ('S' packet) – MỚI */
void master_hybrid_handle_ble_tdoa_report(const ble_tdoa_report_t *rpt);

/* Gửi lên Gateway */
void master_gateway_send(const uint8_t *data, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* MASTER_HYBRID_H */