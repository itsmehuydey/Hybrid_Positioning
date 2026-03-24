#include "ble_hybrid.h"
#include <string.h>
#include "ble_beacon.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT  299702547.0
#endif
#ifndef DWT_TIME_UNITS
#define DWT_TIME_UNITS  (1.0 / 499.2e6 / 128.0)
#endif
#ifndef MH_SLOT_TIMEOUT_MS
#define MH_SLOT_TIMEOUT_MS 5000u
#endif

/* Cửa sổ thời gian để ghép SYNC và TOF vào cùng slot
   500ms tính theo ms hệ thống (FreeRTOS tick)               */
#define MH_MATCH_WINDOW_MS  500u

static inline uint32_t system_time_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* ====================================================================
   Slot lưu dữ liệu 1 cycle
   Dùng cycle_id = cycle_k1 của anchor report.
   TOF từ TAG cũng dùng g_cycle_id = cycle_k1.
   ==================================================================== */
typedef struct {
    uint8_t  used;
    uint16_t cycle_id;

    uint8_t  have_sync[MH_MAX_ANCHORS];
    uint64_t syn_k[MH_MAX_ANCHORS];
    uint64_t syn_k1[MH_MAX_ANCHORS];
    uint64_t T_i[MH_MAX_ANCHORS];     /* A0: = syn_k1; Slave: = poll rx ts */

    uint8_t  have_tof[MH_MAX_ANCHORS];
    float    dist[MH_MAX_ANCHORS];

    uint32_t created_ms;
    uint32_t last_update_ms;
} mh_cycle_slot_t;

#define MH_CYCLE_BUF_SIZE 64
static mh_cycle_slot_t g_cycle_buf[MH_CYCLE_BUF_SIZE];
static uint8_t g_anchor_ids[MH_MAX_ANCHORS] = ANCHOR_ID_LIST;

void master_hybrid_init(void)  { memset(g_cycle_buf, 0, sizeof(g_cycle_buf)); }
void master_hybrid_reset(void) { master_hybrid_init(); }

static mh_cycle_slot_t* mh_find_or_alloc_slot(uint16_t cycle_id)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
        if (g_cycle_buf[i].used && g_cycle_buf[i].cycle_id == cycle_id) {
            g_cycle_buf[i].last_update_ms = now;
            return &g_cycle_buf[i];
        }

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
        if (!g_cycle_buf[i].used) {
            memset(&g_cycle_buf[i], 0, sizeof(mh_cycle_slot_t));
            g_cycle_buf[i].used           = 1;
            g_cycle_buf[i].cycle_id       = cycle_id;
            g_cycle_buf[i].created_ms     = now;
            g_cycle_buf[i].last_update_ms = now;
            return &g_cycle_buf[i];
        }

    int oldest = 0;
    uint32_t oldest_time = g_cycle_buf[0].last_update_ms;
    for (int i = 1; i < MH_CYCLE_BUF_SIZE; i++)
        if ((now - g_cycle_buf[i].last_update_ms) > (now - oldest_time)) {
            oldest = i; oldest_time = g_cycle_buf[i].last_update_ms;
        }

    memset(&g_cycle_buf[oldest], 0, sizeof(mh_cycle_slot_t));
    g_cycle_buf[oldest].used           = 1;
    g_cycle_buf[oldest].cycle_id       = cycle_id;
    g_cycle_buf[oldest].created_ms     = now;
    g_cycle_buf[oldest].last_update_ms = now;
    return &g_cycle_buf[oldest];
}

/* [4] Ref = A0 ưu tiên */
static int mh_select_ref(const mh_cycle_slot_t *slot)
{
    if (slot->have_sync[0]) return 0;
    for (int i = 1; i < MH_MAX_ANCHORS; i++)
        if (slot->have_sync[i]) return i;
    return -1;
}

/* [5] Clock drift R_i,j */
static double mh_calc_drift(const mh_cycle_slot_t *slot, int i, int ref)
{
    double denom = (double)(int64_t)(slot->syn_k1[ref] - slot->syn_k[ref]);
    if (denom == 0.0) return 1.0;
    return (double)(int64_t)(slot->syn_k1[i] - slot->syn_k[i]) / denom;
}

/* [6] Normalize: T_i' = (T_i - syn_k_i) / R_i */
static double mh_normalize_ts(const mh_cycle_slot_t *slot, int i, int ref)
{
    double R = (i == ref) ? 1.0 : mh_calc_drift(slot, i, ref);
    if (R == 0.0) R = 1.0;
    return (double)(int64_t)(slot->T_i[i] - slot->syn_k[i]) / R;
}

