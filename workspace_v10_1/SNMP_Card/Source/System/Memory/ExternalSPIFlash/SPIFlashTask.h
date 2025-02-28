//****************************************************************************
//*  Created on: 19-Dec-2023
//*  Author: Rani Sonawane
//*  File: SPIFlashTask.h
//*
//* Synopsis: SPI Flash FatFs File System Handler
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#ifndef SOURCE_COMMUNICATION_SPI_SPIFLASHTASK_H_
#define SOURCE_COMMUNICATION_SPI_SPIFLASHTASK_H_

#include "ff.h"
#include "fatfs.h"
#include <Common.h>

#define WEBSERVER_DIRECTORY_AVAILABLE       TRUE
#define WEBSERVER_DIRECTORY_NOTAVAILABLE    FALSE

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
    FIL File;
    FATFS g_sFatFs;
    BOOL SPIFlashMount;
} SPIFlashFatFs_FILESYSTEM_STRUCT;
extern SPIFlashFatFs_FILESYSTEM_STRUCT spiflashFileSystemStruct;
//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------
void SPI_Flash_Init(void);
void SPIFalsh_Fatfs_Init();
void SPIFLASHDataLog_ProcessPendingLogs(void);
void SPIFlashFileSystem_Init(void);
void SPIFlashFileSystem_Task();
void SPI_Flash_Erase_Chip(void);

#endif /* SOURCE_COMMUNICATION_SPI_SPIFLASHTASK_H_ */
