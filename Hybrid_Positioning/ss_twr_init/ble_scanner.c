#include "ble_scanner.h"
#include "nrf.h"
#include <string.h>
#include <stdint.h>

#define TARGET_CID 0x0059

static uint8_t scan_buf[64];

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

void ble_scanner_init(void)
{
    hfclk_start_timeout();

    NRF_RADIO->POWER = 1;
    NRF_RADIO->MODE  = RADIO_MODE_MODE_Ble_1Mbit;

    // Access Address ADV: 0x8E89BED6
    NRF_RADIO->PREFIX0     = 0x8E;
    NRF_RADIO->BASE0       = 0x89BED600;
    NRF_RADIO->TXADDRESS   = 0x00;
    NRF_RADIO->RXADDRESSES = (1 << 0);

    NRF_RADIO->PCNF0 =
        (1 << RADIO_PCNF0_S0LEN_Pos) |
        (8 << RADIO_PCNF0_LFLEN_Pos);

    NRF_RADIO->PCNF1 =
        (37 << RADIO_PCNF1_MAXLEN_Pos) |
        (3  << RADIO_PCNF1_BALEN_Pos) |
        (RADIO_PCNF1_ENDIAN_Little << RADIO_PCNF1_ENDIAN_Pos) |
        (1  << RADIO_PCNF1_WHITEEN_Pos);

    NRF_RADIO->CRCINIT = 0x555555;
    NRF_RADIO->CRCPOLY = 0x00065B;
    NRF_RADIO->CRCCNF  =
        (RADIO_CRCCNF_LEN_Three << RADIO_CRCCNF_LEN_Pos) |
        (RADIO_CRCCNF_SKIPADDR_Skip << RADIO_CRCCNF_SKIPADDR_Pos);

    NRF_RADIO->PACKETPTR = (uint32_t)scan_buf;
}
//Parse AdvData
static int parse_manuf_0059(uint8_t *out_payload0)
{
    // scan_buf layout: [S0][LEN][AdvA(6)][AdvData...]
    uint8_t pdu_len = scan_buf[1];
    if (pdu_len < 6) return 0;

    uint8_t advdata_len = (uint8_t)(pdu_len - 6);
    uint8_t *p = scan_buf + 8;     // skip header(2) + AdvA(6)
    uint8_t remain = advdata_len;

    while (remain > 0)
    {
        uint8_t field_len  = p[0];     // bytes after this len byte
        if (field_len == 0) break;
        if ((uint16_t)field_len + 1 > remain) break;

        uint8_t field_type = p[1];

        if (field_type == 0xFF && field_len >= 4)
        {
            uint16_t cid = (uint16_t)p[2] | ((uint16_t)p[3] << 8);
            if (cid == TARGET_CID)
            {
                *out_payload0 = p[4];  // payload[0]
                return 1;
            }
        }

        remain = (uint8_t)(remain - (field_len + 1));
        p += (field_len + 1);
    }

    return 0;
}
//Quét lần lượt 3 kênh 37–38–39
int ble_scan_for_role(uint8_t *new_role)
{
    for (uint8_t ch = 0; ch < 3; ch++)
    {
        NRF_RADIO->FREQUENCY   = ble_freqs[ch];
        NRF_RADIO->DATAWHITEIV = ble_ch_idx[ch];

        NRF_RADIO->PACKETPTR    = (uint32_t)scan_buf;
        NRF_RADIO->EVENTS_READY = 0;
        NRF_RADIO->EVENTS_END   = 0;

        NRF_RADIO->TASKS_RXEN = 1;
        for (volatile int t = 0; t < 20000; t++) {
            if (NRF_RADIO->EVENTS_READY) break;
        }
        if (!NRF_RADIO->EVENTS_READY) {
            NRF_RADIO->TASKS_DISABLE = 1;
            continue;
        }

        NRF_RADIO->TASKS_START = 1;

        // RX window dài hơn chút để dễ bắt
        for (volatile int i = 0; i < 2000000; i++)
        {
            if (NRF_RADIO->EVENTS_END)
            {
                NRF_RADIO->EVENTS_END = 0;

                uint8_t payload0 = 0;
                if (parse_manuf_0059(&payload0))
                {
                    *new_role = payload0;
                    NRF_RADIO->TASKS_DISABLE = 1;
                    return 1;
                }
                break;
            }
        }

        NRF_RADIO->TASKS_DISABLE = 1;
    }

    return 0;
}
