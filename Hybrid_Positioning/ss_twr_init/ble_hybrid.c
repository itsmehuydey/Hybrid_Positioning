#include "ble_hybrid.h"
#include <string.h>

// Nếu dùng DWM1001 + FreeRTOS:
#include "FreeRTOS.h"
#include "task.h"

// ================== CẤU HÌNH TIMEOUT ==================

// Timeout cho 1 cycle (ms) – bạn có thể chỉnh tuỳ tốc độ hệ thống
#ifndef MH_SLOT_TIMEOUT_MS
#define MH_SLOT_TIMEOUT_MS   60u
#endif

// Hàm lấy thời gian hệ thống (ms)
static inline uint32_t system_time_ms(void)
{
    // DWM1001 SDK dùng FreeRTOS, tick = portTICK_PERIOD_MS
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}


// ================== STRUCT MỖI CYCLE ==================

typedef struct
{
    uint8_t  used;
    uint16_t cycle_id;

    uint8_t  have_tdoa[MH_MAX_ANCHORS];
    uint64_t tdoa_ts[MH_MAX_ANCHORS];

    uint8_t  have_tof[MH_MAX_ANCHORS];
    float    tof[MH_MAX_ANCHORS];
    float    dist[MH_MAX_ANCHORS];

    uint32_t created_ms;        // thời điểm slot được tạo (ms)

} mh_cycle_slot_t;


// ================== BUFFER FIFO ==================
static mh_cycle_slot_t g_cycle_buf[MH_MAX_CYCLE_BUF];


// ================== DANH SÁCH ANCHOR ID ==================
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
    // 1) Tìm slot cũ có cùng cycle
    for (int i = 0; i < MH_MAX_CYCLE_BUF; i++)
    {
        if (g_cycle_buf[i].used && g_cycle_buf[i].cycle_id == cycle_id)
        {
            return &g_cycle_buf[i];
        }
    }

    // 2) Tìm slot rỗng
    for (int i = 0; i < MH_MAX_CYCLE_BUF; i++)
    {
        if (!g_cycle_buf[i].used)
        {
            memset(&g_cycle_buf[i], 0, sizeof(mh_cycle_slot_t));
            g_cycle_buf[i].used       = 1;
            g_cycle_buf[i].cycle_id   = cycle_id;
            g_cycle_buf[i].created_ms = system_time_ms();   // NEW
            return &g_cycle_buf[i];
        }
    }

    // 3) Buffer đầy → ghi đè slot cũ nhất (vòng 16-bit)
    int oldest = 0;
    for (int i = 1; i < MH_MAX_CYCLE_BUF; i++)
    {
        uint16_t diff_i  = (uint16_t)(g_cycle_buf[i].cycle_id - g_cycle_buf[oldest].cycle_id);
        uint16_t diff_oi = (uint16_t)(g_cycle_buf[oldest].cycle_id - g_cycle_buf[i].cycle_id);
        if (diff_i > diff_oi)
        {
            oldest = i;
        }
    }

    memset(&g_cycle_buf[oldest], 0, sizeof(mh_cycle_slot_t));
    g_cycle_buf[oldest].used       = 1;
    g_cycle_buf[oldest].cycle_id   = cycle_id;
    g_cycle_buf[oldest].created_ms = system_time_ms();     // NEW

    return &g_cycle_buf[oldest];
}


// Đủ điều kiện để gửi HYBRID?
static int mh_slot_ready_for_hybrid(const mh_cycle_slot_t *slot)
{
    int tdoa_cnt = 0;
    int tof_cnt  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (slot->have_tdoa[i]) tdoa_cnt++;
        if (slot->have_tof[i])  tof_cnt++;
    }

    // Hybrid: cần ít nhất 4 TDOA + 2 TOF
    return (tdoa_cnt >= 4 && tof_cnt >= 2);
}


// Gửi HYBRID 'H' lên gateway
static void mh_emit_hybrid(const mh_cycle_slot_t *slot)
{
    mh_hybrid_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.msg_type = 'H';
    pkt.cycle_id = slot->cycle_id;

    // Gán ID TAG
    pkt.tag_id = TAG_ID;

    // Gán danh sách Anchor ID thực tế (mapping index -> ID)
    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        pkt.anchor_ids[i] = g_anchor_ids[i];
    }

    uint8_t tdoa_mask = 0;
    uint8_t tof_mask  = 0;

    for (int i = 0; i < MH_MAX_ANCHORS; i++)
    {
        if (slot->have_tdoa[i])
        {
            tdoa_mask |= (uint8_t)(1u << i);
            pkt.tdoa_ts[i] = slot->tdoa_ts[i];
        }

        if (slot->have_tof[i])
        {
            tof_mask |= (uint8_t)(1u << i);
            pkt.dist[i] = slot->dist[i];
            pkt.tof[i]  = slot->tof[i];
        }
    }

    pkt.tdoa_mask = tdoa_mask;
    pkt.tof_mask  = tof_mask;

    // Gửi gói HYBRID lên Gateway (BLE/WiFi/UART tuỳ bạn hiện thực)
    master_gateway_send((const uint8_t *)&pkt, sizeof(pkt));
}


// Sau mỗi lần cập nhật buffer thì kiểm tra slot nào đủ điều kiện / timeout
static void mh_try_process_cycles(void)
{
    uint32_t now = system_time_ms();

    for (int i = 0; i < MH_MAX_CYCLE_BUF; i++)
    {
        mh_cycle_slot_t *slot = &g_cycle_buf[i];
        if (!slot->used)
            continue;

        // ===== TIMEOUT CHECK =====
        // Nếu quá MH_SLOT_TIMEOUT_MS mà vẫn chưa đủ dữ liệu → xoá slot
        if ((uint32_t)(now - slot->created_ms) > MH_SLOT_TIMEOUT_MS)
        {
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

    mh_cycle_slot_t *slot = mh_find_or_alloc_slot(pkt.cycle_id);

    slot->tof[pkt.anchor_id]      = pkt.tof;
    slot->dist[pkt.anchor_id]     = pkt.distance;
    slot->have_tof[pkt.anchor_id] = 1;

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

    mh_try_process_cycles();
}
