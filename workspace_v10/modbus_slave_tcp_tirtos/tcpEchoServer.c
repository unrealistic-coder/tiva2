

/*
 *    ======== tcpEchoServer.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include "mb_server.h"
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
#define MIN_MODBUS_REQUEST_LENGTH 12

/*
 *  ======== tcpWorker ========
 *  Task to handle TCP connection. Can be multiple Tasks running
 *  this function.
 */
Void tcpWorker(UArg arg0, UArg arg1)
{
    int  clientfd = (int)arg0;
    int  bytesRcvd;
    int  bytesSent;
    unsigned char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);
    System_flush();

    /*
     * Socket function recv() is used to receive incoming data to buffer
     * specified by pbuf. The same data is echoed back to the remote host
     * using send().
     */
    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {

        // Ensure that the received data is a valid Modbus request
        if (bytesRcvd < MIN_MODBUS_REQUEST_LENGTH) {
            System_printf("Error: Invalid Modbus request length.\n");
            System_flush();
            break;
        }

        // Process Modbus request
        mb_res_pdu response_frame;
        uint16_t responseLength = build_modbus_response(buffer, &response_frame);
        serialize_response(&response_frame, buffer);
        bytesSent = send(clientfd, buffer, responseLength, 0);
        if (bytesSent < 0 || bytesSent != responseLength) {
            System_printf("Error: send failed.\n");
            System_flush();
            break;
        }
    }
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);
    System_flush();
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
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) {
        System_printf("Error: socket not created.\n");
        System_flush();
        goto shutdown;
    } else {
        System_printf("socket created.\n");
        System_flush();
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
        System_flush();
        goto shutdown;
    }

    /*
     * Instructs TCP protocol to listen for connections with up to NUMTCPWORKERS active
     * participants that can wait for a connection.
     */
    status = listen(server, NUMTCPWORKERS);
    if (status == -1) {
        System_printf("Error: listen failed.\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Setup socket option as to enable keeping connections alive
     */
    optval = 1;
    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        System_printf("Error: setsockopt failed\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Accept a connection from a remote host.
     */
    while ((clientfd =
            accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1) {

        System_printf("tcpHandler: Creating thread clientfd = %d\n", clientfd);
        System_flush();

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
        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);
        if (taskHandle == NULL) {
            System_printf("Error: Failed to create new Task\n");
            System_flush();
            close(clientfd);
        }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);
    }

    System_printf("Error: accept failed.\n");
    System_flush();

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
    Board_initGeneral(); //enable gpio pins
    Board_initGPIO(); // no function defnition
    Board_initEMAC(); //fetch user reg , display mac, change if need , ethernet led configure PF0 and PF4 and pin type to eth

    System_printf("Starting the TCP Echo Server example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
