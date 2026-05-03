#ifdef SIMULATION_MODE

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "utils.h"
#include "gmc_kalman.h"

#ifndef N_ANCHORS
#define N_ANCHORS 4
#endif

#define C0 299792458.0
#define TOF_NOISE_STD (0.08 / C0)

/* ================= ANCHORS ================= */
static vec2 true_anchors[N_ANCHORS];
static vec2 calc_anchors[N_ANCHORS];

/* ================= UTIL ================= */
static double dist_2d(vec2 a, vec2 b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

/* Gaussian noise generator */
static double gauss_noise(double std)
{
    static int has_spare = 0;
    static double spare;

    if (has_spare) {
        has_spare = 0;
        return spare;
    }
    has_spare = 1;

    double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);

    double mag = std * sqrt(-2.0 * log(u1));

    spare = mag * sin(2.0 * M_PI * u2);
    return mag * cos(2.0 * M_PI * u2);
}

/* ================= SIMULATION CORE ================= */
void simulate_measurement_cycle(void)
{
    static int init = 0;
    static int tick = 0;
    double sim_time = tick * 0.2;

    static IMM_GMC_Kalman4D_t imm_filter;
    static vec2 tag_pos;
    static vec2 tag_vel;

    if (!init) {
        true_anchors[0] = (vec2){0, 0};
        true_anchors[1] = (vec2){6, 0};
        true_anchors[2] = (vec2){6, 4};
        true_anchors[3] = (vec2){0, 4};

        srand(42);

        printf("\n=== [INIT] ANCHOR GEOMETRY ESTIMATION ===\n");

        double d01 = dist_2d(true_anchors[0], true_anchors[1]);
        double d02 = dist_2d(true_anchors[0], true_anchors[2]);
        double d03 = dist_2d(true_anchors[0], true_anchors[3]);
        double d12 = dist_2d(true_anchors[1], true_anchors[2]);
        double d13 = dist_2d(true_anchors[1], true_anchors[3]);
        double d23 = dist_2d(true_anchors[2], true_anchors[3]);

        calc_anchors[0] = (vec2){0, 0};

        if (calculate_anchor_geometry(d01, d02, d03, d12, d13, d23,
                                      &calc_anchors[1], &calc_anchors[2], &calc_anchors[3])) {
            for(int i=0; i<4; i++) {
                printf("  Anchor %d: (%.2f, %.2f)\n",
                       i, calc_anchors[i].x, calc_anchors[i].y);
            }
        } else {
            printf("Anchor geometry estimation FAILED\n");
        }

        tag_pos = (vec2){1.0, 1.0};
        tag_vel = (vec2){0.0, 0.0};

        imm_gmc_init(&imm_filter, 0.8, 0.05, 1.5, 0.5);
        imm_gmc_reset(&imm_filter, tag_pos.x, tag_pos.y);

        init = 1;
    }

    int is_outlier = 0;
    vec2 target_vel = {0.0, 0.0};

    /* Motion scenario */
    if (sim_time < 5.0) {
        target_vel = (vec2){0.0, 0.0};
    } 
    else if (sim_time < 14.0) {
        target_vel = (vec2){1.2, 0.8};
    } 
    else {
        target_vel = (vec2){0.0, 0.0};
    }

    /* Velocity smoothing */
    double vel_alpha = 0.85;
    tag_vel.x = vel_alpha * tag_vel.x + (1.0 - vel_alpha) * target_vel.x;
    tag_vel.y = vel_alpha * tag_vel.y + (1.0 - vel_alpha) * target_vel.y;

    /* Position update */
    tag_pos.x += tag_vel.x * 0.2 + gauss_noise(0.005);
    tag_pos.y += tag_vel.y * 0.2 + gauss_noise(0.005);

    double d[N_ANCHORS];

    for (int i = 0; i < N_ANCHORS; i++) {
        double true_dist = dist_2d(tag_pos, calc_anchors[i]);

        /* NLOS injection */
        if (i == 1 && sim_time >= 15.0 && sim_time <= 18.0) {
            true_dist += 3.5;
            is_outlier = 1;
        }

        double tof = true_dist / C0 + gauss_noise(TOF_NOISE_STD);
        d[i] = tof * C0;
    }

    vec2 raw_est = {0, 0};
    tof_2d_localize(calc_anchors, N_ANCHORS, d, &raw_est);

    imm_gmc_predict(&imm_filter, 0.2);
    vec2 filt_est = imm_gmc_update(&imm_filter, raw_est);

    /* ================= LOG OUTPUT ================= */
    int should_print = 0;

    if (tick == 5)   should_print = 1;
    if (tick == 26)  should_print = 1;
    if (tick == 45)  should_print = 1;
    if (tick == 71)  should_print = 1;
    if (tick == 80)  should_print = 1;
    if (tick == 95)  should_print = 1;

    if (should_print) {
        printf("\n=== [SIM IMM-GMC 4D] t=%.1f s ===\n", sim_time);

        if (tick == 5)   printf("STATE: STATIC (Jitter test)\n");
        if (tick == 26)  printf("STATE: MOTION START\n");
        if (tick == 45)  printf("STATE: STEADY MOTION\n");
        if (tick == 71)  printf("STATE: DECELERATION / STOP\n");
        if (tick == 80)  printf("WARNING: NLOS ERROR 3.5m AT ANCHOR 1\n");
        if (tick == 95)  printf("STATE: RECOVERED / STABLE\n");

        printf("TRUE POSITION     = (%5.2f, %5.2f)\n", tag_pos.x, tag_pos.y);

        printf("RAW ESTIMATION    = (%5.2f, %5.2f) | Error: %.3fm\n",
               raw_est.x, raw_est.y, dist_2d(raw_est, tag_pos));

        printf("FILTERED ESTIMATE = (%5.2f, %5.2f) | Error: %.3fm\n",
               filt_est.x, filt_est.y, dist_2d(filt_est, tag_pos));

        printf("IMM MODE PROB     = Static: %3.0f%% | Dynamic: %3.0f%%\n",
               imm_filter.mode_prob[0] * 100,
               imm_filter.mode_prob[1] * 100);
    }

    tick++;
}

#endif