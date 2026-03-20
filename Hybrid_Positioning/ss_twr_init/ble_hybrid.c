#include "ble_hybrid.h"
#include <string.h>
#include "ble_beacon.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT   299702547.0
#endif

#ifndef DWT_TIME_UNITS
#define DWT_TIME_UNITS   (1.0 / 499.2e6 / 128.0)
#endif

#ifndef MH_SLOT_TIMEOUT_MS
#define MH_SLOT_TIMEOUT_MS   5000u
#endif

static inline uint32_t system_time_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

/* ------------------------------------------------------------------
   Slot lưu dữ liệu một cycle đo
   ------------------------------------------------------------------ */
typedef struct
{
    uint8_t  used;
    uint16_t cycle_id;

    /* --- TDOA sync data từ Anchor gửi BLE về --- */
    uint8_t  have_sync[MH_MAX_ANCHORS]; /* 1 = có đủ syn_k + syn_k1 + T_i */
    uint64_t syn_k[MH_MAX_ANCHORS];     /* Syn_i^k  */
    uint64_t syn_k1[MH_MAX_ANCHORS];    /* Syn_i^{k+1} */
    uint64_t T_i[MH_MAX_ANCHORS];       /* timestamp tag tại anchor i */

    /* --- TOF distance từ TAG gửi BLE về (GIỮ NGUYÊN) --- */
    uint8_t  have_tof[MH_MAX_ANCHORS];
    float    dist[MH_MAX_ANCHORS];

    uint32_t created_ms;
    uint32_t last_update_ms;

} mh_cycle_slot_t;

#define MH_CYCLE_BUF_SIZE 64
static mh_cycle_slot_t g_cycle_buf[MH_CYCLE_BUF_SIZE];

static uint8_t g_anchor_ids[MH_MAX_ANCHORS] = ANCHOR_ID_LIST;

/* ------------------------------------------------------------------
   Init / Reset
   ------------------------------------------------------------------ */
void master_hybrid_init(void)
{
    memset(g_cycle_buf, 0, sizeof(g_cycle_buf));
}

void master_hybrid_reset(void)
{
    master_hybrid_init();
}

/* ------------------------------------------------------------------
   Tìm hoặc cấp phát slot cho cycle_id
   ------------------------------------------------------------------ */
static mh_cycle_slot_t* mh_find_or_alloc_slot(uint16_t cycle_id)
{
    uint32_t now = system_time_ms();

    /* tìm slot đang tồn tại */
    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        if (g_cycle_buf[i].used && g_cycle_buf[i].cycle_id == cycle_id)
        {
            g_cycle_buf[i].last_update_ms = now;
            return &g_cycle_buf[i];
        }
    }

    /* tìm slot trống */
    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        if (!g_cycle_buf[i].used)
        {
            memset(&g_cycle_buf[i], 0, sizeof(mh_cycle_slot_t));
            g_cycle_buf[i].used           = 1;
            g_cycle_buf[i].cycle_id       = cycle_id;
            g_cycle_buf[i].created_ms     = now;
            g_cycle_buf[i].last_update_ms = now;
            return &g_cycle_buf[i];
        }
    }

    /* evict oldest */
    int oldest = 0;
    uint32_t oldest_time = g_cycle_buf[0].last_update_ms;
    for (int i = 1; i < MH_CYCLE_BUF_SIZE; i++)
    {
        if ((now - g_cycle_buf[i].last_update_ms) > (now - oldest_time))
        {
            oldest = i;
            oldest_time = g_cycle_buf[i].last_update_ms;
        }
    }

    memset(&g_cycle_buf[oldest], 0, sizeof(mh_cycle_slot_t));
    g_cycle_buf[oldest].used           = 1;
    g_cycle_buf[oldest].cycle_id       = cycle_id;
    g_cycle_buf[oldest].created_ms     = now;
    g_cycle_buf[oldest].last_update_ms = now;

    return &g_cycle_buf[oldest];
}

/* ------------------------------------------------------------------
   [4] Chọn reference anchor j
       Ưu tiên anchor 0 (master), fallback sang anchor khác
       Điều kiện: phải có have_sync
   ------------------------------------------------------------------ */
