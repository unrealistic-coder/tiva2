//****************************************************************************
//*
//*
//* File: SerialDriver.h
//*
//* Synopsis: Serial Driver interface - Header File
//*
//*
//****************************************************************************

#ifndef SERIAL_DRIVER_H
#define SERIAL_DRIVER_H

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>



//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------
#define BAUD_9600           (9600)
#define BAUD_19200          (19200)
#define BAUD_38400          (38400)
#define BAUD_57600          (57600)
#define BAUD_115200         (115200)

#define DATA_8BITS			(1)
#define DATA_7BITS			(0)

#define SERIAL_PROTOCOL_COUNT 4
#define MAX_SERIAL_BUFFER_SIZE  256
//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

typedef union __attribute__((packed))
{
    uint16_t statusAll;
    struct
    {
        uint16_t batTestOK:1;
        uint16_t battTestFail:1;
        uint16_t battSilence:1;
        uint16_t shutdownActive:1;
        uint16_t TestinProgress:1;
        uint16_t EPO:1;
        uint16_t UPSFailed:1;
        uint16_t BypassActive:1;
        uint16_t BattLow:1;
        uint16_t utilityFail:1;
        uint16_t LiveStatus:1;
        uint16_t outputRelay:1;
        uint16_t inputRelay:1;
        uint16_t INV:1;
        uint16_t pfc:1;
        uint16_t dcTodc:1;
    }Bits;
}HTTP_STATUS_STRUCT;
typedef enum
{
    // Put serial instances ahead of the rest
    PORT_UART1 = 0,
    PORT_UART2 = 1,
    PORT_UART3 = 2,
    PORT_UART4 = 3,
    SYSTEM_SERIAL_PORT_COUNT=4,
    PORT_INVALID = 255
} UART_PORT;

// Baudrate. 
typedef enum
{
    SERIALPORT_BAUD_NULL = 0,
	SERIALPORT_BAUD_2400,
    SERIALPORT_BAUD_9600,  // = 9600,
    SERIALPORT_BAUD_14400,  // = 14400,
    SERIALPORT_BAUD_19200, // = 19200,
    SERIALPORT_BAUD_38400, // = 38400,
    SERIALPORT_BAUD_57600, // = 57600,
    SERIALPORT_BAUD_115200 // = 115200

} SERIALPORT_BAUDRATE;

typedef enum
{
    SERIALPORT_PARITY_NONE  = 0,
    SERIALPORT_PARITY_ODD   = 1,
    SERIALPORT_PARITY_EVEN  = 2
} SERIALPORT_PARITY;

typedef enum
{
    SERIALPORT_DATA_7BIT = 0,//7 
    SERIALPORT_DATA_8BIT = 1,//8,    
    SERIALPORT_DATA_INVALID = 2,
} SERIALPORT_DATABIT;


typedef enum
{
    SERIALPORT_STOPBIT_INVALID = 0,
    SERIALPORT_STOPBIT_1 = 1,
    SERIALPORT_STOPBIT_2 = 2       
} SERIALPORT_STOPBIT;

typedef enum
{
    SERIAL_STDIO = 0,
    SERIAL_GSM_GPRS,
    SERIAL_MODBUS_RTU_MASTER,
    SERIAL_MODBUS_RTU_SLAVE,
    SERIAL_INVALID = 0xFF   // Invaliid port
} SERIAL_PROTOCOL_TYPE;

typedef void PROTOCOL_FUNC_TX(void *protStruct );
typedef void PROTOCOL_FUNC_RX(void *protStruct, uint8_t character );
typedef void PROTOCOL_FUNC_RXERR( void *protStruct,uint8_t error );
typedef BOOL PROTOCOL_FUNC_STARTUP( void *protStruct, UART_PORT port );
typedef void PROTOCOL_FUNC_SHUTDOWN( void *protStruct, UART_PORT port );

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
    PROTOCOL_FUNC_RX        *rxChar;
    PROTOCOL_FUNC_TX        *txComplete;
    PROTOCOL_FUNC_RXERR     *rxErr;
    PROTOCOL_FUNC_STARTUP   *startup;
    PROTOCOL_FUNC_SHUTDOWN  *shutdown;
} SERIAL_PROTOCOL_FUNC;

typedef struct __attribute__((packed))
{
    uint32_t                UARTBase;
    SERIALPORT_BAUDRATE     baudrate;       // BAUDRATE
    SERIALPORT_PARITY       parity;         // PARITY
    SERIALPORT_DATABIT      databit;        // DATABIT
    SERIALPORT_STOPBIT      stopbit;        // STOPBIT
    SERIAL_PROTOCOL_TYPE    protocolType;   // Protocol Type
} SERIALPORT_CONFIG;

typedef struct __attribute__((packed))
{
    uint16_t  slaveAddress;
    uint16_t  responseTime;
    uint16_t  interframeDelay;
    uint16_t  pollRate;

} SERIAL_RTU_CONFIG;

