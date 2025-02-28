//****************************************************************************
//*
//*
//* File: ErrorHandler.h
//*
//* Synopsis: Error Handler
//*
//*
//****************************************************************************


#ifndef _ERR_HANDLER_H_
#define _ERR_HANDLER_H_

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
//#include <DatabaseHandler.h>

//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------

#define MODBUS_TCP_TASK_LIMIT 	(1000)
#define SNMP_TASK_LIMIT         (2000)
#define WEBSERVER_TASK_LIMIT	(3000)
#define SYS_TASK_LIMIT 			(3000)
#define SPI_FS_TASK_LIMIT	    (2000)
#define ETHERNET_TASK_LIMIT		(3000)
#define USB_APP_TASK_LIMIT 		(3000)
#define GSM_TASK_LIMIT          (5000)
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef enum __ERRFaultEnum
{
	ERR_NO_FAULT  = 0x0000,
	/* System Faults*/
	ERR_EXCEPTION_FAULT  = 0x0081,
	ERR_MPU_FAULT = 0x0082,
	ERR_BUS_FAULT= 0x0083,
	ERR_USAGE_FAULT = 0x0084,
	ERR_HARD_FAULT = 0x0085,

	/*Watchdog Faults*/
	MODBUS_TCP_TASK_FAULT = 0x0091,
	SNMP_TASK_FAULT = 0x0092,

	WEBSERVER_TASK_FAULT 	= 0x0093,
	SPI_FS_TASK_FAULT 	= 0x0094,
	USB_APP_TASK_FAULT		= 0x0095,
	SYS_TASK_FAULT		= 0x0096,
	ETHERNET_TASK_FAULT = 0x0097,
	ETHERNET_MAC_FAULT = 0x0098,
	GSM_COMM_FAULT = 0x0099,
	/*Module Faults*/

	/*SPI Related Faults*/
	ERR_SPI_DRIVER_FAULT = 0x0011,


	/*FreeRTOS Related Faults*/
	ERR_FREERTOS_EVENT_FAULT = 0x0021,
	ERR_FREERTOS_QUEUE_FAULT = 0x0022,
   
} ERRFaultEnum;


//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

typedef struct __attribute__((packed))
{
	Time timeStamp;
	uint16_t OSVersion;
	uint16_t BootVersion;
 	uint16_t faultCode;
	uint16_t reserved1;
	uint16_t reserved2;
	uint16_t reserved3;	
}HARD_FAULT_STRUCT;


typedef struct __attribute__((packed))
{
	Time timeStamp;
	uint16_t OSVersion;
	uint16_t BootVersion;
 	uint16_t faultCode;
	uint16_t reserved;
    uint32_t faultAddress;
    uint32_t stackPtr[64];
}EXCEPTION_FAULT_STRUCT;


typedef struct __attribute__((packed))
{
	EXCEPTION_FAULT_STRUCT exceptionStructArr[6];//size 68 * 4 * 6 = 1632
	HARD_FAULT_STRUCT hardFaultArr[10];// size 4 * 4 * 10 = 160
	uint8_t exceptionCounter;
	uint8_t hardfaultCounter;
    uint8_t reserved[252];
	uint16_t crc;
}FAULT_STRUCT;//total size is 1792



typedef struct __attribute__((packed))
{
	BOOL FeedWatchdog;
	BOOL watchdogInit;
	BOOL tasksInit;
	uint32_t errTimerTick;
	uint16_t USBAppTaskCounter;
	uint16_t FTPTaskCounter;
	uint16_t SNMPTaskCounter;
	uint16_t WebserTaskCounter;
	uint16_t DriveSPITaskCounter;
	uint16_t ModbusTCPTaskCounter;
	uint16_t SysTaskCounter;
	uint16_t EthernetTaskCounter;
	uint16_t SPIFlashFSTaskCounter;
    uint16_t GSMSTaskCounter;
}ERROR_STRUCT;

//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

void Error_MajorFault(uint16_t faultCode );
void Error_GetRegistersFromStack(uint32_t *pulFaultStackAddress,uint16_t faultCode);
void Error_FaultISR(void);
void Error_MPUFaultISR(void);
void Error_BUSFaultISR(void);
void Error_USAGEFaultISR(void);
void Error_WatchdogIntISR(void);
void Error_TimerHandlerISR (void);
void Error_InitWatchdog(uint32_t l_clock);
void Error_ResetTaskWatchdog(uint8_t taskID);
void Error_DisableWatchdogReset(void);
uint32_t * HardFault_Handler( void );
void Error_startTaskMonitoring();
void Error_stopTaskMonitoring();

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------


extern ERROR_STRUCT ErrorStruct;


//----------------------------------------------------------------------------


#endif//_ERR_HANDLER_H_
//==============================================================================
// END OF PWM_HANDLER_H
//==============================================================================


