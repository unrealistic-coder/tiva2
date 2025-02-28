
/*
 *    ======== tcpEchoServer.c ========
 *    Contains BSD sockets code.
 */

//TODO: fatfs folder in this path was deleted
// C:\ti\tirtos_tivac_2_16_01_14\products\tidrivers_tivac_2_16_01_13\packages\ti\mw
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "file_handle.h"
#include "websocket.h"
#include "source/ff.h"


#define TCPPACKETSIZE 512
#define NUMTCPWORKERS 3
#define WEBSOCKET_OPCODE_TEXT 0x1


Void tcpWorker(UArg arg0, UArg arg1)
{
    fdOpenSession((void *)Task_self());
    int  clientfd = (int)arg0;
    int  bytesRcvd;
//    int  bytesSent;
    char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);
    System_flush();

    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
        buffer[bytesRcvd] = '\0';  // Ensure null termination
//        System_printf("Request:%s\n",buffer);

        if (bytesRcvd >= 5) {  // Check only if valid request received
            if (!Mount()) {
                System_printf("Failed to mount filesystem!");
            }
            System_printf("Filesystem mounted successfully.\n");

            if (strncmp(buffer, "GET / ", 6) == 0) {
                if(strstr(buffer, "Upgrade: websocket") != NULL && strstr(buffer, "Connection: Upgrade") != NULL) {
                    // websock reposnse
                    if(handle_websocket_handshake(clientfd, buffer)) {
                        // websocket communication
                        // create_websocket(); // to swicth to another port
                        System_printf("Websocket started\n"); System_flush();
                        while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
//                            sendPongFrame(clientfd);
//                            buffer[bytesRcvd] = '\0';

                            // TODO: handle websocket frames form client
                            uint8_t opcode = buffer[0] & 0x0F;  // Extract opcode
                            uint8_t masked = buffer[1] & 0x80;  // Check if frame is masked
//                            uint8_t fin = buffer[0] & 0x80; // check if frames is fragmented
                            uint64_t payload_length = buffer[1] & 0x7F;  // Extract payload length
                            uint8_t offset = 2;

                            // extract payload length
                            if (payload_length == 126) {
                                // next 2 byte length
                                payload_length = (buffer[offset] << 8) | buffer[offset + 1];
                                offset += 2;
                            } else if (payload_length == 127) {
                                // next 8 byte length
                                payload_length = 0;
                                uint8_t i = 0;
                                for (i = 0; i < 8; i++) {
                                    payload_length = (payload_length << 8) | buffer[offset + i];
                                }
                                offset += 8;
                            }

                            // if masked extract masking key
                            uint8_t masking_key[4];
                            if (masked) {
                                memcpy(masking_key, &buffer[offset], 4);
                                offset += 4;
                            }

                            // Decode the payload
                            uint64_t i = 0;
                            char payload[payload_length + 1];
                            for (i = 0; i < payload_length; i++) {
                                payload[i] = masked ? (buffer[offset + i] ^ masking_key[i % 4]) : buffer[offset + i];
                            }
                            payload[payload_length] = '\0';
                            memset(buffer, 0, sizeof(buffer));



                            // check commands
                            if(!strcmp(payload,"vasu")) {
                                send_websocket_frame(clientfd, payload, payload_length);
                            }
                            else {
                                send_websocket_frame(clientfd,"Invalid", strlen("Invalid"));
                            }

                            System_printf(payload); System_flush();


                            //TODO: hanlde fragmemted frames if neccessary
                        }
                        System_printf("\nWebsocket closed\n"); System_flush();

                    } else {
                        const char bad_request[] =
                                "HTTP/1.1 400 Bad Request\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 15\r\n\r\n"
                                "400 Bad Request";

                        send(clientfd, bad_request, strlen(bad_request), 0);
                    }
                } else {
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n");
                send(clientfd, response, strlen(response), 0);
                // send chunks

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                send_file_web("index.htm", clientfd);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                }
            }
            else if (strncmp(buffer, "GET /styles.css", 15) == 0) {

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                                         "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: text/css\r\n"
                                         "Transfer-Encoding: chunked\r\n"
                                         "\r\n");
                send(clientfd, response, strlen(response), 0);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                send_file_web("styles.css", clientfd);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
            else if (strncmp(buffer, "GET /javascript.js", 18) == 0) {

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: application/javascript\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n");
                send(clientfd, response, strlen(response), 0);
                // send chunks
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                send_file_web("script.js", clientfd);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
            else {
                // 404 Response for unmatched requests
                const char not_found[] =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "404 Not Found";

                send(clientfd, not_found, strlen(not_found), 0);
            }

            //unmount flash
            if (!UnMount()) {
                System_printf("Failed to unmount filesystem! Error: %d\n", res);
            }
            System_printf("Filesystem unmounted successfully.\n");
        }
//        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
    }
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);
    System_flush();

    close(clientfd);
    fdCloseSession((void *)Task_self());
}

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
        taskParams.stackSize = 4096;
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
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();
    format_disk();
    store_webpages();

    System_printf("Starting the TCP Echo Server example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();



    /* Start BIOS */
    BIOS_start();

    return (0);
}
