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
#include <string.h>
#include <stdio.h>
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

// ===============================================================
// THÊM: BIẾN TOÀN CỤC CHO ROLE ĐỘNG & LƯU FLASH
// ===============================================================
uint8_t g_current_role = 0;
uint8_t g_current_node_id = 0;
uint16_t g_my_mac = 0;

#define CONFIG_FLASH_ADDR 0x0007E000 // Sử dụng Page 126 ở cuối bộ nhớ Flash

void flash_config_write(uint8_t role, uint8_t id) {
    NRF_NVMC->CONFIG = 2; // Bật chế độ XÓA (ERASE)
    NRF_NVMC->ERASEPAGE = CONFIG_FLASH_ADDR;
    while(NRF_NVMC->READY == 0);
    
    NRF_NVMC->CONFIG = 1; // Bật chế độ GHI (WRITE)
    uint32_t magic_word = 0xDEADBEEF;
    uint32_t config_data = (id << 8) | role;
    
    ((uint32_t*)CONFIG_FLASH_ADDR)[0] = magic_word;
    ((uint32_t*)CONFIG_FLASH_ADDR)[1] = config_data;
    while(NRF_NVMC->READY == 0);
    
    NRF_NVMC->CONFIG = 0; // Trả về chế độ ĐỌC (READ)
}
// ===============================================================

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

extern void ss_initiator_task_function(void *pvParameter);
extern void ss_responder_task_function(void *pvParameter);

int main(void)
{
    // === Khởi tạo LED ===
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
    LEDS_ON(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);

    // === TẠO MAC & ĐỌC FLASH TÌM CẤU HÌNH ===
    g_my_mac = (uint16_t)(NRF_FICR->DEVICEADDR[0] & 0xFFFF);
    uint32_t *flash_ptr = (uint32_t*)CONFIG_FLASH_ADDR;

    if (flash_ptr[0] == 0xDEADBEEF) {
        g_current_role = (uint8_t)(flash_ptr[1] & 0xFF);
        g_current_node_id = (uint8_t)((flash_ptr[1] >> 8) & 0xFF);
    }

    printf("\r\n=== UWB Hybrid Localization System ===\r\n");
    
    // === CHỜ CẤU HÌNH BLE ===
    ble_raw_beacon_init(0);
    ble_scanner_init();

    uint32_t config_timeout = (g_current_role == 0) ? 0xFFFFFFFF : 5; // Vô hạn nếu chưa có role, 5 nhịp (vài giây) nếu đã có
    uint8_t scan_buf[64];
    uint16_t scan_len;
    
    if (g_current_role == 0) {
        printf("Chua co Role! Phat BLE MAC: %04X va cho lenh...\r\n", g_my_mac);
    } else {
        printf("Da co Role: %d, ID: %d (MAC: %04X). Cho 3s xem co doi Role khong...\r\n", g_current_role, g_current_node_id, g_my_mac);
    }

    while (config_timeout > 0) {
        if (g_current_role == 0) {
            // Phát JSON chứa MAC và Role 0 cho máy tính nhận diện
            char json_mac[30];
            snprintf(json_mac, sizeof(json_mac), "{\"mac\":%u,\"role\":0}", g_my_mac);
            ble_raw_beacon_send_payload((uint8_t*)json_mac, strlen(json_mac));
        }

        if (ble_scan_packet(scan_buf, &scan_len)) {
            // Kiểm tra format từ Python: 0x43 (Header) + MAC (2 byte) + Role (1) + ID (1)
            if (scan_len >= 5 && scan_buf[0] == 0x43) {
                uint16_t target_mac;
                memcpy(&target_mac, &scan_buf[1], 2);
                
                if (target_mac == g_my_mac) {
                    uint8_t new_role = scan_buf[3];
                    uint8_t new_id = scan_buf[4];
                    printf("\r\n=> NHAN LENH DOI ROLE! Role moi: %d, ID: %d. Resetting...\r\n", new_role, new_id);
                    flash_config_write(new_role, new_id);
                    nrf_delay_ms(500);
                    NVIC_SystemReset();
                }
            }
        }
        if (g_current_role != 0) config_timeout--;
    }

    // === IN RA VAI TRÒ CHÍNH THỨC ===
    printf("Role hien tai chay: Role %d, Node ID: %d\r\n", g_current_role, g_current_node_id);

    // === Tạo task LED ===
    xTaskCreate(led_toggle_task_function, "LED0", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_task_handle);
    led_timer_handle = xTimerCreate("LED1", TIMER_PERIOD, pdTRUE, NULL, led_toggle_timer_callback);
    xTimerStart(led_timer_handle, 0);

    // === Cấu hình ngắt DW1000 ===
    nrf_gpio_cfg_input(DW1000_IRQ, NRF_GPIO_PIN_NOPULL);

    // === Khởi tạo DW1000 ===
    reset_DW1000();
    port_set_dw1000_slowrate();

    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
    {
        printf("ERROR: DW1000 init failed!\r\n");
        while (1) { LEDS_INVERT(BSP_LED_2_MASK); nrf_delay_ms(100); }
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

    // SỬA Ở ĐÂY: Dựa vào ROLE động để chạy Task. Cho phép Role 99 chạy task của Tag
    if (g_current_role == 1 || g_current_role == 99) { // ROLE 1 = TAG HOẶC ROLE 99 = CALIB
        xTaskCreate(ss_initiator_task_function, "UWB_INIT",
                    configMINIMAL_STACK_SIZE + 300, NULL, 3, &uwb_task_handle);
    } 
    else if (g_current_role == 2) { // ROLE 2 = ANCHOR
        xTaskCreate(ss_responder_task_function, "UWB_RESP",
                    configMINIMAL_STACK_SIZE + 200, NULL, 3, &uwb_task_handle);
    }

    // === Bắt đầu FreeRTOS ===
    vTaskStartScheduler();

    // Không bao giờ đến đây
    while (1) {}
#endif
}