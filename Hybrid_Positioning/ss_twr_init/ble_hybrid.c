#include "ble_hybrid.h"
#include <string.h>
#include "ble_beacon.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT   299702547.0f
#endif

#ifndef DWT_TIME_UNITS
#define DWT_TIME_UNITS   (1.0/499.2e6/128.0)
#endif

// ================== CẤU HÌNH TIMEOUT ==================
// TĂNG timeout để chờ đủ TOF + TDOA
#ifndef MH_SLOT_TIMEOUT_MS
#define MH_SLOT_TIMEOUT_MS   5000u  // 5s thay vì 1.2s
#endif

static inline uint32_t system_time_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

// ================== STRUCT MỖI CYCLE - TĂNG BUFFER ==================
typedef struct
{
    uint8_t  used;
    uint16_t cycle_id;

    uint8_t  have_tdoa[MH_MAX_ANCHORS];
    uint64_t tdoa_ts[MH_MAX_ANCHORS];

    uint8_t  have_tof[MH_MAX_ANCHORS];
    float    tof[MH_MAX_ANCHORS];
    float    dist[MH_MAX_ANCHORS];

    uint32_t created_ms;
    uint32_t last_update_ms;  // NEW: track last activity

} mh_cycle_slot_t;

// TĂNG buffer từ 16 → 32 slots
#define MH_CYCLE_BUF_SIZE 64
static mh_cycle_slot_t g_cycle_buf[MH_CYCLE_BUF_SIZE];

static uint8_t g_anchor_ids[MH_MAX_ANCHORS] = ANCHOR_ID_LIST;

// ================== INIT / RESET ==================
void master_hybrid_init(void)
{
    memset(g_cycle_buf, 0, sizeof(g_cycle_buf));
}

void master_hybrid_reset(void)
{
    master_hybrid_init();
}

// ================== INTERNAL FUNCTIONS ==================

// Tìm hoặc cấp slot theo cycle_id
static mh_cycle_slot_t* mh_find_or_alloc_slot(uint16_t cycle_id)
{
    uint32_t now = system_time_ms();

    // 1) Tìm slot cũ có cùng cycle
    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        if (g_cycle_buf[i].used && g_cycle_buf[i].cycle_id == cycle_id)
        {
            g_cycle_buf[i].last_update_ms = now;  // refresh activity
            return &g_cycle_buf[i];
        }
    }

    // 2) Tìm slot rỗng
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

    // 3) Buffer đầy → ghi đè slot CŨ NHẤT (idle longest)
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

    printf("[HYBRID] Buffer full, evicting cycle=%u (idle %lu ms)\r\n",
           g_cycle_buf[oldest].cycle_id,
           now - g_cycle_buf[oldest].last_update_ms);

    memset(&g_cycle_buf[oldest], 0, sizeof(mh_cycle_slot_t));
    g_cycle_buf[oldest].used           = 1;
    g_cycle_buf[oldest].cycle_id       = cycle_id;
    g_cycle_buf[oldest].created_ms     = now;
    g_cycle_buf[oldest].last_update_ms = now;

    return &g_cycle_buf[oldest];
}

// Đủ điều kiện để gửi HYBRID?
static int mh_slot_ready_for_hybrid(const mh_cycle_slot_t *slot)
{
    // MUST HAVE: Master's TDOA timestamp (anchor 0)
    if (!slot->have_tdoa[0]) 
    {
        return 0;
    }

    int tof_cnt = 0;
    int tdoa_cnt = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (slot->have_tof[i])  tof_cnt++;
        if (slot->have_tdoa[i]) tdoa_cnt++;
    }

    // Cần ít nhất 2 TOF và 3 TDOA (bao gồm master)
    if (tof_cnt < 2) return 0;
    if (tdoa_cnt < 3) return 0;

    return 1;
}

