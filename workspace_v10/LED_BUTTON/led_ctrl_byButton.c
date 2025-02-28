

/**
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/debug.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

#define CLOCK_FREQ 120000000
#define LED_ON() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

uint32_t g_sysclock;

int loop;
void delay(){
    for(loop = 0; loop<200000; loop++);
}

void sysClock(void){
    g_sysclock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), CLOCK_FREQ);
}

void configureGPIOOUTPUT(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);//enable port n
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);// config potn bit 0 and bit 1 to output
}

void configureGPIOINPUT(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);//enable port j
    while(! SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));//check clock enable
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);// config port j bit 0 to input
    GPIODirModeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
}

void main(void)
{
    sysClock();
    configureGPIOOUTPUT();
    configureGPIOINPUT();
    while(1){
        uint32_t BUTTON_PRESS = GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0);
        if(!BUTTON_PRESS){
            LED_ON();
        }
        else{
            LED_OFF();
        }

//       delay();

//       delay();
    }
}
