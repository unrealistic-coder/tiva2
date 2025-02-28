/*
 * httpserver_websock.c
 *
 *  Created on: 17-Dec-2024
 *      Author: 14169
 */




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
 *  ======== enet_httpServer_tirtos.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "Board.h"

#include <ti/ndk/inc/netmain.h>

/* file system header file */
#include "fs/index.h"
#include "fs/styles.h"
#include "fs/javascript.h"
#include "websockHandler.h"

#define TASKSTACKSIZE   512


Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*
 * An entry for the file getTime.cgi is translated into the C function getTime().
 * Whenever a HTTP POST is made to the file getTime.cgi, the getTime() function
 * is called.
 */


static int handleInput(SOCKET htmlSock, int ContentLength, char *pArgs) {
    char* names[6] = {"vasu","devan","suriya","deepak","harish","rohit"};
    char *userInput = NULL;  // To store user input
    char *buffer, *key, *value;
    int len, parseIndex;
    char htmlbuf[MAX_RESPONSE_SIZE];

    // Step 1: Allocate buffer for the request
    buffer = (char*) mmBulkAlloc(ContentLength + 1);
    if (!buffer) goto ERROR;

    // Step 2: Read the data from the client
    len = NDK_recv(htmlSock, buffer, ContentLength, MSG_WAITALL);
    if (len < 1) goto ERROR;

    // Step 3: Parse the POST data using cgiParseVars() or custom parsing
    parseIndex = 0;
    buffer[ContentLength] = '\0';  // Null-terminate the buffer

    // Process request variables until done
    while (parseIndex != -1) {
        key = (char*)cgiParseVars(buffer, &parseIndex);
        value = (char*)cgiParseVars(buffer, &parseIndex);

        if (!strcmp("userInput", key)) {
            userInput = value;  // Store the user input
        }
    }

    // Step 4: Send HTTP response (status line)
    httpSendStatusLine(htmlSock, HTTP_OK, CONTENT_TYPE_HTML);
    httpSendClientStr(htmlSock, CRLF);

    //check names/ commands
    int i = 0,flag = 0;
    for(i = 0;i<6;i++){
        if(!strcmp(names[i],userInput)) {
            System_sprintf(htmlbuf,"%s is a valid name",userInput);
            flag = 1;
            break;
        }
    }
    if(!flag)
        System_sprintf(htmlbuf,"%s is a invalid name",userInput);


    httpSendClientStr(htmlSock, htmlbuf);

    // Clean up and return
ERROR:
    if (buffer) mmBulkFree(buffer);
    return 1;
}


/*
 * AddWebFiles() is the user defined hook function that is called when the stack
 * is configured. See httpServer.cfg for details.
 *
 * var Global = xdc.useModule('ti.ndk.config.Global');
 * var Http = xdc.useModule('ti.ndk.config.Http');
 * Global.IPv6 = false;
 * var http0Params = new Http.Params();
 * var http0 = Http.create(http0Params);
 * Global.stackInitHook = "&AddWebFiles";
 * Global.stackDeleteHook = "&RemoveWebFiles";
 */
Void AddWebFiles(Void)
{
    /*
     * efs_createfile creates RAM based file. This function creates an internal
     * record of the RAM based file with the indicated filename, file length, and
     * data pointer. Refer to TI Network Developer's Kit (NDK) API Reference Guide
     * (SPRU524K) for details.
     */
    efs_createfile("index.html", INDEX_SIZE, (UINT8 *)INDEX);
    efs_createfile("handleInput.cgi", 0, (UINT8 *)&handleInput);
    efs_createfile("javascript.js", JAVA_SIZE, (UINT8 *)JAVA);
    efs_createfile("styles.css", STYLES_SIZE, (UINT8 *)STYLES);
}

Void RemoveWebFiles(Void)
{
    /*
     * Destroy a RAM Based file.
     */
    efs_destroyfile("index.html");
    efs_destroyfile("handleInput.cgi");
    efs_destroyfile("javascript.js");
    efs_destroyfile("styles.css");

}
Void tcpWorker(UArg arg0, UArg arg1)
{
    int  clientfd = (int)arg0;
    int  bytesRcvd;
    int  bytesSent;
    char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);
    fdOpenSession((void *)Task_self());
    /*
     * Socket function recv() is used to receive incoming data to buffer
     * specified by pbuf. The same data is echoed back to the remote host
     * using send().
     */
    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
        //if websocket upgrade request
        //handle websocket handshake
        //else error
        /* sample client request
         *  GET /chat HTTP/1.1
            Host: example.com:8000
            Upgrade: websocket
            Connection: Upgrade
            Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
            Sec-WebSocket-Version: 13
         *
         */
//        if(strstr(buffer,"Upgrade: websocket")) // return pointer to first occurence of upgrade
//            websocket_respond(buffer);
        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
        if (bytesSent < 0 || bytesSent != bytesRcvd) {
            System_printf("Error: send failed.\n");
            break;
        }
    }
    fdCloseSession((void *)Task_self());
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);

    close(clientfd);
}

/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */
Void tcpHandler(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    int                server;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    int                optval;
    int                optlen = sizeof(optval);
    socklen_t          addrlen = sizeof(clientAddr);
    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;

    /*
     * Create a TCP stream socket.
     */
    fdOpenSession((void *)Task_self());
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    /*
     * Instructs TCP protocol to listen for connections with up to NUMTCPWORKERS active
     * participants that can wait for a connection.
     */
    status = listen(server, NUMTCPWORKERS);
    if (status == -1) {
        System_printf("Error: listen failed.\n");
        goto shutdown;
    }

    /*
     * Setup socket option as to enable keeping connections alive
     */
    optval = 1;
    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        System_printf("Error: setsockopt failed\n");
        goto shutdown;
    }

    /*
     * Accept a connection from a remote host.
     */
    while ((clientfd =
            accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1) {

        System_printf("tcpHandler: Creating thread clientfd = %d\n", clientfd);

        /* Init the Error_Block */
        Error_init(&eb);

        /*
         * Initialize the defaults and set the parameters.
         * Create a new task for each connection to process the receiving data
         * from the remote host.
         */
        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 1280;
        taskParams.priority = 2;
        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);
        if (taskHandle == NULL) {
            System_printf("Error: Failed to create new Task\n");
            close(clientfd);
        }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);
    }

    System_printf("Error: accept failed.\n");

shutdown:
    if (server > 0) {
        close(server);
    }
    fdCloseSession((void *)Task_self());
}

///*
// *  ======== heartBeatFxn ========
// *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
// *  is configured for the heartBeat Task instance.
// */
//Void heartBeatFxn(UArg arg0, UArg arg1)
//{
//    while (1) {
//        Task_sleep((unsigned int)arg0);
//        GPIO_toggle(Board_LED0);
//    }
//}

/*
 *  ======== main ========
 */

int main(void)
{
    Task_Params taskParams;
    Task_Handle taskHandle;
    /* Call board init functions */
    Board_initGeneral();
    Board_initEMAC();
    Board_initGPIO();
    Error_Block eb;

    Error_init(&eb);
    Task_Params_init(&taskParams);
    taskParams.stackSize = TCPHANDLERSTACK;
    taskParams.priority = 1;
    taskParams.arg0 = TCPPORT;
    taskHandle = Task_create((Task_FuncPtr)tcpHandler, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("Failed to create tcpHandler Task\n");
    }

    System_flush();

     /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the HTTP Webserver example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
