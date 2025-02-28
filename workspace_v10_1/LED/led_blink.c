

/**
 * main.c
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "driverlib/debug.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

#include "driverlib/emac.h" // For EMAC functions
#include "inc/hw_emac.h"   // For direct register access

#define CLOCK_FREQ 120000000
#define LED_ON() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

static uint32_t g_sysclock;

int loop;
void delay(){
    for(loop = 0; loop<2000000; loop++);
}

void sysClock(void){
    g_sysclock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_320), CLOCK_FREQ);
}

void configureGPIOOUTPUT(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

void main(void)
{
    sysClock();
    configureGPIOOUTPUT();

//    // Replace with your board's MAC address
//    uint8_t macAddress[6] = {0xB6,0x1A,0x00,0xCC,0x19,0x03};
//
//    // Initialize the Ethernet MAC with the new MAC address
//    EMACAddrSet(EMAC0_BASE, 0, macAddress);
//    uint8_t mac[6] = {0};
//    EMACAddrGet(EMAC0_BASE, 0, mac);

    while(1){
       LED_ON();
       printf("on\n");
       delay();
       LED_OFF();
       printf("off\n");
       delay();
    }
}
