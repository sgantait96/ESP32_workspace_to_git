/*
 * oled_display.h
 *
 *  Created on: Sep 21, 2025
 *      Author: sgant
 */

#ifndef COMPONENTS_OLED_DISPLAY_H_
#define COMPONENTS_OLED_DISPLAY_H_

#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/i2c.h"
#include "ssd1306.h"

/* ==== Default OLED Config (Change here if using another ESP board) ==== */
#define OLED_I2C_PORT      I2C_NUM_0          // Change to I2C_NUM_1 if needed
#define OLED_I2C_ADDRESS   0x3C               // Most SSD1306 use 0x3C or 0x3D
#define OLED_SDA_PIN       GPIO_NUM_21        // Default SDA pin
#define OLED_SCL_PIN       GPIO_NUM_22        // Default SCL pin
#define OLED_FREQ_HZ       400000             // I2C clock frequency (400kHz)

/* ==== Display Resolution ==== */
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64

/* ==== Message Types for Display Task ==== */
typedef enum {
    DISPLAY_MSG_SENSOR_DATA,
    DISPLAY_MSG_SYSTEM_STATUS,
    DISPLAY_MSG_CLEAR,
    DISPLAY_MSG_WIFI_CONFIG
} display_msg_type_t;

/* ==== Sensor Data ==== */
typedef struct {
    float lux;
    float uv_index;
    time_t timestamp;
} sensor_data_t;

/* ==== Display Message ==== */
typedef struct {
    display_msg_type_t type;
    union {
        sensor_data_t sensor_data;
        char status_msg[32];
        struct {
            char ssid[32];
            char password[64];
        } wifi_config;
    } content;
} display_msg_t;

/* ==== Display Config Struct ==== */
typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_address;
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint32_t clk_speed_hz;
} display_config_t;

/* ==== Display Functions ==== */
esp_err_t display_init(const display_config_t *config);
esp_err_t display_send_message(const display_msg_t *msg);
void display_task(void *pvParameters);
void display_show_sensor_data(const sensor_data_t *data);
void display_show_status(const char *status);
void display_show_wifi_config(const char *ssid, const char *password);
void display_clear(void);

#endif /* COMPONENTS_OLED_DISPLAY_H_ */
