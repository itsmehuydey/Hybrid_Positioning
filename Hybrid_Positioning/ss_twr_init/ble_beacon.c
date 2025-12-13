#include "nrf.h"
#include <string.h>
#include <stdint.h>

#define COMPANY_ID_LSB 0x59   // 0x0059
#define COMPANY_ID_MSB 0x00

static uint8_t adv_pdu[40];
static uint8_t g_dev_id = 0;

static const uint8_t ble_freqs[3]  = { 2, 26, 80 };    // ch 37,38,39
static const uint8_t ble_ch_idx[3] = { 37, 38, 39 };

//Bật HFCLK (High Frequency Clock) cho NRF_RADIO
static void hfclk_start_timeout(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    for (volatile int t = 0; t < 2000000; t++) {
        if (NRF_CLOCK->EVENTS_HFCLKSTARTED) break;
    }
}
//Cấu hình toàn bộ NRF_RADIO cho BLE 1 Mbps
static void ble_radio_config(void)
{
    hfclk_start_timeout();

    NRF_RADIO->POWER   = 1;
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_0dBm;
    NRF_RADIO->MODE    = RADIO_MODE_MODE_Ble_1Mbit;

    // Access Address ADV: 0x8E89BED6
    NRF_RADIO->PREFIX0     = 0x8E;
    NRF_RADIO->BASE0       = 0x89BED600;   // 0x89BED6 << 8
    NRF_RADIO->TXADDRESS   = 0x00;
    NRF_RADIO->RXADDRESSES = (1 << 0);

    // Packet format
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

    // CRC BLE
    NRF_RADIO->CRCCNF =
        (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos) |
        (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);

    NRF_RADIO->CRCINIT = 0x555555;
    NRF_RADIO->CRCPOLY = 0x00065B;
}
//Tạo BLE ADV PDU hoàn chỉnh
static void build_adv_manuf(const uint8_t *data, uint8_t len)
{
    // PDU: Header(2) + AdvA(6) + AdvData(...)
    // Header: ADV_NONCONN_IND = 0x42
    uint8_t p = 0;
    adv_pdu[p++] = 0x42;
    adv_pdu[p++] = 0x00; // length update later

    // AdvA (6B)
    adv_pdu[p++] = 0xC0;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = 0x00;
    adv_pdu[p++] = g_dev_id;

    // AD Structure: Manufacturer Data
    // field_len counts: type(1) + companyID(2) + payload(len) = len+3
    if (len > 24) len = 24;           // an toàn
    adv_pdu[p++] = (uint8_t)(len + 3);
    adv_pdu[p++] = 0xFF;
    adv_pdu[p++] = COMPANY_ID_LSB;
    adv_pdu[p++] = COMPANY_ID_MSB;

    memcpy(&adv_pdu[p], data, len);
    p += len;

    // LENGTH = AdvA(6) + AdvData(bytes)
    adv_pdu[1] = (uint8_t)(p - 2);
}
//Phát 1 gói BLE ADV trên 1 kênh
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

    // default payload = dev_id
    uint8_t payload = dev_id;
    build_adv_manuf(&payload, 1);
}
//Phát gói BLE trên cả 3 kênh 37–38–39
void ble_raw_beacon_broadcast(void)
{
    for (uint8_t i = 0; i < 3; i++) {
        ble_raw_beacon_send_index(i);
    }
}
//ble_raw_beacon_send_payload
void ble_raw_beacon_send_payload(const uint8_t *data, uint8_t len)
{
    build_adv_manuf(data, len);
    ble_raw_beacon_broadcast();
}
