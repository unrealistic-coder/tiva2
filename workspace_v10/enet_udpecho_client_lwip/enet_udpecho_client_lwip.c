//*****************************************************************************
//
// enet_udpecho_client_lwip.c - Sample UDP Echo Client using lwIP.
//
// Copyright (c) 2013-2021 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/lwiplib.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "drivers/pinout.h"
#include "lwip/udp.h"
#include "lwip/inet.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Ethernet UDP Echo Client with lwIP (enet_udpecho_client_lwip)</h1>
//!
//! This UDP Echo client application demonstrates the operation of the Tiva
//! Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain
//! an Ethernet address.  If DHCP times out without obtaining an address,
//! AutoIP will be used to obtain a link-local address.  The address that is
//! selected will be shown on the UART.
//!
//! UART0, connected to the ICDI virtual COM port and running at 115,200,
//! 8-N-1, is used to display messages from this application.
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://savannah.nongnu.org/projects/lwip/
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for the Sever IP Address.
// NOTE: User must enter the correct server IP address
//
// Port 23 is chosen for telenet connection.

// TODO: User must change these settings per their application.
//
//*****************************************************************************
#define SERVER_IPADDR "192.168.254.87"
#define CLIENT_PORT 23
#define SERVER_PORT 23

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0



//*****************************************************************************
//
// Global flag that will be set to 1 when the DHCP address is acquired
//
//*****************************************************************************
uint32_t g_ui32ipAddrValid = 0;

//*****************************************************************************
//
// The current IP address.
//
//*****************************************************************************
uint32_t g_ui32IPAddress;

//*****************************************************************************
//
// The system clock frequency.
//
//*****************************************************************************
uint32_t g_ui32SysClock;

//*****************************************************************************
//
// Volatile global flag to manage LED blinking, since it is used in interrupt
// and main application.  The LED blinks at the rate of SYSTICKHZ.
//
//*****************************************************************************
volatile bool g_bLED;

//*****************************************************************************
//
// Global LwIP variables.
//
//*****************************************************************************
struct udp_pcb *upcb;

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
// Global flags to be set when the callback function is called.
//
//*****************************************************************************
struct cbFlag {
    volatile bool EchoRecv;
    volatile bool EchoPoll;
    uint32_t len;
} g_scbFlag;

//*****************************************************************************
//
// Display an lwIP type IP Address.
//
//*****************************************************************************
void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    //
    // Convert the IP Address into a string.
    //
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    //
    // Display the string.
    //
    UARTprintf(pcBuf);

    //
    // Set the flag to indicate the IP address is acquired
    g_ui32ipAddrValid = 1;

}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void
lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    //
    // Get the current IP address.
    //
    ui32NewIPAddress = lwIPLocalIPAddrGet();

    //
    // See if the IP address has changed.
    //
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        //
        // See if there is an IP address assigned.
        //
        if(ui32NewIPAddress == 0xffffffff)
        {
            //
            // Indicate that there is no link.
            //
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            //
            // There is no IP address, so indicate that the DHCP process is
            // running.
            //
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            //
            // Display the new IP address.
            //
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
        }

        //
        // Save the new IP address.
        //
        g_ui32IPAddress = ui32NewIPAddress;
    }

    //
    // If there is not an IP address.
    //
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
        //
        // Do nothing and keep waiting.
        //
    }
}

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(SYSTICKMS);

    //
    // Tell the application to change the state of the LED (in other words
    // blink).
    //
    g_bLED = true;
}

//*****************************************************************************
//
// This callback function is called when payload is received
//
//*****************************************************************************
void
UdpEchoRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
            struct ip_addr *addr, u16_t port)
{
    if (p != NULL) {
        //
        // Set global flag and print a message in main() indicating the number of
        // bytes that has been received from the remote host.
        //
        g_scbFlag.EchoRecv = 1;
        g_scbFlag.len = p->tot_len;
        //
        // Send received packet back to sender.
        //
        udp_sendto(pcb, p, addr, port);

        //
        // Free the pbuf.
        //
        pbuf_free(p);
    }
}

