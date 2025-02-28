#ifndef __WEBSOCKET_H
#define __WEBSOCKET_H
#include <Common.h>
#include "UPSInterface.h"
#include "lwip/api.h"
#include "cmsis_os.h"
#include "SPIFlashTask.h"
#include "FTPServer.h"
#define MAX_VALUE                   (65535)
#define MIN_VALUE                   (-1)

#define MAKE16BIT(B1,B2)        (((uint16_t)B2 << 8) | ((uint16_t)(B1) & 0xFF))
#define MAKE32BIT(B1,B2,B3,B4)  (((uint32_t)B4 << 24) | ((uint32_t)B3 << 16) | ((uint32_t)B2 << 8) | (B1))

#define TYPES_BAUDRATE              (5)
#define TYPES_DATABITS              (2)
#define TYPES_STOPBITS              (2)
#define TYPES_PARITY                (3)

#define WS_USE_SDRAM                0
#define WS_PORT                     8765
#define WS_MAX_CLIENTS              3
#define WS_SEND_BUFFER_SIZE         1024
#define WS_MSG_BUFFER_SIZE          1024 //512
#define WS_CLIENT_RECV_BUFFER_SIZE  1024
#define NO_OF_MOBILE_REG            (5)

#define WS_GUID     "258EAFA5-E914-47DA-95CA-C5AB0DC85B11\0"

#define WS_SEND_BUFFER_START_ADDR   (0xC0000000)
#define WS_SEND_BUFFER_END_ADDR     (WS_SEND_BUFFER_START_ADDR + WS_SEND_BUFFER_SIZE)

#define HTTPSCOKET_CONN_USE_SDRAM                0
#define HTTPSCOKET_CONN_PORT                     80
#define HTTPSCOKET_CONN_MAX_CLIENTS              3
#define HTTP_CONN_SEND_BUF_SIZE                  4096
#define HTTPS_CONN_MSG_BUF_SIZE                  1024//512
#define HTTP_CONN_RX_BUF_SIZE                    1024
#define MAX_PAGESCMD                              25

#define WS_ConnTimeout              3000

#define STARTBYTES                  85
#define MAX_SUB_MODULES             0
#define MAX_MODULES                 0
#define MAX_READ_SUB_GROUPS         0
#define MAX_WRITE_SUB_GROUPS        0
#define MAX_RESET_SUB_GROUPS        0
#define MAX_EMAIL_SUB_GROUPS        0

#define TYPES_BAUDRATE              (5)
#define TYPES_DATABITS              (2)
#define TYPES_STOPBITS              (2)
#define TYPES_PARITY                (3)

typedef enum
{
    WS_TYPE_STRING = 0x81,
    WS_TYPE_BINARY = 0x82
} ws_type_t;


typedef enum
{
    WEBSOCKET_READ_RESPONSE             = 0X81,
    WEBSOCKET_WRITE_RESPONSE            = 0X82,
    WEBSOCKET_START_READING             = 0X83,
    WEBSOCKET_STOP_READING              = 0X84,
    WEBSOCKET_UNSOLICITED               = 0XC0,
    WEBSOCKET_READ_ERROR                = 0XC1,
    WEBSOCKET_WRITE_ERROR               = 0XC2,
    WEBSOCKET_START_READING_ERROR       = 0XC3,
    WEBSOCKET_STOP_READING_ERROR        = 0XC4

} WEBSOCKET_RESPONSE_FNC_CODE;

typedef enum
{
    WEBSOCKET_WRITE                     = 0X00,
    WEBSOCKET_READ                      = 0X01,
    WEBSOCKET_RESET                     = 0X02,
    WEBSOCKET_EMAIL_CONFIG              = 0x03,
    WEBSOCKET_MAX_FNC_CODE              = 0x04
} WEBSOCKET_FNC_CODE;

typedef enum
{
    WEBSOCKET_ALL_DATA_GROUP            = 0X00,
    WEBSOCKET_ALL_SETTING               = 0x01,
    WEBSOCKET_DATALOG_GROUP             = 0X02,
    WEBSOCKET_NO_OF_DATALOGS            = 0X03,
    MAX_READ_GROUPS                     = 0x04

} WEBSOCKET_READ_GROUP;

