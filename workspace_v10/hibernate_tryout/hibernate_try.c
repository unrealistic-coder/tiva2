
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_hibernate.h"
#include "driverlib/sysctl.h"
#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "inc/hw_ints.h"

/*#############################################################################################################################################################*/
#define LED_ON_PN0() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 , GPIO_PIN_0 ) //PN0 LED for normal mode
#define LED_OFF_PN0() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_0 , 0)
#define LED_ON_PN1() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_1 , GPIO_PIN_1) //PN1 LED for hibernation mode
#define LED_OFF_PN1() GPIOPinWrite( GPIO_PORTN_BASE, GPIO_PIN_1 , 0)

// The variable g_ui32SysClock contains the system clock frequency in Hz.
uint32_t g_ui32SysClock;
void init_clock(void){
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL), 120000000);
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

void config_gpio_N(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

void init_hibernate(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_HIBERNATE));
    HibernateEnableExpClk(0); //0 External 32.786-kHZ clock source is enabled.
    HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);
    HibernateRTCEnable(); //enhable RTC
    HibernateRTCSet(0); // reset RTC to zero
    HibernateGPIORetentionEnable();
    HibernateWakeSet(HIBERNATE_WAKE_RTC|HIBERNATE_WAKE_PIN); //wake up by RTC match event and Wake up pin
}

//void init_RTCwakeup(void){
//    // Set the RTC match for a wakeup event after 10 seconds
//    uint32_t ui32CurrentTime = HibernateRTCGet();  // Get current RTC time
//    HibernateRTCSet(0);                            // Reset RTC to 0
//    HibernateRTCMatchSet(0, ui32CurrentTime + 10); // Set match 10 seconds later
//    // Enable the wake-up event on RTC match

//}

//void ManualWakeup(void){
//    HibernateIntEnable(HIBERNATE_INT_PIN_WAKE);
//}

// Main function
int main(void)
{
    config_gpio_N();
    init_clock();
    init_hibernate();
    HibernateRequest();
//    while(1);
    configureUART0();
//    init_RTCwakeup();
    while(1){
//    if (HibernateIsActive())
//    {
        // System is in hibernation mode
        LED_OFF_PN0(); // Turn OFF LED PN0 for normal mode
        LED_ON_PN1();  // Turn ON LED PN1 for hibernation mode
        SysCtlDelay(g_ui32SysClock/4); // Delay before entering hibernation


//    }
//    else{
        LED_ON_PN0();  // Turn ON LED PN0 for normal mode
        LED_OFF_PN1(); // Turn OFF LED PN1 for hibernation mode indication
        // After a short delay, enter hibernation mode
        SysCtlDelay(g_ui32SysClock/4); // Delay before entering hibernation

//    }
//    SysCtlDelay(g_ui32SysClock/400);
//    HibernateDisable();
    }
}
