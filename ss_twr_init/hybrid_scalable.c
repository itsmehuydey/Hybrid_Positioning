#include "hybrid_scalable.h"

/* ---------- matrix helpers ---------- */
static void mat4_set_identity(double A[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            A[i][j] = (i == j) ? 1.0 : 0.0;
}
static void mat4_mul_vec(double y[4], const double A[4][4], const double x[4])
{
    for (int i = 0; i < 4; i++) {
        double s = 0;
        for (int k = 0; k < 4; k++) s += A[i][k] * x[k];
        y[i] = s;
    }
}
static void mat4_add(double C[4][4], const double A[4][4], const double B[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            C[i][j] = A[i][j] + B[i][j];
}
static void mat4_transpose(double T[4][4], const double A[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            T[j][i] = A[i][j];
}
static int mat2_inv(double Inv[2][2], const double A[2][2])
{
    double det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
    if (fabs(det) < 1e-12) return 0;
    double id = 1.0 / det;
    Inv[0][0] =  A[1][1] * id;  Inv[0][1] = -A[0][1] * id;
    Inv[1][0] = -A[1][0] * id;  Inv[1][1] =  A[0][0] * id;
    return 1;
}
static void mat4_mul(double C[4][4], const double A[4][4], const double B[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            double s = 0;
            for (int k = 0; k < 4; k++) s += A[i][k] * B[k][j];
            C[i][j] = s;
        }
}
static void mat4_sub(double C[4][4], const double A[4][4], const double B[4][4])
{
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            C[i][j] = A[i][j] - B[i][j];
}

/* ---------- Kalman ---------- */
static void kalman_init(Kalman2D *kf, double px0, double py0)
{
    memset(kf, 0, sizeof(*kf));
    kf->x[0] = px0; kf->x[1] = py0;
    mat4_set_identity(kf->P);
    kf->Q[0][0] = kf->Q[1][1] = 1e-3;
    kf->Q[2][2] = kf->Q[3][3] = 1e-3;
    kf->R[0][0] = kf->R[1][1] = 0.1 * 0.1;
}
static void kalman_update(Kalman2D *kf, double dt, double zx, double zy)
{
    double F[4][4] = {{1,0,dt,0},{0,1,0,dt},{0,0,1,0},{0,0,0,1}};
    double x_pred[4]; mat4_mul_vec(x_pred, F, kf->x);
    double FT[4][4]; mat4_transpose(FT, F);
    double P_temp[4][4]; mat4_mul(P_temp, F, kf->P);
    double P_pred[4][4]; mat4_mul(P_pred, P_temp, FT);
    mat4_add(P_pred, P_pred, kf->Q);

    double H[2][4] = {{1,0,0,0},{0,1,0,0}};
    double HT[4][2];
    for (int i = 0; i < 4; i++) { HT[i][0] = H[0][i]; HT[i][1] = H[1][i]; }
    double PHT[4][2];
    for (int i = 0; i < 4; i++) {
        PHT[i][0] = P_pred[i][0]*HT[0][0] + P_pred[i][1]*HT[1][0] +
                    P_pred[i][2]*HT[2][0] + P_pred[i][3]*HT[3][0];
        PHT[i][1] = P_pred[i][0]*HT[0][1] + P_pred[i][1]*HT[1][1] +
                    P_pred[i][2]*HT[2][1] + P_pred[i][3]*HT[3][1];
    }
    double S[2][2] = {{PHT[0][0] + kf->R[0][0], PHT[0][1]},
                      {PHT[1][0], PHT[1][1] + kf->R[1][1]}};
    double S_inv[2][2];
    if (!mat2_inv(S_inv, S)) return;
    double K[4][2];
    for (int i = 0; i < 4; i++) {
        K[i][0] = PHT[i][0]*S_inv[0][0] + PHT[i][1]*S_inv[1][0];
        K[i][1] = PHT[i][0]*S_inv[0][1] + PHT[i][1]*S_inv[1][1];
    }
    double z[2] = {zx, zy};
    double Hx[2] = {x_pred[0], x_pred[1]};
    double innov[2] = {z[0]-Hx[0], z[1]-Hx[1]};
    double K_innov[4];
    for (int i = 0; i < 4; i++)
        K_innov[i] = K[i][0]*innov[0] + K[i][1]*innov[1];
    kf->x[0] = x_pred[0] + K_innov[0];
    kf->x[1] = x_pred[1] + K_innov[1];
    kf->x[2] = x_pred[2] + K_innov[2];
    kf->x[3] = x_pred[3] + K_innov[3];
    double KH[4][4];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            KH[i][j] = K[i][0]*H[0][j] + K[i][1]*H[1][j];
    double I_minus_KH[4][4];
    mat4_set_identity(I_minus_KH);
    mat4_sub(I_minus_KH, I_minus_KH, KH);
    mat4_mul(kf->P, I_minus_KH, P_pred);
}

/* ---------- Gauss-Newton helpers ---------- */
static void compute_residual_hybrid(const HybridData *hd, const vec2 *x,
                                   double r[], int *M_tot)
{
    const vec2 *anc = hd->anc;
    int M = hd->m_tdoa;
    *M_tot = M;
    for (int k = 0; k < M; ++k) {
        int i = hd->idx_tdoa[k];
        double d0 = vnorm(vsub(*x, anc[0]));
        double di = vnorm(vsub(*x, anc[i]));
        r[k] = di - d0 - hd->Delta_d[k];
    }
    if (hd->d1 >= 0) { r[*M_tot] = vnorm(vsub(*x, anc[hd->a1])) - hd->d1; (*M_tot)++; }
    if (hd->d2 >= 0) { r[*M_tot] = vnorm(vsub(*x, anc[hd->a2])) - hd->d2; (*M_tot)++; }
}
static void compute_jacobian_hybrid(const HybridData *hd, const vec2 *x,
                                   double G[][2], int M_tot)
{
    const vec2 *anc = hd->anc;
    int M = hd->m_tdoa;
    for (int k = 0; k < M; ++k) {
        int i = hd->idx_tdoa[k];
        vec2 dx0 = vsub(*x, anc[0]);
        vec2 dxi = vsub(*x, anc[i]);
        double n0 = vnorm(dx0);
        double ni = vnorm(dxi);
        G[k][0] = (dxi.x / ni) - (dx0.x / n0);
        G[k][1] = (dxi.y / ni) - (dx0.y / n0);
    }
    int idx = M;
    if (hd->d1 >= 0) {
        vec2 dx = vsub(*x, anc[hd->a1]);
        double n = vnorm(dx);
        G[idx][0] = dx.x / n; G[idx][1] = dx.y / n; idx++;
    }
    if (hd->d2 >= 0) {
        vec2 dx = vsub(*x, anc[hd->a2]);
        double n = vnorm(dx);
        G[idx][0] = dx.x / n; G[idx][1] = dx.y / n;
    }
}
static int gauss_newton_hybrid(const HybridData *hd, vec2 *x,
                               int max_it, double tol_res, double tol_step)
{
    double r[34], G[34][2];
    double lambda = 1e-3;
    int it = 0;
    for (; it < max_it; ++it) {
        int M_tot;
        compute_residual_hybrid(hd, x, r, &M_tot);
        compute_jacobian_hybrid(hd, x, G, M_tot);

        double JTr[2] = {0,0};
        for (int k = 0; k < M_tot; ++k) {
            JTr[0] += G[k][0] * r[k];
            JTr[1] += G[k][1] * r[k];
        }
        double JTJ[2][2] = {{0,0},{0,0}};
        for (int k = 0; k < M_tot; ++k) {
            JTJ[0][0] += G[k][0]*G[k][0];
            JTJ[0][1] += G[k][0]*G[k][1];
            JTJ[1][0] += G[k][1]*G[k][0];
            JTJ[1][1] += G[k][1]*G[k][1];
        }
        JTJ[0][0] += lambda; JTJ[1][1] += lambda;

        double Inv[2][2];
        if (!mat2_inv(Inv, JTJ)) { lambda *= 10; continue; }
        double dx[2];
        dx[0] = -(Inv[0][0]*JTr[0] + Inv[0][1]*JTr[1]);
        dx[1] = -(Inv[1][0]*JTr[0] + Inv[1][1]*JTr[1]);

        double step = sqrt(dx[0]*dx[0] + dx[1]*dx[1]);
        if (step > 1.5) { double s = 1.5/step; dx[0]*=s; dx[1]*=s; }

        x->x += dx[0]; x->y += dx[1];

        double res_new = 0;
        compute_residual_hybrid(hd, x, r, &M_tot);
        for (int k = 0; k < M_tot; ++k) res_new += r[k]*r[k];
        if (res_new < tol_res*tol_res*M_tot || step < tol_step) break;
        lambda = (step > 0.1) ? lambda/10 : lambda*2;
    }
    return (it < max_it) ? it : 0;
}

/* ---------- clock offset calibration ---------- */
static void estimate_clock_offsets(const vec2 tag_pos[], const vec2 anc[],
                                   int N_anc,
                                   const double t0_meas[],
                                   const double ti_meas[][N_TDOA],
                                   int N_points, double phi[])
{
    memset(phi, 0, sizeof(double)*(N_anc-1));
    for (int i = 0; i < N_anc-1; ++i) {
        double sum_phi = 0.0;
        for (int n = 0; n < N_points; ++n) {
            double d0 = vnorm(vsub(tag_pos[n], anc[0]));
            double di = vnorm(vsub(tag_pos[n], anc[i+1]));
            double delta_t = ti_meas[n][i] - t0_meas[n];
            sum_phi += delta_t - (di - d0) / C0;
        }
        phi[i] = sum_phi / N_points;
    }
}

/* ---------- main localisation function ---------- */
int hybrid_localize(const vec2 anc[], int N_anc,
                    double t0,
                    const double ti[],
                    const double phi[],
                    double d1, double d2,
                    int a1, int a2,
                    vec2 *pos_est,
                    Kalman2D *kf)
{
    if (!anc || !pos_est || N_anc < 4) {
        printf("ERROR: Hybrid TDOA+TOF requires at least 4 anchors (got %d).\n", N_anc);
        return -1;
    }

    const int M = N_anc - 1;
    double Delta_d[32];
    for (int i = 0; i < M; ++i) {
        double t_corr = ti[i] - phi[i];
        Delta_d[i] = (t_corr - t0) * C0;
    }

    int idx_tdoa[32];
    for (int i = 0; i < M; ++i) idx_tdoa[i] = i + 1;

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

    vec2 x0 = *pos_est;
    if (x0.x == 0 && x0.y == 0) {
        x0.x = (anc[0].x + anc[N_anc-1].x) * 0.5;
        x0.y = (anc[0].y + anc[N_anc-1].y) * 0.5;
    }
    int it = gauss_newton_hybrid(&hd, &x0, 50, 1e-5, 1e-6);
    *pos_est = x0;

    if (kf) {
        const double dt = 0.1;
        if (kf->x[0] == 0 && kf->x[1] == 0)
            kalman_init(kf, pos_est->x, pos_est->y);
        kalman_update(kf, dt, pos_est->x, pos_est->y);
        pos_est->x = kf->x[0];
        pos_est->y = kf->x[1];
    }
    return it;
}