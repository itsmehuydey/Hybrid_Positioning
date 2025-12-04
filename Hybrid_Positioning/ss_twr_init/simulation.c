// simulation.c — phiên bản mới có so sánh TOF-ONLY
#ifdef SIMULATION_MODE
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "hybrid_scalable.h"

#ifndef N_ANCHORS
#define N_ANCHORS 4
#endif
#ifndef N_TDOA
#define N_TDOA (N_ANCHORS - 1)
#endif
#ifndef POLL_RX_TO_RESP_TX_DLY_UUS
#define POLL_RX_TO_RESP_TX_DLY_UUS 1100
#endif

/* Cấu hình mô phỏng */
#define SIM_TRUE_POS_X      4.0
#define SIM_TRUE_POS_Y      3.0

/* Mô phỏng nhiễu thực tế (25 cm) */
#define TOA_NOISE_STD       (8e-10)          // 1ns ~ 30cm
#define TOF_NOISE_STD       (0.25 / C0)      // 25cm
#define CLOCK_OFFSET_STD    (6e-10)          // ~18 cm drift TDOA

vec2   anc[N_ANCHORS];
double phi[N_ANCHORS - 1];
static double sim_time = 0.0;

/* Gaussian noise */
static double gauss_noise(double std)
{
    static int has_spare = 0;
    static double spare;
    if (has_spare) { has_spare = 0; return spare * std; }
    has_spare = 1;
    double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double mag = std * sqrt(-2.0 * log(u1));
    spare = mag * sin(2.0 * M_PI * u2);
    return mag * cos(2.0 * M_PI * u2);
}

/* Mô phỏng SS-TWR */
static int simulate_ss_twr(int anchor_id, double *toa_meas, double *tof_meas)
{
    vec2 tag_pos = {SIM_TRUE_POS_X, SIM_TRUE_POS_Y};
    double dist = vnorm(vsub(tag_pos, anc[anchor_id]));
    double tof_true = dist / C0;

    double tx_poll = 0.0;
    double rx_poll_anchor = tof_true;

    if (anchor_id > 0) {
        rx_poll_anchor += phi[anchor_id - 1];
    }

    double delay_anchor = POLL_RX_TO_RESP_TX_DLY_UUS * 1e-6;
    double tx_resp_anchor = rx_poll_anchor + delay_anchor;
    double rx_resp_tag = tx_resp_anchor + tof_true;

    *toa_meas = rx_poll_anchor + gauss_noise(TOA_NOISE_STD);

    double rtd = rx_resp_tag - tx_poll;
    double tof_calc = (rtd - delay_anchor) / 2.0;

    *tof_meas = tof_calc + gauss_noise(TOF_NOISE_STD);

    return 1;
}

/* Clock offset mô phỏng */
static void init_clock_offsets(void)
{
    for (int i = 0; i < N_ANCHORS-1; ++i) {
        phi[i] = gauss_noise(CLOCK_OFFSET_STD);
    }
}