typedef enum
{
    WEBSOCKET_ETHERNET_SETTING          = 0x00,
    WEBSOCKET_SNMP_SETTING              = 0x01,
    WEBSOCKET_MODBUS_TCP_SETTING        = 0x02,
    WEBSOCKET_MON_SETTING               = 0x03,
    WEBSOCKET_COM1_SETTING              = 0x04,
    WEBSOCKET_PROTOCOL_SETTING          = 0x05,
    WEBSOCKET_COM2_SETTING              = 0x06,
    WEBSOCKET_GSMSMS_SETTING            = 0x07,
    WEBSOCKET_FLAG_SETTING              = 0x08,
    WEBSOCKET_SCHEDULAR_SETTING         = 0x09,
    WEBSOCKET_DATALOG_SETTING           = 0x0A,
    WEBSOCKET_ALARMLOG_SETTING          = 0x0B,
    WEBSOCKET_SNMPTRAP_SETTING          = 0x0C,
    WEBSOCKET_DATETIME_SETTING          = 0x0D,
    WEBSOCKET_LOGINTERVAL_SETTING       = 0x0E,
    MAX_WRITE_GROUPS                    = 0x0F

} WEBSOCKET_WRITE_GROUP;

typedef enum
{
    WEBSOCKET_COM1_DIAG_SETTING         = 0x00,
    WEBSOCKET_COM2_DIAG_SETTING         = 0x01,
    WEBSOCKET_MODBUSTCP_DIAG_SETTING    = 0x02,
    WEBSOCKET_SNMP_DIAG_SETTING         = 0x03,
    WEBSOCKET_ETHERNET_DIAG_SETTING     = 0x04,
    MAX_RESET_GROUPS                    = 0x05

} WEBSOCKET_RESET_GROUP;

typedef enum
{
    WEBSOCKET_SAVE_EMAIL_CONFIG         = 0x00,
    WEBSOCKET_TEST_EMAIL                = 0x01,
    MAX_EMAIL_GROUPS                    = 0x02

} WEBSOCKET_EMAIL_GROUP;

typedef struct __attribute__((packed))
{
    // Link counters - 7
    uint32_t  framesRecv;
    uint32_t  frameSent;
    uint32_t  goodTransaction;
    uint32_t  goodException;
    uint32_t  clientConnections;
    uint32_t  otherError;

} HTTPCONN_SOCKET_DIAG_COUNTER;

typedef struct __attribute__((packed))
{
   char pages[MAX_PAGESCMD];
    uint8_t pagecmdsize;
}HTTP_SERVER_CONN_PAGE;


extern HTTP_SERVER_CONN_PAGE httpconnGETcommndList[20];

typedef struct
{
	uint8_t *protocol;
	uint32_t prtcl_size;
	uint8_t *message;
	uint32_t msg_size;
	ws_type_t msg_type;
} ws_msg_t;

typedef struct
{
	uint16_t LogFormate;
	REG_uint8_t Log_cnt_check;
	uint8_t Reserved;
} DataLog_Headers;


typedef struct
{
	struct netconn *accepted_sock;
	osThreadId taskID;
//	void *server_ptr;
    uint8_t      rxBuffer[WS_CLIENT_RECV_BUFFER_SIZE];    // NULL on init
    uint16_t     rxBufferSize;  // 0 on init
    struct netbuf *inbuf;
    uint8_t  txBuffer[WS_SEND_BUFFER_SIZE+10];
    uint8_t  send_buff[WS_SEND_BUFFER_SIZE+10];
    uint16_t txIndex;
	uint8_t established;
	ws_type_t msg_type;
	uint8_t *payload;
} WS_CLIENT_STRUCT;

typedef struct
{

	struct netconn *ws_Server;
	WS_CLIENT_STRUCT ws_clients[WS_MAX_CLIENTS];
	uint32_t connected_clients_cnt;
} WEB_SOCKET_STRUCT;

