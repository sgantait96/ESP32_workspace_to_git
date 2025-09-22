

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "ML8511.h"

// OLED Pin Definitions for XIAO ESP32S3
#define I2C_SDA     5   // D5 on XIAO ESP32S3
#define I2C_SCL     6   // D6 on XIAO ESP32S3

static const char *TAG = "MAIN";
static SSD1306_t dev;   // OLED device handle

// Function to display UV data on OLED
void display_uv_data(float uv_intensity, int uv_raw, int ref_raw)
{
    char line1[32], line2[32], line3[32], line4[32];
    
    // Clear the screen
    ssd1306_clear_screen(&dev, false);
    
    // Create formatted strings
    snprintf(line1, sizeof(line1), "UV Intensity:");
    snprintf(line2, sizeof(line2), "%.2f mW/cm²", uv_intensity);
    snprintf(line3, sizeof(line3), "UV: %d", uv_raw);
    snprintf(line4, sizeof(line4), "REF: %d", ref_raw);
    
    // Display on OLED
    ssd1306_display_text(&dev, 0, line1, strlen(line1), false);
    ssd1306_display_text(&dev, 1, line2, strlen(line2), false);
    ssd1306_display_text(&dev, 3, line3, strlen(line3), false);
    ssd1306_display_text(&dev, 4, line4, strlen(line4), false);
    
    // Add some decorative elements
    ssd1306_display_text(&dev, 6, "------------", 12, false);
    ssd1306_display_text(&dev, 7, "XIAO ESP32S3", 12, false);
}

void app_main(void)
{
    // ==== Initialize I2C for OLED ====
    i2c_master_init(&dev, I2C_SDA, I2C_SCL, -1);
    ssd1306_init(&dev, 128, 64);  // Initialize 128x64 OLED
    
    // ==== Clear screen and show welcome message ====
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "UV Sensor System", 16, false);
    ssd1306_display_text(&dev, 2, "Initializing...", 15, false);
    ssd1306_display_text(&dev, 4, "XIAO ESP32S3", 12, false);
    
    ESP_LOGI(TAG, "OLED Display Initialized");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Show welcome for 2 seconds

    // ==== Initialize the UV sensor ====
    ml8511_init();
    ESP_LOGI(TAG, "UV Sensor Initialized");
    
    // ==== Show sensor ready message ====
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 2, "Sensor Ready!", 13, false);
    ssd1306_display_text(&dev, 4, "Reading data...", 15, false);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // ==== Start continuous reading ====
    ml8511_start_continuous_reading();
    ESP_LOGI(TAG, "Continuous reading started");
    
    // ==== Main loop - read and display data ====
    int counter = 0;
    int display_counter = 0;
    
    while(1) {
        // Read sensor data every second
        int uv_raw, ref_raw;
        float uv_intensity = ml8511_read_uv_intensity();
        ml8511_get_raw_values(&uv_raw, &ref_raw);
        
        // Log to serial
        ESP_LOGI(TAG, "UV: %d, REF: %d, Intensity: %.2f mW/cm²", uv_raw, ref_raw, uv_intensity);
        
        // Update OLED display every 2 seconds to prevent flickering
        if (display_counter % 2 == 0) {
            display_uv_data(uv_intensity, uv_raw, ref_raw);
        }
        
        // Optional: Add different display modes or additional information
        if (counter % 30 == 0) { // Every 30 seconds
            // You could add status messages or cycle through different displays
            ESP_LOGI(TAG, "System running for %d seconds", counter);
        }
        
        counter++;
        display_counter++;
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second
    }
}
