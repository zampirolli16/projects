#include <stdio.h>
#include "driver/twai.h"  // Include the TWAI (CAN) driver library
#include "esp_log.h"      // Include ESP logging library
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as5600.h"
#include "esp_timer.h"
#include "math.h"

#define TAG "CAN_DRIVER"  // Define a tag for logging messages

void app_main() {
    // Configure the CAN driver parameters (TX = 4; RX = 5)
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_4, GPIO_NUM_5, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    int tempo = 0;

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

    // Setup Anemometro
    as5600_init();
    float angulo_anterior = as5600_get_angle_degrees();
    int64_t tempo_anterior = esp_timer_get_time();
    int flag = 0;
    int T_media = 20;
    int count_media;
    float vet_media[T_media];
    float rpm_media = 0;

    while (1){
        //lógica anemometro
        flag = 0;
        float angulo_atual = as5600_get_angle_degrees();
        int64_t tempo_atual = esp_timer_get_time();

        
        float delta_angulo = angulo_atual - angulo_anterior;
        if (delta_angulo < -0.1f) {
            delta_angulo += 360.0; // Correção de overflow (ex: 359 → 5 graus)
            flag=1;
        }

        float delta_tempo = (tempo_atual - tempo_anterior) / 1000000.0; // micros → segundos
        float rpm = (delta_angulo / 360.0f) * (60.0f / delta_tempo);
        if (fabs(delta_angulo) < 1.0f) {
        rpm = 0.0f;
        }

        //Filtro média móvel
        if (count_media < T_media){
            vet_media[count_media] = rpm;
            int i = 0;
            rpm_media = 0;
            while (rpm_media < T_media){
                rpm_media = (rpm_media + vet_media[i])/T_media;
                i++;
            }
        }

        //printf("RPM: %.2f\n", rpm);
        //printf("angulo anterior: %.2f\n",angulo_anterior);
        //printf("angulo atual: %.2f\n",angulo_atual);

        
        
        if (flag == 1){
            printf("negativo\n");
        }

        angulo_anterior = angulo_atual;
        tempo_anterior = tempo_atual;
        vTaskDelay (pdMS_TO_TICKS(1));
        tempo++;

        // Prepare a message to send
        if (tempo == 100){
            int rpm_send = (int)round(rpm);
            twai_message_t anemometro;          
            anemometro.identifier = 0x001;     
            anemometro.data_length_code = 1;       
            anemometro.data[0] = rpm_send;  
            
            printf ("rpm mensagem: %d\n", anemometro.data[0]);

            // Transmit the message
            if (twai_transmit(&anemometro, pdMS_TO_TICKS(1)) == ESP_OK) {
                ESP_LOGI(TAG, "Message transmitted");
            } else {
                ESP_LOGE(TAG, "Failed to transmit message");
            }

            tempo = 0;
        }
        
    }

    // Stop and uninstall the CAN driver
    twai_stop();                         
    twai_driver_uninstall();             
    ESP_LOGI(TAG, "TWAI driver uninstalled");
}

