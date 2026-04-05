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

int ble_scan_for_config(web_config_t *out_config)
{
    // Lấy 2 byte cuối MAC address của mạch nRF52
    uint16_t my_mac16 = (uint16_t)(NRF_FICR->DEVICEADDR[0] & 0xFFFF);

    for (uint8_t ch = 0; ch < 3; ch++)
    {
        NRF_RADIO->FREQUENCY   = ble_freqs[ch];
        NRF_RADIO->DATAWHITEIV = ble_ch_idx[ch];
        NRF_RADIO->PACKETPTR   = (uint32_t)scan_buf;

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

        // Quét trong 1 khoảng thời gian
        for (volatile int i = 0; i < 5000000; i++)
        {
            if (NRF_RADIO->EVENTS_END)
            {
                NRF_RADIO->EVENTS_END = 0;
                uint8_t pdu_len = scan_buf[1];
                
                if (pdu_len >= 6) {
                    uint8_t *p = scan_buf + 8;
                    uint8_t remain = pdu_len - 6;

                    while (remain > 0)
                    {
                        uint8_t fl = p[0];
                        if (fl == 0 || fl + 1 > remain) break;

                        // Tìm Manufacturer Data (0xFF) và Nordic CID (0x0059)
                        if (p[1] == 0xFF && p[2] == (TARGET_CID & 0xFF) && p[3] == (TARGET_CID >> 8))
                        {
                            uint8_t payload_len = fl - 3;
                            
                            // Kiểm tra kích thước gói cấu hình (5 byte)
                            if (payload_len >= sizeof(web_config_t)) {
                                web_config_t *received_cfg = (web_config_t*)&p[4];

                                // Kiểm tra Magic Byte 'C' và MAC
                                if (received_cfg->magic_byte == 'C' && 
                                   (received_cfg->target_mac == my_mac16 || received_cfg->target_mac == 0xFFFF)) 
                                {
                                    memcpy(out_config, received_cfg, sizeof(web_config_t));
                                    NRF_RADIO->TASKS_DISABLE = 1;
                                    return 1; // Nhan thanh cong
                                }
                            }
                        }
                        remain -= (fl + 1);
                        p += (fl + 1);
                    }
                }
                break;
            }
        }
        NRF_RADIO->TASKS_DISABLE = 1;
    }
    return 0;
}

// Hàm gốc để không lỗi các chỗ khác
int ble_scan_packet(uint8_t *out, uint16_t *out_len)
{
    for (uint8_t ch = 0; ch < 3; ch++)
    {
        NRF_RADIO->FREQUENCY   = ble_freqs[ch];
        NRF_RADIO->DATAWHITEIV = ble_ch_idx[ch];
        NRF_RADIO->PACKETPTR   = (uint32_t)scan_buf;

        NRF_RADIO->EVENTS_READY = 0;
        NRF_RADIO->EVENTS_END   = 0;

        NRF_RADIO->TASKS_RXEN = 1;
        for (volatile int t = 0; t < 20000; t++)
            if (NRF_RADIO->EVENTS_READY) break;

        if (!NRF_RADIO->EVENTS_READY) {
            NRF_RADIO->TASKS_DISABLE = 1;
            continue;
        }

        NRF_RADIO->TASKS_START = 1;

        for (volatile int t = 0; t < 200000; t++)
        {
            if (NRF_RADIO->EVENTS_END)
            {
                NRF_RADIO->EVENTS_END = 0;

                uint8_t pdu_len = scan_buf[1];
                uint8_t *p = scan_buf + 8;
                uint8_t remain = pdu_len - 6;

                while (remain > 0)
                {
                    uint8_t fl = p[0];
                    if (fl == 0 || fl + 1 > remain) break;

                    if (p[1] == 0xFF)
                    {
                        uint16_t cid = p[2] | (p[3] << 8);
                        if (cid == TARGET_CID)
                        {
                            uint8_t plen = fl - 3;
                            memcpy(out, &p[4], plen);
                            *out_len = plen;
                            NRF_RADIO->TASKS_DISABLE = 1;
                            return 1;
                        }
                    }
                    remain -= (fl + 1);
                    p += (fl + 1);
                }
                break;
            }
        }
        NRF_RADIO->TASKS_DISABLE = 1;
    }
    return 0;
}