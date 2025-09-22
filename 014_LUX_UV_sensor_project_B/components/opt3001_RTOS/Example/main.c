/*
 * main.c
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "opt3001_RTOS.h"

static const char *TAG = "MAIN";

// Queue for sensor readings
QueueHandle_t sensor_queue;

// Task function prototypes
void sensor_read_task(void *pvParameters);
void data_process_task(void *pvParameters);

void app_main(void) {
    ESP_LOGI(TAG, "Starting OPT3001 FreeRTOS application");
    
    // Create queue for sensor data
    sensor_queue = xQueueCreate(10, sizeof(opt3001_reading_t));
    if (sensor_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create sensor queue");
        return;
    }

    // Configure OPT3001 device
    opt3001_dev_t opt3001 = {
        .i2c_port = I2C_NUM_0,
        .i2c_address = OPT3001_I2C_ADDRESS_DEFAULT,
        .sda_pin = GPIO_NUM_5,
        .scl_pin = GPIO_NUM_6,
        .i2c_freq_hz = 100000,
        .i2c_mutex = NULL,
        .initialized = false
    };

    // Initialize sensor
    opt3001_err_t err = opt3001_init(&opt3001);
    if (err != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize OPT3001");
        vQueueDelete(sensor_queue);
        return;
    }

    // Configure for continuous measurement
    err = opt3001_configure_continuous(&opt3001);
    if (err != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure OPT3001");
        opt3001_deinit(&opt3001);
        vQueueDelete(sensor_queue);
        return;
    }

    // Create tasks
    xTaskCreate(sensor_read_task, "sensor_read", 4096, &opt3001, 5, NULL);
    xTaskCreate(data_process_task, "data_process", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG, "Application tasks started");
}

void sensor_read_task(void *pvParameters) {
    opt3001_dev_t *dev = (opt3001_dev_t *)pvParameters;
    opt3001_reading_t reading;
    
    while (1) {
        // Read sensor data
        opt3001_err_t err = opt3001_read_result(dev, &reading);
        
        if (err == OPT3001_SUCCESS) {
            // Send reading to processing task
            if (xQueueSend(sensor_queue, &reading, pdMS_TO_TICKS(100)) != pdPASS) {
                ESP_LOGW(TAG, "Queue full, dropping reading");
            }
        } else {
            ESP_LOGE(TAG, "Failed to read sensor: %d", err);
        }

        // Task delay - adjust based on your needs
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void data_process_task(void *pvParameters) {
    opt3001_reading_t reading;
    
    while (1) {
        // Wait for sensor data
        if (xQueueReceive(sensor_queue, &reading, portMAX_DELAY) == pdPASS) {
            // Process the reading
            ESP_LOGI(TAG, "Light: %.2f lux (Raw: 0x%04X)", reading.lux, reading.raw.raw_data);
            
            // Add your processing logic here:
            if (reading.lux < 10.0) {
                ESP_LOGW(TAG, "Low light condition detected");
            } else if (reading.lux > 1000.0) {
                ESP_LOGW(TAG, "High light condition detected");
            }
        }
    }
}

