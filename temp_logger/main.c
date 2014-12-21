//*****************************************************************************
//	Author: 		Mitchel Pigsley
//	Created: 		12/19/2014
//	Last Updated: 	2/21/2014
//	Description:	Get temperature reading from a TMP36 component and upload
//					data to web server at a rate of once per minute.
//
//*****************************************************************************

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"

//Free_rtos/ti-rtos includes
#include "osi.h"

//Common interface includes
#include "gpio_if.h"
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "common.h"
#include "pinmux.h"

//My includes
#include "temp.h"
#include "network.h"

#if defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t           CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

//*****************************************************************************
//
//! \brief  Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
// In case of TI-RTOS vector table is initialize by OS itself
#ifndef USE_TIRTOS
    // Set vector table base
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif //USE_TIRTOS

    // Enable Processor
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

//****************************************************************************
//
//! \brief Start simplelink, connect to the ap and run the ping test
//!
//! This function starts the simplelink, connect to the ap and start the ping
//! test on the default gateway for the ap
//!
//! \param[in]  pvParameters - Pointer to the list of parameters that
//!             can bepassed to the task while creating it
//!
//! \return  None
//
//****************************************************************************
void WlanStationMode(void *pvParameters)
{
	long lRetVal = -1;
	InitializeNetwork();

    // Following function configure the device to default state by cleaning
    // the persistent settings stored in NVMEM (viz. connection profiles &
    // policies, power policy etc)
    //
    // Applications may choose to skip this step if the developer is sure
    // that the device is in its default state at start of applicaton
    //
    // Note that all profiles and persistent settings that were done on the
    // device will be lost

    if((lRetVal = ConfigureSimpleLinkToDefaultState()) < 0)
    {
        if (DEVICE_NOT_IN_STATION_MODE == lRetVal)
        {
            UART_PRINT("Failed to configure the device in its default state\n\r");
        }

        LOOP_FOREVER();
    }

    UART_PRINT("Device is configured in default state \n\r");

    // Assumption is that the device is configured in station mode already
    // and it is in its default state
    lRetVal = sl_Start(0, 0, 0);
    if (lRetVal < 0 || ROLE_STA != lRetVal)
    {
        UART_PRINT("Failed to start the device \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Device started as STATION \n\r");

    //Connecting to WLAN AP
    if(WlanConnect() < 0)
    {
        UART_PRINT("Failed to establish connection w/ an AP \n\r");
        LOOP_FOREVER();
    }

    UART_PRINT("Connection established w/ AP and IP is aquired \n\r");
    UART_PRINT("Pinging...! \n\r");

    // Checking the Lan connection by pinging to AP gateway
    if(CheckLanConnection() < 0)
    {
        UART_PRINT("Device couldn't ping the gateway \n\r");
        LOOP_FOREVER();
    }

    // Turn on GREEN LED when device gets PING response from AP
    GPIO_IF_LedOn(MCU_EXECUTE_SUCCESS_IND);

    // Checking the internet connection by pinging to external host
    if(CheckInternetConnection() < 0)
    {
        UART_PRINT("Device couldn't ping the external host \n\r");
        LOOP_FOREVER();
    }

    // Turn on ORAGE LED when device gets PING response from AP
    GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);

    UART_PRINT("Device pinged both the gateway and the external host \n\r");

    UART_PRINT("WLAN STATION example executed successfully \n\r");

    // power off the network processor
    sl_Stop(SL_STOP_TIMEOUT);

    LOOP_FOREVER();
}

//*****************************************************************************
//                         TEMPERATURE FUNCTION
//*****************************************************************************
void LogTemperature( void *pvParameters ) {
	UART_PRINT("Temperature: %f\r\n", GetTemperature());
}

//*****************************************************************************
//                            MAIN FUNCTION
//*****************************************************************************
void main()
{
	long lRetVal = -1;

    // Board Initialization
    BoardInit();
    
    // configure the GPIO pins for LEDs,UART
    PinMuxConfig();

    // Configure the UART
#ifndef NOTERM
    InitTerm();
#endif  //NOTERM
    
    // Display Application Banner
    DisplayBanner(APPLICATION_NAME);
    
    // Configure all 3 LEDs
    GPIO_IF_LedConfigure(LED1|LED2|LED3);

    // switch off all LEDs
    GPIO_IF_LedOff(MCU_ALL_LED_IND);
    
    // Start the SimpleLink Host
    if((lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY)) < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Start the WlanStationMode task
    if((lRetVal = osi_TaskCreate( WlanStationMode, (const signed char*)"Wlan Station Task", \
    		OSI_STACK_SIZE, NULL, 1, NULL )) < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    // Start the Temperature Logger task
    if((lRetVal = osi_TaskCreate( LogTemperature, (const signed char*)"Temperature Logger Task", \
    		OSI_STACK_SIZE, NULL, 2, NULL )) < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
     
    // Start the task scheduler
    osi_start();
  }
