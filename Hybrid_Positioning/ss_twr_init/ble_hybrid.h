#ifndef MASTER_HYBRID_H
#define MASTER_HYBRID_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =========================
// CẤU HÌNH HỆ THỐNG
// =========================

// Số anchor tối đa trong hệ thống
#define MH_MAX_ANCHORS    4

// Số cycle tối đa lưu FIFO
#define MH_MAX_CYCLE_BUF  16

// ID của TAG – cấu hình khi nạp firmware
#ifndef TAG_ID
#define TAG_ID  1
#endif

// Danh sách Anchor ID thực tế (mapping index -> ID)
#ifndef ANCHOR_ID_LIST
#define ANCHOR_ID_LIST {0, 1, 2, 3}
#endif


// ================= BLE PACKET TỪ TAG → MASTER =================
// TAG gửi TOF qua BLE với format này
#pragma pack(push, 1)
typedef struct
{
    uint8_t  msg_type;   // 'T' = TOF packet
    uint8_t  anchor_id;  // ID anchor tương ứng khoảng cách
    uint16_t cycle_id;   // ID chu kỳ đo
    float    tof;        // ToF (giây)
    float    distance;   // Khoảng cách (m)

} mh_ble_tof_packet_t;


// ================= HYBRID → GATEWAY =================
// MASTER gửi lên Gateway sau khi gom đủ TOF + TDOA

typedef struct
{
    uint8_t  msg_type;          // 'H'
    uint16_t cycle_id;          // Chu kỳ tương ứng

    uint8_t  tag_id;            // ID của TAG
    uint8_t  anchor_ids[MH_MAX_ANCHORS]; // Danh sách ID anchor thực tế

    uint8_t  tdoa_mask;         // bitmask anchor có TDOA
    uint8_t  tof_mask;          // bitmask anchor có TOF

    uint64_t tdoa_ts[MH_MAX_ANCHORS]; // timestamp từ anchors
    float    dist[MH_MAX_ANCHORS];    // khoảng cách (m)
    float    tof[MH_MAX_ANCHORS];     // ToF (s)

} mh_hybrid_packet_t;

#pragma pack(pop)


// ================== API CHO MASTER ==================

// Khởi tạo / reset FIFO
void master_hybrid_init(void);
void master_hybrid_reset(void);

// RX BLE từ TAG (TOF)
void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len);

// RX UWB TDOA từ ANCHOR (ví dụ từ callback UWB)
void master_hybrid_handle_uwb_tdoa(uint8_t anchor_id,
                                   uint16_t cycle_id,
                                   uint64_t timestamp);

// Hàm bạn tự implement bên ngoài để gửi lên Gateway
void master_gateway_send(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // MASTER_HYBRID_H
