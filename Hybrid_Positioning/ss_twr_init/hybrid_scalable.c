#include "hybrid_scalable.h"
#include "FreeRTOS.h"
#include "task.h"

/* ============================================================
   Compute residual vector r(x) in 3D
   ============================================================ */
void compute_residual_hybrid(const HybridData *hd, const vec3 *x,
                             double r[], int *M_tot)
{
    const vec3 *anc = hd->anc;
    int M = hd->m_tdoa;
    *M_tot = M;

    /* ----- TDOA part: r_k = (||x-x_i|| - ||x-x_0||) - Δd_k  ----- */
    for (int k = 0; k < M; ++k) {
        int i = hd->idx_tdoa[k];

        double d0 = vnorm3(vsub3(*x, anc[0]));
        double di = vnorm3(vsub3(*x, anc[i]));

        r[k] = (di - d0) - hd->Delta_d[k];
    }

    /* ----- TOF part: r = ||x - a|| - d̂ ----- */
    int idx = M;

    if (hd->d1 >= 0) {
        r[idx] = vnorm3(vsub3(*x, anc[hd->a1])) - hd->d1;
        idx++;
    }

    if (hd->d2 >= 0) {
        r[idx] = vnorm3(vsub3(*x, anc[hd->a2])) - hd->d2;
        idx++;
    }

    *M_tot = idx;
}

/* ============================================================
   Compute Jacobian G(x) in 3D: each row is 3D gradient
   ============================================================ */
void compute_jacobian_hybrid(const HybridData *hd, const vec3 *x,
                             double G[][3], int M_tot)
{
    const vec3 *anc = hd->anc;
    int M = hd->m_tdoa;

    /* ----- TDOA gradients: ∇(di - d0) ----- */
    for (int k = 0; k < M; ++k) {
        int i = hd->idx_tdoa[k];

        vec3 dx0 = vsub3(*x, anc[0]);
        vec3 dxi = vsub3(*x, anc[i]);

        double n0 = vnorm3(dx0);
        double ni = vnorm3(dxi);

        G[k][0] = (dxi.x/ni) - (dx0.x/n0);
        G[k][1] = (dxi.y/ni) - (dx0.y/n0);
        G[k][2] = (dxi.z/ni) - (dx0.z/n0);
    }

    /* ----- TOF gradients: ∇||x-a|| ----- */
    int idx = M;

    if (hd->d1 >= 0) {
        vec3 dx = vsub3(*x, anc[hd->a1]);
        double n = vnorm3(dx);

        G[idx][0] = dx.x/n;
        G[idx][1] = dx.y/n;
        G[idx][2] = dx.z/n;
        idx++;
    }

    if (hd->d2 >= 0) {
        vec3 dx = vsub3(*x, anc[hd->a2]);
        double n = vnorm3(dx);

        G[idx][0] = dx.x/n;
        G[idx][1] = dx.y/n;
        G[idx][2] = dx.z/n;
    }
}

/* ============================================================
   Gauss–Newton for 3D
   ============================================================ */
