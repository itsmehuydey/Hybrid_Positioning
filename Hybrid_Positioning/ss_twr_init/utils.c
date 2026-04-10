#include "utils.h"
#include <math.h>
#include <stdlib.h>

/* ================= CONFIG ================= */
#define DET_EPS     1e-6
#define STEP_LIMIT  0.15
#define MAX_POS     100.0

int tof_3d_localize(const vec3 anc[], int num_anchors,
                    const double distances[],
                    vec3 *pos_est)
{
    // Cần ít nhất 4 Anchor để giải hệ 3D chính xác
    if (num_anchors < 4 || !anc || !distances || !pos_est)
        return -1;

    /* ===== Reset nếu lỗi ===== */
    if (isnan(pos_est->x) || isnan(pos_est->y) || isnan(pos_est->z) ||
        isinf(pos_est->x) || isinf(pos_est->y) || isinf(pos_est->z) ||
        fabs(pos_est->x) > MAX_POS || fabs(pos_est->y) > MAX_POS || fabs(pos_est->z) > MAX_POS)
    {
        pos_est->x = 0.0;
        pos_est->y = 0.0;
        pos_est->z = 0.0;
    }

    double curr_x, curr_y, curr_z;

    /* ===== Initial guess: anchor gần nhất ===== */
    int min_i = 0;
    for (int i = 1; i < num_anchors; i++) {
        if (distances[i] < distances[min_i])
            min_i = i;
    }
    curr_x = anc[min_i].x;
    curr_y = anc[min_i].y;
    curr_z = anc[min_i].z;

    double lambda = 0.1;
    const int max_it = 50;

    for (int iter = 0; iter < max_it; iter++) {

        double jtj[3][3] = {{0}};
        double jtf[3] = {0};
        double curr_cost = 0.0;

        /* ===== Build system 3x3 ===== */
        for (int i = 0; i < num_anchors; i++) {

            double dx = curr_x - anc[i].x;
            double dy = curr_y - anc[i].y;
            double dz = curr_z - anc[i].z;

            double r = sqrt(dx*dx + dy*dy + dz*dz);
            if (r < 1e-6) r = 1e-6;

            double res = r - distances[i];
            curr_cost += res * res;

            double jx = dx / r;
            double jy = dy / r;
            double jz = dz / r;

            jtj[0][0] += jx * jx; jtj[0][1] += jx * jy; jtj[0][2] += jx * jz;
            jtj[1][0] += jy * jx; jtj[1][1] += jy * jy; jtj[1][2] += jy * jz;
            jtj[2][0] += jz * jx; jtj[2][1] += jz * jy; jtj[2][2] += jz * jz;

            jtf[0] += jx * res;
            jtf[1] += jy * res;
            jtf[2] += jz * res;
        }

        /* ===== LM damping ===== */
        jtj[0][0] += lambda;
        jtj[1][1] += lambda;
        jtj[2][2] += lambda;

        /* ===== Determinant 3x3 ===== */
        double det = jtj[0][0]*(jtj[1][1]*jtj[2][2] - jtj[1][2]*jtj[2][1])
                   - jtj[0][1]*(jtj[1][0]*jtj[2][2] - jtj[1][2]*jtj[2][0])
                   + jtj[0][2]*(jtj[1][0]*jtj[2][1] - jtj[1][1]*jtj[2][0]);

        if (fabs(det) < DET_EPS) {
            lambda *= 5.0;
            continue;
        }

        /* ===== Inverse 3x3 & Solve ===== */
        double inv[3][3];
        inv[0][0] =  (jtj[1][1]*jtj[2][2] - jtj[1][2]*jtj[2][1]) / det;
        inv[0][1] = -(jtj[0][1]*jtj[2][2] - jtj[0][2]*jtj[2][1]) / det;
        inv[0][2] =  (jtj[0][1]*jtj[1][2] - jtj[0][2]*jtj[1][1]) / det;

        inv[1][0] = -(jtj[1][0]*jtj[2][2] - jtj[1][2]*jtj[2][0]) / det;
        inv[1][1] =  (jtj[0][0]*jtj[2][2] - jtj[0][2]*jtj[2][0]) / det;
        inv[1][2] = -(jtj[0][0]*jtj[1][2] - jtj[0][2]*jtj[1][0]) / det;

        inv[2][0] =  (jtj[1][0]*jtj[2][1] - jtj[1][1]*jtj[2][0]) / det;
        inv[2][1] = -(jtj[0][0]*jtj[2][1] - jtj[0][1]*jtj[2][0]) / det;
        inv[2][2] =  (jtj[0][0]*jtj[1][1] - jtj[0][1]*jtj[1][0]) / det;

        double dx_step = -(inv[0][0]*jtf[0] + inv[0][1]*jtf[1] + inv[0][2]*jtf[2]);
        double dy_step = -(inv[1][0]*jtf[0] + inv[1][1]*jtf[1] + inv[1][2]*jtf[2]);
        double dz_step = -(inv[2][0]*jtf[0] + inv[2][1]*jtf[1] + inv[2][2]*jtf[2]);

        /* ===== Clamp step ===== */
        if (dx_step > STEP_LIMIT) dx_step = STEP_LIMIT;
        if (dx_step < -STEP_LIMIT) dx_step = -STEP_LIMIT;
        if (dy_step > STEP_LIMIT) dy_step = STEP_LIMIT;
        if (dy_step < -STEP_LIMIT) dy_step = -STEP_LIMIT;
        if (dz_step > STEP_LIMIT) dz_step = STEP_LIMIT;
        if (dz_step < -STEP_LIMIT) dz_step = -STEP_LIMIT;

        double next_x = curr_x + dx_step;
        double next_y = curr_y + dy_step;
        double next_z = curr_z + dz_step;

        /* ===== Guard ===== */
        if (isnan(next_x) || isnan(next_y) || isnan(next_z) ||
            fabs(next_x) > MAX_POS || fabs(next_y) > MAX_POS || fabs(next_z) > MAX_POS)
        {
            lambda *= 5.0;
            continue;
        }

        /* ===== Compute next cost ===== */
        double next_cost = 0.0;
        for (int i = 0; i < num_anchors; i++) {
            double tx = next_x - anc[i].x;
            double ty = next_y - anc[i].y;
            double tz = next_z - anc[i].z;
            double tr = sqrt(tx*tx + ty*ty + tz*tz);
            double res = tr - distances[i];
            next_cost += res * res;
        }

        /* ===== Accept / Reject ===== */
        if (next_cost < curr_cost) {
            curr_x = next_x;
            curr_y = next_y;
            curr_z = next_z;

            lambda *= 0.7;
            if (lambda < 1e-5) lambda = 1e-5;

            if (sqrt(dx_step*dx_step + dy_step*dy_step + dz_step*dz_step) < 1e-4)
                break;
        } else {
            lambda *= 2.0;
            if (lambda > 1e6)
                break;
        }
    }

    pos_est->x = curr_x;
    pos_est->y = curr_y;
    pos_est->z = curr_z;

    return 1;
}


