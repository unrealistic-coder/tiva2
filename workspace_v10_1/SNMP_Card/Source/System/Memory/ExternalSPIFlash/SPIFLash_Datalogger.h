/*
 * SPIFLash_Datalogger.h
 *
 *  Created on: 27-Apr-2023
 *      Author: Rani Sonawane
 */

#ifndef SOURCE_SYSTEM_MEMORY_EXTERNALSPIFLASH_SPIFLASH_DATALOGGER_H_
#define SOURCE_SYSTEM_MEMORY_EXTERNALSPIFLASH_SPIFLASH_DATALOGGER_H_
#include <Common.h>
#include "SPIFlashTask.h"
//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------
#define YEAR_OFFSET          (0)//(2020)
#define NO_OF_ALARM_GROUPS   (16)
#define NO_OF_LATEST_LOGS		(30)
#define SYSTEM_ROOT             "/DL"
#define WEBPAGE_ROOT_DIR        "/WEB"
#define DEFAULT_INTERVAL        (5000)
//osMutexId Log_Mutex = NULL;
//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------


//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
typedef enum
{
    SPI_FLASH_EMPTY_NO_DIR=0,
    SPI_FLASH_YEAR_DIR_AVAILABLE,
    SPI_FLASH_YEAR_DIR_NOTAVAILABLE,
    SPI_FLASH_MONTH_DIR_AVAILABLE,
    SPI_FLASH_MONTH_DIR_NOTAVAILABLE,
    SPI_FLASH_DATE_DIR_AVAILABLE,
    SPI_FLASH_MEMORY_NOT_FULL,
    SPI_FLASH_MEMORY_FULL,
    SPI_FLASH_UNLINK_RECORD,
    SPI_FLASH_READY_TO_WRITE_NXT_RECORD,
    MAX_NUM
}SPI_FLASH_DATALOG_STATE;

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

typedef union
{
	uint8_t TravBuff[1];
	struct __attribute__ ((packed))
	{
        Time 	 		DateTime;
        uint16_t        SNMPParam1[8];     //66 28 are using 40 byte reserved
        uint16_t        SNMPAlarms[4];      //8
        uint16_t        SNMPStatus;
        uint16_t        RESERVED[3];      //8
	};
}LOG_HOLD_STRUCT;

typedef union
{
	uint8_t TravBuff[1];
	struct __attribute__((packed))
	{
		REG_uint16_t SNMPVersion;
		REG_uint16_t FormatVersion;
		REG_uint16_t ProductVersion;
		uint16_t logSize;
		uint16_t headerCRC;
	};
}DATALOG_FILE_HEADER;


typedef union
{
	uint8_t TravBuff[1];
	struct __attribute__((packed))
	{
		DATALOG_FILE_HEADER dataLogFileHeader;
		LOG_HOLD_STRUCT dataLogHoldStruct[NO_OF_LATEST_LOGS];
	};
}DATALOG_FILE_FORMAT;
typedef struct
{
  uint8_t  groupIndex:4;
  uint8_t  CurrentState:1;
}DATABASE_ALARM_PROPERTY;

typedef struct __attribute__((packed))
{
   uint64_t alarmslogbuf[4];
} DATABASE_ALARM_STRUCT;
typedef struct __attribute__((packed))
{
    DATALOG_FILE_FORMAT dataLogFileFormat;
	DATABASE_ALARM_STRUCT *pDBAlarmsStruct;
	uint32_t StartTime_LogInterval;
	uint32_t StartTime_AlarmLogInterval;
	uint32_t logInterval;
	uint16_t fillIndex;
	uint16_t LogCounter;
	uint8_t hasAlarm;	
} DATALOG_STRUCT;


extern uint8_t SPIFSTaskNo;
//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

void SPIFlashDataLog_SaveDataLogInTemporaryHold(uint16_t *pTimestamp);
void SPIFlashDataLog_ProcessPendingLogs(void);
void SPIFlashDataLog_UpdateDataLogInterval(void);
uint16_t HTTP_ReadDataLogParam(uint16_t Index);
void HTTP_WriteDataLogParam(uint16_t Index, uint16_t Value);
FRESULT SPIFLASHDataLog_CreateAndOpenFile(char*pFileName);
BOOL SPIFlash_ReadData(Time *pGivenDay, uint16_t Index, LOG_HOLD_STRUCT *pLatest);
FRESULT SPIFLASHWEB_CreateDirectory(void);
void DetectAlarmsToLog();
uint16_t SPIFlash_GetRecordSize(uint8_t RecType, Time *pFileDate);
void GetCurrentTimeStamp(uint16_t *pTimestamp);
uint8_t CalculatetotalDatalogFileSize();


//============================================================================
//                              > EXTERN VARIABLES <
//----------------------------------------------------------------------------

extern DATALOG_STRUCT dataLogStruct;



#endif /* SOURCE_SYSTEM_MEMORY_EXTERNALSPIFLASH_SPIFLASH_DATALOGGER_H_ */
