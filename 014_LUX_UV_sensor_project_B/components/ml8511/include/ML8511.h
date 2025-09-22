/*
 * ML8511.h
 *
 *  Created on: Sep 21, 2025
 *      Author: sgant
 */

#ifndef COMPONENTS_ML8511_INCLUDE_ML8511_H_
#define COMPONENTS_ML8511_INCLUDE_ML8511_H_

#include <stdint.h>



// XIAO ESP32S3 Pin Definitions
#define UVOUT_ADC_CHANNEL     ADC1_CHANNEL_0   // A0 (GPIO1)
#define REF_3V3_ADC_CHANNEL   ADC1_CHANNEL_1   // A1 (GPIO2)
#define ADC_UNIT              ADC_UNIT_1

// ADC configuration
#define ADC_ATTEN            ADC_ATTEN_DB_12
#define ADC_BITWIDTH         ADC_BITWIDTH_12
#define NUM_SAMPLES           8

// Initialize the ML8511 sensor
void ml8511_init(void);

// Read UV intensity value
float ml8511_read_uv_intensity(void);

// Get raw ADC values for debugging
void ml8511_get_raw_values(int *uv_level, int *ref_level);

// Start continuous reading task
void ml8511_start_continuous_reading(void);

// Stop continuous reading task
void ml8511_stop_continuous_reading(void);




#endif /* COMPONENTS_ML8511_INCLUDE_ML8511_H_ */
