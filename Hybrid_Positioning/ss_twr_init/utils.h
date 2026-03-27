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

#ifdef __cplusplus
}
#endif

#endif /* TOF_2D_SOLVER_H */