#include "driver/twai.h"  // Include the TWAI (CAN) driver library
#include "esp_log.h"      // Include ESP logging library

#define TAG "CAN_DRIVER"  // Define a tag for logging messages

void app_main() {
    // Configure the CAN driver parameters
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Initialize the CAN driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver");
        return; // Exit the function if driver installation fails
    }

    // Start the CAN driver
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver");
        return; // Exit the function if driver start fails
    }

    // Prepare a message to send
    twai_message_t tx_message;          
    tx_message.identifier = 0x100;     
    tx_message.data_length_code = 8;   
    for (int i = 0; i < 8; i++) {      
        tx_message.data[i] = i;        
    }

    // Transmit the message
    if (twai_transmit(&tx_message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        ESP_LOGI(TAG, "Message transmitted");
    } else {
        ESP_LOGE(TAG, "Failed to transmit message");
    }

    // Receive a message
    twai_message_t rx_message;  // Declare a message structure for receiving
    if (twai_receive(&rx_message, pdMS_TO_TICKS(1000)) == ESP_OK) {
        ESP_LOGI(TAG, "Message received");
        ESP_LOGI(TAG, "ID: 0x%x, DLC: %d, Data:", rx_message.identifier, rx_message.data_length_code);
        for (int i = 0; i < rx_message.data_length_code; i++) {
            ESP_LOGI(TAG, "Data[%d]: 0x%x", i, rx_message.data[i]);
        }
    } else {
        ESP_LOGE(TAG, "Failed to receive message");
    }

    // Stop and uninstall the CAN driver
    twai_stop();                         
    twai_driver_uninstall();             
    ESP_LOGI(TAG, "TWAI driver uninstalled");
}