typedef struct
{
	uint8_t startByte_1;
	uint8_t startByte_2;
	uint8_t Request_id;
	uint8_t FunctionCode;
	uint8_t Module;
	uint8_t subModule;
	uint8_t Group;
	uint8_t subGroup;
}WS_HEADER_STRUCT;

typedef struct __attribute__((packed))
{
    // Link counters - 7
    uint32_t  framesRecv;
    uint32_t  frameSent;
    uint32_t  WebSocketCloseCounter;
    uint32_t  HandshakeCounter;
    uint32_t  goodException;
    uint32_t  clientConnections;
    uint32_t  otherError;
    uint32_t  LenthError;

} WEBSOCKET_TCP_DIAG_COUNTER;

typedef struct __attribute__((packed))
{
    // ============
    int sockfd;
    struct sockaddr_in remote;
    // =============
    // Receiver
    uint8_t      rxBuffer[WS_CLIENT_RECV_BUFFER_SIZE];    // NULL on init
    uint16_t     rxBufferSize;  // 0 on init
    // =============
    // Transmitter
    uint8_t      txBuffer[WS_SEND_BUFFER_SIZE];    // NULL on init
    uint16_t     txRemainingSize;  // 0 on init

    uint8_t  send_buff[WS_SEND_BUFFER_SIZE];
    uint16_t txIndex;
	uint8_t established;
	ws_type_t msg_type;
	uint8_t *payload;

    uint32_t connectionTimeoutTime;
    uint32_t startTimeConnectionTimeout;
} WEBSOCKET_TCP_CONN_STRUCT;

typedef struct __attribute__((packed))
{
    // ============

    WEBSOCKET_TCP_DIAG_COUNTER    WebsocketDiag;
    WEBSOCKET_TCP_CONN_STRUCT     ClientConnection[WS_MAX_CLIENTS];
    SPIFlashFatFs_FILESYSTEM_STRUCT  webspiflashFileSystemStruct;
    uint8_t ws_clientIndex;
    int listenSockfd;
} WEBSOCKET_TCP_RAM_STRUCT;

extern WEBSOCKET_TCP_RAM_STRUCT WebSocketStruct;
#if defined WEBSERVER_USING_SPI_FLASH
typedef struct __attribute__((packed))
{
    // ============
    FIL fileHandle;
    int sockfd;
    struct sockaddr_in remote;
    // =============
    // Receiver
    uint8_t      rxBuffer[HTTP_CONN_RX_BUF_SIZE];    // NULL on init
    uint16_t     rxBufferSize;  // 0 on init
    // =============
    // Transmitter
    uint8_t      txBuffer[HTTP_CONN_SEND_BUF_SIZE];    // NULL on init
    uint16_t     txRemainingSize;  // 0 on init

    uint8_t  send_buff[HTTPS_CONN_MSG_BUF_SIZE];
    uint16_t txIndex;
    uint8_t established;
    ws_type_t msg_type;
    uint8_t *payload;
    HTTP_SERVER_CONN_PAGE *httpcmd;
    uint32_t connectionTimeoutTime;
    uint32_t startTimeConnectionTimeout;
}HTTP_SOCKET_CONN_STRUCT;


typedef struct __attribute__((packed))
{

    HTTPCONN_SOCKET_DIAG_COUNTER  HttpConnectDiag;
    HTTP_SOCKET_CONN_STRUCT ClientConnection[HTTPSCOKET_CONN_MAX_CLIENTS];

    uint8_t httpconn_clientIndex;
    int listenSockfd;

}HTTPCONNSOCKET_RAM_STRUCT;

extern HTTPCONNSOCKET_RAM_STRUCT HttpServerStruct;
void HtppServer_netconn_thread_SocketOperation(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruc1ptr);
void HttpConnServer_Init(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruct);
#endif
void WS_Task(void const * argument);
uint8_t GetDays(Time from, Time to);
Time GetNextDate(Time current, uint8_t daysToAdd);
void WS_Init(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketStruct1);
void WS_SocketOperation(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr);
void ws_send_message(WS_CLIENT_STRUCT *pWebsocket, ws_msg_t *msg );
uint16_t isLeapYear(uint16_t year);
#endif

/***************************** END OF FILE ************************************/

