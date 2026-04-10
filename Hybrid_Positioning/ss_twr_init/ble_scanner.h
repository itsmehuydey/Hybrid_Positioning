#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <stdint.h>

// Gói tin cực ngắn: Tổng cộng 5 byte (Hoàn hảo cho BLE Beacon)
#pragma pack(push, 1)
typedef struct {
    uint8_t  magic_byte;  // Bắt buộc là chữ 'C' (0x43) để biết đây là lệnh cấu hình
    uint16_t target_mac;  // 2 byte cuối của MAC mạch cần đổi (hoặc 0xFFFF để đổi tất cả)
    uint8_t  role;        // 1 = TAG, 2 = ANCHOR
    uint8_t  node_id;     // ID muốn gán cho mạch (0, 1, 2,...)
} web_config_t;
#pragma pack(pop)

/**
 * @brief Khởi tạo BLE scanner (RAW NRF_RADIO)
 */
void ble_scanner_init(void);

/**
 * @brief Quét và đợi lệnh cấu hình từ Web
 * @param out_config: Con trỏ lưu dữ liệu cấu hình trả về
 * @return 1 nếu nhận đúng cấu hình của mình, 0 nếu không có
 */
int ble_scan_for_config(web_config_t *out_config);

/**
 * @brief Hàm quét gói tin thông thường (Giữ nguyên gốc)
 */
int ble_scan_packet(uint8_t *out, uint16_t *out_len);
int ble_scan_for_geometry(uint8_t my_id, float *out_x, float *out_y, float *out_z);

#endif