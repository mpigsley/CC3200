/*
 * temp.c
 *
 *  Created on: Dec 19, 2014
 *      Author: Mitchel Pigsley
 */

// Driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "rom_map.h"

// Common interface includes
#include "pin.h"
#include "adc.h"

// My includes
#include "temp.h"

// My defines
#define ADC_PIN PIN_58

#define NUM_AVGS 512
#define RESOLUTION 1.46/4096

#define RES_Q 93 // RESOLUTION ^^ - Q18
#define TO_F_Q 471859 // 1.8 - Q18

#define Round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// Global variables
uint32_t temperature = 0;

int32_t ToQ(int32_t num)
{
	return num << Q_NUM;
}

int32_t QMultiply(int32_t a, int32_t b)
{

	int64_t mult = ((int64_t)a) * ((int64_t)b);
	return (int32_t)(mult >> Q_NUM);
}

int32_t QDivide(int32_t a, int32_t b)
{
	int64_t preMult = ((int64_t)a) << Q_NUM;
	return (int32_t)(preMult / b);
}

void GetTemperature() {
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
	uint16_t i = 0;
	int32_t sampleTotal = 0, numSamples = 0;
    for (i = 0; i < NUM_AVGS; i++) {
		if (MAP_ADCFIFOLvlGet(ADC_BASE, adcChannel)) {
			unsigned long sample = MAP_ADCFIFORead(ADC_BASE, adcChannel);
			sample = (sample >> 2) & 0x00000FFF; // Get 12 bit sample out
			sampleTotal += sample;
			numSamples++;
		}
    }

    int32_t millivolts = ToQ(Round(sampleTotal / numSamples));
    millivolts = QMultiply(millivolts, RES_Q);
    millivolts = QMultiply(millivolts, ToQ(1000));
    int32_t celcius = QDivide(millivolts - ToQ(500), ToQ(10));
    temperature = QMultiply(celcius, TO_F_Q) + ToQ(32);
}
