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

/* ===================== SIM CONFIG 3D ===================== */

#define SIM_TRUE_POS_X      3.4
#define SIM_TRUE_POS_Y      2.1
#define SIM_TRUE_POS_Z      1.2

/* Sai số thực tế */
#define TOA_NOISE_STD       (5e-10)
#define TOF_NOISE_STD       (0.15 / C0)

vec3 anc[N_ANCHORS];
double phi[N_ANCHORS - 1];
static double sim_time = 0.0;

/* Bộ nhớ cho Deferred Sync */
static double Syn[N_ANCHORS][2];   
static int syn_idx = 0;

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

/* ===================== SS-TWR 3D ===================== */

static int simulate_ss_twr(int anchor_id, double *toa_meas, double *tof_meas)
{
    vec3 tag_pos = {SIM_TRUE_POS_X, SIM_TRUE_POS_Y, SIM_TRUE_POS_Z};

    double dist = vnorm3(vsub3(tag_pos, anc[anchor_id]));
    double tof_true = dist / C0;

    double tx_poll = 0.0;
    double rx_poll_anchor = tof_true;

    /* Clock offset không dùng nữa → luôn = 0 trong Deferred Sync */
    double delay_anchor = POLL_RX_TO_RESP_TX_DLY_UUS * 1e-6;
    double tx_resp_anchor = rx_poll_anchor + delay_anchor;
    double rx_resp_tag = tx_resp_anchor + tof_true;

    *toa_meas = rx_poll_anchor + gauss_noise(TOA_NOISE_STD);

    double rtd = rx_resp_tag - tx_poll;
    double tof_calc = (rtd - delay_anchor) / 2.0;

    *tof_meas = tof_calc + gauss_noise(TOF_NOISE_STD);

    return 1;
}

/* Clock offset = 0 (Deferred Sync không cần clock offset noise) */
static void init_clock_offsets(void)
{
    for (int i = 0; i < N_ANCHORS - 1; ++i)
        phi[i] = 0.0;
}

/* ===================== TOF-only 3D ===================== */

static vec3 tof_only_localize(double d[])
{
    vec3 p1 = anc[0], p2 = anc[1], p3 = anc[2], p4 = anc[3];
    double r1 = d[0], r2 = d[1], r3 = d[2], r4 = d[3];

    vec3 ex = vsub3(p2, p1);
    double d12 = vnorm3(ex);
    ex.x /= d12; ex.y /= d12; ex.z /= d12;

    vec3 p1p3 = vsub3(p3, p1);
    double i = p1p3.x*ex.x + p1p3.y*ex.y + p1p3.z*ex.z;

    vec3 temp = {p1p3.x - i*ex.x, p1p3.y - i*ex.y, p1p3.z - i*ex.z};
    double temp_norm = vnorm3(temp);
    vec3 ey = {temp.x/temp_norm, temp.y/temp_norm, temp.z/temp_norm};

    vec3 ez = {
        ex.y*ey.z - ex.z*ey.y,
        ex.z*ey.x - ex.x*ey.z,
        ex.x*ey.y - ex.y*ey.x
    };

    vec3 p1p4 = vsub3(p4, p1);
    double j = p1p3.x*ey.x + p1p3.y*ey.y + p1p3.z*ey.z;

    double x = (r1*r1 - r2*r2 + d12*d12) / (2*d12);
    double y = (r1*r1 - r3*r3 + i*i + j*j - 2*i*x) / (2*j);

    double zz = r1*r1 - x*x - y*y;
    if (zz < 0) zz = 0;
    double z = sqrt(zz);

    vec3 result;
    result.x = p1.x + x*ex.x + y*ey.x + z*ez.x;
    result.y = p1.y + x*ex.y + y*ey.y + z*ez.y;
    result.z = p1.z + x*ex.z + y*ey.z + z*ez.z;

    return result;
}

/* ===================== MAIN SIMULATION ===================== */

void simulate_measurement_cycle(void)
{
    static int initialized = 0;

    if (!initialized) {

        /* Anchor layout hợp lý */
        anc[0] = (vec3){0.0, 0.0, 3.0};
        anc[1] = (vec3){6.0, 0.0, 1.4};
        anc[2] = (vec3){6.0, 4.0, 2.9};
        anc[3] = (vec3){0.0, 4.0, 1.6};

        init_clock_offsets();
        srand(42);
        initialized = 1;
    }

    sim_time += 0.5;

    vec3 pos_hybrid = {0,0,0};
    vec3 pos_tof    = {0,0,0};

    double t0 = 0.0;
    double ti[N_ANCHORS - 1];
    double d[N_ANCHORS];

    printf("\n=== [SIM 3D CYCLE] t=%.1f s ===\n", sim_time);

    int valid_cnt = 0;

    /* ===== SS-TWR đo từng anchor ===== */
    for (int i = 0; i < N_ANCHORS; ++i) {
        double toa, tof;
        simulate_ss_twr(i, &toa, &tof);

        if (i == 0) t0 = toa;
        else        ti[i - 1] = toa;

        d[i] = tof * C0;
        valid_cnt++;
    }

    if (valid_cnt < 4) return;

    /* ===== Lưu TOA vào Syn để tính R_i(k) ===== */
    for (int i = 0; i < N_ANCHORS; i++) {
        Syn[i][syn_idx] = (i == 0 ? t0 : ti[i - 1]);
    }
    syn_idx = 1 - syn_idx;

    /* ===== 1) TOF only ===== */
    pos_tof = tof_only_localize(d);

    double err_tof = vnorm3(vsub3(pos_tof,
                    (vec3){SIM_TRUE_POS_X, SIM_TRUE_POS_Y, SIM_TRUE_POS_Z}));

    /* ===== 2) Tính Deferred Sync ===== */
    for (int i = 1; i < N_ANCHORS; i++) {

        double Syn_i_k  = Syn[i][0];
        double Syn_i_k1 = Syn[i][1];
        double Syn_0_k  = Syn[0][0];
        double Syn_0_k1 = Syn[0][1];

        double Ri = (Syn_i_k1 - Syn_i_k) /
                    (Syn_0_k1 - Syn_0_k);    // công thức (2.27)

        double DeltaTr = ti[i - 1] - t0;      // raw TDOA
        double DeltaTk = DeltaTr * Ri;        // công thức (2.28)

        phi[i - 1] = DeltaTr - DeltaTk;       // công thức (2.29)
    }

    /* ===== 3) Hybrid GN ===== */
    int it = hybrid_localize(
                anc, N_ANCHORS,
                t0, ti, phi,
                d[1], d[3],   // TOF anchor #1 và #3
                1, 3,
                &pos_hybrid
            );

    double err_hybrid = vnorm3(
                    vsub3(pos_hybrid,
                          (vec3){SIM_TRUE_POS_X, SIM_TRUE_POS_Y, SIM_TRUE_POS_Z}));

    /* ===== OUTPUT ===== */
    printf("TOF-Only 3D   = (%.3f , %.3f , %.3f)   err=%.3f m\n",
           pos_tof.x, pos_tof.y, pos_tof.z, err_tof);

    printf("HYBRID 3D GN  = (%.3f , %.3f , %.3f)   err=%.3f m   it=%d\n",
           pos_hybrid.x, pos_hybrid.y, pos_hybrid.z, err_hybrid, it);

    printf("Improvement   = %.3f m\n", err_tof - err_hybrid);
}

#endif
