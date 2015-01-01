//*****************************************************************************
//	Author: 		Mitchel Pigsley
//	Created: 		12/29/2014
//	Last Updated: 	12/29/2014
//	Description:	Get temperature reading from a TMP36 component and upload
//					data to web server at a rate of once per minute.
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Driverlib includes
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_nvic.h"
#include "hw_types.h"
#include "hw_wdt.h"
#include "wdt.h"
#include "prcm.h"
#include "debug.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "utils.h"

// Common interface includes
#include "gpio_if.h"
#include "wdt_if.h"
#include "pinmux.h"
#include "common.h"
#ifndef NOTERM
#include "uart_if.h"
#endif

// My includes
#include "temp.h"

// My defines
#define APPLICATION_NAME        	"Temperature Logger"
#define APPLICATION_VERSION     	"0.0.1"
#define DEBUG						TRUE

#define FOREVER						1
#define MAP_SysCtlClockGet			80000000
#define SEC_TO_TICKS(s)				(MAP_SysCtlClockGet * s)
#define DEEP_SLEEP_SEC_TO_TICKS(s)	((MAP_SysCtlClockGet / 2) * s)

#ifndef DEBUG
#define WD_PERIOD_MS				60
#else
#define WD_PERIOD_MS				1
#endif

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

static void DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *********************************************************\n\r");
    UART_PRINT("\t\t           CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *********************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

void SensorLoggerInt(void)
{
	UART_PRINT("Temperature: %ld\r\n", GetTemperature());
    MAP_WatchdogIntClear(WDT_BASE);
}

static void BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  // Set vector table base
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void main(void)
{
    BoardInit();
    PinMuxConfig();

#ifndef NOTERM
    InitTerm();
#endif

    DisplayBanner(APPLICATION_NAME);

    GPIO_IF_LedConfigure(LED1);
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

	// Enable Watchdog Timer
#ifndef DEBUG
    WDT_IF_Init(SensorLoggerInt, DEEP_SLEEP_SEC_TO_TICKS(WD_PERIOD_MS));
#else
    WDT_IF_Init(SensorLoggerInt, SEC_TO_TICKS(WD_PERIOD_MS));
#endif
    if(!MAP_WatchdogRunning(WDT_BASE))
    {
       WDT_IF_DeInit();
    }

#ifndef DEBUG
    // Enable the Sleep Clock
    MAP_PRCMPeripheralClkEnable(PRCM_WDT, PRCM_DSLP_MODE_CLK);
    while(FOREVER)
    {
        UART_PRINT("\n\rEntering Deep Sleep\n\r");
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        MAP_UtilsDelay(800000);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
        MAP_PRCMDeepSleepEnter();
    }
#else
    LOOP_FOREVER();
#endif
}
