#include "utils.h"
#include <math.h>
#include <stdlib.h>

/* ================= CONFIG ================= */
#define DET_EPS     1e-6
#define STEP_LIMIT  0.3
#define MAX_POS     100.0

int tof_2d_localize(const vec2 anc[], int num_anchors,
                    const double distances[],
                    vec2 *pos_est)
{
    if (num_anchors < 3 || !anc || !distances || !pos_est)
        return -1;

    /* ===== Reset nếu lỗi ===== */
    if (isnan(pos_est->x) || isnan(pos_est->y) ||
        isinf(pos_est->x) || isinf(pos_est->y) ||
        fabs(pos_est->x) > MAX_POS || fabs(pos_est->y) > MAX_POS)
    {
        pos_est->x = 0.0;
        pos_est->y = 0.0;
    }

    double curr_x, curr_y;

    /* ===== Initial guess: anchor gần nhất ===== */
    int min_i = 0;
    for (int i = 1; i < num_anchors; i++) {
        if (distances[i] < distances[min_i])
            min_i = i;
    }
    curr_x = anc[min_i].x;
    curr_y = anc[min_i].y;

    double lambda = 0.1;
    const int max_it = 50;

    for (int iter = 0; iter < max_it; iter++) {

        double jtj[2][2] = {{0}};
        double jtf[2] = {0};
        double curr_cost = 0.0;

        /* ===== Build system ===== */
        for (int i = 0; i < num_anchors; i++) {

            double dx = curr_x - anc[i].x;
            double dy = curr_y - anc[i].y;

            double r = sqrt(dx*dx + dy*dy);
            if (r < 1e-6) r = 1e-6;

            double res = r - distances[i];
            curr_cost += res * res;

            double jx = dx / r;
            double jy = dy / r;

            jtj[0][0] += jx * jx;
            jtj[0][1] += jx * jy;
            jtj[1][0] += jy * jx;
            jtj[1][1] += jy * jy;

            jtf[0] += jx * res;
            jtf[1] += jy * res;
        }

        /* ===== LM damping ===== */
        jtj[0][0] += lambda;
        jtj[1][1] += lambda;

        double det = jtj[0][0]*jtj[1][1] - jtj[0][1]*jtj[1][0];

        if (fabs(det) < DET_EPS) {
            lambda *= 5.0;
            continue;
        }

        double dx = (-jtf[0]*jtj[1][1] + jtf[1]*jtj[0][1]) / det;
        double dy = (-jtf[1]*jtj[0][0] + jtf[0]*jtj[1][0]) / det;

        /* ===== Clamp step ===== */
        if (dx > STEP_LIMIT) dx = STEP_LIMIT;
        if (dx < -STEP_LIMIT) dx = -STEP_LIMIT;
        if (dy > STEP_LIMIT) dy = STEP_LIMIT;
        if (dy < -STEP_LIMIT) dy = -STEP_LIMIT;

        double next_x = curr_x + dx;
        double next_y = curr_y + dy;

        /* ===== Guard ===== */
        if (isnan(next_x) || isnan(next_y) ||
            fabs(next_x) > MAX_POS || fabs(next_y) > MAX_POS)
        {
            lambda *= 5.0;
            continue;
        }

        /* ===== Compute next cost ===== */
        double next_cost = 0.0;
        for (int i = 0; i < num_anchors; i++) {
            double tx = next_x - anc[i].x;
            double ty = next_y - anc[i].y;
            double tr = sqrt(tx*tx + ty*ty);
            double res = tr - distances[i];
            next_cost += res * res;
        }

        /* ===== Accept / Reject ===== */
        if (next_cost < curr_cost) {
            curr_x = next_x;
            curr_y = next_y;

            lambda *= 0.7;
            if (lambda < 1e-5) lambda = 1e-5;

            if (sqrt(dx*dx + dy*dy) < 1e-4)
                break;
        } else {
            lambda *= 2.0;
            if (lambda > 1e6)
                break;
        }
    }

    pos_est->x = curr_x;
    pos_est->y = curr_y;

    return 1;
}