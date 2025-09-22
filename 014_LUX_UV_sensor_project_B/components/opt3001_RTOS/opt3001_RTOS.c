/*
 * opt3001_RTOS.c
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */


#include "opt3001_RTOS.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "OPT3001";

// Private function declarations
static opt3001_err_t opt3001_write_register(opt3001_dev_t *dev, opt3001_register_t reg, uint16_t data);
static opt3001_err_t opt3001_read_register(opt3001_dev_t *dev, opt3001_register_t reg, uint16_t *data);
static opt3001_err_t opt3001_take_mutex(opt3001_dev_t *dev, TickType_t timeout);
static opt3001_err_t opt3001_give_mutex(opt3001_dev_t *dev);

opt3001_err_t opt3001_init(opt3001_dev_t *dev) {
    if (dev == NULL) {
        return OPT3001_ERR_INVALID_ARG;
    }

    // Create mutex for thread-safe I2C access
    dev->i2c_mutex = xSemaphoreCreateMutex();
    if (dev->i2c_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create I2C mutex");
        return OPT3001_ERR_MUTEX;
    }

    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = dev->sda_pin,
        .scl_io_num = dev->scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = dev->i2c_freq_hz,
    };

    esp_err_t err = i2c_param_config(dev->i2c_port, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C parameter config failed: %s", esp_err_to_name(err));
        vSemaphoreDelete(dev->i2c_mutex);
        return OPT3001_ERR_I2C;
    }

    err = i2c_driver_install(dev->i2c_port, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        vSemaphoreDelete(dev->i2c_mutex);
        return OPT3001_ERR_I2C;
    }

    // Verify device presence
    uint16_t manufacturer_id;
    opt3001_err_t ret = opt3001_read_manufacturer_id(dev, &manufacturer_id);
    if (ret != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read manufacturer ID");
        i2c_driver_delete(dev->i2c_port);
        vSemaphoreDelete(dev->i2c_mutex);
        return ret;
    }

    uint16_t device_id;
    ret = opt3001_read_device_id(dev, &device_id);
    if (ret != OPT3001_SUCCESS) {
        ESP_LOGE(TAG, "Failed to read device ID");
        i2c_driver_delete(dev->i2c_port);
        vSemaphoreDelete(dev->i2c_mutex);
        return ret;
    }

    dev->initialized = true;
    ESP_LOGI(TAG, "OPT3001 initialized successfully");
    ESP_LOGI(TAG, "Manufacturer ID: 0x%04X", manufacturer_id);
    ESP_LOGI(TAG, "Device ID: 0x%04X", device_id);

    return OPT3001_SUCCESS;
}

opt3001_err_t opt3001_deinit(opt3001_dev_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return OPT3001_ERR_INVALID_ARG;
    }

    opt3001_take_mutex(dev, portMAX_DELAY);
    i2c_driver_delete(dev->i2c_port);
    vSemaphoreDelete(dev->i2c_mutex);
    dev->initialized = false;

    return OPT3001_SUCCESS;
}

opt3001_err_t opt3001_read_manufacturer_id(opt3001_dev_t *dev, uint16_t *manufacturer_id) {
    return opt3001_read_register(dev, OPT3001_MANUFACTURER_ID, manufacturer_id);
}

opt3001_err_t opt3001_read_device_id(opt3001_dev_t *dev, uint16_t *device_id) {
    return opt3001_read_register(dev, OPT3001_DEVICE_ID, device_id);
}

opt3001_err_t opt3001_read_config(opt3001_dev_t *dev, opt3001_config_t *config) {
    uint16_t data;
    opt3001_err_t err = opt3001_read_register(dev, OPT3001_CONFIG, &data);
    if (err == OPT3001_SUCCESS) {
        config->raw_data = data;
    }
    return err;
}

opt3001_err_t opt3001_write_config(opt3001_dev_t *dev, opt3001_config_t config) {
    return opt3001_write_register(dev, OPT3001_CONFIG, config.raw_data);
}

opt3001_err_t opt3001_read_result(opt3001_dev_t *dev, opt3001_reading_t *reading) {
    uint16_t data;
    opt3001_err_t err = opt3001_read_register(dev, OPT3001_RESULT, &data);
    
    if (err != OPT3001_SUCCESS) {
        reading->error = err;
        reading->lux = 0.0f;
        return err;
    }

    opt3001_er_t er;
    er.raw_data = data;
    reading->raw = er;
    reading->lux = 0.01f * powf(2.0f, (float)er.exponent) * (float)er.result;
    reading->error = OPT3001_SUCCESS;

    return OPT3001_SUCCESS;
}

