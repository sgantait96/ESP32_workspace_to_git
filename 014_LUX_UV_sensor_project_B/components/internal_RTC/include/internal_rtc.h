/*
 * internal_rtc.h
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */

#ifndef COMPONENTS_INTERNAL_RTC_INCLUDE_INTERNAL_RTC_H_
#define COMPONENTS_INTERNAL_RTC_INCLUDE_INTERNAL_RTC_H_


#include <time.h>
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RTC_SUCCESS = 0,
    RTC_ERR_INVALID_ARG = -1,
    RTC_ERR_SNTP = -2,
    RTC_ERR_TIMEOUT = -3,
    RTC_ERR_NOT_INITIALIZED = -4,
} rtc_err_t;

typedef struct {
    bool initialized;
    bool sntp_synced;
    time_t last_sync_time;
    char timezone[64];
} rtc_dev_t;

// Initialization
rtc_err_t rtc_init(rtc_dev_t *dev);
rtc_err_t rtc_deinit(rtc_dev_t *dev);

// Time functions
rtc_err_t rtc_get_time(struct tm *timeinfo);
rtc_err_t rtc_set_time(const struct tm *timeinfo);
rtc_err_t rtc_get_timestamp(time_t *timestamp);
rtc_err_t rtc_set_timestamp(time_t timestamp);

// SNTP functions
rtc_err_t rtc_sntp_start(rtc_dev_t *dev);
rtc_err_t rtc_sntp_stop(rtc_dev_t *dev);
rtc_err_t rtc_sntp_sync(rtc_dev_t *dev, TickType_t timeout);
bool rtc_is_synced(rtc_dev_t *dev);

// Timezone functions
rtc_err_t rtc_set_timezone(rtc_dev_t *dev, const char *tz);
rtc_err_t rtc_get_timezone(rtc_dev_t *dev, char *tz, size_t size);

// Utility functions
rtc_err_t rtc_get_formatted_time(char *buffer, size_t size);
rtc_err_t rtc_get_formatted_date(char *buffer, size_t size);

// Deep sleep with RTC wakeup
rtc_err_t rtc_sleep_until(struct tm *wakeup_time);
rtc_err_t rtc_sleep_seconds(uint32_t seconds);

#ifdef __cplusplus
}
#endif



#endif /* COMPONENTS_INTERNAL_RTC_INCLUDE_INTERNAL_RTC_H_ */