typedef struct __attribute__((packed))
{
    // Link counters - 7
    uint32_t  Hard_Fault;
    uint32_t  ExceptionFault;
    uint32_t  DateTimeHardFault;
    uint32_t  DateTimeExceFault;
    uint32_t  OsBootHardFault;
    uint32_t  OsBootExceFault;
    uint32_t  HardFaultCode;
    uint32_t  ExecFaultAddress;
    uint32_t  ResHardFault;
    uint32_t  ResExecFault;

} SERIAL_DIAG_COUNTER;


typedef struct __attribute__((packed))
{
    // ============
    UART_PORT                   serialPort;       // internal. May be redundent
    SERIAL_PROTOCOL_TYPE        protocolType;
    SERIALPORT_CONFIG           *pSerialPortCfg;   // internal use
    SERIAL_RTU_CONFIG           pSerialCfg;
    SERIAL_DIAG_COUNTER         serialDiag;

    // =============
    // Receiver
    uint8_t      rxBuffer[MAX_SERIAL_BUFFER_SIZE];    // NULL on init
    uint16_t     rxBufferSize;  // 0 on init
    uint8_t      waitRspNode;

    // =============
    // Transmitter
    uint8_t      txBuffer[MAX_SERIAL_BUFFER_SIZE];    // NULL on init
    uint16_t     txRemainingSize;  // 0 on init
    uint16_t     txBufferIndex;  // 0 on init

    // =============
//    ENUM_MODBUS_RX_STATE    rxState;
//    ENUM_MODBUS_TX_STATE    txState;
//    ENUM_MODBUS_MASTER_STATE    masterState;
//
//    // Timer
//    COM_TIMER_ID                respWaitTimer;    // Response Timer
//    COM_TIMER_ID                rxInterFrameTimer;  // T3.5 timer
//    COM_TIMER_ID                txInterFrameTimer;  // T3.5 timer
//    COM_TIMER_ID                pollRateTimer;    // Poll Timer
} SERIAL_RAM_STRUCT;

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

uint8_t CalcCRC8(const uint8_t *inpBuffer,int inSize);
//void UPS_UartTxIntEnable (void);
void UPS_UartTxIntEnable (UART_PORT Port);
void UPS_UartTxIntDisable (UART_PORT port);
void UPS_UartRxIntEnable (UART_PORT port);
void UPS_UartRxIntDisable (UART_PORT port);
void UPS_UartIntHandler(void);
//void UPS_UartWriteChar ( uint8_t dataByte);
void UPS_UartWriteChar ( uint8_t dataByte, UART_PORT port);
void  GSM_Uart2IntHandler(void);

void UPS_InitProtocol(void);
void UPS_ReInitProtocol(UART_PORT Port);
void UPS_StdioRx(void *protStruct, uint8_t character );
void UPS_StdioTx(void *protStruct);
BOOL UPS_StdioStartup (void *protStruct, UART_PORT port );
void UPS_StdioShutdown (void *protStruct, UART_PORT port );
void UPS_StdioRxErr (void *protStruct, UART_PORT port );
void UPS_UART_Config_FromEEPROM(UART_PORT port);
void HTTPResponse_UART_ResetDiagnostic(uint8_t Port);
void UPS_UART0_ISR(void);
void UPS_UART1_ISR(void);
void UPS_UART2_ISR(void);
void UPS_UART3_ISR(void);

void UPS_UartInit(void);
SERIAL_PROTOCOL_TYPE UPS_GetProtocolType( UART_PORT port );
void* UPS_GetProtocolDrvStruct(UART_PORT port);

uint16_t HTTPRequest_UPS_Get_Protocol (UART_PORT port);
uint16_t HTTPRequest_UPS_Get_BaudRate (UART_PORT port);
uint16_t HTTPRequest_UPS_Get_DataBits (UART_PORT port);
uint16_t HTTPRequest_UPS_Get_StopBits (UART_PORT port);
uint16_t HTTPRequest_UPS_Get_Parity   (UART_PORT port);

void HTTPResponse_UPS_Put_BaudRate (UART_PORT port, uint32_t BaudRate);
void HTTPResponse_UPS_Put_DataBits (UART_PORT port, uint32_t Index);
void HTTPResponse_UPS_Put_StopBits (UART_PORT port, uint32_t Index);
void HTTPResponse_UPS_Put_Parity   (UART_PORT port, uint32_t Index);

void HTTP_TrapSetParam(uint16_t Index, uint32_t Value);
void HTTP_FlagSetParam(uint16_t Index, uint32_t Value);
void HTTP_StatusDisplay(uint16_t Index);


//============================================================================
//                              > EXTERN VARIABLES <
//----------------------------------------------------------------------------
extern SERIALPORT_CONFIG upsPortConfig[SYSTEM_SERIAL_PORT_COUNT];
extern HTTP_STATUS_STRUCT pStatus;
#endif

