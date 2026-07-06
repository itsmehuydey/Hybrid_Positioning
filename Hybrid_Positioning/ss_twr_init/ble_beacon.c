#include "nrf.h"
#include <string.h>
#include <stdint.h>

#define COMPANY_ID_LSB 0x59
#define COMPANY_ID_MSB 0x00

static uint8_t adv_pdu[40];
static uint8_t g_dev_id = 0;

static const uint8_t ble_freqs[3]  = { 2, 26, 80 };
static const uint8_t ble_ch_idx[3] = { 37, 38, 39 };

static void hfclk_start_timeout(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    for (volatile int t = 0; t < 2000000; t++) {
        if (NRF_CLOCK->EVENTS_HFCLKSTARTED) break;
    }
}
static void ble_radio_config(void)
{
    hfclk_start_timeout();

    NRF_RADIO->POWER   = 1;
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm;
    NRF_RADIO->MODE    = RADIO_MODE_MODE_Ble_1Mbit;

    NRF_RADIO->PREFIX0     = 0x8E;
    NRF_RADIO->BASE0       = 0x89BED600;
    NRF_RADIO->TXADDRESS   = 0x00;
    NRF_RADIO->RXADDRESSES = (1 << 0);

    
    NRF_RADIO->PCNF0 =
        (1  << RADIO_PCNF0_S0LEN_Pos) |
        (8  << RADIO_PCNF0_LFLEN_Pos) |
        (0  << RADIO_PCNF0_S1LEN_Pos);

    NRF_RADIO->PCNF1 =
        (37 << RADIO_PCNF1_MAXLEN_Pos) |
        (0  << RADIO_PCNF1_STATLEN_Pos) |
        (3  << RADIO_PCNF1_BALEN_Pos) |
        (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
        (1  << RADIO_PCNF1_WHITEEN_Pos);

    NRF_RADIO->CRCCNF =
        (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos) |
        (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);

    NRF_RADIO->CRCINIT = 0x555555;
    NRF_RADIO->CRCPOLY = 0x00065B;
}
static void build_adv_manuf(const uint8_t *data, uint8_t len)
{
    
    uint8_t p = 0;
    adv_pdu[p++] = 0x42;
    adv_pdu[p++] = 0x00;

    adv_pdu[p++] = 0xC0;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = g_dev_id;

    if (len > 24) len = 24;
    adv_pdu[p++] = (uint8_t)(len + 3);
    adv_pdu[p++] = 0xFF;
    adv_pdu[p++] = COMPANY_ID_LSB;
    adv_pdu[p++] = COMPANY_ID_MSB;

    memcpy(&adv_pdu[p], data, len);
    p += len;


    adv_pdu[1] = (uint8_t)(p - 2);
}
static void ble_raw_beacon_send_index(uint8_t idx)
{
    NRF_RADIO->FREQUENCY   = ble_freqs[idx];
    NRF_RADIO->DATAWHITEIV = ble_ch_idx[idx];
    NRF_RADIO->PACKETPTR   = (uint32_t)adv_pdu;

    NRF_RADIO->EVENTS_READY    = 0;
    NRF_RADIO->EVENTS_END      = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;

    NRF_RADIO->TASKS_TXEN = 1;
    for (volatile int t = 0; t < 200000; t++) {
        if (NRF_RADIO->EVENTS_READY) break;
    }
    if (!NRF_RADIO->EVENTS_READY) {
        NRF_RADIO->TASKS_DISABLE = 1;
        return;
    }

    NRF_RADIO->TASKS_START = 1;
    for (volatile int t = 0; t < 300000; t++) {
        if (NRF_RADIO->EVENTS_END) break;
    }

    NRF_RADIO->TASKS_DISABLE = 1;
    for (volatile int t = 0; t < 200000; t++) {
        if (NRF_RADIO->EVENTS_DISABLED) break;
    }
}

void ble_raw_beacon_init(uint8_t dev_id)
{
    static uint8_t inited = 0;
    g_dev_id = dev_id;

    if (!inited) {
        ble_radio_config();
        inited = 1;
    }

    uint8_t payload = dev_id;
    build_adv_manuf(&payload, 1);
}
void ble_raw_beacon_broadcast(void)
{
    for (uint8_t i = 0; i < 3; i++) {
        ble_raw_beacon_send_index(i);
    }
}
void ble_raw_beacon_send_payload(const uint8_t *data, uint8_t len)
{
    build_adv_manuf(data, len);
    ble_raw_beacon_broadcast();
}

void ble_beacon_send_geometry(uint8_t target_id, float x, float y)
{
    uint8_t payload[10];
    payload[0] = 'G';
    payload[1] = target_id;
    memcpy(&payload[2], &x, 4);
    memcpy(&payload[6], &y, 4);

    ble_raw_beacon_send_payload(payload, 10);
}

void ble_beacon_send_tag_pos(uint8_t id, uint8_t seq, uint32_t timestamp, float x, float y)
{
    uint8_t payload[15];
    
    payload[0] = '{';
    payload[1] = id;
    payload[2] = seq;
    
    memcpy(&payload[3], &timestamp, 4);
    memcpy(&payload[7], &x, 4);
    memcpy(&payload[11], &y, 4);

    ble_raw_beacon_send_payload(payload, 15);
}

// === THÊM MỚI ===
void ble_beacon_send_presence(uint8_t id, float x, float y)
{
    uint8_t payload[10];
    payload[0] = 'P'; // Magic byte 'P': Presence (Sự hiện diện)
    payload[1] = id;
    
    // Đính kèm luôn tọa độ của Anchor để Tag đỡ phải nhớ
    memcpy(&payload[2], &x, 4);
    memcpy(&payload[6], &y, 4);

    // Tận dụng hàm gửi broadcast sẵn có
    ble_raw_beacon_send_payload(payload, 10);
}
// ================