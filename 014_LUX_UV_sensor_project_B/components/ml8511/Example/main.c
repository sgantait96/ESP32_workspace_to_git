/*
 * main.c
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ML8511.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // Initialize the UV sensor
    ml8511_init();
    
    // Start continuous reading in the background
    ml8511_start_continuous_reading();
    
    // Main loop - you can do other work here
    int counter = 0;
    while(1) {
        // You can add other tasks here
        if (counter % 10 == 0) {
            // Read a single value on demand every 10 seconds
            float uv_intensity = ml8511_read_uv_intensity();
            ESP_LOGI(TAG, "On-demand reading: %.2f mW/cm²", uv_intensity);
        }
        counter++;
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}

