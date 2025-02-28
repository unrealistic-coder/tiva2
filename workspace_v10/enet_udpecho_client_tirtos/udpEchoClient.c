/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <string.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>

/* NDK BSD support */
#include <sys/socket.h>

/* Example/Board Header file */
#include "Board.h"

#define UDPPACKETSIZE 256

extern int inet_addr(const char *str);

/*
 * Define the IP address and the port number of the UDP server
 * that this client is connection to.
 * TODO: User must change this IP address when running in his network.
 */
#define SERVER_IPADDR "192.168.254.92"

/*
 *  ======== udpHandler ========
 *  Manage client socket creation and new Task to process received data.
 *
 */
Void udpHandler(UArg arg0, UArg arg1)
{
    int                bytesRcvd;
    int                bytesSent;
    int                status;
    int                clientfd;
    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               buffer[UDPPACKETSIZE];
    char msg[] = "Hello from TM4C1294XL Connected Launchpad\r\n";

    /*
     * Create a UDP datagram socket.
     */
    clientfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientfd == -1) {
        System_printf("Error: socket not created.\n");
        goto shutdown;
    }

    /*
     * Setup the Local IP address and port
     */
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(arg0);

    /*
     * Associates and reserves a port for use by the socket
     */
    status = bind(clientfd, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    if (status == -1) {
        System_printf("Error: bind failed.\n");
        goto shutdown;
    }

    /*
     * Setup the Server IP address and port to send data to.
     */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(SERVER_IPADDR);
    servAddr.sin_port = htons(arg0);
    addrlen = sizeof(servAddr);

    /*
     * Send the greetings message using sendto() function
     */
    bytesSent = sendto(clientfd, (char *)msg, strlen(msg), 0,
                       (struct sockaddr *)&servAddr, sizeof(servAddr));

    while (1) {

        /*
         * Use Recvfrom() to receive data from the remote host.
         * Recvfrom is blocking. Therefore, a call to Recvfrom() will return
         * immediately only if a packet is available on the socket.
         */
        bytesRcvd = recvfrom(clientfd, buffer, UDPPACKETSIZE, 0,
                             (struct sockaddr *)&servAddr,
                             &addrlen);

        if (bytesRcvd > 0) {
            /*
             * Echo back the received data using sendto() function.
             */
            bytesSent = sendto(clientfd, buffer, bytesRcvd, 0,
                               (struct sockaddr *)&servAddr, sizeof(servAddr));
            if (bytesSent < 0 || bytesSent != bytesRcvd) {
                System_printf("Error: sendto failed.\n");
                goto shutdown;
            }
        }
    }

shutdown:
    if (clientfd > 0) {
        close(clientfd);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the UDP Echo Client example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
