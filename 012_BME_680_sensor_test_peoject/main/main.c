#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "dht.h"  // ESP-IDF DHT driver header

#define I2C_MASTER_SCL_IO 6         // I2C SCL pin
#define I2C_MASTER_SDA_IO 5         // I2C SDA pin
#define I2C_MASTER_NUM I2C_NUM_0     // I2C port
#define I2C_MASTER_FREQ_HZ 100000
#define OLED_ADDR 0x3C               // SSD1306 I2C address

#define DHT_GPIO 4                   // GPIO where DHT22 data line is connected

#if defined(CONFIG_EXAMPLE_TYPE_DHT11)
#define SENSOR_TYPE DHT_TYPE_DHT11
#endif
#if defined(CONFIG_EXAMPLE_TYPE_AM2301)
#define SENSOR_TYPE DHT_TYPE_AM2301
#endif
#if defined(CONFIG_EXAMPLE_TYPE_SI7021)
#define SENSOR_TYPE DHT_TYPE_SI7021
#endif

void dht_oled_task(void *pvParameters)
{
    float temperature = 0.0f, humidity = 0.0f;

    while (1) {
		esp_err_t ret = dht_read_float_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity,
											&temperature);
		if (ret == ESP_OK) {
            char line1[20], line2[20];

            snprintf(line1, sizeof(line1), "Temp: %.1f C", temperature);
            snprintf(line2, sizeof(line2), "Humi: %.1f %%", humidity);

            ssd1306_clear_screen((ssd1306_handle_t)pvParameters, 0x00);
            ssd1306_draw_string((ssd1306_handle_t)pvParameters, 0, 0, (const uint8_t *)line1, 12, 1);
            ssd1306_draw_string((ssd1306_handle_t)pvParameters, 0, 16, (const uint8_t *)line2, 12, 1);
            ssd1306_refresh_gram((ssd1306_handle_t)pvParameters);
        } else {
            ssd1306_clear_screen((ssd1306_handle_t)pvParameters, 0x00);
            ssd1306_draw_string((ssd1306_handle_t)pvParameters, 0, 0, (const uint8_t *)"DHT22 Error", 12, 1);
            ssd1306_refresh_gram((ssd1306_handle_t)pvParameters);
            printf("Could not read data from sensor\n");
        }

 

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main()
{
    // Initialize I2C for SSD1306 OLED
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    // Create OLED device handle
    ssd1306_handle_t oled = ssd1306_create(I2C_MASTER_NUM, OLED_ADDR);//old library
    if (oled == NULL) {
        printf("Failed to initialize OLED\n");
        return;
    }

    ssd1306_clear_screen(oled, 0x00);
    ssd1306_refresh_gram(oled);

    // Create task and pass OLED handle as parameter
    xTaskCreate(dht_oled_task, "dht_oled_task", configMINIMAL_STACK_SIZE * 3, (void *)oled, 5, NULL);
}