// Gửi HYBRID 'H' lên gateway
static void mh_emit_hybrid(const mh_cycle_slot_t *slot)
{
    mh_hybrid_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.msg_type = 'H';
    pkt.cycle_id = slot->cycle_id;
    pkt.tag_id   = TAG_ID;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        pkt.anchor_ids[i] = g_anchor_ids[i];

    pkt.ref_idx = 0; // master = anchor 0

    uint64_t ts_ref = slot->tdoa_ts[pkt.ref_idx];

    uint8_t tdoa_mask = 0;
    uint8_t tof_mask  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        /* ===== TDOA ===== */
        if (i != pkt.ref_idx && slot->have_tdoa[i])
        {
            int64_t dt = (int64_t)(slot->tdoa_ts[i] - ts_ref);

            pkt.delta_d[i] =
                (float)((double)dt * DWT_TIME_UNITS * (double)SPEED_OF_LIGHT);

            tdoa_mask |= (uint8_t)(1u << i);
        }

        /* ===== TOF ===== */
        if (slot->have_tof[i])
        {
            pkt.dist[i] = slot->dist[i];
            tof_mask |= (uint8_t)(1u << i);
        }
    }

    pkt.tdoa_mask = tdoa_mask;
    pkt.tof_mask  = tof_mask;

    printf("[MASTER][HYBRID READY] cycle=%u tdoa_mask=0x%02X tof_mask=0x%02X\r\n",
           slot->cycle_id, pkt.tdoa_mask, pkt.tof_mask);

    // Debug: show actual data
    printf("  TDOA deltas: ");
    for (int i = 0; i < MH_MAX_ANCHORS; i++) {
        if (tdoa_mask & (1 << i)) {
            printf("A%d=%.2fm ", i, pkt.delta_d[i]);
        }
    }
    printf("\r\n  TOF dists: ");
    for (int i = 0; i < MH_MAX_ANCHORS; i++) {
        if (tof_mask & (1 << i)) {
            printf("A%d=%.2fm ", i, pkt.dist[i]);
        }
    }
    printf("\r\n");

    master_gateway_send((const uint8_t *)&pkt, sizeof(pkt));
}

// Kiểm tra và xử lý tất cả slots
static void mh_try_process_cycles(void)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        mh_cycle_slot_t *slot = &g_cycle_buf[i];
        if (!slot->used)
            continue;

        // ===== TIMEOUT CHECK =====
        if ((now - slot->created_ms) > MH_SLOT_TIMEOUT_MS)
        {
            printf("[HYBRID] Timeout cycle=%u (age=%lu ms, TOF=%u TDOA=%u)\r\n",
                   slot->cycle_id,
                   now - slot->created_ms,
                   slot->have_tof[0] + slot->have_tof[1] + slot->have_tof[2] + slot->have_tof[3],
                   slot->have_tdoa[0] + slot->have_tdoa[1] + slot->have_tdoa[2] + slot->have_tdoa[3]);
            
            memset(slot, 0, sizeof(mh_cycle_slot_t));
            continue;
        }

        // ===== READY CHECK =====
        if (mh_slot_ready_for_hybrid(slot))
        {
            mh_emit_hybrid(slot);
            memset(slot, 0, sizeof(mh_cycle_slot_t));
        }
    }
}

// ================== PUBLIC API ==================

// TAG gửi TOF về MASTER qua BLE
void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len)
{   
    if (data == NULL) return;
    if (len < sizeof(mh_ble_tof_packet_t)) return;

    mh_ble_tof_packet_t pkt;
    memcpy(&pkt, data, sizeof(pkt));

    if (pkt.msg_type != 'T') return;
    if (pkt.anchor_id >= MH_MAX_ANCHORS) return;

    printf("[MASTER][BLE RX] TOF | cycle=%u anchor=%u dist=%.2f m\r\n",
           pkt.cycle_id, pkt.anchor_id, pkt.distance);

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(pkt.cycle_id);

    slot->dist[pkt.anchor_id]     = pkt.distance;
    slot->have_tof[pkt.anchor_id] = 1;

    printf("[HYBRID] TOF stored | cycle=%u A%u (age=%lu ms)\r\n",
           pkt.cycle_id, pkt.anchor_id,
           system_time_ms() - slot->created_ms);

    mh_try_process_cycles();
}

// ANCHOR gửi TDOA (timestamp) về MASTER bằng UWB
void master_hybrid_handle_uwb_tdoa(uint8_t anchor_id,
                                   uint16_t cycle_id,
                                   uint64_t timestamp)
{
    if (anchor_id >= MH_MAX_ANCHORS) return;

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(cycle_id);

    slot->tdoa_ts[anchor_id]   = timestamp;
    slot->have_tdoa[anchor_id] = 1;

    printf("[HYBRID] TDOA stored | cycle=%u A%u ts=%llu (age=%lu ms)\r\n",
           cycle_id, anchor_id, (unsigned long long)timestamp,
           system_time_ms() - slot->created_ms);

    mh_try_process_cycles();
}

void master_gateway_send(const uint8_t *data, uint16_t len)
{
    const mh_hybrid_packet_t *pkt = (const mh_hybrid_packet_t *)data;

    printf("[GW TX] HYBRID | cycle=%u tag=%u ref_idx=%u tdoa=0x%02X tof=0x%02X\r\n",
           pkt->cycle_id, pkt->tag_id, pkt->ref_idx,
           pkt->tdoa_mask, pkt->tof_mask);

    // Gửi 5 lần với retry để đảm bảo gateway nhận được
    for (int i = 0; i < 5; i++)
    {
        ble_raw_beacon_send_payload(
            (const uint8_t *)pkt,
            sizeof(mh_hybrid_packet_t)
        );
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}