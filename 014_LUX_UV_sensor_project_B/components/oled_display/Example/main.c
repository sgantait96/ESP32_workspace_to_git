/*
 * main.c
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"

#define I2C_SDA     5   // Change as per wiring
#define I2C_SCL     6   // Change as per wiring
//#define OLED_RESET  -1   // Change as per wiring

void app_main(void)
{
    SSD1306_t dev;   // OLED device handle

    // ==== Initialize I2C for OLED ====
    i2c_master_init(&dev, I2C_SDA, I2C_SCL, -1);
    ssd1306_init(&dev, 128, 64);  // Initialize 128x64 OLED

    // ==== Clear screen ====
    ssd1306_clear_screen(&dev, false);

    // ==== Display some sample text ====
    ssd1306_display_text(&dev, 0, "Hello ESP32!", 12, false);
    ssd1306_display_text(&dev, 2, "SSD1306 OLED", 13, false);
    ssd1306_display_text(&dev, 4, "Sample String", 13, false);

    // ==== Keep running ====
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

