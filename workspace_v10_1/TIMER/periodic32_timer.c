/*
 * periodic32_timer.c
 *
 *  Created on: 24-Oct-2024
 *      Author: 14169
 */
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#define NUMBER_OF_INTS          1000

static volatile uint32_t g_ui32Counter = 0;

void
InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

void
Timer0AIntHandler(void)
{
    //
    // Clear the timer interrupt flag.
    //
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Update the periodic interrupt counter.
    //
    g_ui32Counter++;

    //
    // Once NUMBER_OF_INTS interrupts have been received, turn off the
    // TIMER0B interrupt.
    //
    if(g_ui32Counter == NUMBER_OF_INTS)
    {
        //
        // Disable the Timer0A interrupt.
        //
        IntDisable(INT_TIMER0A);

        //
        // Turn off Timer0A interrupt.
        //
        TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

        //
        // Clear any pending interrupt flag.
        //
        TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    }
}

int
main(void)
{

    uint32_t ui32SysClock;


    uint32_t ui32PrevCount = 0;

    //
    // Set the clocking to run directly from the external crystal/oscillator.
    // TODO: The SYSCTL_XTAL_ value must be changed to match the value of the
    // crystal on your board.
    //

    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_OSC), 25000000);
    //
    // The Timer0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    //
    // Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for Timer operation.
    //
    InitConsole();

    //
    // Display the example setup on the console.
    //
    UARTprintf("32-Bit Timer Interrupt ->");
    UARTprintf("\n   Timer = Timer0B");
    UARTprintf("\n   Mode = Periodic");
    UARTprintf("\n   Number of interrupts = %d", NUMBER_OF_INTS);
    UARTprintf("\n   Rate = 1ms\n\n");

    //
    // Configure Timer0B as a 32-bit periodic timer.
    //
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    //
    // Set the Timer0B load value to 5s.
    //
    TimerLoadSet(TIMER0_BASE, TIMER_A, 5*ui32SysClock); //max value of 1 byte ie (2^16)-1
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the Timer0B interrupt for timer timeout.
    //
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the Timer0B interrupt on the processor (NVIC).
    //
    IntEnable(INT_TIMER0A);

    //
    // Initialize the interrupt counter.
    //
    g_ui32Counter = 0;

    //
    // Enable Timer0B.
    //
    TimerEnable(TIMER0_BASE, TIMER_A);

    //
    // Loop forever while the Timer0B runs.
    //
    while(1)
    {
        //
        // If the interrupt count changed, print the new value
        //
        if(ui32PrevCount != g_ui32Counter)
        {
            //
            // Print the periodic interrupt counter.
            //
            UARTprintf("Number of interrupts: %d\r", g_ui32Counter);
            ui32PrevCount = g_ui32Counter;
        }
    }
}
