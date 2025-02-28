/*
 * Copyright (c) 2014-2021, Texas Instruments Incorporated
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
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>

#include <dns.h>

/* Example/Board Header file */
#include "Board.h"

#define DNSHANDLERSTACK 2048
#define NUM_HOST 4
#define INET_ADDRSTRLEN 16
static char strIp[INET_ADDRSTRLEN];


/*
 * List of host names to resolve by DNS
 */
const char *hostname[NUM_HOST] = { "pool.ntp.org", "www.google.com", "api.openweathermap.org", "www.ti.com" };
/*
 *  ======== dnsWorker ========
 *  Task to request DNS service
 */
Void dnsWorker(UArg arg0, UArg arg1)
{
    struct addrinfo *server_data = NULL;
    int result;
    int i;

    for (i=0;i<NUM_HOST;i++)
    {
        System_printf("!!!!!!!!!!\n", result);
        System_flush();

        /*
         * Given the host name, which identify a Internet host, getaddrinfo
         * returns one or more addrinfo structure, each of which contains
         * an Internet address that can be specified in a call to bind or
         * connect
         */
        result = getaddrinfo(hostname[i], "0", NULL, &server_data);

        if (result == 0 && server_data != NULL)
        {
            struct sockaddr addr = *(server_data->ai_addr);
            /*
             * convert IP address structure to string format
             */
            inet_ntop(AF_INET, &((struct sockaddr_in *)&addr)->sin_addr, strIp, INET_ADDRSTRLEN);
            System_printf("HOSTNAME: %s\tResolved address:%s\n", hostname[i],strIp);
            System_flush();

            freeaddrinfo(server_data);
            server_data = NULL;
        }

        Task_sleep(1000); // Sleep for 1 seconds
    }
}

/*
 *  ======== dnsHooks ========
 *  Creates new Task to request DNS service.
 */
Void dnsHooks(UArg arg0, UArg arg1)
{

    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;

    /* Init the Error_Block */
    Error_init(&eb);

    /* Initialize the defaults and set the parameters. */
    Task_Params_init(&taskParams);
    taskParams.stackSize = DNSHANDLERSTACK;
    taskParams.priority = 1;
    taskHandle = Task_create((Task_FuncPtr)dnsWorker, &taskParams, &eb);
    if (taskHandle == NULL) {
        System_printf("Error: Failed to create new Task\n");
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

    System_printf("Starting the DNS request example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