int gauss_newton_hybrid(const HybridData *hd, vec3 *x,
                        int max_it, double tol_res, double tol_step)
{
    static double r[40];
    static double G[40][3];

    double lambda = 1e-3;

    for (int it = 0; it < max_it; ++it) {

        int M_tot;
        compute_residual_hybrid(hd, x, r, &M_tot);
        compute_jacobian_hybrid(hd, x, G, M_tot);

        /* J^T r */
        double JTr[3] = {0};
        for (int k = 0; k < M_tot; ++k) {
            JTr[0] += G[k][0] * r[k];
            JTr[1] += G[k][1] * r[k];
            JTr[2] += G[k][2] * r[k];
        }

        /* J^T J (3×3) */
        double JTJ[3][3] = {{0}};
        for (int k = 0; k < M_tot; ++k) {
            JTJ[0][0] += G[k][0]*G[k][0];
            JTJ[0][1] += G[k][0]*G[k][1];
            JTJ[0][2] += G[k][0]*G[k][2];

            JTJ[1][0] += G[k][1]*G[k][0];
            JTJ[1][1] += G[k][1]*G[k][1];
            JTJ[1][2] += G[k][1]*G[k][2];

            JTJ[2][0] += G[k][2]*G[k][0];
            JTJ[2][1] += G[k][2]*G[k][1];
            JTJ[2][2] += G[k][2]*G[k][2];
        }

        /* damping */
        JTJ[0][0] += lambda;
        JTJ[1][1] += lambda;
        JTJ[2][2] += lambda;

        /* invert 3×3 */
        double det =
            JTJ[0][0]*(JTJ[1][1]*JTJ[2][2] - JTJ[1][2]*JTJ[2][1]) -
            JTJ[0][1]*(JTJ[1][0]*JTJ[2][2] - JTJ[1][2]*JTJ[2][0]) +
            JTJ[0][2]*(JTJ[1][0]*JTJ[2][1] - JTJ[1][1]*JTJ[2][0]);

        if (fabs(det) < 1e-12) { lambda *= 10; continue; }
        double id = 1.0/det;

        double Inv[3][3];
        Inv[0][0] =  (JTJ[1][1]*JTJ[2][2] - JTJ[1][2]*JTJ[2][1]) * id;
        Inv[0][1] = -(JTJ[0][1]*JTJ[2][2] - JTJ[0][2]*JTJ[2][1]) * id;
        Inv[0][2] =  (JTJ[0][1]*JTJ[1][2] - JTJ[0][2]*JTJ[1][1]) * id;

        Inv[1][0] = -(JTJ[1][0]*JTJ[2][2] - JTJ[1][2]*JTJ[2][0]) * id;
        Inv[1][1] =  (JTJ[0][0]*JTJ[2][2] - JTJ[0][2]*JTJ[2][0]) * id;
        Inv[1][2] = -(JTJ[0][0]*JTJ[1][2] - JTJ[0][2]*JTJ[1][0]) * id;

        Inv[2][0] =  (JTJ[1][0]*JTJ[2][1] - JTJ[1][1]*JTJ[2][0]) * id;
        Inv[2][1] = -(JTJ[0][0]*JTJ[2][1] - JTJ[0][1]*JTJ[2][0]) * id;
        Inv[2][2] =  (JTJ[0][0]*JTJ[1][1] - JTJ[0][1]*JTJ[1][0]) * id;

        /* Δ = -(JTJ)^(-1) JTr */
        double dx[3];
        dx[0] = -(Inv[0][0]*JTr[0] + Inv[0][1]*JTr[1] + Inv[0][2]*JTr[2]);
        dx[1] = -(Inv[1][0]*JTr[0] + Inv[1][1]*JTr[1] + Inv[1][2]*JTr[2]);
        dx[2] = -(Inv[2][0]*JTr[0] + Inv[2][1]*JTr[1] + Inv[2][2]*JTr[2]);

        double step = sqrt(dx[0]*dx[0] + dx[1]*dx[1] + dx[2]*dx[2]);

        if (step > 1.0) { double s = 1.0/step; dx[0]*=s; dx[1]*=s; dx[2]*=s; }

        /* update */
        x->x += dx[0];
        x->y += dx[1];
        x->z += dx[2];

        /* check convergence */
        compute_residual_hybrid(hd, x, r, &M_tot);
        double res2 = 0;
        for (int k = 0; k < M_tot; k++) res2 += r[k]*r[k];

        if (res2 < tol_res || step < tol_step)
            return it;
    }
    return -1;
}

/* ============================================================
   Estimate clock offsets (3D)
   ============================================================ */
void estimate_clock_offsets(const vec3 tag_pos[], const vec3 anc[],
                            int N_anc,
                            const double t0_meas[],
                            const double ti_meas[][N_TDOA],
                            int N_points, double phi[])
{
    memset(phi, 0, sizeof(double)*(N_anc-1));

    for (int i = 0; i < N_anc-1; ++i) {
        double sum = 0;
        for (int n = 0; n < N_points; ++n) {
            double d0 = vnorm3(vsub3(tag_pos[n], anc[0]));
            double di = vnorm3(vsub3(tag_pos[n], anc[i+1]));

            double delta_t = ti_meas[n][i] - t0_meas[n];
            sum += delta_t - (di - d0)/C0;
        }
        phi[i] = sum / N_points;
    }
}

/* ============================================================
   Main localization
   ============================================================ */
int hybrid_localize(const vec3 anc[], int N_anc,
                    double t0,
                    const double ti[],
                    const double phi[],
                    double d1, double d2,
                    int a1, int a2,
                    vec3 *pos_est)
{
    if (N_anc < 4) return -1;

    int M = N_anc - 1;
    static double Delta_d[32];
    static int idx_tdoa[32];

    for (int i=0;i<M;i++){
        Delta_d[i] = ( (ti[i] - t0) - phi[i] ) * C0;
        idx_tdoa[i] = i + 1;
    }

    HybridData hd = {
        .anc = anc,
        .idx_tdoa = idx_tdoa,
        .m_tdoa = M,
        .Delta_d = Delta_d,
        .a1 = a1,
        .a2 = a2,
        .d1 = d1,
        .d2 = d2
    };

    vec3 x = *pos_est;

    /* Auto initial guess if needed */
    if (x.x==0 && x.y==0 && x.z==0) {
        x.x = (anc[0].x + anc[N_anc-1].x)*0.5;
        x.y = (anc[0].y + anc[N_anc-1].y)*0.5;
        x.z = (anc[0].z + anc[N_anc-1].z)*0.5;
    }

    int it = gauss_newton_hybrid(&hd, &x, 60, 1e-6, 1e-6);

    *pos_est = x;
    return it;
}
