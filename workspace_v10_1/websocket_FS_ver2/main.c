
/*
 *    ======== tcpEchoServer.c ========
 *    Contains BSD sockets code.
 */
//TODO: C:/ti/tirtos_tivac_2_16_01_14/packages
// removed from general->variables->show sysstem variables->COM_TIRTSC_TIRTOS_REPOS

#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Startup.h>
#include "SST26VF032B.h"


#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>
#include <ti/sysbios/knl/Clock.h>

/* NDK BSD support */
#include <sys/socket.h>

/* Example/Board Header file */
#include "Board.h"

// C std headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Tiva C Drivers
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include <source/user_diskio.h>
#include <encoder.h>// this has sha 1 and b64 encode
#include <fats.h> // fatfs related functions are here
#include <fsdata.h> // folder structure and file array are maintained here


#define TCPPACKETSIZE 560
#define NUMTCPWORKERS 3
#define BUFFER_SIZE 1024
#define DELAY_MS(i)      (Task_sleep(((i) * 1000) / Clock_tickPeriod))
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* get_requested_path(char* request) {
    // GET /fs/index.html HTTP/1.1
    // Host: 192.168.1.100
    // User-Agent: Mozilla/5.0
    // Accept: text/html

    char* start = strstr(request, "GET ");
    if (!start) return NULL;  // Not a GET request

    start += 4;  // Move past "GET "
    char* end = strstr(start, " ");
    if (!end) return NULL;  // Invalid request format

    //first root page
    if(strncmp(start,"/",1) == 0){
        return "/MainPage.shtml";
    }

    *end = '\0';  // Terminate string at the first space after the path
    return start;  // Return the requested path (e.g., "/fs/index.html")
}

void send_404_response(int sock) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
                             "HTTP/1.1 404 Not Found\r\n"
                             "Content-Type: text/html\r\n\r\n"
                             "<html><body><h1>404 Not Found</h1></body></html>\r\n"
                             );
    send(sock, response, strlen(response), 0);
}
// MIME Type Detection
const char* get_mime_type(const char* file_path) {

    char* ext = strrchr(file_path, '.');
    if (!ext) return "application/octet-stream";  // Default unknown type

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".shtml") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";

    return "application/octet-stream";
}

void send_http_headers(char* file_path, int sock) {
    char response[BUFFER_SIZE];

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Transfer-Encoding: chunked\r\n"
             "Connection: keep-alive\r\n"
             "\r\n",
             get_mime_type(file_path));

    send(sock, response, strlen(response), 0);
}

Void tcpWorker(UArg arg0, UArg arg1) {
    int clientfd = (int)arg0;
    char buffer[TCPPACKETSIZE];
    mount();
    bool keep_alive = 1;
    while (1) {  // Handle multiple requests per connection
        int bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0);
//        if (bytesRcvd <= 0) break;  // Exit loop if client closes connection
        // Check if request contains `Connection: close`
        if (strstr(buffer, "Connection: close") != NULL) {
            keep_alive = 0; // Close after this request
        }


        buffer[bytesRcvd] = '\0';  // Null-terminate request
        char* path = get_requested_path(buffer);
        System_printf("path:(%s)\n",path); System_flush();
        if (path == NULL) {
            send_404_response(clientfd);
            continue;
        }
        send_http_headers(path, clientfd);
        char root[100];
        snprintf(root, sizeof(root), "/fs%s", path);
        System_printf("Opening file: %s\n", root);
        System_flush();

        if(!send_file_web(root, clientfd)){
            send(clientfd, "0\r\n\r\n",5, 0); // send end chunk
            send_404_response(clientfd);// Serve the requested file
        }
        if(!keep_alive) break;
    }
    unmount();
    close(clientfd);
    Task_exit();
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
        taskParams.stackSize = 16384;
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
    Task_exit();  // Ensure the task exits properly
}

/*
 * Startup function
 */
void flash_init(void)
{

//    format_disk();
    System_printf("Storing the webpages in SPI Flash\n");
    System_flush();

    mount();

    //TODO: can add a logic where i can check for these files exist already and skip storing

/*     Store the folder structure maintained in fsdata.h   */
    if (store_folder(&root_folder, "") != 0) {
        System_printf("Failed to store folder structure.\n");
    } else {
        System_printf("Folder structure stored successfully.\n");
    }
//    System_flush();

/*    SCAN THE FOLDER AND LIST ALL FOLDERS AND FILES   */
    char buff[100];
    strcpy(buff, "/");
    if (scan_files(buff) != FR_OK) {
        System_printf("Failed to scan dir");
        System_flush();
        return;
    }

    unmount();
    System_printf("Webpages stored successfully in SPI Flash\n");
    System_flush();
    Task_exit();
}

void led_blink(void){
    while(1){
        LED_ON_MACRO();
        DELAY_MS(1000);
        LED_OFF_MACRO();
        DELAY_MS(1000);
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
    Led_Init();


    System_printf("Starting the TCP Echo Server example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    // SysMin will only print to the console when you call flush or exit
    System_flush();
//     Create a task for one-time initialization
    Task_Handle taskHandle;
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = 512; // Adjust stack size as needed
    taskParams.priority = 1;    // Set an appropriate priority
    taskHandle = Task_create((Task_FuncPtr)led_blink, &taskParams, NULL);
    if (taskHandle == NULL) {
        System_printf("Error: Failed to create new Task\n");
        System_flush();
    }

    //     Create a task for flash - init
//    Task_Handle taskHandle1;
//    Task_Params taskParams1;
//    Task_Params_init(&taskParams1);
//    taskParams1.stackSize = 50550; // Adjust stack size as needed
//    taskParams1.priority = 1;    // Set an appropriate priority
//    taskHandle1 = Task_create((Task_FuncPtr)flash_init, &taskParams1, NULL);
//    if (taskHandle1 == NULL) {
//        System_printf("Error: Failed to create new Task\n");
//        System_flush();
//    }

    BIOS_start();

    return (0);
}
