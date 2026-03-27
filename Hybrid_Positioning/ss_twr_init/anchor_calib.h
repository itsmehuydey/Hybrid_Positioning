#ifndef ANCHOR_CALIB_H
#define ANCHOR_CALIB_H

#include <stdint.h>
#include <stdbool.h>

/* =============================================================
   QUY ƯỚC NODE_ID trong toàn bộ dự án:
     NODE_ID = 1       → Tag (Initiator)
     NODE_ID = 2       → Anchor gốc (Reference), cố định tại (0, 0)
     NODE_ID = 3,4,5…  → Anchor slave, tự tính tọa độ khi khởi động

   CÁCH self-calibration hoạt động:
     - Anchor gốc (ID=2): ngay lập tức trả về (0.0, 0.0)
     - Anchor 1   (ID=3): chờ 1 slot (5s), đo TWR đến ID=2
                          → đặt tại (d02, 0.0) theo trục X (quy ước)
     - Anchor 2   (ID=4): chờ 2 slot (10s), đo TWR đến ID=2 và ID=3
                          → giải trilateration 2D
     - Anchor 3   (ID=5): chờ 3 slot (15s), đo TWR đến ID=2,3,4
                          → giải trilateration 2D
     Sau khi xong, mỗi anchor nhúng (pos_x, pos_y, id) vào gói
     phản hồi UWB → Tag tự đọc được, không cần hardcode.
   ============================================================= */

#define ANCHOR_REF_ID   2u      /* NODE_ID của anchor gốc (0,0)     */
#define CALIB_SLOT_MS   5000u   /* Thời gian chờ mỗi slot (ms)      */

/**
 * @brief  Tự động hiệu chỉnh vị trí anchor.
 *
 * @param  my_node_id  NODE_ID của anchor đang chạy hàm này
 * @param  out_x       [out] Tọa độ X sau khi hiệu chỉnh (m)
 * @param  out_y       [out] Tọa độ Y sau khi hiệu chỉnh (m)
 * @return true  nếu thành công
 * @return false nếu không đủ tham chiếu (anchor giữ (0,0) làm fallback)
 */
bool anchor_self_calibrate(uint8_t my_node_id, float *out_x, float *out_y);

#endif /* ANCHOR_CALIB_H */
