/*
 * Copyright (c) 2015-2021, Texas Instruments Incorporated
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
 *  ======== sntp.c ========
 *  SNTP Client example application
 */
#include <string.h>
#include <time.h>

/* XDCtools Header files */
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Seconds.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>
#include <ti/net/sntp/sntp.h>

/* Example/Board Header file */
#include "Board.h"

#include <sys/socket.h>

/*
 * NTP server and port to obtain network time and number of servers to attempt
 */
#define NTP_HOSTNAME "north-america.pool.ntp.org"
#define NTP_PORT         "123"
#define NTP_SERVERS      3
#define NTP_SERVERS_SIZE (NTP_SERVERS * sizeof(struct sockaddr_in))
#define NTPTASKSTACKSIZE 2048

/*
 * Time Zone Adjustment.
 * TODO: Adjust your local time zone with respect to Greenwich Mean Time (GMT)
 * For example, North America Central Time Zone is 6 hours behind GMT
 */
#define CENTRAL_TIME_ADJUST -6*60*60
#define TIME_ZONE_ADJUST CENTRAL_TIME_ADJUST

unsigned char ntpServers[NTP_SERVERS_SIZE];
static Semaphore_Handle semHandle = NULL;

/*
 *  ======== printError ========
 */
void printError(char *errString, int code)
{
    System_printf("Error! code = %d, desc = %s\n", code, errString);
    BIOS_exit(code);
}

/*
 *  ======== timeUpdateHook ========
 *  Called after NTP time sync
 */
void timeUpdateHook(void *p)
{
    Semaphore_post(semHandle);
}

/*
 *  ======== startNTP ========
 *  This is the function that generates the request to the NTP server for network time.
 */
void startNTP(void)
{
    int ret;
    int currPos;
    time_t ts;
    struct sockaddr_in ntpAddr;
    struct addrinfo hints;
    struct addrinfo *addrs;
    struct addrinfo *currAddr;
    Semaphore_Params semParams;

    /*
     * Clear the addrinfo first.
     */
    memset(&hints, 0, sizeof(struct addrinfo));

    /*
     * Use SOCK_DGRAM (UDP) as the socket type
     */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    /*
     * First resolve the IP Address of the NTP server using the DNS service..
     * The getaddrinfo can return a linked-list of IP addresses
     */
    ret = getaddrinfo(NTP_HOSTNAME, NTP_PORT, NULL, &addrs);
    if (ret != 0) {
        printError("startNTP: NTP host cannot be resolved!", ret);
    }

    currPos = 0;

    /*
     * Loop through the NTP servers
     */
    for (currAddr = addrs; currAddr != NULL; currAddr = currAddr->ai_next) {
        if (currPos < NTP_SERVERS_SIZE) {
            ntpAddr = *(struct sockaddr_in *)(currAddr->ai_addr);
            /*
             * Copy the list of NTP servers to netServers
             */
            memcpy(ntpServers + currPos, &ntpAddr, sizeof(struct sockaddr_in));
            currPos += sizeof(struct sockaddr_in);
        }
        else {
            break;
        }
    }

    freeaddrinfo(addrs);

    /*
     * Initialize and start the SNTP client Task. Called to create and start SNTP
     * client Task and Semaphores. User must pass in pointers to functions for
     * getting and setting the current time and the list of NTP servers to
     * communicate with. timeUpdateHook() is called upon successful time
     * synchronization.
     */
    ret = SNTP_start(Seconds_get, Seconds_set, timeUpdateHook,
            (struct sockaddr *)&ntpServers, NTP_SERVERS, 0);
    if (ret == 0) {
        printError("startNTP: SNTP cannot be started!", -1);
    }

    /*
     * Create a semaphore that will block until after successful NTP time
     * synchronization.  Once unblocked, we will print the current time to the
     * console.
     */
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    semHandle = Semaphore_create(0, &semParams, NULL);
    if (semHandle == NULL) {
        printError("startNTP: Cannot create semaphore!", -1);
    }

    SNTP_forceTimeSync();
    /*
     * Wait here until the semaphore is posted.
     */
    Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

    /*
     * Adjust for the Central Time Zone.
     */
    Seconds_set((unsigned int)((int)Seconds_get()+TIME_ZONE_ADJUST));
    ts = time(NULL);

    /*
     * Print the current time to the console.
     */
    System_printf("Current time: %s\n", ctime(&ts));
    System_flush();

}

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    static Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

    /* Create a NTP task when the IP address is added */
    if (fAdd && !taskHandle) {
        Error_init(&eb);

        Task_Params_init(&taskParams);
        taskParams.stackSize = NTPTASKSTACKSIZE;
        taskParams.priority = 1;
        taskHandle = Task_create((Task_FuncPtr)startNTP, &taskParams, &eb);
        if (taskHandle == NULL) {
            printError("netIPAddrHook: Failed to create NTP Task\n", -1);
        }
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

    System_printf("Starting the SNTP example\nSystem provider is set to "
            "SysMin. Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
