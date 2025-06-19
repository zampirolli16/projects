// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as5600.h"
#include "esp_timer.h"
#include "math.h"

void app_main() {
    as5600_init();
    //float zero = 0.00f;
    float angulo_anterior = as5600_get_angle_degrees();
    int64_t tempo_anterior = esp_timer_get_time();
    int flag =0;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(500));
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

        printf("RPM: %.2f\n", rpm);
        printf("angulo anterior: %.2f\n",angulo_anterior);
        printf("angulo atual: %.2f\n",angulo_atual);
        
        if (flag == 1){
            printf("negativo\n");
        }

        angulo_anterior = angulo_atual;
        tempo_anterior = tempo_atual;

    }
}