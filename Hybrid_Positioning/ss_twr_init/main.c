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

#ifndef USER_BUTTON_PIN
#define USER_BUTTON_PIN 2 
#endif

uint8_t g_current_role = 0;
uint8_t g_current_node_id = 0;
uint16_t g_my_mac = 0;

float g_my_pos_x = 0.0f;
float g_my_pos_y = 0.0f;

#define CONFIG_FLASH_ADDR 0x0007E000 

void flash_config_write(uint8_t role, uint8_t id, float x, float y) {
    __disable_irq(); 
    
    NRF_NVMC->CONFIG = 2; 
    NRF_NVMC->ERASEPAGE = CONFIG_FLASH_ADDR;
    while(NRF_NVMC->READY == 0);
    
    NRF_NVMC->CONFIG = 1; 
    uint32_t magic_word = 0xDEADBEEF;
    uint32_t config_data = (id << 8) | role;
    
    uint32_t raw_x, raw_y;
    memcpy(&raw_x, &x, 4);
    memcpy(&raw_y, &y, 4);
    
    ((uint32_t*)CONFIG_FLASH_ADDR)[0] = magic_word;
    ((uint32_t*)CONFIG_FLASH_ADDR)[1] = config_data;
    ((uint32_t*)CONFIG_FLASH_ADDR)[2] = raw_x;
    ((uint32_t*)CONFIG_FLASH_ADDR)[3] = raw_y;
    while(NRF_NVMC->READY == 0);
    
    NRF_NVMC->CONFIG = 0; 
    
    __enable_irq(); 
}

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

// Đã xóa led_timer_handle để tránh xung đột và quy về 1 hàm quản lý LED duy nhất

static void led_toggle_task_function(void *pvParameter) {
    UNUSED_PARAMETER(pvParameter);
    
    // Tắt toàn bộ LED trước khi bắt đầu Task
    LEDS_OFF(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);

    while (true) {
        // Phân loại nháy LED dựa trên Role khi đã vào FreeRTOS
        if (g_current_role == 1 || g_current_role == 99) {
            LEDS_INVERT(BSP_LED_1_MASK);               // TAG nháy LED 1
            LEDS_OFF(BSP_LED_0_MASK | BSP_LED_2_MASK);
        } else if (g_current_role == 2) {
            LEDS_INVERT(BSP_LED_2_MASK);               // ANCHOR nháy LED 2
            LEDS_OFF(BSP_LED_0_MASK | BSP_LED_1_MASK);
        }

        // Xử lý nút bấm Reset Role
        if (nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
            vTaskDelay(50); 
            if (nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
                printf("\r\n=> USER BUTTON PRESSED! Ghi de Role & ID ve 0...\r\n");
                flash_config_write(0, 0, g_my_pos_x, g_my_pos_y);
                while(nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
                    vTaskDelay(10);
                }
                NVIC_SystemReset(); 
            }
        }
        vTaskDelay(TASK_DELAY);
    }
}

