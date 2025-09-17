/*
 * opt3001.c
 *
 *  Created on: Sep 14, 2025
 *      Author: sgant
 */



#include "opt3001.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "OPT3001";

esp_err_t opt3001_init(opt3001_t *sensor, i2c_port_t i2c_port, uint8_t address) {
    if (!sensor) return ESP_ERR_INVALID_ARG;
    sensor->i2c_port = i2c_port;
    sensor->address = address;
    return ESP_OK;
}

static esp_err_t opt3001_write_16bit_reg(opt3001_t *sensor, uint8_t reg, uint16_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret;

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value >> 8, true);
    i2c_master_write_byte(cmd, value & 0xFF, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t opt3001_read_16bit_reg(opt3001_t *sensor, uint8_t reg, uint16_t *value) {
    if (!value) return ESP_ERR_INVALID_ARG;
    uint8_t data[2];
    esp_err_t ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    // Set register pointer
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) return ret;

    // Read data
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (sensor->address << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data[1], I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(sensor->i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) return ret;

    *value = ((uint16_t)data[0] << 8) | data[1];
    return ESP_OK;
}

esp_err_t opt3001_read_register(opt3001_t *sensor, uint8_t reg, uint16_t *value) {
    return opt3001_read_16bit_reg(sensor, reg, value);
}

esp_err_t opt3001_write_register(opt3001_t *sensor, uint8_t reg, uint16_t value) {
    return opt3001_write_16bit_reg(sensor, reg, value);
}

esp_err_t opt3001_start(opt3001_t *sensor) {
    // Write default config (continuous conversion)
    return opt3001_write_register(sensor, OPT3001_REG_CONFIG, OPT3001_CONFIG_DEFAULT);
}

esp_err_t opt3001_read_lux(opt3001_t *sensor, float *lux) {
    if (!lux) return ESP_ERR_INVALID_ARG;
    uint16_t raw;
    esp_err_t ret = opt3001_read_register(sensor, OPT3001_REG_RESULT, &raw);
    if (ret != ESP_OK) return ret;

    uint16_t exponent = (raw & 0xF000) >> 12;
    uint16_t mantissa = raw & 0x0FFF;

    *lux = (float)(mantissa * (0.01 * (1 << exponent)));
    return ESP_OK;
}

