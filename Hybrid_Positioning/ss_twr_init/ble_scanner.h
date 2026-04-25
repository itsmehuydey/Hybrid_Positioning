#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <stdint.h>

// Gói lệnh cũ (magic 'C'): Chỉ có role + id, không có tọa độ
#pragma pack(push, 1)
typedef struct {
    uint8_t  magic_byte;  // Bắt buộc là chữ 'C' (0x43)
    uint16_t target_mac;  // 2 byte cuối của MAC mạch cần đổi (hoặc 0xFFFF để đổi tất cả)
    uint8_t  role;        // 1 = TAG, 2 = ANCHOR
    uint8_t  node_id;     // ID muốn gán cho mạch (0, 1, 2,...)
} web_config_t;
#pragma pack(pop)

// Gói lệnh mới (magic 'A'): Có tọa độ x, y thực tế của anchor/tag
// Tổng 13 byte: 'A'(1) + mac(2) + role(1) + id(1) + x(4) + y(4)
#pragma pack(push, 1)
typedef struct {
    uint8_t  magic_byte;  // Bắt buộc là chữ 'A' (0x41)
    uint16_t target_mac;  // 2 byte cuối MAC (hoặc 0xFFFF cho tất cả)
    uint8_t  role;        // 1 = TAG, 2 = ANCHOR
    uint8_t  node_id;     // ID
    float    x;           // Tọa độ X thực tế (mét)
    float    y;           // Tọa độ Y thực tế (mét)
} anchor_config_t;
#pragma pack(pop)

/**
 * @brief Khởi tạo BLE scanner (RAW NRF_RADIO)
 */
void ble_scanner_init(void);

/**
 * @brief Quét gói lệnh cũ 'C' (role + id, không có tọa độ)
 * @return 1 nếu nhận thành công
 */
int ble_scan_for_config(web_config_t *out_config);

/**
 * @brief Quét gói lệnh mới 'A' (role + id + tọa độ x/y)
 * @return 1 nếu nhận thành công
 */
int ble_scan_for_anchor_config(anchor_config_t *out_config);

/**
 * @brief Hàm quét gói tin thông thường
 */
int ble_scan_packet(uint8_t *out, uint16_t *out_len);
int ble_scan_for_geometry(uint8_t my_id, float *out_x, float *out_y);

#endif