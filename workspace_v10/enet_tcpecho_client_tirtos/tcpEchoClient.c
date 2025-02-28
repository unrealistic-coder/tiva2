/*
 * Copyright (c) 2014-2015, Texas Instruments Incorporated
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
 *    ======== tcpEchoClient.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>

/* NDK BSD support */
#include <sys/socket.h>

/* Example/Board Header file */
#include "Board.h"

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3

extern int inet_addr(const char *str);

/*
 * Define the IP address and the port number of the TCP server
 * that this client is connection to.
 * TODO: User must change this IP address when running in his network.
 */
#define SERVER_IPADDR "192.168.254.92"
#define SERVER_PORT 23

/*
 *  ======== tcpWorker ========
 *  Task to handle data processing when data is received by client.
 */
Void tcpWorker(UArg arg0, UArg arg1)
{
    int  clientfd = (int)arg0;
    int  bytesRcvd;
    int  bytesSent;
    char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);

    /*
     * Socket function recv() is used to receive incoming data to buffer
     * specified by pbuf. The same data is echoed back to the remote host
     * using send().
     */
    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0)
    {
        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
        if (bytesSent < 0 || bytesSent != bytesRcvd)
        {
            System_printf("Error: send failed.\n");
            break;
        }
    }
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);

    close(clientfd);
}
/*
 *  ======== tcpHandler ========
 *  Manage client socket creation and new Task to handle received data processing
 */
Void tcpHandler(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    struct sockaddr_in servAddr;
    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;
    char msg[] = "Hello from TM4C1294XL Connected LaunchPad\n";

    /*
     * Create a TCP stream socket.
     */
    clientfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientfd < 0) {
        System_printf("tcpHandler: socket failed\n");
        close(clientfd);
    }

    /*
     * Setup the Server IP address and port
     */
    memset((char *) &servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = inet_addr(SERVER_IPADDR);

    /*
     * Connect to the server
     */
    status = connect(clientfd, (struct sockaddr *) &servAddr, sizeof(servAddr));

    if (status < 0) {
        System_printf("Error: client connect failed. (%d)\n",fdError());
        close(clientfd);
    }

    /*
     * Send a greetings message to the server
     */
    send(clientfd, msg, strlen(msg), 0);

    /* Init the Error_Block */
    Error_init(&eb);

    /*
     * Initialize the defaults and set the parameters.
     * Create a new task for each connection to process the receiving data
     * from the remote host
     */
    Task_Params_init(&taskParams);
    taskParams.arg0 = (UArg)clientfd;
    taskParams.stackSize = 1280;
    taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("Error: Failed to create new Task\n");
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

    System_printf("Starting the TCP Echo Client example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
