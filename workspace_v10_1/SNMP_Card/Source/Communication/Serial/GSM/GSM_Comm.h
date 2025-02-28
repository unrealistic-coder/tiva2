/******************************************************************************
FILE       : GSM_Comm.h
VERSION    : 1.0
AUTHORS    : Rani A Sonawane
DATE       : 26/10/2023
BRIEF      : GSM GPRS Serial Interface Header file
//==============================================================================
 *******************************************************************************/

#ifndef __GSM_H__
#define __GSM_H__

//==============================================================================
// INCLUDE FILES
//==============================================================================
#include "Common.h"
#include "UPSInterface.h"
#include "COMTimer.h"

#define  GSM_ATC_RXSIZE           25          //  at-command rx buffer size
#define  GSM_ATC_SEARCH_MAX       6            //  maximum of always search in buffer
#define MAX_GSM_BUFFER_SIZE	  128

typedef struct __attribute__((packed))
{
    // Link counters - 7
    uint32_t  charRecv;
    uint32_t  framesRecv;
    uint32_t  charSent;
    uint32_t  frameSent;
    uint32_t  NumberofSMSsent;
    uint32_t  NumberofSMSNotsent;
    uint32_t  ModemState;
    uint32_t  ModemConnected;
    uint32_t  TransctionmPassCnt;
	uint32_t  TransactionFailedCnt;
} GSM_DIAG_COUNTER;

extern enum {
	    STATUS_INIT = 0,
		STATUS_OK,
	    STATUS_ERROR,
	    STATUS_CONN_CLOSED,
		SYSTEM_BAUD_RATE_SET,
		MODEM_BAUD_RATE_SET,
		DEFAULT_BAUD_RATE_SET,
		ATE0_CMD_RESP_OK,
		AT_CMD_RESP_OK,
		MODEM_CONFIG_IN_TEXT_MODE,
		MOBILE_NUM_NOT_VALID,
		SENDING_MOBILE_NUM,
		SENDING_SMS_INFO,
	    STATUS_TIMEOUT,
	    STATUS_OK_4G
}modem_status;

#define QUERY_BUFF_SIZE           61 
#define GsmGprsCommon_delay(x)	  vTaskDelay(x)

typedef struct __attribute__((packed))

{
    //Diagnostic Counters
    uint32_t  charRecv;
    uint32_t  framesRecv;
    uint32_t  charSent;
    uint32_t  frameSent;
    uint32_t  goodTransaction;
    uint32_t  goodException;
    uint32_t  mismatchErrors;
    uint32_t  badCRC;
    uint32_t  respTimeout;
    uint32_t  otherError;
} GSM_UART_DIAG_COUNTER;


typedef union
{
    uint16_t all;
    uint8_t byte[2];

    struct
    {
         uint8_t GSMSMS_Enabled:1;
         uint8_t GSM_Enabled:1;
         uint8_t GPRS_Enabled:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b8:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
    } bits;
} GSM_SettingStruct;

typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {

        uint16_t CONFIG_GPRS_Port_Number;
        uint16_t CONFIG_Server_Connection_Interval;
        REG_uint32_t CONFIG_GPRS_IP_Address;
        REG_uint64_t CONFIG_MobilNumbers[5];
        uint16_t CONFIG_AlarmGroupsForSMS;
        uint16_t CONFIG_GSMSettReserved;//[71];//74
        uint32_t GSMBaudRate;
        GSM_SettingStruct ConfigFlags;
        uint16_t CRC_CheckGSMSett;

    };

}CONFIG_GSMSettings;
typedef enum
{
    STATE_GSM_RX_IDLE = 0,
    STATE_GSM_RX_RECEIVING,
	STATE_GSM_RX_RECEIVED,
    STATE_GSM_RX_PROCESSING,
    STATE_GSM_RX_STATE_COUNT
} ENUM_GSM_RX_STATE;

typedef enum
{
    STATE_GSM_TX_IDLE = 0,
    STATE_GSM_TX_TRANSMITTING,
    STATE_GSM_TX_TRANSMITTED,
    STATE_GSM_TX_PKT_COMPLETED,
    STATE_GSM_TX_STATE_COUNT
} ENUM_GSM_TX_STATE;

