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
#include "opt3001.h"

static const char *TAG = "MAIN";

void app_main(void) {
    // Configure OPT3001 device
    opt3001_dev_t opt3001 = {
        .i2c_port = I2C_NUM_0,
        .i2c_address = OPT3001_I2C_ADDRESS_DEFAULT,
        .sda_pin = GPIO_NUM_5,  // Adjust based on your XIAO ESP32S3 pinout
        .scl_pin = GPIO_NUM_6,  // Adjust based on your XIAO ESP32S3 pinout
        .i2c_freq_hz = 100000,
    };

    // Initialize sensor
    opt3001_err_t err = opt3001_init(&opt3001);
    if (err != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize OPT3001");
        return;
    }

    // Configure for continuous measurement
    err = opt3001_configure_continuous(&opt3001);
    if (err != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to configure OPT3001");
        return;
    }

    while (1) {
        opt3001_reading_t reading;
        err = opt3001_read_result(&opt3001, &reading);
        
        if (err == OPT3001_SUCCESS) {
            ESP_LOGI(TAG, "Light: %.2f lux", reading.lux);
        } else {
            ESP_LOGE(TAG, "Failed to read sensor: %d", err);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