static int mh_select_ref_anchor(const mh_cycle_slot_t *slot)
{
    if (slot->have_sync[0]) return 0;
    for (int i = 1; i < MH_MAX_ANCHORS; i++)
        if (slot->have_sync[i]) return i;
    return -1;
}

/* ------------------------------------------------------------------
   [5] Tính clock drift
       R_i = (Syn_i^{k+1} - Syn_i^k) / (Syn_j^{k+1} - Syn_j^k)
   ------------------------------------------------------------------ */
static double mh_calc_drift(const mh_cycle_slot_t *slot, int i, int ref)
{
    double denom = (double)(int64_t)(slot->syn_k1[ref] - slot->syn_k[ref]);
    if (denom == 0.0) return 1.0;
    double numer = (double)(int64_t)(slot->syn_k1[i]   - slot->syn_k[i]);
    return numer / denom;
}

/* ------------------------------------------------------------------
   [6] Normalize timestamp T_i về đồng hồ của ref anchor
       T_i' = (T_i - Syn_i^k) / R_i
       T_j' = (T_j - Syn_j^k)        (R_j = 1 vì j là ref)
   ------------------------------------------------------------------ */
static double mh_normalize_ts(const mh_cycle_slot_t *slot, int i, int ref)
{
    double R_i = (i == ref) ? 1.0 : mh_calc_drift(slot, i, ref);
    if (R_i == 0.0) R_i = 1.0;
    double Ti_shifted = (double)(int64_t)(slot->T_i[i] - slot->syn_k[i]);
    return Ti_shifted / R_i;
}

/* ------------------------------------------------------------------
   Kiểm tra slot đã đủ dữ liệu để tính Hybrid chưa
   Cần: ít nhất 3 anchor có sync data + ít nhất 2 anchor có TOF
   ------------------------------------------------------------------ */
static int mh_slot_ready_for_hybrid(const mh_cycle_slot_t *slot)
{
    int sync_cnt = 0;
    int tof_cnt  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (slot->have_sync[i]) sync_cnt++;
        if (slot->have_tof[i])  tof_cnt++;
    }

    if (sync_cnt < 3) return 0;
    if (tof_cnt  < 2) return 0;

    return 1;
}

/* ------------------------------------------------------------------
   Tổng hợp và phát gói Hybrid lên Gateway
   ------------------------------------------------------------------ */
static void mh_emit_hybrid(const mh_cycle_slot_t *slot)
{
    /* [4] Chọn reference anchor j */
    int ref = mh_select_ref_anchor(slot);
    if (ref < 0)
    {
        printf("[MASTER] No valid ref anchor, skip cycle=%u\r\n", slot->cycle_id);
        return;
    }

    mh_hybrid_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.msg_type = 'H';
    pkt.cycle_id = slot->cycle_id;
    pkt.tag_id   = TAG_ID;
    pkt.ref_idx  = (uint8_t)ref;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        pkt.anchor_ids[i] = g_anchor_ids[i];

    /* [6] T_j' = (T_j - Syn_j^k)  (ref anchor, R=1) */
    double T_ref_norm = mh_normalize_ts(slot, ref, ref);

    uint8_t tdoa_mask = 0;
    uint8_t tof_mask  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (i != ref && slot->have_sync[i])
        {
            /* [5] tính R_i, [6] normalize T_i */
            double T_i_norm = mh_normalize_ts(slot, i, ref);

            /* [7] TDoA: Δt_i = T_i' - T_j' */
            double delta_t = (T_i_norm - T_ref_norm) * DWT_TIME_UNITS; /* seconds */

            /* đổi ra khoảng cách chênh lệch Δd (m) */
            pkt.delta_d[i] = (float)(delta_t * SPEED_OF_LIGHT);

            tdoa_mask |= (uint8_t)(1u << i);
        }

        if (slot->have_tof[i])
        {
            pkt.dist[i] = slot->dist[i];
            tof_mask   |= (uint8_t)(1u << i);
        }
    }

    pkt.tdoa_mask = tdoa_mask;
    pkt.tof_mask  = tof_mask;

    printf("[MASTER] HYBRID READY | cycle=%u ref=A%d TDOA:0x%02X TOF:0x%02X\r\n",
           slot->cycle_id, ref, pkt.tdoa_mask, pkt.tof_mask);

    master_gateway_send((const uint8_t *)&pkt, sizeof(pkt));
}

