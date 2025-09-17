/*
 * status_led.h
 *
 *  Created on: Sep 10, 2025
 *      Author: sgant
 */

#ifndef MAIN_STATUS_LED_H_
#define MAIN_STATUS_LED_H_


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Onboard LED pin (change if needed)
#define STATUS_LED_GPIO   21

// LED states
typedef enum
{
    LED_OFF = 0,                 // LED OFF
    LED_ON,                      // Solid ON
    LED_BLINK_FAST,              // Blink every 200 ms
    LED_BLINK_SLOW,              // Blink every 1 second
} status_led_state_t;

// Public functions
void status_led_init(void);
void status_led_set_state(status_led_state_t state);



#endif /* MAIN_STATUS_LED_H_ */
