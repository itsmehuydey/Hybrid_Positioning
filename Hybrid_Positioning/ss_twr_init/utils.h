#ifndef TOF_3D_SOLVER_H
#define TOF_3D_SOLVER_H

#include <stdio.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Cấu trúc dữ liệu Vector 3D
   ============================================================ */
typedef struct {
    double x;
    double y;
    double z;
} vec3;

/* Hàm hỗ trợ tính khoảng cách giữa 2 điểm 3D */
static inline double vdist3(vec3 a, vec3 b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

/* ============================================================
   Hàm giải tọa độ 3D bằng ToF (Time of Flight) Levenberg-Marquardt
   ============================================================ */
int tof_3d_localize(const vec3 anc[], int num_anchors,
                    const double distances[],
                    vec3 *pos_est);

#ifdef __cplusplus
}
#endif

#endif /* TOF_3D_SOLVER_H */