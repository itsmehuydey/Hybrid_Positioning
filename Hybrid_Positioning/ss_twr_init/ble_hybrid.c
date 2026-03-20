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

#ifndef MH_SLOT_TIMEOUT_MS
#define MH_SLOT_TIMEOUT_MS   5000u
#endif

static inline uint32_t system_time_ms(void)
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

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
    uint32_t last_update_ms;

} mh_cycle_slot_t;

#define MH_CYCLE_BUF_SIZE 64
static mh_cycle_slot_t g_cycle_buf[MH_CYCLE_BUF_SIZE];

static uint8_t g_anchor_ids[MH_MAX_ANCHORS] = ANCHOR_ID_LIST;

void master_hybrid_init(void)
{
    memset(g_cycle_buf, 0, sizeof(g_cycle_buf));
}

void master_hybrid_reset(void)
{
    master_hybrid_init();
}

static mh_cycle_slot_t* mh_find_or_alloc_slot(uint16_t cycle_id)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        if (g_cycle_buf[i].used && g_cycle_buf[i].cycle_id == cycle_id)
        {
            g_cycle_buf[i].last_update_ms = now;
            return &g_cycle_buf[i];
        }
    }

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

static int mh_slot_ready_for_hybrid(const mh_cycle_slot_t *slot)
{
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

    if (tof_cnt < 2) return 0;
    if (tdoa_cnt < 3) return 0;

    return 1;
}

static void mh_emit_hybrid(const mh_cycle_slot_t *slot)
{
    mh_hybrid_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.msg_type = 'H';
    pkt.cycle_id = slot->cycle_id;
    pkt.tag_id   = TAG_ID;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
        pkt.anchor_ids[i] = g_anchor_ids[i];

    pkt.ref_idx = 0;

    uint64_t ts_ref = slot->tdoa_ts[pkt.ref_idx];

    uint8_t tdoa_mask = 0;
    uint8_t tof_mask  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (i != pkt.ref_idx && slot->have_tdoa[i])
        {
            int64_t dt = (int64_t)(slot->tdoa_ts[i] - ts_ref);

            pkt.delta_d[i] =
                (float)((double)dt * DWT_TIME_UNITS * (double)SPEED_OF_LIGHT);

            tdoa_mask |= (uint8_t)(1u << i);
        }

        if (slot->have_tof[i])
        {
            pkt.dist[i] = slot->dist[i];
            tof_mask |= (uint8_t)(1u << i);
        }
    }

    pkt.tdoa_mask = tdoa_mask;
    pkt.tof_mask  = tof_mask;

    printf("[MASTER] HYBRID READY | cycle=%u TDOA:0x%02X TOF:0x%02X\r\n",
           slot->cycle_id, pkt.tdoa_mask, pkt.tof_mask);

    master_gateway_send((const uint8_t *)&pkt, sizeof(pkt));
}

static void mh_try_process_cycles(void)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_CYCLE_BUF_SIZE; i++)
    {
        mh_cycle_slot_t *slot = &g_cycle_buf[i];
        if (!slot->used)
            continue;

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

void master_hybrid_handle_ble_data(const uint8_t *data, uint16_t len)
{   
    if (data == NULL) return;
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

void master_hybrid_handle_uwb_tdoa(uint8_t anchor_id,
                                   uint16_t cycle_id,
                                   uint64_t timestamp)
{
    if (anchor_id >= MH_MAX_ANCHORS) return;

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(cycle_id);

    slot->tdoa_ts[anchor_id]   = timestamp;
    slot->have_tdoa[anchor_id] = 1;

    printf("[MASTER] RX TDOA from A%u | cycle=%u\r\n", anchor_id, cycle_id);

    mh_try_process_cycles();
}

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