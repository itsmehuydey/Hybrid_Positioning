// ==========================================
// FILE: main.c
// ==========================================
#include "sdk_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "bsp.h"
#include "boards.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_spi.h"
#include "nrf_uart.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf.h"
#include "app_error.h"
#include <string.h>
#include "port_platform.h"
#include "deca_types.h"
#include "deca_param_types.h"
#include "deca_regs.h"
#include "deca_device_api.h"
#include "uart.h"
#include "utils.h"
#include "ble_beacon.h"
#include "ble_scanner.h"
#include "ble_hybrid.h"

static dwt_config_t config = {
    5, DWT_PRF_64M, DWT_PLEN_128, DWT_PAC8, 10, 10, 0, DWT_BR_6M8, DWT_PHRMODE_STD, (129 + 8 - 8)
};

#define POLL_TX_TO_RESP_RX_DLY_UUS 100
#define TX_ANT_DLY 16300
#define RX_ANT_DLY 16456
#define TASK_DELAY 200
#define TIMER_PERIOD 2000

TaskHandle_t led_task_handle = NULL;
TaskHandle_t uwb_task_handle = NULL;
TimerHandle_t led_timer_handle = NULL;
SemaphoreHandle_t radio_mutex = NULL;

extern void ss_initiator_task_function(void *pvParameter);
extern void ss_responder_task_function(void *pvParameter);
extern void set_anchor_config(uint8_t id, float x, float y);
extern void set_tag_config(uint8_t id);

static void led_toggle_task_function(void *pvParameter) {
    UNUSED_PARAMETER(pvParameter);
    while (true) {
        LEDS_INVERT(BSP_LED_0_MASK);
        vTaskDelay(TASK_DELAY);
    }
}

static void led_toggle_timer_callback(void *pvParameter) {
    UNUSED_PARAMETER(pvParameter);
    LEDS_INVERT(BSP_LED_1_MASK);
}

void ble_scan_task(void *pvParameter) {
    web_config_t cfg;
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(3000));
        if (xSemaphoreTake(radio_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            ble_scanner_init();
            if (ble_scan_for_config(&cfg)) {
                if (cfg.magic_byte == 'C' && (cfg.role != NRF_POWER->GPREGRET || cfg.node_id != NRF_POWER->GPREGRET2)) {
                    NRF_POWER->GPREGRET = cfg.role;
                    NRF_POWER->GPREGRET2 = cfg.node_id;
                    NVIC_SystemReset();
                }
            }
            NRF_RADIO->TASKS_DISABLE = 1;
            xSemaphoreGive(radio_mutex);
        }
    }
}

int main(void)
{
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
    LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);

    uint16_t my_mac16 = (uint16_t)(NRF_FICR->DEVICEADDR[0] & 0xFFFF);
    uint8_t saved_role = NRF_POWER->GPREGRET;
    uint8_t saved_id = NRF_POWER->GPREGRET2;

    printf("\r\n=======================================\r\n");
    printf("MAC: %04X | ROLE: %d | ID: %d\r\n", my_mac16, saved_role, saved_id);
    printf("=======================================\r\n");

    nrf_gpio_cfg_input(DW1000_IRQ, NRF_GPIO_PIN_NOPULL);
    reset_DW1000();
    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        while (1) { LEDS_INVERT(BSP_LED_2_MASK); nrf_delay_ms(100); }
    }

    port_set_dw1000_fastrate();
    dwt_configure(&config);
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(0);

    if (saved_role != 1 && saved_role != 2) {
        uint32_t counter = 0;
        web_config_t cfg;
        
        while(1) {
            if (counter % 50 == 0) {
                printf("{\"mac\":\"%04X\",\"role\":\"unassigned\"}\r\n", my_mac16);
                
                #pragma pack(push, 1)
                typedef struct {
                    uint8_t start_byte;
                    uint8_t mac_h;
                    uint8_t mac_l;
                    uint8_t pad1;
                    uint8_t pad2;
                } ble_mac_pkt_t;
                #pragma pack(pop)

                ble_mac_pkt_t pkt;
                pkt.start_byte = 'U';
                pkt.mac_h = (my_mac16 >> 8) & 0xFF;
                pkt.mac_l = my_mac16 & 0xFF;
                pkt.pad1 = 0;
                pkt.pad2 = 0;
                
                ble_raw_beacon_init(0);
                ble_raw_beacon_send_payload((uint8_t *)&pkt, sizeof(pkt));
            }

            ble_scanner_init();
            if (ble_scan_for_config(&cfg)) {
                if (cfg.magic_byte == 'C') {
                    NRF_POWER->GPREGRET = cfg.role;
                    NRF_POWER->GPREGRET2 = cfg.node_id;
                    NVIC_SystemReset();
                }
            }
            nrf_delay_ms(10);
            counter++;
        }
    }

    radio_mutex = xSemaphoreCreateMutex();

    xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_task_handle);
    led_timer_handle = xTimerCreate("LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    xTimerStart(led_timer_handle, 0);

    xTaskCreate(ble_scan_task, "BLE_SCAN", configMINIMAL_STACK_SIZE + 200, NULL, 1, NULL);

    if (saved_role == 1) {
        set_tag_config(saved_id);
        xTaskCreate(ss_initiator_task_function, "UWB_TAG", configMINIMAL_STACK_SIZE + 300, NULL, 3, &uwb_task_handle);
    } 
    else if (saved_role == 2) {
        float ax = 0.0f, ay = 0.0f;
        if (saved_id == 0)      { ax = 0.0f; ay = 0.0f; }
        else if (saved_id == 1) { ax = 1.0f; ay = 0.0f; }
        else if (saved_id == 2) { ax = 0.0f; ay = 2.0f; }
        else                    { ax = 1.0f; ay = 2.0f; }

        set_anchor_config(saved_id, ax, ay);
        
        if (saved_id == 0) {
            master_hybrid_init();
            master_hybrid_reset();
        }

        xTaskCreate(ss_responder_task_function, "UWB_ANC", configMINIMAL_STACK_SIZE + 200, NULL, 3, &uwb_task_handle);
    }

    vTaskStartScheduler();
    while (1) {}
}