int calculate_anchor_geometry(double d01, double d02, double d03,
                              double d12, double d13, double d23,
                              vec3 *a1, vec3 *a2, vec3 *a3)
{
    if (d01 <= 0 || d02 <= 0 || d03 <= 0 || d12 <= 0 || d13 <= 0 || d23 <= 0) return 0;

    // 1. Tính A1 (Nằm trên trục X dương)
    a1->x = d01;
    a1->y = 0.0;
    a1->z = 0.0;

    // 2. Tính A2 (Nằm trên mặt phẳng XY)
    // Áp dụng định lý hàm số Cosin cho tam giác A0-A1-A2
    a2->x = (d02 * d02 + d01 * d01 - d12 * d12) / (2.0 * d01);
    
    double y2_sq = d02 * d02 - (a2->x * a2->x);
    if (y2_sq < 0) y2_sq = 0.0; // Tránh lỗi số học do nhiễu UWB
    a2->y = sqrt(y2_sq);
    a2->z = 0.0;

    // 3. Tính A3 (Không gian 3D)
    a3->x = (d03 * d03 + d01 * d01 - d13 * d13) / (2.0 * d01);

    if (a2->y < 1e-6) return 0; // Tránh chia cho 0 nếu A0, A1, A2 thẳng hàng

    a3->y = (d02 * d02 + d03 * d03 - d23 * d23 - 2.0 * a3->x * a2->x) / (2.0 * a2->y);

    double z3_sq = d03 * d03 - (a3->x * a3->x) - (a3->y * a3->y);
    if (z3_sq < 0) z3_sq = 0.0;
    a3->z = sqrt(z3_sq); // Lấy phần dương, giả định A3 được treo trên cao so với mặt phẳng

    return 1;
}