extern void ss_initiator_task_function(void *pvParameter);
extern void ss_responder_task_function(void *pvParameter);

 int main(void) {
    LEDS_CONFIGURE(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK);
    LEDS_OFF(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK); // Khởi tạo tắt hết
    
    nrf_gpio_cfg_input(USER_BUTTON_PIN, NRF_GPIO_PIN_PULLUP);

    g_my_mac = (uint16_t)(NRF_FICR->DEVICEADDR[0] & 0xFFFF);
    uint32_t *flash_ptr = (uint32_t*)CONFIG_FLASH_ADDR;

    if (flash_ptr[0] == 0xDEADBEEF) {
        g_current_role = (uint8_t)(flash_ptr[1] & 0xFF);
        g_current_node_id = (uint8_t)((flash_ptr[1] >> 8) & 0xFF);
        memcpy(&g_my_pos_x, &flash_ptr[2], 4);
        memcpy(&g_my_pos_y, &flash_ptr[3], 4);
    }

    printf("\r\n=== UWB Hybrid Localization System ===\r\n");
    
    ble_raw_beacon_init(0);
    ble_scanner_init();

    uint32_t config_timeout = (g_current_role == 0) ? 0xFFFFFFFF : 5; 
    uint8_t scan_buf[64];
    uint16_t scan_len;
    uint32_t unconfig_timer = 0; // Biến đếm thời gian cho Role 0
    
    if (g_current_role == 0) {
        printf("Chua co Role! Phat BLE MAC: %04X moi 5 giay va cho lenh...\r\n", g_my_mac);
    } else {
        printf("Da co Role: %d, ID: %d (MAC: %04X). Cho vai giay xem co doi Role khong...\r\n", g_current_role, g_current_node_id, g_my_mac);
    }

    while (config_timeout > 0) {
        if (nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
            nrf_delay_ms(50); 
            if (nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
                printf("\r\n=> USER BUTTON PRESSED! Resetting Role & ID to 0...\r\n");
                flash_config_write(0, 0, g_my_pos_x, g_my_pos_y);
                while(nrf_gpio_pin_read(USER_BUTTON_PIN) == 0) {
                    nrf_delay_ms(10);
                }
                NVIC_SystemReset();
            }
        }

        if (g_current_role == 0) {
            // unconfig_timer tăng 1 mỗi 10ms.
            // 500 * 10ms = 5000ms (5 giây) gửi tín hiệu BLE 1 lần
            if (unconfig_timer % 500 == 0) {
                uint8_t pending_payload[3];
                pending_payload[0] = 'U'; 
                pending_payload[1] = (uint8_t)(g_my_mac & 0xFF);
                pending_payload[2] = (uint8_t)((g_my_mac >> 8) & 0xFF);
                
                ble_raw_beacon_send_payload(pending_payload, 3);
            }
            
            // 50 * 10ms = 500ms nháy LED 0 một lần để báo hiệu Unconfigured
            if (unconfig_timer % 50 == 0) {
                LEDS_INVERT(BSP_LED_0_MASK);
            }
            LEDS_OFF(BSP_LED_1_MASK | BSP_LED_2_MASK); // Giữ các LED khác tắt
            
            unconfig_timer++;
        }

        if (ble_scan_packet(scan_buf, &scan_len)) {
            if (scan_len >= 5 && scan_buf[0] == 0x43) {
                uint16_t target_mac;
                memcpy(&target_mac, &scan_buf[1], 2);
                
                if (target_mac == g_my_mac) {
                    uint8_t new_role = scan_buf[3];
                    uint8_t new_id = scan_buf[4];
                    printf("\r\n=> NHAN LENH DOI ROLE! Role moi: %d, ID: %d. Resetting...\r\n", new_role, new_id);
                    flash_config_write(new_role, new_id, g_my_pos_x, g_my_pos_y);
                    nrf_delay_ms(500);
                    NVIC_SystemReset();
                }
            }
        }

        if (g_current_role != 0) {
            config_timeout--;
            nrf_delay_ms(1000); 
        } else {
            // Giảm delay xuống 10ms thay vì không có delay, vừa tạo chu kỳ đếm, vừa không lỡ gói BLE Scan
            nrf_delay_ms(10); 
        }
    }

    printf("Role hien tai chay: Role %d, Node ID: %d\r\n", g_current_role, g_current_node_id);

    // Chạy duy nhất Task quản lý LED (đã bỏ led_timer_handle)
    xTaskCreate(led_toggle_task_function, "LED_TASK", configMINIMAL_STACK_SIZE + 200, NULL, 2, &led_task_handle);

    nrf_gpio_cfg_input(DW1000_IRQ, NRF_GPIO_PIN_NOPULL);
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

#ifdef SIMULATION_MODE
    while (1) {
        simulate_measurement_cycle();
        nrf_delay_ms(1000);
    }
#else
    if (g_current_role == 1 || g_current_role == 99) { 
        xTaskCreate(ss_initiator_task_function, "UWB_INIT", configMINIMAL_STACK_SIZE + 300, NULL, 3, &uwb_task_handle);
    } 
    else if (g_current_role == 2) { 
        xTaskCreate(ss_responder_task_function, "UWB_RESP", configMINIMAL_STACK_SIZE + 200, NULL, 3, &uwb_task_handle);
    }

    vTaskStartScheduler();
    while (1) {}
#endif
}