
/*
 *  ======== enet_httpServer_tirtos.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "Board.h"

// it has NDK socket.h not the BSD support in sys/socket.h
#include <ti/ndk/inc/netmain.h>

/* file system header file */
#include "fs/index.h"
#include "fs/styles.h"
#include "fs/javascript.h"


#define TASKSTACKSIZE   512

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*
 * An entry for the file getTime.cgi is translated into the C function getTime().
 * Whenever a HTTP POST is made to the file getTime.cgi, the getTime() function
 * is called.
 */
//Int getTime(SOCKET s, int length)
//{
//    Char buf[200];
//    static UInt scalar = 0;
//
//    if (scalar == 0) {
//        scalar = 1000000u / Clock_tickPeriod;
//    }
//    /*
//     * Send the status of this request to the client.
//     * Ex:
//     *      HTTP/1.1 200 OK
//     *      Content-Type: text/html
//     */
//    httpSendStatusLine(s, HTTP_OK, CONTENT_TYPE_HTML);
//
//    httpSendClientStr(s, CRLF);
//    /*
//     * Below lines of code construct the body of the HTML to
//     * send to the browser. When opened on the browser it will
//     * display the Elapsed Time.
//     */
//    httpSendClientStr(s,
//        "<html><head><title>SYS/BIOS Clock "\
//	"Time</title></head><body><h1>Elapsed Time</h1>\n");
//    System_sprintf(buf, "<p>Up for %d seconds</p>\n",
//        ((unsigned long)Clock_getTicks() / scalar));
//    httpSendClientStr(s, buf);
//    httpSendClientStr(s, "</table></body></html>");
//    return (1);
//}

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
    efs_createfile("javascript.js", JAVA_SIZE, (UINT8 *)JAVA);
    efs_createfile("styles.css", STYLES_SIZE, (UINT8 *)STYLES);
}

Void RemoveWebFiles(Void)
{
    /*
     * Destroy a RAM Based file.
     */
    efs_destroyfile("index.html");
    efs_destroyfile("javascript.js");
    efs_destroyfile("styles.css");
}

/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    while (1) {
        Task_sleep((unsigned int)arg0);
        GPIO_toggle(Board_LED0);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;
    /* Call board init functions */
    Board_initGeneral();
    Board_initEMAC();
    Board_initGPIO();

    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);

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