//*****************************************************************************
//
// Initialize UDP client and send a greeting message to the server.
//
//*****************************************************************************
void
UdpClientInit(void)
{
    struct pbuf *p;
    struct ip_addr ServerIPaddr;
    err_t err;
    char *greeting = "Hello! Greeting from EK-TM4C129XL LaunchPad \r\n";

    //
    // Create a new UDP control block.
    //
    upcb = udp_new();

    if (upcb!=NULL)
    {
        //
        // Bind the PCB to local address=IP_ADDR_ANY at port 23.
        // Also register the callback function to call when receiving payload.
        //
        udp_bind(upcb, IP_ADDR_ANY, CLIENT_PORT);
        udp_recv(upcb, UdpEchoRecv, NULL);

        //
        // Assign destination server IP address.
        //
        ServerIPaddr.addr = inet_addr(SERVER_IPADDR);

        //
        // Configure destination IP address and port.
        //
        err = udp_connect(upcb, &ServerIPaddr, SERVER_PORT);

        if (err == ERR_OK)
        {
            //
            // Allocate pbuf from pool.
            //
            p = pbuf_alloc(PBUF_TRANSPORT,strlen((char*)greeting), PBUF_RAM);

            if (p != NULL)
            {
                //
                // Copy greeting to pbuf.
                //
                pbuf_take(p, (char*)greeting, strlen((char*)greeting));

                //
                // Send the UDP data.
                //
                udp_send(upcb, p);

                //
                // Free pbuf.
                //
                pbuf_free(p);

            }
            else
            {
                //
                // Free the UDP connection, so we can accept new clients.
                //
                udp_remove(upcb);
                UARTprintf("\n\r can not allocate pbuf ");
            }
        }
        else
        {
            //
            // Free the UDP connection, so we can accept new clients.
            //
            udp_remove(upcb);
            UARTprintf("\n\r can not connect udp pcb");
        }
    }
    else
    {
        UARTprintf("\n\r can not create udp pcb");
    }

    //
    // Disconnect the remote address from the PCB.
    //
    udp_disconnect(upcb);

}

//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

    //
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    //
    MAP_SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    //
    // Run from the PLL at 120 MHz.
    // Note: SYSCTL_CFG_VCO_240 is a new setting provided in TivaWare 2.2.x and
    // later to better reflect the actual VCO speed due to SYSCTL#22.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                             SYSCTL_OSC_MAIN |
                                             SYSCTL_USE_PLL |
                                             SYSCTL_CFG_VCO_240), 120000000);

    //
    // Configure the device pins.
    //
    PinoutSet(true, false);

    //
    // Configure UART.
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);

    //
    // Clear the terminal and print banner.
    //
    UARTprintf("\033[2J\033[H");
    UARTprintf("Ethernet lwIP UDP Echo client example\n\n");

    //
    // Configure Port N1 for as an output for the animation LED.
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);

    //
    // Initialize LED to OFF (0).
    //
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, ~GPIO_PIN_1);

    //
    // Configure SysTick for a periodic interrupt.
    //
    MAP_SysTickPeriodSet(g_ui32SysClock / SYSTICKHZ);
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    //
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        // Let the user know there is no MAC address
        //
        UARTprintf("No MAC programmed!\n");
        while(1)
        {
        }
    }

    //
    // Tell the user what we are doing just now.
    //
    UARTprintf("Waiting for IP.\n");

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    //
    // Initialize the lwIP library, using DHCP.
    //
    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    //
    // Wait until the IP address is acquired before the client connect
    // to the server.
    //
    while (g_ui32ipAddrValid == 0);

    //
    // Start the client to send a greeting message to the external host.
    //
    UdpClientInit();

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    //
    // Loop forever, processing the LED blinking.  All the work is done in
    // interrupt handlers.
    //
    while(1)
    {
        if (g_scbFlag.EchoRecv == 1)
        {
            g_scbFlag.EchoRecv = 0;
            UARTprintf("\rReceiving %d bytes of data", g_scbFlag.len);
        }

        if (g_bLED == 1)
        {
            //
            // Clear the flag.
            //
            g_bLED = false;

            //
            // Toggle the LED.
            //
            MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                             (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1) ^
                                     GPIO_PIN_1));
            SysCtlDelay(g_ui32SysClock / 3);

        }
    }

}


