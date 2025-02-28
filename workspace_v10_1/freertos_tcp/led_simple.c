/* FreeRTOS 10 Tiva Demo
 *
 * main.c
 *
 * Andy Kobyljanec
 *
 * This is a simple demonstration project of FreeRTOS 8.2 on the Tiva Launchpad
 * EK-TM4C1294XL.  TivaWare driverlib sourcecode is included.
 */

#include <stdint.h>
#include <stdbool.h>

#define LED_ON_MACRO() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF_MACRO() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"

// FreeRTOS includes
#include <FreeRTOSConfig.h>
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Routing.h"
#include "task.h"
#include "queue.h"

// Demo Task declarations
void demoLEDTask(void *pvParameters);

void Led_Init(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}
//// Main function
//int main(void)
//{
//    Led_Init();
//    // Initialize system clock to 120 MHz
//    uint32_t output_clock_rate_hz;
//    output_clock_rate_hz = MAP_SysCtlClockFreqSet(
//                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
//                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
//                               SYSTEM_CLOCK);
//    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);
//
//    // Create demo tasks
//    xTaskCreate(demoLEDTask, (const portCHAR *)"LEDs",
//                configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//
//    vTaskStartScheduler();
//
//    // Code should never reach this point
//    return 0;
//}

/* The MAC address array is not declared const as the MAC address will
   normally be read from an EEPROM and not hard coded (in real deployed
   applications).*/
static uint8_t ucMACAddress[ 6 ] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

/* Define the network addressing.  These parameters will be used if either
   ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
   failed. */
static const uint8_t ucIPAddress[ 4 ] = { 10, 10, 10, 200 };
static const uint8_t ucNetMask[ 4 ] = { 255, 0, 0, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 10, 10, 10, 1 };

/* The following is the address of an OpenDNS server. */
static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };

void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                           struct xNetworkEndPoint * pxEndPoint )
{
static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* Both eNetworkUp and eNetworkDown events can be processed here. */
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the TCP/IP stack if they have not already
        been created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /*
             * For convenience, tasks that use FreeRTOS-Plus-TCP can be created here
             * to ensure they are not created before the network is usable.
             */

            xTasksAlreadyCreated = pdTRUE;
        }
    }
    /* Print out the network configuration, which may have come from a DHCP
     * server. */
    showEndPoint( pxEndPoint );
}

int main( void )
{

    /* Initialise the interface descriptor for WinPCap for example. */
    pxTM4C_FillInterfaceDescriptor( 0, &( xInterfaces[ 0 ] ) );

    FreeRTOS_FillEndPoint( &( xInterfaces[ 0 ] ), &( xEndPoints[ 0 ] ), ucIPAddress,
            ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );


    #if ( ipconfigUSE_DHCP != 0 )
    {
        /* End-point 0 wants to use DHCPv4. */
        xEndPoints[ 0 ].bits.bWantDHCP = pdTRUE;
    }
    #endif /* ( ipconfigUSE_DHCP != 0 ) */

    /* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
       are created in the vApplicationIPNetworkEventHook() hook function
       below.  The hook function is called when the network connects. */
     FreeRTOS_IPInit_Multi();

    /*
     * Other RTOS tasks can be created here.
     */

    /* Start the RTOS scheduler. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
       line will never be reached.  If the following line does execute, then
       there was insufficient FreeRTOS heap memory available for the idle and/or
       timer tasks to be created. */
    for( ;; );
}


// Flash the LEDs on the launchpad
void demoLEDTask(void *pvParameters)
{
    for (;;)
    {
        // Turn on LED 1
        GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1);
        vTaskDelay(1000);

        // Turn on LED 2
        GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0);
        vTaskDelay(1000);

    }
}



/*  ASSERT() Error function
 *
 *  failed ASSERTS() from driverlib/debug.h are executed in this function
 */
void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    while (1)
    {
    }
}
