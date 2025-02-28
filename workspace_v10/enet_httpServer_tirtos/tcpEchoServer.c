

/*
 *    ======== tcpEchoServer.c ========
 *    Contains BSD sockets code.
 */

#include <string.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>


#define BSD 1 // user defined macro, implying we are using BSD Sockets here.
#include <ti/ndk/inc/netmain.h> //netmain.h->stacksys.h-> macro BSD = 0

/* Example/Board Header file */
#include "Board.h"

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3

/* Web server files */
#include "fs/index.h"
#include "fs/styles.h"
#include "fs/javascript.h"


/* Web server file handling */
Void AddWebFiles(Void)
{
    efs_createfile("index.html", INDEX_SIZE, (UINT8 *)INDEX);
    efs_createfile("javascript.js", JAVA_SIZE, (UINT8 *)JAVA);
    efs_createfile("styles.css", STYLES_SIZE, (UINT8 *)STYLES);
}

Void RemoveWebFiles(Void)
{
    efs_destroyfile("index.html");
    efs_destroyfile("javascript.js");
    efs_destroyfile("styles.css");
}


#include <stdio.h>
#include <stdlib.h>
#include "sha1.h" // Include a SHA-1 library (you'll need one)
#include "base64.h" // Include a Base64 encoding library (you'll need one)

// Function to process the WebSocket handshake
int processWebSocketHandshake(int clientfd, char *request, int requestLength) {
    char *keyStart, *keyEnd, key[256];
    char response[512];
    const char *webSocketGUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char sha1Hash[20];
//    char base64Key[64];
//    char *acceptKey;

    // Find "Sec-WebSocket-Key" in the HTTP headers
    keyStart = strstr(request, "Sec-WebSocket-Key: ");
    if (!keyStart) {
        System_printf("Error: Missing Sec-WebSocket-Key header\n");
        System_flush();
        return 0;
    }

    // Extract the WebSocket key
    keyStart += strlen("Sec-WebSocket-Key: ");
    keyEnd = strstr(keyStart, "\r\n");
    if (!keyEnd) {
        System_printf("Error: Malformed Sec-WebSocket-Key header\n");
        System_flush();
        return 0;
    }
    int keyLength = keyEnd - keyStart;
    strncpy(key, keyStart, keyLength);
    key[keyLength] = '\0';

    // Concatenate the key with the GUID
    char concatenatedKey[512];
    snprintf(concatenatedKey, sizeof(concatenatedKey), "%s%s", key, webSocketGUID);

    // Compute SHA-1 hash of the concatenated key
    SHA1((unsigned char *)concatenatedKey, strlen(concatenatedKey), sha1Hash);

    // Encode the SHA-1 hash in Base64
    char* base64Key = base64_encode((char*)sha1Hash);
    if(base64Key == NULL)
        System_printf("Error: Failed to allocate heap memory\n");
        System_flush();
        return 0;

    // Construct the WebSocket handshake response
    snprintf(response, sizeof(response),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n"
             "\r\n",
             base64Key);
    free(base64Key);
    // Send the response back to the client
    if (send(clientfd, response, strlen(response), 0) < 0) {
        System_printf("Error: Failed to send WebSocket handshake response\n");
        System_flush();
        return 0;
    }

    System_printf("WebSocket handshake response sent successfully\n");
    System_flush();
    return 1;
}

void handleWebSocketFrame(int clientfd, char *frame, int frameLength) {
    unsigned char opcode;
    unsigned char mask[4];
    unsigned char *payload;
    int payloadLength;
    int i;

    // Parse the frame header
    opcode = frame[0] & 0x0F; // Extract opcode
    payloadLength = frame[1] & 0x7F;

    if (payloadLength == 126) {
        // Extended payload length (2 bytes)
        payloadLength = (frame[2] << 8) | frame[3];
        memcpy(mask, frame + 4, 4);
        payload = (unsigned char *)(frame + 8);
    } else if (payloadLength == 127) {
        // Extended payload length (8 bytes, not supported for simplicity)
        System_printf("Error: Payload length too large\n");
        System_flush();
        return;
    } else {
        // Normal payload length
        memcpy(mask, frame + 2, 4);
        payload = (unsigned char *)(frame + 6);
    }

    // Unmask the payload
    for (i = 0; i < payloadLength; i++) {
        payload[i] ^= mask[i % 4];
    }

    // Process the opcode
    switch (opcode) {
        case 0x1: // Text frame
            System_printf("Received Text: %.*s\n", payloadLength, payload);
            System_flush();
            break;
        case 0x8: // Connection close frame
            System_printf("WebSocket client requested to close the connection\n");
            System_flush();
            close(clientfd);
            return;
        default:
            System_printf("Received unsupported opcode: 0x%X\n", opcode);
            System_flush();
            break;
    }

    // Echo the payload back to the client (optional)
    send(clientfd, frame, frameLength, 0);
}


void webWorker(UArg arg0, UArg arg1)
{
  int  clientfd = (int)arg0;
    int  bytesRead;
//    int  bytesSent;
    unsigned char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);
    System_flush();

    /*
     * Socket function recv() is used to receive incoming data to buffer
     * specified by pbuf. The same data is echoed back to the remote host
     * using send().
     */
    /* Perform WebSocket handshake */
    bytesRead = recv(clientfd, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0 || !processWebSocketHandshake(clientfd, buffer, bytesRead)) {
        System_printf("Error: Invalid WebSocket handshake\n");
        System_flush();
        close(clientfd);
        return;
    }

    System_printf("WebSocket handshake successful\n");
    System_flush();

    /* Handle WebSocket communication */
    while ((bytesRead = recv(clientfd, buffer, sizeof(buffer), 0)) > 0) {
        handleWebSocketFrame(clientfd, buffer, bytesRead);
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
        taskHandle = Task_create((Task_FuncPtr)webWorker, &taskParams, &eb);
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
