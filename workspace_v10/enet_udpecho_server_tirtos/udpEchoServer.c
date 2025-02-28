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

/*
 *    ======== udpEcho.c ========
 *    Contains BSD sockets code.
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

/*
 *  ======== udpHandler ========
 *  Task to echo UDP messages.
 *
 */
Void udpHandler(UArg arg0, UArg arg1)
{
    int                bytesRcvd;
    int                bytesSent;
    int                status;
    int                server;
    fd_set             readSet;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    socklen_t          addrlen;
    char               buffer[UDPPACKETSIZE];

    /*
     * Create a UDP datagram socket.
     */
    server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server == -1) {
        System_printf("Error: socket not created.\n");
        goto shutdown;
    }

    /*
     * Setup the local IP address and port to bind to.
     */
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    /*
     * Associates and reserves a port for use by the socket
     */
    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        System_printf("Error: bind failed.\n");
        goto shutdown;
    }

    do {
        /*
         *  readSet and addrlen are value-result arguments, which must be reset
         *  in between each select() and recvfrom() call
         */
        FD_ZERO(&readSet); /* initializes the file descriptor set readSet to be empty */
        FD_SET(server, &readSet); /* add server to the file descriptor set readSet */
        addrlen = sizeof(clientAddr);

        /*
         * Wait forever for the reply. The select() function blocks until
         * there is activity on any of the specified sets of file descriptors.
         */
        status = select(server + 1, &readSet, NULL, NULL, NULL);
        if (status > 0) {
            /*
             * if server is a member of the file descriptor set readSet
             */
            if (FD_ISSET(server, &readSet)) {
                /*
                 * Use Recvfrom() to receive data from the remote host.
                 * Recvfrom is blocking. Therefore, a call to Recvfrom() will return
                 * immediately only if a packet is available on the socket.
                 */
                bytesRcvd = recvfrom(server, buffer, UDPPACKETSIZE, 0,
                        (struct sockaddr *)&clientAddr, &addrlen);

                if (bytesRcvd > 0) {
                    /*
                     * Echo back the received data using sendto() function.
                     */
                    bytesSent = sendto(server, buffer, bytesRcvd, 0,
                            (struct sockaddr *)&clientAddr, addrlen);
                    if (bytesSent < 0 || bytesSent != bytesRcvd) {
                        System_printf("Error: sendto failed.\n");
                        goto shutdown;
                    }
                }
            }
        }
    } while (status > 0);

shutdown:
    if (server > 0) {
        close(server);
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

    System_printf("Starting the UDP Echo Server example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