opt3001_err_t opt3001_configure_continuous(opt3001_dev_t *dev) {
    opt3001_config_t config = {
        .raw_data = 0
    };
    config.mode_of_conversion_operation = 0x2; // Continuous conversion
    config.conversion_time = 0x1; // 800ms conversion time
    config.range_number = 0xC; // Automatic full-scale range
    
    return opt3001_write_config(dev, config);
}

opt3001_err_t opt3001_configure_single_shot(opt3001_dev_t *dev) {
    opt3001_config_t config = {
        .raw_data = 0
    };
    config.mode_of_conversion_operation = 0x1; // Single-shot
    config.conversion_time = 0x1; // 800ms conversion time
    config.range_number = 0xC; // Automatic full-scale range
    
    return opt3001_write_config(dev, config);
}

opt3001_err_t opt3001_start_single_measurement(opt3001_dev_t *dev) {
    opt3001_config_t config;
    opt3001_err_t err = opt3001_read_config(dev, &config);
    if (err != OPT3001_SUCCESS) {
        return err;
    }

    // Set single-shot mode and start conversion
    config.mode_of_conversion_operation = 0x1;
    return opt3001_write_config(dev, config);
}

opt3001_err_t opt3001_wait_for_conversion(opt3001_dev_t *dev, TickType_t timeout) {
    TickType_t start_time = xTaskGetTickCount();
    
    while ((xTaskGetTickCount() - start_time) < timeout) {
        opt3001_config_t config;
        opt3001_err_t err = opt3001_read_config(dev, &config);
        if (err != OPT3001_SUCCESS) {
            return err;
        }

        if (config.conversion_ready) {
            return OPT3001_SUCCESS;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return OPT3001_ERR_TIMEOUT;
}

// Private functions
static opt3001_err_t opt3001_take_mutex(opt3001_dev_t *dev, TickType_t timeout) {
    if (xSemaphoreTake(dev->i2c_mutex, timeout) != pdTRUE) {
        return OPT3001_ERR_TIMEOUT;
    }
    return OPT3001_SUCCESS;
}

static opt3001_err_t opt3001_give_mutex(opt3001_dev_t *dev) {
    xSemaphoreGive(dev->i2c_mutex);
    return OPT3001_SUCCESS;
}

static opt3001_err_t opt3001_write_register(opt3001_dev_t *dev, opt3001_register_t reg, uint16_t data) {
    opt3001_err_t mutex_err = opt3001_take_mutex(dev, pdMS_TO_TICKS(1000));
    if (mutex_err != OPT3001_SUCCESS) {
        return mutex_err;
    }

    uint8_t write_buf[3] = {
        reg,
        (uint8_t)(data >> 8),
        (uint8_t)(data & 0xFF)
    };

    esp_err_t err = i2c_master_write_to_device(dev->i2c_port, dev->i2c_address,
                                             write_buf, sizeof(write_buf),
                                             pdMS_TO_TICKS(1000));
    
    opt3001_give_mutex(dev);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(err));
        return OPT3001_ERR_I2C;
    }
    
    return OPT3001_SUCCESS;
}

static opt3001_err_t opt3001_read_register(opt3001_dev_t *dev, opt3001_register_t reg, uint16_t *data) {
    opt3001_err_t mutex_err = opt3001_take_mutex(dev, pdMS_TO_TICKS(1000));
    if (mutex_err != OPT3001_SUCCESS) {
        return mutex_err;
    }

    uint8_t write_buf[1] = { reg };
    uint8_t read_buf[2] = { 0 };

    // Write register address
    esp_err_t err = i2c_master_write_to_device(dev->i2c_port, dev->i2c_address,
                                             write_buf, sizeof(write_buf),
                                             pdMS_TO_TICKS(1000));
    if (err != ESP_OK) {
        opt3001_give_mutex(dev);
        ESP_LOGE(TAG, "I2C write (register address) failed: %s", esp_err_to_name(err));
        return OPT3001_ERR_I2C;
    }

    // Read register data
    err = i2c_master_read_from_device(dev->i2c_port, dev->i2c_address,
                                    read_buf, sizeof(read_buf),
                                    pdMS_TO_TICKS(1000));
    opt3001_give_mutex(dev);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
        return OPT3001_ERR_I2C;
    }

    *data = (read_buf[0] << 8) | read_buf[1];
    return OPT3001_SUCCESS;
}