/* ======================== 1) TOF-ONLY POSITIONING ======================== */
/* Trilateration 2D cơ bản: chỉ dùng d1..d4 */
static vec2 tof_only_localize(double d[])
{
    /*
     * Dùng công thức giải 3 đường tròn trong 2D:
     * Phương pháp linearization (Bowen, 2008)
     */

    vec2 p1 = anc[0];
    vec2 p2 = anc[1];
    vec2 p3 = anc[2];
    vec2 p4 = anc[3];

    double x1=p1.x, y1=p1.y;
    double x2=p2.x, y2=p2.y;
    double x3=p3.x, y3=p3.y;
    double x4=p4.x, y4=p4.y;

    double r1=d[0], r2=d[1], r3=d[2], r4=d[3];

    /* giải tuyến tính 3 phương trình */
    double A1 = 2*(x2-x1); double B1 = 2*(y2-y1);
    double C1 = r1*r1 - r2*r2 - x1*x1 + x2*x2 - y1*y1 + y2*y2;

    double A2 = 2*(x3-x1); double B2 = 2*(y3-y1);
    double C2 = r1*r1 - r3*r3 - x1*x1 + x3*x3 - y1*y1 + y3*y3;

    double A3 = 2*(x4-x1); double B3 = 2*(y4-y1);
    double C3 = r1*r1 - r4*r4 - x1*x1 + x4*x4 - y1*y1 + y4*y4;

    /* least squares */
    double M[3][2] = { {A1, B1}, {A2, B2}, {A3, B3} };
    double C[3]    = { C1, C2, C3 };

    double MTM[2][2] = {0};
    double MTC[2]    = {0};

    for(int i=0;i<3;i++){
        MTM[0][0] += M[i][0]*M[i][0];
        MTM[0][1] += M[i][0]*M[i][1];
        MTM[1][0] += M[i][1]*M[i][0];
        MTM[1][1] += M[i][1]*M[i][1];

        MTC[0]    += M[i][0]*C[i];
        MTC[1]    += M[i][1]*C[i];
    }

    double det = MTM[0][0]*MTM[1][1] - MTM[0][1]*MTM[1][0];
    vec2 sol = {0,0};

    if (fabs(det) > 1e-9) {
        sol.x = ( MTC[0]*MTM[1][1] - MTM[0][1]*MTC[1] ) / det;
        sol.y = ( MTM[0][0]*MTC[1] - MTC[0]*MTM[1][0] ) / det;
    }

    return sol;
}

/* ======================== MAIN SIMULATION ======================== */

void simulate_measurement_cycle(void)
{
    static int initialized = 0;
    if (!initialized) {
        anc[0] = (vec2){5.0, 7.0};
        anc[1] = (vec2){7.0, 1.0};
        anc[2] = (vec2){1.0, 5.0};
        anc[3] = (vec2){7.0, 5.0};

        init_clock_offsets();
        srand(42);
        initialized = 1;
    }

    sim_time += 0.5;

    vec2 pos_hybrid = {0,0};
    vec2 pos_tof    = {0,0};

    double t0 = 0.0;
    double ti[N_ANCHORS-1];
    double d[N_ANCHORS];

    printf("\n=== [SIM CYCLE] t=%.1f s ===\n", sim_time);

    int valid_cnt = 0;
    for (int i = 0; i < N_ANCHORS; ++i) {
        double toa, tof;
        simulate_ss_twr(i, &toa, &tof);

        if (i==0) t0 = toa;
        else      ti[i-1] = toa;

        d[i] = tof * C0;   // để dùng cho TOF-only
        valid_cnt++;
    }

    if (valid_cnt < 4) return;

    /* --- 1) TOF-only localization --- */
    pos_tof = tof_only_localize(d);
    double err_tof =
        vnorm(vsub(pos_tof,
                   (vec2){SIM_TRUE_POS_X, SIM_TRUE_POS_Y}));

    /* --- 2) Hybrid GN localization --- */
    int it = hybrid_localize(anc, N_ANCHORS,
                             t0, ti, phi,
                             d[1], d[N_ANCHORS-1],
                             1, N_ANCHORS-1,
                             &pos_hybrid);

    double err_hybrid =
        vnorm(vsub(pos_hybrid,
                   (vec2){SIM_TRUE_POS_X, SIM_TRUE_POS_Y}));

    /* --- OUTPUT --- */
    printf("TOF-Only     = (%.3f , %.3f)  err=%.3f m\n",
           pos_tof.x, pos_tof.y, err_tof);

    printf("HYBRID (GN)  = (%.3f , %.3f)  err=%.3f m   it=%d\n",
           pos_hybrid.x, pos_hybrid.y, err_hybrid, it);

    printf("Improvement  = %.3f m\n", err_tof - err_hybrid);
}
#endif
