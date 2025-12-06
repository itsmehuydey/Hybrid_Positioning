#include "sdk_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
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
#include "app_util_platform.h"
#include "app_error.h"
#include <string.h>
#include "port_platform.h"
#include "deca_types.h"
#include "deca_param_types.h"
#include "deca_regs.h"
#include "deca_device_api.h"
#include "uart.h"
#include "hybrid_scalable.h"
#define SIMULATION_MODE
#include "simulation.c"

#ifndef NODE_ID
#define NODE_ID 1
#warning "NODE_ID not defined, defaulting to 1 (Tag)"
#endif


// Tọa độ các anchor (chỉ dùng cho Tag)
#if NODE_ID == 1
vec3 anc[N_ANCHORS] = {
    {0.0, 0.0, 2.5},  
    {2.0, 0.0, 2.5},
    {2.0, 1.0, 2.5},
    {0.0, 1.0, 2.5}
};

vec3 pos_est = {0, 0, 1.0}; 
#endif

// Cấu hình DW1000
static dwt_config_t config = {
    5,                     // Channel
    DWT_PRF_64M,           // Pulse repetition frequency
    DWT_PLEN_128,          // Preamble length
    DWT_PAC8,              // Preamble acquisition chunk size
    10, 10,                // Tx and Rx preamble codes
    0,                     // Non-standard SFD
    DWT_BR_6M8,            // Data rate
    DWT_PHRMODE_STD,       // PHY header mode
    (129 + 8 - 8)          // SFD length
};

#define PRE_TIMEOUT 1000
#define POLL_TX_TO_RESP_RX_DLY_UUS 100
#define TX_ANT_DLY 16300
#define RX_ANT_DLY 16456

#define TASK_DELAY 200
#define TIMER_PERIOD 2000

// Task handles
TaskHandle_t led_task_handle = NULL;
TaskHandle_t uwb_task_handle = NULL;
TimerHandle_t led_timer_handle = NULL;

// Hàm LED nhấp nháy
static void led_toggle_task_function(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    while (true)
    {
        LEDS_INVERT(BSP_LED_0_MASK);
        vTaskDelay(TASK_DELAY);
    }
}

static void led_toggle_timer_callback(void *pvParameter)
{
    UNUSED_PARAMETER(pvParameter);
    LEDS_INVERT(BSP_LED_1_MASK);
}

// Forward declarations
extern void ss_initiator_task_function(void *pvParameter);
extern void ss_responder_task_function(void *pvParameter);

int main(void)
{
    // === Khởi tạo LED ===
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
    LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);

    // === Tạo task LED ===
    xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_task_handle);
    led_timer_handle = xTimerCreate("LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    xTimerStart(led_timer_handle, 0);

    // === Cấu hình UART ===
    boUART_Init();
    printf("\r\n=== UWB Hybrid Localization System ===\r\n");
    printf("Node ID: %d ", NODE_ID);
#if NODE_ID == 1
    printf("(TAG - Initiator)\r\n");
#else
    printf("(ANCHOR %d)\r\n", NODE_ID - 2);
#endif

    // === Cấu hình ngắt DW1000 ===
    nrf_gpio_cfg_input(DW1000_IRQ, NRF_GPIO_PIN_NOPULL);

    // === Khởi tạo DW1000 ===
    reset_DW1000();
    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
    {
        printf("ERROR: DW1000 init failed!\r\n");
        while (1) { LEDS_INVERT(BSP_LED_2_MASK); vTaskDelay(100); }
    }

    port_set_dw1000_fastrate();
    dwt_configure(&config);
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(0);

#ifdef SIMULATION_MODE
    printf("[SIMULATION MODE] Running virtual UWB measurements...\r\n");
    while (1) {
        simulate_measurement_cycle();
        nrf_delay_ms(1000);
    }
#else

    // === Tạo task UWB phù hợp với NODE_ID ===
#if NODE_ID == 1
    // Tag: chạy initiator
    xTaskCreate(ss_initiator_task_function, "UWB_INIT", configMINIMAL_STACK_SIZE + 300, NULL, 3, &uwb_task_handle);
    printf("Starting as TAG (Initiator)...\r\n");
#else
    // Anchor: chạy responder
    xTaskCreate(ss_responder_task_function, "UWB_RESP", configMINIMAL_STACK_SIZE + 200, NULL, 3, &uwb_task_handle);
    printf("Starting as ANCHOR (Responder)...\r\n");
#endif

    // === Bắt đầu FreeRTOS ===
    vTaskStartScheduler();

    // Không bao giờ đến đây
    while (1) {}
#endif
}