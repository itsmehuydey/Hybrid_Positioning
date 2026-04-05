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
//#define SIMULATION_MODE
#include "simulation.c"

// Cấu hình DW1000
static dwt_config_t config = {
    5, DWT_PRF_64M, DWT_PLEN_128, DWT_PAC8, 10, 10, 0, DWT_BR_6M8, DWT_PHRMODE_STD, (129 + 8 - 8)
};

#define PRE_TIMEOUT 1000
#define POLL_TX_TO_RESP_RX_DLY_UUS 100
#define TX_ANT_DLY 16300
#define RX_ANT_DLY 16456

#define TASK_DELAY 200
#define TIMER_PERIOD 2000

TaskHandle_t led_task_handle = NULL;
TaskHandle_t uwb_task_handle = NULL;
TimerHandle_t led_timer_handle = NULL;

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

// Khai báo hàm cấu hình từ các file khác
extern void ss_initiator_task_function(void *pvParameter);
extern void ss_responder_task_function(void *pvParameter);
extern void set_anchor_config(uint8_t id, float x, float y);
extern void set_tag_config(uint8_t id);

int main(void)
{
    // === Khởi tạo LED ===
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
    LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);

    // === Cấu hình UART ===
    // boUART_Init();

    // Lấy MAC Address của vi điều khiển
    uint16_t my_mac16 = (uint16_t)(NRF_FICR->DEVICEADDR[0] & 0xFFFF);

    printf("\r\n=======================================\r\n");
    printf("=== UWB Hybrid Localization System ===\r\n");
    printf("MAC Address mach nay: %04X\r\n", my_mac16);
    printf("Dang cho cau hinh tu Web...\r\n");
    printf("=======================================\r\n");

    // Khởi tạo bộ quét BLE để bắt lệnh từ Web
    ble_scanner_init();

    web_config_t my_config;
    
    // Vòng lặp vô hạn chờ lệnh (Không chạy UWB nếu chưa có lệnh)
    while(1) {
        if (ble_scan_for_config(&my_config)) {
            printf("\r\n>> DA NHAN LENH CAU HINH <<\r\n");
            printf("Magic: %c | MAC: %04X | Role: %d | ID: %d\r\n", 
                   my_config.magic_byte, my_config.target_mac, my_config.role, my_config.node_id);
            break; // Thoát vòng lặp chờ
        }
        nrf_delay_ms(10); // Chờ 10ms rồi quét tiếp
    }

    // Tắt Radio sau khi nhận xong để nhường chỗ cho UWB/BLE Beacon
    NRF_RADIO->TASKS_DISABLE = 1;

    // === Tạo task LED ===
    xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_task_handle);
    led_timer_handle = xTimerCreate("LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    xTimerStart(led_timer_handle, 0);

    // === Cấu hình ngắt DW1000 ===
    nrf_gpio_cfg_input(DW1000_IRQ, NRF_GPIO_PIN_NOPULL);

    // === Khởi tạo DW1000 ===
    reset_DW1000();
    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR) {
        printf("ERROR: DW1000 init failed!\r\n");
        while (1) { LEDS_INVERT(BSP_LED_2_MASK); nrf_delay_ms(100); }
    }

    port_set_dw1000_fastrate();
    dwt_configure(&config);
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
    dwt_setrxtimeout(0);

    // === PHÂN LUỒNG ROLE SAU KHI NHẬN CẤU HÌNH ===
    if (my_config.role == 1) {
        // Khởi động TAG
        printf("KHOI DONG CHE DO: TAG (ID = %d)\r\n", my_config.node_id);
        set_tag_config(my_config.node_id);
        xTaskCreate(ss_initiator_task_function, "UWB_INIT", configMINIMAL_STACK_SIZE + 300, NULL, 3, &uwb_task_handle);
    } 
    else if (my_config.role == 2) {
        // Khởi động ANCHOR
        float ax = 0.0f, ay = 0.0f;
        // Tự động gán tọa độ dựa theo ID (Giống macro if-else cũ)
        if (my_config.node_id == 0)      { ax = 0.0f; ay = 0.0f; } // Góc dưới trái
        else if (my_config.node_id == 1) { ax = 1.0f; ay = 0.0f; } // Góc dưới phải
        else if (my_config.node_id == 2) { ax = 0.0f; ay = 2.0f; } // Góc trên trái
        else                             { ax = 1.0f; ay = 2.0f; } // Góc trên phải
        
        printf("KHOI DONG CHE DO: ANCHOR (ID = %d, Toa do: %.1f, %.1f)\r\n", my_config.node_id, ax, ay);
        set_anchor_config(my_config.node_id, ax, ay);

        // Nếu ID = 0, nó là Master Anchor (Kéo theo các init phụ)
        if (my_config.node_id == 0) {
            master_hybrid_init();
            master_hybrid_reset();
            ble_scanner_init(); // Master quét BLE của Tag
        }
        
        xTaskCreate(ss_responder_task_function, "UWB_RESP", configMINIMAL_STACK_SIZE + 200, NULL, 3, &uwb_task_handle);
    }

    // === Bắt đầu FreeRTOS ===
    vTaskStartScheduler();

    while (1) {}
}