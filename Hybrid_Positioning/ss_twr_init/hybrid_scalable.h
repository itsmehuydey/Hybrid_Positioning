#ifndef HYBRID_SCALABLE_H
#define HYBRID_SCALABLE_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static const double C0 = 299792458.0;

#ifndef N_ANCHORS
#define N_ANCHORS 4
#warning "N_ANCHORS not defined, using default = 4"
#endif

#ifndef N_TDOA
#define N_TDOA (N_ANCHORS - 1)
#endif

#ifndef TOF_A1
#define TOF_A1 1
#endif

#ifndef TOF_A2
#define TOF_A2 (N_ANCHORS - 1)
#endif

#define N_CALIB_POINTS 50
#define NOISE_LEVEL    0.05
#define TOF_NOISE      0.10

typedef struct {
    double x, y;
} vec2;

static inline vec2 vsub(vec2 a, vec2 b) {
    return (vec2){a.x - b.x, a.y - b.y};
}

static inline double vnorm(vec2 a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

/* Hybrid data struct (GN + TDOA + TOF) */
typedef struct {
    const vec2   *anc;
    const int    *idx_tdoa;
    int           m_tdoa;
    const double *Delta_d;
    int           a1, a2;
    double        d1, d2;
} HybridData;

/* ---------- function prototypes ---------- */
static void mat4_set_identity(double A[4][4]);
static void mat4_mul_vec(double y[4], const double A[4][4], const double x[4]);
static void mat4_add(double C[4][4], const double A[4][4], const double B[4][4]);
static void mat4_transpose(double T[4][4], const double A[4][4]);
static int  mat2_inv(double Inv[2][2], const double A[2][2]);
static void mat4_mul(double C[4][4], const double A[4][4], const double B[4][4]);
static void mat4_sub(double C[4][4], const double A[4][4], const double B[4][4]);

static void compute_residual_hybrid(const HybridData *hd, const vec2 *x,
                                    double r[], int *M_tot);

static void compute_jacobian_hybrid(const HybridData *hd, const vec2 *x,
                                    double G[][2], int M_tot);

static int gauss_newton_hybrid(const HybridData *hd, vec2 *x,
                               int max_it, double tol_res, double tol_step);

static void estimate_clock_offsets(const vec2 tag_pos[], const vec2 anc[],
                                   int N_anc,
                                   const double t0_meas[],
                                   const double ti_meas[][N_TDOA],
                                   int N_points, double phi[]);

/* --- main localisation function (KHÔNG còn Kalman) --- */
int hybrid_localize(const vec2 anc[], int N_anc,
                    double t0,
                    const double ti[],
                    const double phi[],
                    double d1, double d2,
                    int a1, int a2,
                    vec2 *pos_est);

#endif /* HYBRID_SCALABLE_H */
