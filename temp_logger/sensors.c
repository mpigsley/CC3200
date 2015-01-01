/*
 * sensors.c
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
#include "sensors.h"
#include "q_number.h"

// My defines
#define TEMP_PIN PIN_58
#define LIGHT_PIN PIN_59

#define NUM_AVGS 512
#define RESOLUTION 1.46/4096
#define ADC_RES 93 // RESOLUTION ^^ - Q18

#define TEMP_TO_F 471859 // 1.8 - Q18
#define LUX_OFFSET 45283 // .188 - Q18
#define LUX_MULT 223649989 // 853.157 - Q18
#define LUX_DEFAULT 26214 // .1 - Q18

uint32_t GetVoltageFromPin(unsigned int pin);

// Return in Degrees Fahrenheit
uint32_t GetTemperature()
{
    int32_t volts = GetVoltageFromPin(TEMP_PIN);
    int32_t millivolts = QMultiply(volts, ToQ(1000));
    int32_t celcius = QDivide(millivolts - ToQ(500), ToQ(10));
    return QMultiply(celcius, TEMP_TO_F) + ToQ(32); // Fahrenheit Calculation
}

// Return in Lux
uint32_t GetAmbientLight()
{
    int32_t volts = GetVoltageFromPin(LIGHT_PIN) - LUX_OFFSET;
    if (volts < 0)
    {
    	return LUX_DEFAULT; // Shouldn't be possible..
    }
    return QMultiply(volts, LUX_MULT);
}

uint32_t GetVoltageFromPin(unsigned int pin)
{
	// Pinmux for the selected ADC input pin
	MAP_PinTypeADC(pin, PIN_MODE_255);

	// Convert pin number to channel number
	unsigned int adcChannel;
	switch(pin) {
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

    // Sample multiple times and average
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

    // Return the voltage on the pin
    return QMultiply(ToQ(Round(sampleTotal / numSamples)), ADC_RES);
}
