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
#include <stdio.h>
#include "freertos_lwip.h"

#define LED_ON_MACRO() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF_MACRO() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

// TivaWare includes
#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/inc/hw_memmap.h"
#include "driverlib/inc/hw_nvic.h"
#include "driverlib/inc/hw_types.h"
#include "driverlib/inc/hw_memmap.h"
#include "third_party/fatfs/SST26VF032B.h"
#include "idle_task.h"
#include "lwip_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

uint32_t g_ui32SysClock;
// Demo Task declarations
void demoLEDTask(void *pvParameters);

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while(1)
    {
    }
}
// Main function
int main(void)
{
    Led_Init();
    // Initialize system clock to 120 MHz
    g_ui32SysClock = ROM_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                               SYSTEM_CLOCK);
    ASSERT(g_ui32SysClock == SYSTEM_CLOCK);

    // Create demo tasks
    xTaskCreate(demoLEDTask, (const portCHAR *)"LEDs",
                configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    if(lwIPTaskInit() != 0)
    {
        printf("Failed to create lwIP tasks!\n");
        while(1)
        {
        }
    }

    vTaskStartScheduler();

    // Code should never reach this point
    return 0;
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


//
///*  ASSERT() Error function
// *
// *  failed ASSERTS() from driverlib/debug.h are executed in this function
// */
//void __error__(char *pcFilename, uint32_t ui32Line)
//{
//    // Place a breakpoint here to capture errors until logging routine is finished
//    while (1)
//    {
//    }
//}
