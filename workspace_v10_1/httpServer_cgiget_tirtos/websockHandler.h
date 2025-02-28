

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

#define TCPPORT 8080
#define TCPHANDLERSTACK 1024
//uint8_t websocket_respond(buffer)

/*
 *  ======== tcpWorker ========
 *  Task to handle TCP connection. Can be multiple Tasks running
 *  this function.
 */
Void tcpWorker(UArg arg0, UArg arg1);

/*
 *  ======== tcpHandler ========
 *  Creates new Task to handle new TCP connections.
 */
Void tcpHandler(UArg arg0, UArg arg1);