typedef enum
{
    STATE_GSM_REQUEST_IDLE = 0,
	STATE_GSM_REQUEST_BUILD,
	STATE_GSM_REQUEST_TRANSMITTING,
    STATE_GSM_REQUEST_SENT,
    STATE_GSM_REQUEST_WAIT_RSP,
}ENUM_GSM_REQUEST_STATE;


typedef struct
{
  uint8_t           power:1;
  uint8_t           netReg:1;
  uint8_t           netChange:1;
  uint8_t           textMode:1;

}GSM_STATUS;

enum{

    SET_RMON_SERVERIP=93,           //93
    GET_RMON_SERVERIP,

    SET_RMON_CONN_INTERVAL,
    GET_RMON_CONN_INTERVAL,

    SET_RMON_DISP_REBOOT,
    DUMMY_6,

    SET_RMON_FACTSETT,
    GET_RMON_FACTSETT,

    SET_RMON_CLIENTSETT,
    GET_RMON_CLIENTSETT,

    SET_RMON_MODEMTEST,
    GET_RMON_MODEMTEST        //104

};

typedef struct __attribute__((packed))
{
    // ============
    UART_PORT                	serialPort;       // internal. May be redundent
    CONFIG_GSMSettings	    GsmCfg;
    GSM_DIAG_COUNTER   	GsmDiag;

	uint8_t 		  signal;
	GSM_STATUS	  status;

	//=================================Msg Commands=======================//
  	uint8_t rxBuffer[GSM_ATC_RXSIZE];
  	uint8_t recvbyte;
  	uint16_t rxIndex;
  	uint32_t GSMBaudRateSet;
    // =============
    // Transmitter
    uint8_t      txBuffer[MAX_GSM_BUFFER_SIZE*2];    // NULL on init
    uint16_t     txRemainingSize;  // 0 on init
    uint16_t     txBufferIndex;  // 0 on init
    // =============
    ENUM_GSM_RX_STATE    rxState;
	ENUM_GSM_TX_STATE    txState;
	ENUM_GSM_REQUEST_STATE    requestState;
    // Timer
    COM_TIMER_ID                TransactionTimout;    // Response Timer
    COM_TIMER_ID                rxInterFrameTimer;  // T3.5 timer
} GSM_RAM_STRUCT;

extern GSM_RAM_STRUCT  GsmStruct;
extern char	SendDatabuff[45];
extern uint8_t SerialBuff2[QUERY_BUFF_SIZE];  //40
//==============================================================================
// GLOBAL FUNCTION DECLARATIONS
//==============================================================================
void GSM_Modem_Init(uint32_t ui32Base, uint32_t BaudRate);
void GSM_Perform_IO(uint8_t Port);
void MX_UART2_Init(uint32_t baudrate);
void GPRS_Communication(uint8_t Port);
uint8_t Modem_CheckQuery(uint8_t * MDM_QueryBuff);
int8_t GsmCommon_GSMcommand(const char * command, uint32_t timeout_ms, char * answer, uint16_t answer_size, int items, char * str1, char * str2);
void GsmCommon_SendCommand(GSM_RAM_STRUCT * pGsmStruct, uint8_t * data, uint16_t len);
BOOL SetModemBaudRate(uint32_t BaudRate);
BOOL GSMModemDiagnosticsAtPowerUp(uint32_t ModemBaud, uint32_t systemsetBaud);
void GSM_CommTimeoutInit();
void GsmCommon_Timeout_AppTimeout(COM_TIMER_ID id, void * param);
void GSM_Timeout_AppTimeout(COM_TIMER_ID id, void * param);
void GSM_Timeout_RxInterChar(COM_TIMER_ID id, void * param);
void GSM_Timeout_RxInterFrame(COM_TIMER_ID id, void * param);
void GsmCommon_Timeout_RxInterFrame(COM_TIMER_ID id, void * param);
void GsmCommon_Transaction_Timeout(COM_TIMER_ID id, void * param);
void GSMRxCallback(uint8_t character);
#endif // __GSM_H__
//==============================================================================
// END OF FILE
//==============================================================================
