#ifndef TOF_2D_SOLVER_H
#define TOF_2D_SOLVER_H

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Cấu trúc dữ liệu Vector 2D
   ============================================================ */
typedef struct {
    double x;
    double y;
} vec2;

/* Hàm hỗ trợ tính khoảng cách giữa 2 điểm 2D */
static inline double vdist2(vec2 a, vec2 b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

/* ============================================================
   Hàm giải tọa độ 2D bằng ToF (Time of Flight) Levenberg-Marquardt
   ============================================================ */
int tof_2d_localize(const vec2 anc[], int num_anchors,
                    const double distances[],
                    vec2 *pos_est);

int calculate_anchor_geometry(double d01, double d02, double d03,
                              double d12, double d13, double d23,
                              vec2 *a1, vec2 *a2, vec2 *a3);

/* ============================================================
   Bộ lọc 2D Exponential Moving Average (EMA) cho Tag
   ============================================================ */
typedef struct {
    double x;
    double y;
    double alpha;    // Trọng số làm mượt (0.0 < alpha <= 1.0)
    int initialized; // Cờ đánh dấu đã khởi tạo điểm đầu tiên
} ema_2d_t;

void ema_2d_init(ema_2d_t *filter, double alpha);
vec2 ema_2d_update(ema_2d_t *filter, vec2 raw_meas);

#ifdef __cplusplus
}
#endif

#endif /* TOF_2D_SOLVER_H */