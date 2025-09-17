#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "opt3001.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_SDA_IO           21
#define I2C_MASTER_SCL_IO           22
#define I2C_MASTER_FREQ_HZ          100000

static const char *TAG = "OPT3001_EXAMPLE";

static esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) return ret;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main(void) {
     i2c_master_init();

    opt3001_t sensor;
    opt3001_init(&sensor, I2C_MASTER_NUM, OPT3001_I2C_ADDRESS);
    opt3001_start(&sensor);

    while (1) {
        float lux;
        esp_err_t ret = opt3001_read_lux(&sensor, &lux);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Ambient Light: %.2f lux", lux);
        } else {
            ESP_LOGE(TAG, "Failed to read lux");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
