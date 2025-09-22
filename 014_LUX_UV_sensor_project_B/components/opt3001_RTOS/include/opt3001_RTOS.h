/*
 * opt3001_RTOS.h
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */

#ifndef COMPONENTS_OPT3001_RTOS_INCLUDE_OPT3001_RTOS_H_
#define COMPONENTS_OPT3001_RTOS_INCLUDE_OPT3001_RTOS_H_


#include <stdint.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPT3001_I2C_ADDRESS_DEFAULT 0x44

typedef enum {
    OPT3001_RESULT = 0x00,
    OPT3001_CONFIG = 0x01,
    OPT3001_LOW_LIMIT = 0x02,
    OPT3001_HIGH_LIMIT = 0x03,
    OPT3001_MANUFACTURER_ID = 0x7E,
    OPT3001_DEVICE_ID = 0x7F,
} opt3001_register_t;

typedef enum {
    OPT3001_SUCCESS = 0,
    OPT3001_ERR_INVALID_ARG = -1,
    OPT3001_ERR_I2C = -2,
    OPT3001_ERR_TIMEOUT = -3,
    OPT3001_ERR_NOT_FOUND = -4,
    OPT3001_ERR_MUTEX = -5,
} opt3001_err_t;

typedef union {
    uint16_t raw_data;
    struct {
        uint16_t result : 12;
        uint8_t exponent : 4;
    };
} opt3001_er_t;

typedef union {
    struct {
        uint8_t fault_count : 2;
        uint8_t mask_exponent : 1;
        uint8_t polarity : 1;
        uint8_t latch : 1;
        uint8_t flag_low : 1;
        uint8_t flag_high : 1;
        uint8_t conversion_ready : 1;
        uint8_t overflow_flag : 1;
        uint8_t mode_of_conversion_operation : 2;
        uint8_t conversion_time : 1;
        uint8_t range_number : 4;
    };
    uint16_t raw_data;
} opt3001_config_t;

typedef struct {
    float lux;
    opt3001_er_t raw;
    opt3001_err_t error;
} opt3001_reading_t;

typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_address;
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint32_t i2c_freq_hz;
    SemaphoreHandle_t i2c_mutex;
    bool initialized;
} opt3001_dev_t;

// Initialization
opt3001_err_t opt3001_init(opt3001_dev_t *dev);
opt3001_err_t opt3001_deinit(opt3001_dev_t *dev);

// Basic operations
opt3001_err_t opt3001_read_manufacturer_id(opt3001_dev_t *dev, uint16_t *manufacturer_id);
opt3001_err_t opt3001_read_device_id(opt3001_dev_t *dev, uint16_t *device_id);
opt3001_err_t opt3001_read_config(opt3001_dev_t *dev, opt3001_config_t *config);
opt3001_err_t opt3001_write_config(opt3001_dev_t *dev, opt3001_config_t config);

// Measurement functions
opt3001_err_t opt3001_read_result(opt3001_dev_t *dev, opt3001_reading_t *reading);
opt3001_err_t opt3001_read_high_limit(opt3001_dev_t *dev, opt3001_reading_t *reading);
opt3001_err_t opt3001_read_low_limit(opt3001_dev_t *dev, opt3001_reading_t *reading);

// Configuration helpers
opt3001_err_t opt3001_configure_continuous(opt3001_dev_t *dev);
opt3001_err_t opt3001_configure_single_shot(opt3001_dev_t *dev);
opt3001_err_t opt3001_start_single_measurement(opt3001_dev_t *dev);

// FreeRTOS utilities
opt3001_err_t opt3001_wait_for_conversion(opt3001_dev_t *dev, TickType_t timeout);

#ifdef __cplusplus
}
#endif




#endif /* COMPONENTS_OPT3001_RTOS_INCLUDE_OPT3001_RTOS_H_ */
