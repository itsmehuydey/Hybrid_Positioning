#ifdef SIMULATION_MODE
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifndef N_ANCHORS
#define N_ANCHORS 4
#endif

/* ===================== SIM CONFIG 2D ===================== */

#define SIM_TRUE_POS_X      3.4
#define SIM_TRUE_POS_Y      2.1
#define SIM_TRUE_POS_Z      0.0  /* Không dùng trong 2D */

/* Hằng số tốc độ ánh sáng */
#define C0 299792458.0

/* Sai số mô phỏng TOF (0.15m noise) */
#define TOF_NOISE_STD       (0.15 / C0)

/* Định nghĩa Vector 2D */
typedef struct {
    double x;
    double y;
} vec2;

vec2 anc[N_ANCHORS];
static double sim_time = 0.0;

/* ===================== UTILS ===================== */

static double dist_2d(vec2 a, vec2 b) {
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

/* Hàm tạo Gaussian noise đơn giản */
static double gauss_noise(double std)
{
    static int has_spare = 0;
    static double spare;
    if (has_spare) { 
        has_spare = 0; 
        return spare;  // <--- SỬA Ở ĐÂY: Bỏ "* std" đi
    }

    has_spare = 1;
    double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double mag = std * sqrt(-2.0 * log(u1));
    spare = mag * sin(2.0 * M_PI * u2);
    return mag * cos(2.0 * M_PI * u2);
}

/* ===================== SENSOR SIMULATION ===================== */

/* Giả lập đo khoảng cách ToF có nhiễu */
static double simulate_tof_measurement(int anchor_id)
{
    vec2 tag_pos = {SIM_TRUE_POS_X, SIM_TRUE_POS_Y};
    double true_dist = dist_2d(tag_pos, anc[anchor_id]);
    double tof_true = true_dist / C0;
    
    /* Thêm nhiễu Gauss vào thời gian bay */
    double measured_tof = tof_true + gauss_noise(TOF_NOISE_STD);
    
    /* Trả về khoảng cách (mét) */
    return measured_tof * C0;
}

/* ===================== 2D TOF SOLVER (Levenberg-Marquardt) ===================== */

/* Giải hệ phương trình 2D ToF bằng LM (giống bản Python) */
static vec2 solve_2d_tof_lm(double *distances, vec2 init_guess, int max_iter, double tol)
{
    double curr_x = init_guess.x;
    double curr_y = init_guess.y;
    double lam = 0.01; /* Damping factor */

    for (int iter = 0; iter < max_iter; iter++) {
        double jtj[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
        double jtf[2] = {0.0, 0.0};
        double max_step = 0.0;

        for (int i = 0; i < N_ANCHORS; i++) {
            double dx = curr_x - anc[i].x;
            double dy = curr_y - anc[i].y;
            double r_est = sqrt(dx*dx + dy*dy);
            
            if (r_est < 1e-6) r_est = 1e-6; // Tránh chia cho 0

            double res = r_est - distances[i];
            
            // Jacobian
            double df_dx = dx / r_est;
            double df_dy = dy / r_est;

            // Tích lũy JTJ và JTf
            jtj[0][0] += df_dx * df_dx;
            jtj[0][1] += df_dx * df_dy;
            jtj[1][0] += df_dy * df_dx;
            jtj[1][1] += df_dy * df_dy;

            jtf[0] += df_dx * res;
            jtf[1] += df_dy * res;
        }

        // Thêm Levenberg-Marquardt damping
        jtj[0][0] += lam;
        jtj[1][1] += lam;

        // Cramer's rule giải hệ 2x2
        double det = jtj[0][0] * jtj[1][1] - jtj[0][1] * jtj[1][0];
        if (fabs(det) < 1e-12) {
            break; // Ma trận suy biến
        }

        double delta_x = (-jtf[0] * jtj[1][1] - (-jtf[1] * jtj[0][1])) / det;
        double delta_y = (jtj[0][0] * (-jtf[1]) - jtj[1][0] * (-jtf[0])) / det;

        curr_x += delta_x;
        curr_y += delta_y;

        max_step = sqrt(delta_x*delta_x + delta_y*delta_y);
        
        if (max_step < tol) {
            break; // Hội tụ
        }
    }

    vec2 result = {curr_x, curr_y};
    return result;
}

/* ===================== MAIN SIMULATION ===================== */

void simulate_measurement_cycle(void)
{
    static int initialized = 0;

    if (!initialized) {
        /* Thiết lập tọa độ 4 Anchor (2D) */
        anc[0] = (vec2){0.0, 0.0};
        anc[1] = (vec2){6.0, 0.0};
        anc[2] = (vec2){6.0, 4.0};
        anc[3] = (vec2){0.0, 4.0};

        srand(42); // Cố định seed để dễ debug
        initialized = 1;
    }

    sim_time += 0.5;
    double measured_distances[N_ANCHORS];

    printf("\n=== [SIM 2D ToF CYCLE] t=%.1f s ===\n", sim_time);

    /* 1. Đo khoảng cách từ Tag đến các Anchor */
    printf("Measured Distances:\n");
    for (int i = 0; i < N_ANCHORS; ++i) {
        measured_distances[i] = simulate_tof_measurement(i);
        printf("  Anchor %d: %.3f m\n", i, measured_distances[i]);
    }

    /* 2. Giải tọa độ bằng ToF (LM 2D Solver) */
    vec2 init_guess = {3.0, 2.0}; // Lấy điểm giữa làm điểm bắt đầu
    vec2 est_pos = solve_2d_tof_lm(measured_distances, init_guess, 50, 1e-4);

    /* 3. Đánh giá sai số */
    vec2 true_pos = {SIM_TRUE_POS_X, SIM_TRUE_POS_Y};
    double err = dist_2d(est_pos, true_pos);

    /* ===== OUTPUT ===== */
    printf("TRUE POS      = (%.3f , %.3f)\n", true_pos.x, true_pos.y);
    printf("EST TOF 2D    = (%.3f , %.3f)\n", est_pos.x, est_pos.y);
    printf("ERROR         = %.3f m\n", err);
}

#endif // SIMULATION_MODE