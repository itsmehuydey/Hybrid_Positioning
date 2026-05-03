#include "gmc_kalman.h"
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ================= TUNING ================= */
#define JITTER_DEADBAND 0.05
#define STILL_VEL_CLAMP 0.02
#define MIN_WEIGHT 1e-8
#define MIN_LIKELIHOOD 1e-6

/* =========================================================
 *                   KALMAN 4D CORE
 * ========================================================= */

void gmc_kalman4d_init(GMC_Kalman4D_t *kf, double q, double r, double alpha, double beta)
{
    kf->q = q;
    kf->r = r;
    kf->alpha = alpha;
    kf->beta = beta;

    memset(kf->x, 0, sizeof(kf->x));
    memset(kf->P, 0, sizeof(kf->P));

    kf->P[0][0] = 1.0;
    kf->P[1][1] = 1.0;
    kf->P[2][2] = 0.5;
    kf->P[3][3] = 0.5;
}

void gmc_kalman4d_reset(GMC_Kalman4D_t *kf, double x, double y)
{
    memset(kf->x, 0, sizeof(kf->x));
    memset(kf->P, 0, sizeof(kf->P));

    kf->x[0] = x;
    kf->x[1] = y;

    kf->P[0][0] = 0.5;
    kf->P[1][1] = 0.5;
    kf->P[2][2] = 0.4;
    kf->P[3][3] = 0.4;
}

/* =========================================================
 *                   PREDICT
 * ========================================================= */
void gmc_kalman4d_predict(GMC_Kalman4D_t *kf, double dt)
{
    kf->x[0] += kf->x[2] * dt;
    kf->x[1] += kf->x[3] * dt;

    double q = kf->q;
    double dt2 = dt * dt;

    kf->P[0][0] += dt2 * kf->P[2][2] + 0.05 * q;
    kf->P[1][1] += dt2 * kf->P[3][3] + 0.05 * q;

    kf->P[2][2] += q * dt;
    kf->P[3][3] += q * dt;

    /* covariance symmetry fix */
    kf->P[0][2] = kf->P[2][0] = 0.98 * kf->P[0][2];
    kf->P[1][3] = kf->P[3][1] = 0.98 * kf->P[1][3];
}

/* =========================================================
 *                   GMC UPDATE (FIXED)
 * ========================================================= */
vec2 gmc_kalman4d_update(GMC_Kalman4D_t *kf, vec2 z)
{
    double dx = z.x - kf->x[0];
    double dy = z.y - kf->x[1];
    double e = sqrt(dx*dx + dy*dy);

    /* GMC kernel */
    double norm = e / (kf->beta + 1e-9);
    double w = exp(-pow(norm, kf->alpha));

    if (w < MIN_WEIGHT) w = MIN_WEIGHT;

    /* IMPORTANT FIX:
       clamp r_eff để tránh explosion covariance */
    double r_eff = kf->r / (w + 1e-6);

    double Kx = kf->P[0][0] / (kf->P[0][0] + r_eff);
    double Ky = kf->P[1][1] / (kf->P[1][1] + r_eff);

    /* jitter suppression */
    double v = sqrt(kf->x[2]*kf->x[2] + kf->x[3]*kf->x[3]);
    if (e < JITTER_DEADBAND && v < STILL_VEL_CLAMP) {
        Kx *= 0.05;
        Ky *= 0.05;
    }

    double x_old = kf->x[0];
    double y_old = kf->x[1];

    /* update state */
    kf->x[0] += Kx * dx;
    kf->x[1] += Ky * dy;

    /* covariance update (FIXED STABILITY FORM) */
    kf->P[0][0] *= (1.0 - Kx);
    kf->P[1][1] *= (1.0 - Ky);

    /* velocity update */
    double vx = kf->x[0] - x_old;
    double vy = kf->x[1] - y_old;

    double a = (w < 0.3) ? 0.9 : 0.75;

    kf->x[2] = a * kf->x[2] + (1 - a) * vx;
    kf->x[3] = a * kf->x[3] + (1 - a) * vy;

    if (fabs(kf->x[2]) < STILL_VEL_CLAMP) kf->x[2] = 0;
    if (fabs(kf->x[3]) < STILL_VEL_CLAMP) kf->x[3] = 0;

    return (vec2){kf->x[0], kf->x[1]};
}

/* =========================================================
 *                   IMM INIT (FIXED)
 * ========================================================= */
void imm_gmc_init(IMM_GMC_Kalman4D_t *imm, double q, double r, double alpha, double beta)
{
    imm->q = q;
    imm->r = r;
    imm->alpha = alpha;
    imm->beta = beta;

    /* FIX: reduce mode divergence */
    gmc_kalman4d_init(&imm->models[0], q * 0.2, r, alpha, beta * 0.9);
    gmc_kalman4d_init(&imm->models[1], q * 2.0, r, alpha, beta * 1.2);

    imm->mode_prob[0] = 0.7;
    imm->mode_prob[1] = 0.3;

    imm->mode_trans[0][0] = 0.95;
    imm->mode_trans[0][1] = 0.05;
    imm->mode_trans[1][0] = 0.05;
    imm->mode_trans[1][1] = 0.95;
}

void imm_gmc_reset(IMM_GMC_Kalman4D_t *imm, double x, double y)
{
    for (int i = 0; i < 2; i++)
        gmc_kalman4d_reset(&imm->models[i], x, y);

    imm->mode_prob[0] = 0.7;
    imm->mode_prob[1] = 0.3;
}

/* =========================================================
 *                   IMM PREDICT (FIXED)
 * ========================================================= */
void imm_gmc_predict(IMM_GMC_Kalman4D_t *imm, double dt)
{
    for (int i = 0; i < 2; i++)
        gmc_kalman4d_predict(&imm->models[i], dt);
}

/* =========================================================
 *                   IMM UPDATE (FIXED)
 * ========================================================= */
vec2 imm_gmc_update(IMM_GMC_Kalman4D_t *imm, vec2 z)
{
    vec2 est[2];
    double lk[2];

    double sum_lk = 0;

    for (int i = 0; i < 2; i++) {
        double dx = z.x - imm->models[i].x[0];
        double dy = z.y - imm->models[i].x[1];
        double e = sqrt(dx*dx + dy*dy);

        double w = exp(-pow(e / (imm->models[i].beta + 1e-9),
                            imm->models[i].alpha));

        lk[i] = w + MIN_LIKELIHOOD;
        sum_lk += lk[i];

        est[i] = gmc_kalman4d_update(&imm->models[i], z);
    }

    /* normalize likelihood */
    lk[0] /= sum_lk;
    lk[1] /= sum_lk;

    /* mode update (stable bayes) */
    double p0 = imm->mode_prob[0] * lk[0];
    double p1 = imm->mode_prob[1] * lk[1];

    double s = p0 + p1 + 1e-12;

    imm->mode_prob[0] = p0 / s;
    imm->mode_prob[1] = p1 / s;

    /* output fusion */
    return (vec2){
        imm->mode_prob[0] * est[0].x + imm->mode_prob[1] * est[1].x,
        imm->mode_prob[0] * est[0].y + imm->mode_prob[1] * est[1].y
    };
}