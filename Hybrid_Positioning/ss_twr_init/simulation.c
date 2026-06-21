#ifdef SIMULATION_MODE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef N_ANCHORS
#define N_ANCHORS 4
#endif

#define SIM_TRUE_POS_X   3.4
#define SIM_TRUE_POS_Y   2.1

#define C0              299792458.0
#define TOF_NOISE_STD   (0.15 / C0)

#define DET_EPS         1e-6
#define STEP_LIMIT      0.15
#define MAX_POS         100.0

typedef struct
{
    double x;
    double y;
} vec2;

typedef struct
{
    double x;
    double y;
    double alpha;
    int initialized;
} ema_2d_t;

static vec2 anc[N_ANCHORS];
static double sim_time = 0.0;

static vec2 pos_est = {0.0, 0.0};
static ema_2d_t ema_filter;

/* ====================================================== */

static double dist_2d(vec2 a, vec2 b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;

    return sqrt(dx * dx + dy * dy);
}

static double gauss_noise(double std)
{
    static int has_spare = 0;
    static double spare;

    if (has_spare)
    {
        has_spare = 0;
        return spare;
    }

    double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
    double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);

    double mag = std * sqrt(-2.0 * log(u1));

    spare = mag * sin(2.0 * M_PI * u2);
    has_spare = 1;

    return mag * cos(2.0 * M_PI * u2);
}

/* ====================================================== */

static void ema_2d_init(ema_2d_t *f, double alpha)
{
    f->x = 0.0;
    f->y = 0.0;
    f->alpha = alpha;
    f->initialized = 0;
}

static vec2 ema_2d_update(ema_2d_t *f, vec2 in)
{
    if (!f->initialized)
    {
        f->x = in.x;
        f->y = in.y;
        f->initialized = 1;
    }
    else
    {
        f->x = f->alpha * in.x +
               (1.0 - f->alpha) * f->x;

        f->y = f->alpha * in.y +
               (1.0 - f->alpha) * f->y;
    }

    vec2 out = {f->x, f->y};
    return out;
}

/* ====================================================== */

static double simulate_tof_measurement(int anchor_id)
{
    vec2 tag_pos =
    {
        SIM_TRUE_POS_X,
        SIM_TRUE_POS_Y
    };

    double true_dist =
        dist_2d(tag_pos, anc[anchor_id]);

    double tof =
        true_dist / C0;

    tof += gauss_noise(TOF_NOISE_STD);

    return tof * C0;
}

/* ====================================================== */

