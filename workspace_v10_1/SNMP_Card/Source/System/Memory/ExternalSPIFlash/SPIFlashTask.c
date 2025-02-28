/*
 * SPIFlashTask.c
 *
 *  Created on: 19-Dec-2023
 *      Author: Rani Sonawane
 */

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include "SPIFlashTask.h"
#include "SPIFLash_Datalogger.h"
#include "FTPServer.h"
#include "SYSTask.h"
#include "ErrorHandler.h"
#include <UPSInterface.h>
#include "websocket.h"
#include "SPI_Flash.h"
#include <EthernetTask.h>
#include <OnChipEEPROM.h>
SPIFlashFatFs_FILESYSTEM_STRUCT spiflashFileSystemStruct;


//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL ENUMERATED TYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL STRUCTURES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------
FIL SPIFilesysDatalog;

void SPIFlashFileSystem_Init(void)
{
    SPI_Flash_Init();
    MX_FATFS_Init();
    if(Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.all==0)
        Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.all=6000;

    dataLogStruct.logInterval=Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.all;
}
//=============================================================================
//****************************************************************************
//*
//* Function name: USBFileSystem_StateHandler
//*
//* Return:   None
//*
//* Description:
//*   This function Handles USB file system states
//*
//****************************************************************************
//=============================================================================
static void SPIFlashFileSystem_Mount(void)
{

    FRESULT fres;
    fres = f_mount(&spiflashFileSystemStruct.g_sFatFs, "0", 1);
    if (fres == FR_OK)
    {
       spiflashFileSystemStruct.SPIFlashMount=1;

    }
    else
    {
        SPIFalsh_Fatfs_Init();
    }
}
//=============================================================================
//****************************************************************************
//*
//* Function name: USBFileSystem_StateHandler
//*
//* Return:   None
//*
//* Description:
//*   This function Handles USB file system states
//*
//****************************************************************************
//=============================================================================
void SPIFalsh_Fatfs_Init()
{
    FRESULT fres;
    SPI_Flash_Erase_Chip();
    fres = f_mkfs("", FM_FAT, 0, spiflashFileSystemStruct.File.buf, sizeof(spiflashFileSystemStruct.File.buf));
    fres = f_mount(&spiflashFileSystemStruct.g_sFatFs, "0", 1);
    if (fres == FR_OK)
    {
        spiflashFileSystemStruct.SPIFlashMount=1;

    }
    else
    {
        while(SPI_Flash_WriteBusy());
    }
}
//=============================================================================
//****************************************************************************
//*
//* Function name: CheckAvailabilityOF_SPIFlashDirectory
//*
//* Return:   None
//*
//* Description:
//*
//*
//****************************************************************************
//==============================================================================
static FRESULT CheckAvailabilityOF_SPIFlashDirectory()
{
    FRESULT DirStatus  = FR_NO_PATH;
    char CurrentDir[30]  = {0};
    char FileName[30]  = {0};
    sprintf(CurrentDir, "%s","/WEB/CSS");
    DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));

    uint32_t size = 0,itr=0;
    FRESULT fres=FR_NO_FILE;

    if((DirStatus == FR_OK))
    {
           memset(&CurrentDir,0x00,30); //clearning dir name
           sprintf(CurrentDir, "%s","/WEB/images");
           DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
           if((DirStatus == FR_OK))
           {
                memset(&CurrentDir,0x00,30);
                sprintf(CurrentDir, "%s","/WEB/Script");
                DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
                if((DirStatus == FR_OK))
                {
                    //do
                  //  {
                    for(itr=0;itr<19;itr++)
                    {
                        Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
                        memset(&FileName,0x00,30);
                        memcpy(FileName, (const char *)httpconnGETcommndList[itr].pages+4, (const char *)httpconnGETcommndList[itr].pagecmdsize);
                        fres=FatFS_fopen( &spiflashFileSystemStruct.File, FileName, FA_READ);
                        if(fres== FR_OK)
                        {
                            size = f_size(&spiflashFileSystemStruct.File);
                            FatFS_fclose( &spiflashFileSystemStruct.File);
                            DirStatus = FR_OK;
                        }
                        else
                        {
                         //   FatFS_fclose( &spiflashFileSystemStruct.File);
                            DirStatus  = FR_NO_PATH;
                            itr= 20;
                        }
                    }

                }
           }
    }

   return DirStatus;
}
//=============================================================================
//****************************************************************************
//*
//* Function name: SPIFlashFileSystem_Task
//*
//* Return:   None
//*
//* Description:
//*
//*
//****************************************************************************
//==============================================================================
void SPIFlashFileSystem_Task()
{
    SPIFlashFileSystem_Mount();
    while(1)
     {
        Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
        if((networkConfig.CurrentState == STATE_LWIP_WORKING))
         {
             WS_SocketOperation((WEBSOCKET_TCP_RAM_STRUCT *) &WebSocketStruct);

             if( (Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.FTPServerEnable ==1))
             {
                FTPServer_SocketOperation(&FTPConnHandle);
             }
         }

        if((spiflashFileSystemStruct.SPIFlashMount == TRUE))
        {
            SPIFLASHDataLog_ProcessPendingLogs();
          #if defined WEBSERVER_USING_SPI_FLASH
            HtppServer_netconn_thread_SocketOperation(&HttpServerStruct);//WEB SERVER FROM SPI FLASH
          #endif

        }
        vTaskDelay(10);
      }
}
//==============================================================================
// END OF FILE
//==============================================================================

