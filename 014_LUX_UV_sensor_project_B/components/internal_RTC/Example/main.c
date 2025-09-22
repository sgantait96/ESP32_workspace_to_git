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
#include "opt3001.h"
#include "rtc.h"
#include "wifi_manager.h"  // We'll create this later

static const char *TAG = "MAIN";

// Global devices
opt3001_dev_t g_opt3001;
rtc_dev_t g_rtc;
QueueHandle_t g_sensor_queue;

void sensor_read_task(void *pvParameters);
void data_process_task(void *pvParameters);
void rtc_sync_task(void *pvParameters);

void app_main(void) {
    ESP_LOGI(TAG, "Starting OPT3001 + RTC FreeRTOS application");
    
    // Initialize RTC first
    ESP_LOGI(TAG, "Initializing RTC...");
    rtc_err_t rtc_err = rtc_init(&g_rtc);
    if (rtc_err != RTC_SUCCESS) {
        ESP_LOGE(TAG, "Failed to initialize RTC: %d", rtc_err);
        return;
    }

    // Set timezone (adjust for your location)
    rtc_set_timezone(&g_rtc, "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00");

   
    // Create tasks
    xTaskCreate(rtc_sync_task, "rtc_sync", 4096, NULL, 3, NULL);

    ESP_LOGI(TAG, "Application tasks started");

    // Display initial time
    char time_str[64];
    char date_str[64];
    rtc_get_formatted_time(time_str, sizeof(time_str));
    rtc_get_formatted_date(date_str, sizeof(date_str));
    ESP_LOGI(TAG, "Current time: %s %s", date_str, time_str);
}



void rtc_sync_task(void *pvParameters) {
    // Wait a bit for other services to start
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    while (1) {
        // Try to sync time every hour
        ESP_LOGI(TAG, "Attempting SNTP synchronization...");
        
        // Start SNTP (WiFi must be connected first - we'll add this later)
        rtc_sntp_start(&g_rtc);
        
        // Wait for sync with timeout
        rtc_err_t sync_err = rtc_sntp_sync(&g_rtc, pdMS_TO_TICKS(30000));
        if (sync_err == RTC_SUCCESS) {
            char time_str[64];
            char date_str[64];
            rtc_get_formatted_time(time_str, sizeof(time_str));
            rtc_get_formatted_date(date_str, sizeof(date_str));
            ESP_LOGI(TAG, "Time synchronized: %s %s", date_str, time_str);
        } else {
            ESP_LOGW(TAG, "SNTP synchronization failed");
        }
        
        // Stop SNTP to save resources
        rtc_sntp_stop(&g_rtc);
        
        // Wait 1 hour before next sync attempt
        vTaskDelay(pdMS_TO_TICKS(3600000));
    }
}


