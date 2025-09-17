#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "OLEDDisplay.h"

static const char *TAG = "oled-example";

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)
#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM    I2C_NUM_0        /*!< I2C port number for master dev */

SemaphoreHandle_t print_mux = NULL;

static void i2c_test_task(void *arg)
{
		OLEDDisplay_t *oled = OLEDDisplay_init(I2C_MASTER_NUM,0x3C,I2C_MASTER_SDA_IO,I2C_MASTER_SCL_IO);

		OLEDDisplay_fillRect(oled,7,7,18,18);
		OLEDDisplay_drawRect(oled,6,32,20,20);
		OLEDDisplay_display(oled);
		vTaskDelay(500 / portTICK_PERIOD_MS);

    OLEDDisplay_setTextAlignment(oled,TEXT_ALIGN_CENTER);
    OLEDDisplay_setFont(oled,ArialMT_Plain_24);
    OLEDDisplay_drawString(oled,64, 00, "Test");
		OLEDDisplay_display(oled);
		vTaskDelay(500 / portTICK_PERIOD_MS);

    OLEDDisplay_setFont(oled,ArialMT_Plain_16);
    OLEDDisplay_drawString(oled,64, 25, "Testing");
		OLEDDisplay_display(oled);
		vTaskDelay(500 / portTICK_PERIOD_MS);

    OLEDDisplay_setFont(oled,ArialMT_Plain_10);
    OLEDDisplay_drawString(oled,64, 40, "This is a test");
		OLEDDisplay_display(oled);

		while(1) {
			OLEDDisplay_setColor(oled,INVERSE);
			OLEDDisplay_fillRect(oled,6,6,20,20);
			OLEDDisplay_fillRect(oled,6,32,20,20);
			OLEDDisplay_display(oled);
			vTaskDelay(250 / portTICK_PERIOD_MS);
		}

    vSemaphoreDelete(print_mux);
    vTaskDelete(NULL);
}

void app_main(void)
{
    print_mux = xSemaphoreCreateMutex();
		ESP_LOGI(TAG,"Running");
    xTaskCreate(i2c_test_task, "i2c_test_task_0", 1024 * 2, (void *)0, 10, NULL);
}
