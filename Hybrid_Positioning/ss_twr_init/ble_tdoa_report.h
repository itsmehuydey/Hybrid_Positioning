#ifndef BLE_TDOA_REPORT_H
#define BLE_TDOA_REPORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * BLE packet gửi từ Anchor → Master
 * msg_type = 'S'
 * Chứa: anchor_id, cycle_id, Syn_i^k, Syn_i^{k+1}, T_i
 */
#pragma pack(push, 1)
typedef struct
{
    uint8_t  msg_type;   /* 'S' = Sync+TDOA report */
    uint8_t  anchor_id;
    uint16_t cycle_id;
    uint64_t syn_k;      /* Syn_i^k  : timestamp nhận BLINK(k)   */
    uint64_t syn_k1;     /* Syn_i^{k+1}: timestamp nhận BLINK(k+1) */
    uint64_t T_i;        /* timestamp khi nhận tín hiệu tag       */
} ble_tdoa_report_t;     /* 28 bytes tổng */
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* BLE_TDOA_REPORT_H */