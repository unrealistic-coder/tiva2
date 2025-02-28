#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
/**
 * main.c
 */

uint32_t g_ui32SysClock;
uint8_t count;

void sysClock(void){
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), 20000000);
}

void configureUART0(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC); //16 Mhz
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

// buttons PJ0 and PJ1 is configured
void configureGPIOINPUT(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);//enable port j
    while(! SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));//check clock enable
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);// config port j bit 0 to input
    GPIODirModeSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
}

int main(void)
{
    sysClock();
    configureUART0();
    configureGPIOINPUT();

    UARTprintf("uart transmitting \r\n");

    while(1){
        uint32_t BUTTON_PRESS_0 = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0);
        uint32_t BUTTON_PRESS_1 = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1);


        if(!(BUTTON_PRESS_0 & GPIO_PIN_0)){
            UARTprintf("count = %d\r\n", count++);
            SysCtlDelay(g_ui32SysClock/12); // 250 ms
        }
        else if(!(BUTTON_PRESS_1 & GPIO_PIN_1)){
            UARTprintf("count = %d\r\n", count--);
            SysCtlDelay(g_ui32SysClock/12);
        }
        SysCtlDelay(g_ui32SysClock/200);
    }
//	return 0;
}
