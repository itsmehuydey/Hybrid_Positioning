// simulation.c
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

/* ---------- Cấu hình mô phỏng ---------- */
#define SIM_TRUE_POS_X      4.0
#define SIM_TRUE_POS_Y      3.0

#define TOA_NOISE_STD       (1.0e-10)    // ~3 cm
#define TOF_NOISE_STD       (0.02 / C0)  // 2 cm
#define CLOCK_OFFSET_STD    (5.0e-10)    // ~15 cm drift

vec2   anc[N_ANCHORS];
double phi[N_ANCHORS - 1];
static double sim_time = 0.0;

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

static int simulate_ss_twr(int anchor_id, double *toa_meas, double *tof_meas)
{
    vec2 tag_pos = {SIM_TRUE_POS_X, SIM_TRUE_POS_Y};
    double dist = vnorm(vsub(tag_pos, anc[anchor_id]));
    double tof_true = dist / C0;

    double tx_poll = 0.0;
    double rx_poll_anchor = tof_true;

    /* ÁP DỤNG CLOCK OFFSET */
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

static void init_clock_offsets(void)
{
    for (int i = 0; i < N_ANCHORS-1; ++i)
        phi[i] = gauss_noise(CLOCK_OFFSET_STD);
}

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

    vec2 pos_est_local = {0};
    Kalman2D kf_local = {0};

    double t0, ti[N_ANCHORS-1];
    double d1 = -1.0, d2 = -1.0;
    int a1 = 1, a2 = N_ANCHORS-1;
    int valid_cnt = 0;

    printf("\n=== [SIM CYCLE] t=%.1fs ===\n", sim_time);
    printf("N_ANCHORS = %d\n", N_ANCHORS);
    printf("TAG TRUE : (%.3f, %.3f)\n", SIM_TRUE_POS_X, SIM_TRUE_POS_Y);
    printf("phi      : ");
    for (int i=0;i<N_ANCHORS-1;i++) printf("%.3e ", phi[i]);
    printf("\n");

    for (int i = 0; i < N_ANCHORS; ++i) {
        double toa, tof;
        if (simulate_ss_twr(i, &toa, &tof)) {
            if (i == 0) {
                t0 = toa;
                printf("A0: TOA=%.3e s (ref)\n", toa);
            } else {
                ti[i-1] = toa;
                printf("A%d: TOA=%.3e s, D=%.3f m\n", i, toa, tof * C0);
            }
            if (i == a1) d1 = tof * C0;
            if (i == a2) d2 = tof * C0;
            ++valid_cnt;
        }
    }

    if (valid_cnt < 3) return;

    int it = hybrid_localize(anc, N_ANCHORS, t0, ti, phi, d1, d2, a1, a2, &pos_est_local, &kf_local);
    double err = vnorm(vsub(pos_est_local, (vec2){SIM_TRUE_POS_X, SIM_TRUE_POS_Y}));

    printf("GN it = %d -> Est = (%.6f, %.6f)\n", it, pos_est_local.x, pos_est_local.y);
    printf("ERROR = %.6f m\n", err);
}
#endif