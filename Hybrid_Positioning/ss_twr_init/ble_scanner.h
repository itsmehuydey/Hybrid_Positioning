#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <stdint.h>

/**
 * @brief Khởi tạo BLE scanner (RAW NRF_RADIO)
 */
void ble_scanner_init(void);

/**
 * @brief Quét BLE và trả về 1 nếu tìm thấy Manufacturer Data 0x0059
 * @param new_role: byte payload gửi từ beacon
 */
int ble_scan_for_role(uint8_t *new_role);
int ble_scan_packet(uint8_t *out, uint16_t *out_len);
#endif
