#ifndef GMC_KALMAN_H
#define GMC_KALMAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

/* ================= KALMAN 4D STATE =================
   State:
   x[0] = x position
   x[1] = y position
   x[2] = vx
   x[3] = vy
*/
typedef struct {
    double x[4];
    double P[4][4];

    double q;        // Process noise
    double r;        // Measurement noise (base)
    double alpha;    // Tham số hình dạng (Shape parameter - α) trong GMC GGKF
    double beta;     // Tham số tỷ lệ (Scale parameter - β) trong GMC GGKF
} GMC_Kalman4D_t;

/* ================= IMM CONFIG ================= */
#define IMM_MODE_COUNT 2

typedef struct {
    GMC_Kalman4D_t models[IMM_MODE_COUNT];

    double mode_prob[IMM_MODE_COUNT];
    double mode_trans[IMM_MODE_COUNT][IMM_MODE_COUNT];

    double q;
    double r;
    double alpha;
    double beta;
} IMM_GMC_Kalman4D_t;

/* ================= KALMAN API ================= */
void gmc_kalman4d_init(GMC_Kalman4D_t *kf, double process_var, double meas_var, double alpha, double beta);
void gmc_kalman4d_reset(GMC_Kalman4D_t *kf, double x, double y);
void gmc_kalman4d_predict(GMC_Kalman4D_t *kf, double dt);
vec2 gmc_kalman4d_update(GMC_Kalman4D_t *kf, vec2 meas);

/* ================= IMM API ================= */
void imm_gmc_init(IMM_GMC_Kalman4D_t *imm, double q, double r, double alpha, double beta);
void imm_gmc_reset(IMM_GMC_Kalman4D_t *imm, double x, double y);
void imm_gmc_predict(IMM_GMC_Kalman4D_t *imm, double dt);
vec2 imm_gmc_update(IMM_GMC_Kalman4D_t *imm, vec2 meas);

#ifdef __cplusplus
}
#endif

#endif /* GMC_KALMAN_H */