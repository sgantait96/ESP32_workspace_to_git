/*
 * internal_rtc.c
 *
 *  Created on: Sep 22, 2025
 *      Author: sgant
 */


#include "internal_rtc.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"

static const char *TAG = "RTC";

static rtc_dev_t *g_rtc_dev = NULL;

// SNTP callback function
static void sntp_callback(struct timeval *tv) {
    if (g_rtc_dev != NULL) {
        g_rtc_dev->sntp_synced = true;
        g_rtc_dev->last_sync_time = time(NULL);
        ESP_LOGI(TAG, "SNTP time synchronized: %s", ctime(&g_rtc_dev->last_sync_time));
    }
}

rtc_err_t rtc_init(rtc_dev_t *dev) {
    if (dev == NULL) {
        return RTC_ERR_INVALID_ARG;
    }

    // Initialize the internal RTC
    setenv("TZ", "UTC", 1);
    tzset();

    dev->initialized = true;
    dev->sntp_synced = false;
    dev->last_sync_time = 0;
    strncpy(dev->timezone, "UTC", sizeof(dev->timezone) - 1);

    g_rtc_dev = dev;

    ESP_LOGI(TAG, "RTC initialized successfully");
    return RTC_SUCCESS;
}

rtc_err_t rtc_deinit(rtc_dev_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    rtc_sntp_stop(dev);
    dev->initialized = false;
    g_rtc_dev = NULL;

    ESP_LOGI(TAG, "RTC deinitialized");
    return RTC_SUCCESS;
}

rtc_err_t rtc_get_time(struct tm *timeinfo) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    time_t now;
    time(&now);
    localtime_r(&now, timeinfo);

    return RTC_SUCCESS;
}

rtc_err_t rtc_set_time(const struct tm *timeinfo) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized || timeinfo == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    struct timeval tv = {
        .tv_sec = mktime((struct tm *)timeinfo),
        .tv_usec = 0
    };
    settimeofday(&tv, NULL);

    ESP_LOGI(TAG, "RTC time set manually");
    return RTC_SUCCESS;
}

rtc_err_t rtc_get_timestamp(time_t *timestamp) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized || timestamp == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    time(timestamp);
    return RTC_SUCCESS;
}

rtc_err_t rtc_set_timestamp(time_t timestamp) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    struct timeval tv = {
        .tv_sec = timestamp,
        .tv_usec = 0
    };
    settimeofday(&tv, NULL);

    ESP_LOGI(TAG, "RTC timestamp set: %ld", timestamp);
    return RTC_SUCCESS;
}

rtc_err_t rtc_sntp_start(rtc_dev_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    // Stop any existing SNTP service
    rtc_sntp_stop(dev);

    // Configure SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.nist.gov");
    esp_sntp_setservername(2, "time.google.com");

    // Set sync mode and callback
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_set_time_sync_notification_cb(sntp_callback);

    // Initialize SNTP
    esp_sntp_init();

    ESP_LOGI(TAG, "SNTP service started");
    return RTC_SUCCESS;
}

rtc_err_t rtc_sntp_stop(rtc_dev_t *dev) {
    if (dev == NULL || !dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    esp_sntp_stop();
    dev->sntp_synced = false;

    ESP_LOGI(TAG, "SNTP service stopped");
    return RTC_SUCCESS;
}

rtc_err_t rtc_sntp_sync(rtc_dev_t *dev, TickType_t timeout) {
    if (dev == NULL || !dev->initialized) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    TickType_t start_time = xTaskGetTickCount();
    
    while ((xTaskGetTickCount() - start_time) < timeout) {
        if (dev->sntp_synced) {
            return RTC_SUCCESS;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    return RTC_ERR_TIMEOUT;
}

bool rtc_is_synced(rtc_dev_t *dev) {
    return (dev != NULL && dev->initialized && dev->sntp_synced);
}

rtc_err_t rtc_set_timezone(rtc_dev_t *dev, const char *tz) {
    if (dev == NULL || !dev->initialized || tz == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    setenv("TZ", tz, 1);
    tzset();
    strncpy(dev->timezone, tz, sizeof(dev->timezone) - 1);
    dev->timezone[sizeof(dev->timezone) - 1] = '\0';

    ESP_LOGI(TAG, "Timezone set to: %s", tz);
    return RTC_SUCCESS;
}

rtc_err_t rtc_get_timezone(rtc_dev_t *dev, char *tz, size_t size) {
    if (dev == NULL || !dev->initialized || tz == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    strncpy(tz, dev->timezone, size - 1);
    tz[size - 1] = '\0';
    return RTC_SUCCESS;
}

rtc_err_t rtc_get_formatted_time(char *buffer, size_t size) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized || buffer == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    struct tm timeinfo;
    rtc_err_t err = rtc_get_time(&timeinfo);
    if (err != RTC_SUCCESS) {
        return err;
    }

    strftime(buffer, size, "%H:%M:%S", &timeinfo);
    return RTC_SUCCESS;
}

rtc_err_t rtc_get_formatted_date(char *buffer, size_t size) {
    if (g_rtc_dev == NULL || !g_rtc_dev->initialized || buffer == NULL) {
        return RTC_ERR_NOT_INITIALIZED;
    }

    struct tm timeinfo;
    rtc_err_t err = rtc_get_time(&timeinfo);
    if (err != RTC_SUCCESS) {
        return err;
    }

    strftime(buffer, size, "%Y-%m-%d", &timeinfo);
    return RTC_SUCCESS;
}

rtc_err_t rtc_sleep_until(struct tm *wakeup_time) {
    if (wakeup_time == NULL) {
        return RTC_ERR_INVALID_ARG;
    }

    time_t now;
    time(&now);
    time_t wakeup_ts = mktime(wakeup_time);
    
    if (wakeup_ts <= now) {
        return RTC_ERR_INVALID_ARG;
    }

    uint64_t sleep_us = (wakeup_ts - now) * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleep_us);
    esp_deep_sleep_start();

    return RTC_SUCCESS; // This won't be reached
}

rtc_err_t rtc_sleep_seconds(uint32_t seconds) {
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
    esp_deep_sleep_start();
    
    return RTC_SUCCESS; // This won't be reached
}

