#include <stdint.h>
#include <stdbool.h>
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"

//uint32_t g_ui32SysClock;

void
InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

//void
//sysClock(void)
//{
//    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 20000000);
//}

void
InitButtons(void)
{
    // sysClock(); // enable this to use buttons or add here
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);//enable port j
    while(! SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));//check clock enable
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);// config port j bit 0 to input
    GPIODirModeSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
}

