#ifndef UWB_CALIB_H
#define UWB_CALIB_H

#include <stdint.h>
#include <stdbool.h>

void uwb_calib_init(void);
float measure_inter_anchor_tof(uint8_t target_id);
void serve_as_inter_anchor_responder(uint32_t timeout_ms, uint8_t my_id);
void send_ble_distance(uint8_t src, uint8_t dst, float dist);
uint64_t get_rx_timestamp_u64(void);
void resp_msg_set_ts(uint8_t *ts_field, const uint64_t ts);

#endif