

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
#define LED_ON() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

static uint32_t g_sysclock;

int loop;
void delay(){
    for(loop = 0; loop<2000000; loop++);
}

void sysClock(void){
    g_sysclock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), CLOCK_FREQ);
}

void configureGPIOOUTPUT(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

void main(void)
{
    sysClock();
    configureGPIOOUTPUT();
    while(1){
       LED_ON();
       delay();
       LED_OFF();
       delay();
    }
}