/* Điều kiện: A0 có sync + ít nhất 1 slave có sync + ít nhất 1 TOF */
static int mh_slot_ready(const mh_cycle_slot_t *slot)
{
    if (!slot->have_sync[0]) return 0;
    int slave_sync = 0, tof_cnt = 0;
    for (int i = 1; i < MH_MAX_ANCHORS; i++)
        if (slot->have_sync[i]) slave_sync++;
    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        if (slot->have_tof[i])  tof_cnt++;
    return (slave_sync >= 1 && tof_cnt >= 1);
}

static void mh_emit(const mh_cycle_slot_t *slot)
{
    int ref = mh_select_ref(slot);
    if (ref < 0) return;

    mh_hybrid_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.msg_type = 'H';
    pkt.cycle_id = slot->cycle_id;
    pkt.tag_id   = TAG_ID;
    pkt.ref_idx  = (uint8_t)ref;
    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        pkt.anchor_ids[i] = g_anchor_ids[i];

    double T_ref = mh_normalize_ts(slot, ref, ref);
    uint8_t tmask = 0, dmask = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++) {
        if (i != ref && slot->have_sync[i]) {
            double dt = (mh_normalize_ts(slot, i, ref) - T_ref) * DWT_TIME_UNITS;
            pkt.delta_d[i] = (float)(dt * SPEED_OF_LIGHT);
            tmask |= (uint8_t)(1u << i);
        }
        if (slot->have_tof[i]) {
            pkt.dist[i] = slot->dist[i];
            dmask |= (uint8_t)(1u << i);
        }
    }
    pkt.tdoa_mask = tmask;
    pkt.tof_mask  = dmask;

    printf("[MASTER] HYBRID cyc=%u ref=A%d TDOA=0x%02X TOF=0x%02X\r\n",
           slot->cycle_id, ref, tmask, dmask);

    /* In delta_d để dễ kiểm tra */
    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        if (tmask & (1u << i))
            printf("  A%d dd=%.2fm d=%.2fm\r\n",
                   i, pkt.delta_d[i],
                   (dmask & (1u << i)) ? pkt.dist[i] : -1.0f);

    master_gateway_send((const uint8_t *)&pkt, sizeof(pkt));
}

static void mh_try_process(void)
{
    uint32_t now = system_time_ms();
    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++) {
        mh_cycle_slot_t *s = &g_cycle_buf[i];
        if (!s->used) continue;
        if ((now - s->created_ms) > MH_SLOT_TIMEOUT_MS)
        { memset(s, 0, sizeof(*s)); continue; }
        if (mh_slot_ready(s)) { mh_emit(s); memset(s, 0, sizeof(*s)); }
    }
}

/* Nhận BLE 'S' – từ slave hoặc A0 self-feed */
void master_hybrid_handle_ble_tdoa_report(const ble_tdoa_report_t *rpt)
{
    if (!rpt || rpt->anchor_id >= MH_MAX_ANCHORS) return;

    uint8_t id = rpt->anchor_id;
    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(rpt->cycle_id);

    slot->syn_k[id]     = rpt->syn_k;
    slot->syn_k1[id]    = rpt->syn_k1;
    slot->T_i[id]       = rpt->T_i;
    slot->have_sync[id] = 1;

    printf("[MASTER] SYNC A%u cyc=%u\r\n", id, rpt->cycle_id);
    mh_try_process();
}

/* Nhận BLE data – route 'S' / 'T' */
void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len)
{
    if (!data || len == 0) return;

    if (data[0] == 'S' && len >= sizeof(ble_tdoa_report_t)) {
        master_hybrid_handle_ble_tdoa_report((const ble_tdoa_report_t *)data);
        return;
    }

    if (data[0] != 'T' || len < sizeof(mh_ble_tof_packet_t)) return;
    mh_ble_tof_packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));
    if (pkt.anchor_id >= MH_MAX_ANCHORS) return;

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(pkt.cycle_id);
    slot->dist[pkt.anchor_id]     = pkt.distance;
    slot->have_tof[pkt.anchor_id] = 1;

    printf("[MASTER] TOF A%u cyc=%u d=%.2fm\r\n",
           pkt.anchor_id, pkt.cycle_id, pkt.distance);
    mh_try_process();
}

void master_gateway_send(const uint8_t *data, uint16_t len)
{
    const mh_hybrid_packet_t *pkt = (const mh_hybrid_packet_t *)data;
    (void)len;
    for (int i = 0; i < 5; i++) {
        ble_raw_beacon_send_payload((const uint8_t *)pkt, sizeof(*pkt));
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}