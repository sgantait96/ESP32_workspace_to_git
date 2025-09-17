/*
 * opt3001.h
 *
 *  Created on: Sep 14, 2025
 *      Author: sgant
 */

#ifndef MAIN_OPT3001_H_
#define MAIN_OPT3001_H_

#include "driver/i2c.h"
#include "esp_err.h"

#define OPT3001_I2C_ADDRESS           0x44  // Default I2C address
#define OPT3001_REG_RESULT            0x00
#define OPT3001_REG_CONFIG            0x01
#define OPT3001_REG_LOW_LIMIT         0x02
#define OPT3001_REG_HIGH_LIMIT        0x03
#define OPT3001_REG_MANUFACTURER_ID   0x7E
#define OPT3001_REG_DEVICE_ID         0x7F

// Config register settings (default config can be set as per datasheet)
#define OPT3001_CONFIG_DEFAULT        0xCC10

typedef struct {
    i2c_port_t i2c_port;
    uint8_t address;
} opt3001_t;

/**
 * @brief Initialize OPT3001 sensor with given I2C port and address
 */
esp_err_t opt3001_init(opt3001_t *sensor, i2c_port_t i2c_port, uint8_t address);

/**
 * @brief Read raw 16-bit register value from sensor
 */
esp_err_t opt3001_read_register(opt3001_t *sensor, uint8_t reg, uint16_t *value);

/**
 * @brief Write 16-bit register value to sensor
 */
esp_err_t opt3001_write_register(opt3001_t *sensor, uint8_t reg, uint16_t value);

/**
 * @brief Startup and configure OPT3001 with default configuration
 */
esp_err_t opt3001_start(opt3001_t *sensor);

/**
 * @brief Read ambient light level in lux from sensor
 */
esp_err_t opt3001_read_lux(opt3001_t *sensor, float *lux);




#endif /* MAIN_OPT3001_H_ */
