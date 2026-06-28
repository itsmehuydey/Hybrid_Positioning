#ifndef BLE_RAW_BEACON_H
#define BLE_RAW_BEACON_H

#include <stdint.h>

/**
 * @brief Khởi tạo gói BLE ADV với tên DEV_x
 * @param dev_id 0–255 (sẽ ra tên DEV_<dev_id>)
 */
void ble_raw_beacon_init(uint8_t dev_id);

/**
 * @brief Phát 1 lần trên 3 kênh quảng bá (37, 38, 39).
 * Gọi lại hàm này trong task (ví dụ mỗi 100 ms).
 */
void ble_raw_beacon_broadcast(void);

void ble_raw_beacon_send_payload(const uint8_t *data, uint8_t len);

/* Hàm cũ: Gửi tọa độ Anchor */
void ble_beacon_send_geometry(uint8_t target_id, float x, float y);

/* =========================================================================
   HÀM MỚI: Gửi tọa độ Tag đã lọc kèm Timestamp cho Gateway (15 bytes)
   ========================================================================= */
void ble_beacon_send_tag_pos(uint8_t id, uint8_t seq, uint32_t timestamp, float x, float y);

// === THÊM MỚI ===
/* Hàm báo sự hiện diện của Anchor (ID, Tọa độ) */
void ble_beacon_send_presence(uint8_t id, float x, float y);
// ================

#endif