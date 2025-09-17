/*
 * status_led.c
 *
 *  Created on: Sep 10, 2025
 *      Author: sgant
 */

#include "status_led.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

 status_led_state_t current_state = LED_OFF;
static TaskHandle_t led_task_handle = NULL;

// LED control task
static void status_led_task(void *pvParameters)
{
    while (1)
    {
        switch (current_state)
        {
            case LED_OFF:
                gpio_set_level(STATUS_LED_GPIO, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_ON:
                gpio_set_level(STATUS_LED_GPIO, 1);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_BLINK_FAST:
                gpio_set_level(STATUS_LED_GPIO, 1);
                vTaskDelay(pdMS_TO_TICKS(200));
                gpio_set_level(STATUS_LED_GPIO, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
                break;

            case LED_BLINK_SLOW:
                gpio_set_level(STATUS_LED_GPIO, 1);
                vTaskDelay(pdMS_TO_TICKS(1000));
                gpio_set_level(STATUS_LED_GPIO, 0);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
        }
    }
}

void status_led_init(void)
{
    // Configure LED pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << STATUS_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // Start LED task
    xTaskCreate(status_led_task, "status_led_task", 2048, NULL, 1, &led_task_handle);
}

void status_led_set_state(status_led_state_t state)
{
    current_state = state;
}



