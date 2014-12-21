/*
 * temp.c
 *
 *  Created on: Dec 19, 2014
 *      Author: Mitchel Pigsley
 */

#include "utils.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_adc.h"
#include "hw_ints.h"
#include "hw_gprcm.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "uart.h"
#include "pinmux.h"
#include "pin.h"
#include "adc.h"
#include "temp.h"

#define ADC_PIN PIN_58
#define NUM_AVGS 512
#define RESOLUTION 1.46/4096

float GetTemperature() {
	// Pinmux for the selected ADC input pin
	MAP_PinTypeADC(ADC_PIN,PIN_MODE_255);

	// Convert pin number to channel number
	unsigned int adcChannel;
	switch(ADC_PIN) {
		case PIN_58: {adcChannel = ADC_CH_1;}break;
		case PIN_59: {adcChannel = ADC_CH_2;}break;
		case PIN_60: {adcChannel = ADC_CH_3;}break;
		default: break;
	}

    // Enable ADC channel
    MAP_ADCChannelEnable(ADC_BASE, adcChannel);

    // Configure ADC timer which is used to timestamp the ADC data samples
    MAP_ADCTimerConfig(ADC_BASE, 2^17);

    // Enable ADC timer which is used to timestamp the ADC data samples
    MAP_ADCTimerEnable(ADC_BASE);

    // Enable ADC module
    MAP_ADCEnable(ADC_BASE);

    // Sample 512 times and average
	unsigned int i = 0, numSamples = 0;
	unsigned long sampleTotal = 0;
    for (i = 0; i < NUM_AVGS; i++) {
		if (MAP_ADCFIFOLvlGet(ADC_BASE, adcChannel)) {
			unsigned long sample = MAP_ADCFIFORead(ADC_BASE, adcChannel);
			sample = (sample >> 2) & 0x00000FFF; // Get 12 bit sample out
			sampleTotal += sample;
			numSamples++;
		}
    }

    float millivolts = (sampleTotal * RESOLUTION * 1000) / numSamples;
    float celcius = (millivolts - 500) / 10;
    float fahrenheit = celcius*1.8 + 32;
	return fahrenheit;
}
