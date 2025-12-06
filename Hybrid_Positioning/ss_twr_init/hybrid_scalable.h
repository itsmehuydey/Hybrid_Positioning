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

/* ===== 3D vector ===== */
typedef struct {
    double x, y, z;
} vec3;

static inline vec3 vsub3(vec3 a, vec3 b) {
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline double vnorm3(vec3 a) {
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

/* ===== Hybrid Data for Gauss–Newton (3D) ===== */
typedef struct {
    const vec3   *anc;       // anchor positions (3D)
    const int    *idx_tdoa;  // index list for TDOA measurements
    int           m_tdoa;    // number of TDOA equations
    const double *Delta_d;   // corrected TDOA ranges

    int    a1, a2;           // anchors used for TOF
    double d1, d2;           // corrected TOF distances
} HybridData;

/* ===== Function Prototypes ===== */

/* residual vector r(x) */
void compute_residual_hybrid(const HybridData *hd, const vec3 *x,
                             double r[], int *M_tot);

/* Jacobian G(x) with 3 columns (∂r/∂x, ∂r/∂y, ∂r/∂z)  */
void compute_jacobian_hybrid(const HybridData *hd, const vec3 *x,
                             double G[][3], int M_tot);

/* Gauss–Newton solver */
int gauss_newton_hybrid(const HybridData *hd, vec3 *x,
                        int max_it, double tol_res, double tol_step);

/* Clock offset estimation (3D tag positions) */
void estimate_clock_offsets(const vec3 tag_pos[], const vec3 anc[],
                            int N_anc,
                            const double t0_meas[],
                            const double ti_meas[][N_TDOA],
                            int N_points, double phi[]);

/* Main localization function */
int hybrid_localize(const vec3 anc[], int N_anc,
                    double t0,
                    const double ti[],
                    const double phi[],
                    double d1, double d2,
                    int a1, int a2,
                    vec3 *pos_est);

#endif /* HYBRID_SCALABLE_H */