static vec2 solve_2d_tof_lm(double distances[])
{
    if (isnan(pos_est.x) || isnan(pos_est.y) ||
        fabs(pos_est.x) > MAX_POS ||
        fabs(pos_est.y) > MAX_POS)
    {
        pos_est.x = 0.0;
        pos_est.y = 0.0;
    }

    double curr_x = pos_est.x;
    double curr_y = pos_est.y;

    if (fabs(curr_x) < 1e-3 &&
        fabs(curr_y) < 1e-3)
    {
        int min_i = 0;

        for (int i = 1; i < N_ANCHORS; i++)
        {
            if (distances[i] < distances[min_i])
                min_i = i;
        }

        curr_x = anc[min_i].x;
        curr_y = anc[min_i].y;
    }

    double lambda = 0.1;

    for (int iter = 0; iter < 50; iter++)
    {
        double jtj[2][2] = {{0}};
        double jtf[2] = {0};

        double curr_cost = 0.0;

        for (int i = 0; i < N_ANCHORS; i++)
        {
            double dx = curr_x - anc[i].x;
            double dy = curr_y - anc[i].y;

            double r =
                sqrt(dx * dx + dy * dy);

            if (r < 1e-6)
                r = 1e-6;

            double res =
                r - distances[i];

            double w =
                1.0 / (fabs(res) + 0.1);

            double jx = dx / r;
            double jy = dy / r;

            curr_cost +=
                w * res * res;

            jtj[0][0] += w * jx * jx;
            jtj[0][1] += w * jx * jy;
            jtj[1][0] += w * jy * jx;
            jtj[1][1] += w * jy * jy;

            jtf[0] += w * jx * res;
            jtf[1] += w * jy * res;
        }

        jtj[0][0] += lambda;
        jtj[1][1] += lambda;

        double det =
            jtj[0][0] * jtj[1][1] -
            jtj[0][1] * jtj[1][0];

        if (fabs(det) < DET_EPS)
        {
            lambda *= 5.0;
            continue;
        }

        double inv00 =  jtj[1][1] / det;
        double inv01 = -jtj[0][1] / det;
        double inv10 = -jtj[1][0] / det;
        double inv11 =  jtj[0][0] / det;

        double step_x =
            -(inv00 * jtf[0] +
              inv01 * jtf[1]);

        double step_y =
            -(inv10 * jtf[0] +
              inv11 * jtf[1]);

        if (step_x > STEP_LIMIT) step_x = STEP_LIMIT;
        if (step_x < -STEP_LIMIT) step_x = -STEP_LIMIT;

        if (step_y > STEP_LIMIT) step_y = STEP_LIMIT;
        if (step_y < -STEP_LIMIT) step_y = -STEP_LIMIT;

        double next_x = curr_x + step_x;
        double next_y = curr_y + step_y;

        double next_cost = 0.0;

        for (int i = 0; i < N_ANCHORS; i++)
        {
            double dx = next_x - anc[i].x;
            double dy = next_y - anc[i].y;

            double r =
                sqrt(dx * dx + dy * dy);

            double res =
                r - distances[i];

            double w =
                1.0 / (fabs(res) + 0.1);

            next_cost +=
                w * res * res;
        }

        if (next_cost < curr_cost)
        {
            curr_x = next_x;
            curr_y = next_y;

            lambda *= 0.7;

            if (lambda < 1e-5)
                lambda = 1e-5;

            if (sqrt(step_x * step_x +
                     step_y * step_y) < 1e-4)
            {
                break;
            }
        }
        else
        {
            lambda *= 2.0;

            if (lambda > 1e6)
                break;
        }
    }

    pos_est.x = curr_x;
    pos_est.y = curr_y;

    return pos_est;
}

/* ====================================================== */

void simulate_measurement_cycle(void)
{
    static int initialized = 0;

    if (!initialized)
    {
        anc[0] = (vec2){0.0, 0.0};
        anc[1] = (vec2){6.0, 0.0};
        anc[2] = (vec2){6.0, 4.0};
        anc[3] = (vec2){0.0, 4.0};

        ema_2d_init(&ema_filter, 0.2);

        srand(42);

        initialized = 1;
    }

    sim_time += 0.5;

    double distances[N_ANCHORS];

    for (int i = 0; i < N_ANCHORS; i++)
    {
        distances[i] =
            simulate_tof_measurement(i);
    }

    vec2 raw_pos =
        solve_2d_tof_lm(distances);

    vec2 ema_pos =
        ema_2d_update(
            &ema_filter,
            raw_pos);

    vec2 true_pos =
    {
        SIM_TRUE_POS_X,
        SIM_TRUE_POS_Y
    };

    double raw_err =
        dist_2d(raw_pos, true_pos);

    double ema_err =
        dist_2d(ema_pos, true_pos);

    printf("\n=== SIM t=%.1f s ===\n", sim_time);

    printf("TRUE : (%.3f %.3f)\n",
           true_pos.x,
           true_pos.y);

    printf("RAW  : (%.3f %.3f) err=%.3f m\n",
           raw_pos.x,
           raw_pos.y,
           raw_err);

    printf("EMA  : (%.3f %.3f) err=%.3f m\n",
           ema_pos.x,
           ema_pos.y,
           ema_err);
}

#endif