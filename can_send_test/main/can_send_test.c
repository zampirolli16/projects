#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/twai.h"  // TWAI is the CAN controller used in ESP32

#define TX_GPIO_NUM   5
#define RX_GPIO_NUM   4

void app_main(void) {
    // Configure the CAN interface
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS(); // 500 kbps
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    printf("Initializing CAN driver...\n");

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        printf("Failed to install CAN driver\n");
        return;
    }

    if (twai_start() != ESP_OK) {
        printf("Failed to start CAN driver\n");
        return;
    }

    printf("CAN Sender Ready\n");

    while (1) {
        // Send standard frame
        twai_message_t message = {
            .identifier = 0x12,
            .data_length_code = 5,
            .data = {'h', 'e', 'l', 'l', 'o'},
            .flags = TWAI_MSG_FLAG_NONE
        };

        if (twai_transmit(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
            printf("Standard CAN message sent\n");
        } else {
            printf("Failed to send standard message\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));

        // Send extended frame
        twai_message_t ext_message = {
            .identifier = 0xABCDEF,
            .data_length_code = 5,
            .data = {'w', 'o', 'r', 'l', 'd'},
            .flags = TWAI_MSG_FLAG_EXTD
        };

        if (twai_transmit(&ext_message, pdMS_TO_TICKS(1000)) == ESP_OK) {
            printf("Extended CAN message sent\n");
        } else {
            printf("Failed to send extended message\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
