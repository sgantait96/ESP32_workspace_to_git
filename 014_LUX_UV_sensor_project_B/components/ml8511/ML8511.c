#include "ml8511.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

// Try to include the new ADC API if available
#if __has_include("esp_adc/adc_oneshot.h")
    #include "esp_adc/adc_oneshot.h"
    #define USE_NEW_ADC_API 1
#else
    // Fallback to legacy ADC driver
    #include "driver/adc.h"
    #define USE_NEW_ADC_API 0
#endif


// Tag for logging
static const char *TAG = "ML8511";

#if USE_NEW_ADC_API
static adc_oneshot_unit_handle_t adc_handle = NULL;
#else
// For legacy API
#define ADC_WIDTH ADC_WIDTH_BIT_12
#endif

static uint8_t is_continuous_reading = 0;
static TaskHandle_t uv_sensor_task_handle = NULL;

// Takes an average of readings on a given ADC channel
static int average_analog_read(adc_channel_t channel)
{
    uint32_t runningValue = 0;
    
    for(int x = 0; x < NUM_SAMPLES; x++)
    {
        #if USE_NEW_ADC_API
        int raw_value;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &raw_value));
        runningValue += raw_value;
        #else
        runningValue += adc1_get_raw((adc1_channel_t)channel);
        #endif
        vTaskDelay(pdMS_TO_TICKS(1)); // Small delay between samples
    }
    runningValue /= NUM_SAMPLES;

    return (int)runningValue;
}

// The Arduino Map function but for floats
static float map_float(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Read sensor and calculate UV intensity
static float read_sensor_data(int *uv_level, int *ref_level)
{
    int uv_raw = average_analog_read(UVOUT_ADC_CHANNEL);
    int ref_raw = average_analog_read(REF_3V3_ADC_CHANNEL);

    if (uv_level) *uv_level = uv_raw;
    if (ref_level) *ref_level = ref_raw;

    // Convert ADC readings to voltage (12-bit ADC, 3.3V reference)
    float output_voltage = (uv_raw * 3.3) / 4095.0;
    float ref_voltage = (ref_raw * 3.3) / 4095.0;

    // Use the 3.3V power pin as a reference to get accurate output value
    float adjusted_voltage = (3.3 / ref_voltage) * output_voltage;

    return map_float(adjusted_voltage, 0.99, 2.8, 0.0, 15.0);
}

// Task to continuously read and process UV sensor data
static void uv_sensor_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting continuous UV sensor reading");
    
    while(is_continuous_reading)
    {
        int uv_level, ref_level;
        float uv_intensity = read_sensor_data(&uv_level, &ref_level);

        ESP_LOGI(TAG, "UV Level: %d, REF Level: %d, Intensity: %.2f mW/cm²", 
                 uv_level, ref_level, uv_intensity);

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
    
    ESP_LOGI(TAG, "Stopping continuous UV sensor reading");
    vTaskDelete(NULL);
}

void ml8511_init(void)
{
    #if USE_NEW_ADC_API
    // ADC oneshot unit configuration
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    
    // Create ADC unit
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // ADC channel configuration
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    // Configure UV output channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, UVOUT_ADC_CHANNEL, &config));
    
    // Configure 3.3V reference channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, REF_3V3_ADC_CHANNEL, &config));
    
    ESP_LOGI(TAG, "ML8511 sensor initialized with new ADC API");
    
    #else
    // Configure ADC width for legacy API
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH));
    
    // Configure ADC attenuation for both channels
    ESP_ERROR_CHECK(adc1_config_channel_atten((adc1_channel_t)UVOUT_ADC_CHANNEL, ADC_ATTEN_DB_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten((adc1_channel_t)REF_3V3_ADC_CHANNEL, ADC_ATTEN_DB_12));
    
    ESP_LOGI(TAG, "ML8511 sensor initialized with legacy ADC API");
    #endif
}

float ml8511_read_uv_intensity(void)
{
    return read_sensor_data(NULL, NULL);
}

void ml8511_get_raw_values(int *uv_level, int *ref_level)
{
    if (uv_level) *uv_level = average_analog_read(UVOUT_ADC_CHANNEL);
    if (ref_level) *ref_level = average_analog_read(REF_3V3_ADC_CHANNEL);
}

void ml8511_start_continuous_reading(void)
{
    if (uv_sensor_task_handle != NULL) {
        ESP_LOGW(TAG, "Continuous reading already started");
        return;
    }
    
    is_continuous_reading = 1;
    xTaskCreate(uv_sensor_task, "uv_sensor_task", 4096, NULL, 5, &uv_sensor_task_handle);
    ESP_LOGI(TAG, "Continuous reading started");
}

void ml8511_stop_continuous_reading(void)
{
    if (uv_sensor_task_handle == NULL) {
        ESP_LOGW(TAG, "Continuous reading not running");
        return;
    }
    
    is_continuous_reading = 0;
    // Wait for task to finish
    vTaskDelay(pdMS_TO_TICKS(1100)); // Slightly more than 1 second
    uv_sensor_task_handle = NULL;
    ESP_LOGI(TAG, "Continuous reading stopped");
}