/* ------------------------------------------------------------------
   Duyệt tất cả slot, xử lý nếu đã đủ hoặc hết timeout
   ------------------------------------------------------------------ */
static void mh_try_process_cycles(void)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        mh_cycle_slot_t *slot = &g_cycle_buf[i];
        if (!slot->used) continue;

        if ((now - slot->created_ms) > MH_SLOT_TIMEOUT_MS)
        {
            memset(slot, 0, sizeof(mh_cycle_slot_t));
            continue;
        }

        if (mh_slot_ready_for_hybrid(slot))
        {
            mh_emit_hybrid(slot);
            memset(slot, 0, sizeof(mh_cycle_slot_t));
        }
    }
}

/* ------------------------------------------------------------------
   [3] Nhận BLE TDOA report từ Anchor ('S' packet)
       Lưu syn_k, syn_k1, T_i vào slot
   ------------------------------------------------------------------ */
void master_hybrid_handle_ble_tdoa_report(const ble_tdoa_report_t *rpt)
{
    if (rpt == NULL) return;
    if (rpt->anchor_id >= MH_MAX_ANCHORS) return;

    uint8_t id = rpt->anchor_id;

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(rpt->cycle_id);

    slot->syn_k[id]     = rpt->syn_k;
    slot->syn_k1[id]    = rpt->syn_k1;
    slot->T_i[id]       = rpt->T_i;
    slot->have_sync[id] = 1;

    printf("[MASTER] RX SYNC RPT A%u | cycle=%u syn_k=%llu syn_k1=%llu Ti=%llu\r\n",
           id, rpt->cycle_id,
           (unsigned long long)rpt->syn_k,
           (unsigned long long)rpt->syn_k1,
           (unsigned long long)rpt->T_i);

    mh_try_process_cycles();
}

/* ------------------------------------------------------------------
   RX BLE từ TAG (TOF packet 'T') – GIỮ NGUYÊN logic cũ
   Thêm: route packet 'S' sang master_hybrid_handle_ble_tdoa_report
   ------------------------------------------------------------------ */
void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) return;

    /* Route packet 'S' từ Anchor */
    if (len >= sizeof(ble_tdoa_report_t) && data[0] == 'S')
    {
        const ble_tdoa_report_t *rpt = (const ble_tdoa_report_t *)data;
        master_hybrid_handle_ble_tdoa_report(rpt);
        return;
    }

    /* Packet 'T' từ TAG (TOF) – GIỮ NGUYÊN */
    if (len < sizeof(mh_ble_tof_packet_t)) return;

    mh_ble_tof_packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));

    if (pkt.msg_type != 'T') return;
    if (pkt.anchor_id >= MH_MAX_ANCHORS) return;

    printf("[MASTER] RX TOF via BLE | cycle=%u A%u dist=%.2f m\r\n",
           pkt.cycle_id, pkt.anchor_id, pkt.distance);

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(pkt.cycle_id);

    slot->dist[pkt.anchor_id]     = pkt.distance;
    slot->have_tof[pkt.anchor_id] = 1;

    mh_try_process_cycles();
}

/* ------------------------------------------------------------------
   Gửi gói Hybrid lên Gateway qua BLE beacon – GIỮ NGUYÊN
   ------------------------------------------------------------------ */
void master_gateway_send(const uint8_t *data, uint16_t len)
{
    const mh_hybrid_packet_t *pkt = (const mh_hybrid_packet_t *)data;

    printf("[MASTER] SEND HYBRID to Gateway | cycle=%u\r\n", pkt->cycle_id);

    for (int i = 0; i < 5; i++)
    {
        ble_raw_beacon_send_payload(
            (const uint8_t *)pkt,
            sizeof(mh_hybrid_packet_t)
        );
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}