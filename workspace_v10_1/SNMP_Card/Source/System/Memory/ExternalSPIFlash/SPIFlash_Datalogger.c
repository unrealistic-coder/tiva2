/*
 * SPIFlash_Datalogger.c
 *
 *  Created on: 27-Apr-2023
 *      Author: Rani Sonawane
 */

/*-----------------------------------------------------------------------*/
#include "SPIFLash_Datalogger.h"
#include <SerialDriver.h>
#include <ErrorHandler.h>
#include <OnChipEEPROM.h>
#include <ModbusTCPServer.h>
#include <UPSInterface.h>
#include <SYSTask.h>
#include <SPI_Flash.h>
#include "ff.h"
#include "fatfs.h"
#include "websocket.h"
#include "SPIExtRTC.h"
//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------
#define MAX_SIZE_SPI_FLASH_TO_WRITE    900000
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

Time GlobalDateTime;
DATALOG_STRUCT dataLogStruct;
extern FIL SPIFilesysDatalog;

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------

//****************************************************************************
//*
//* Function name: USBDataLog_GetTodaysDate
//*
//* Return:   None
//*
//* Description:
//*   This function fetches todays Date from Database
//*
//****************************************************************************
static BOOL SPIFLASHDataLog_GetTodaysDate(void)
{

	uint16_t TemptDataTime[6];


    TemptDataTime[0]= GlobalDateTime.Year;
    TemptDataTime[1]= GlobalDateTime.Date;
    TemptDataTime[2]= GlobalDateTime.Month;
    TemptDataTime[3]= GlobalDateTime.Minutes;
    TemptDataTime[4]= GlobalDateTime.Hours;
    TemptDataTime[5]= GlobalDateTime.Seconds;



        return TRUE;

}

//****************************************************************************
//*
//* Function name: USBDataLog_SaveDataLogInTemporaryHold
//*
//* Return:   None
//*
//* Description:
//*   This function fetches data from database and Holds Datalog in Temporary 
//*  buffer.
//*
//****************************************************************************
void SPIFLASHDataLog_SaveDataLogInTemporaryHold(uint16_t *Timestamp)
{
	LOG_HOLD_STRUCT *pLogHoldPtr;
	uint8_t index=0;

	pLogHoldPtr=dataLogStruct.dataLogFileFormat.dataLogHoldStruct;

	index=dataLogStruct.fillIndex;

	pLogHoldPtr[index].DateTime.Year=Timestamp[0];
	pLogHoldPtr[index].DateTime.Date=Timestamp[1];
	pLogHoldPtr[index].DateTime.Month=Timestamp[2];
	pLogHoldPtr[index].DateTime.Minutes=Timestamp[3];
	pLogHoldPtr[index].DateTime.Hours=Timestamp[4];
	pLogHoldPtr[index].DateTime.Seconds=Timestamp[5];
	
	pLogHoldPtr[index].SNMPAlarms[0]=UPSDb.warningFlags.warning16_bit[0];
	pLogHoldPtr[index].SNMPAlarms[1]=UPSDb.warningFlags.warning16_bit[1];
    pLogHoldPtr[index].SNMPAlarms[2]=UPSDb.warningFlags.warning16_bit[2];
    pLogHoldPtr[index].SNMPAlarms[3]=UPSDb.warningFlags.warning16_bit[3];

    pLogHoldPtr[index].SNMPParam1[0]= UPSDb.InputVolt;
    pLogHoldPtr[index].SNMPParam1[1]= UPSDb.OutputVolt;
    pLogHoldPtr[index].SNMPParam1[2]= UPSDb.OutputFreq;
    pLogHoldPtr[index].SNMPParam1[3]= UPSDb.OutputLoadPercent;
    pLogHoldPtr[index].SNMPParam1[4]= UPSDb.BattVolt;
    pLogHoldPtr[index].SNMPParam1[5]= UPSDb.MaxTemp;
    pLogHoldPtr[index].SNMPParam1[6]=0;//42
    pLogHoldPtr[index].SNMPParam1[7]= 0;//44

	pLogHoldPtr[index].SNMPStatus=UPSDb.upsStatus.statusAll;
	pLogHoldPtr[index].RESERVED[0]=0;
	pLogHoldPtr[index].RESERVED[1]=0;
	pLogHoldPtr[index].RESERVED[2]=0;

	dataLogStruct.fillIndex++;
	if(dataLogStruct.fillIndex >= (NO_OF_LATEST_LOGS))
	{
		dataLogStruct.fillIndex=0;
	}

	dataLogStruct.LogCounter++;
	
	if(dataLogStruct.LogCounter >= (NO_OF_LATEST_LOGS))
	{
		dataLogStruct.LogCounter=NO_OF_LATEST_LOGS;
	}
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_GetSNMPCardLogFileInfo
//*
//* Return:   None
//*
//* Description:
//*   This function initialises RCD datalog file version and formate version
//*
//****************************************************************************
//==============================================================================
static void SPIFLASHDataLog_GetSNMPCardLogFileInfo(void)
{
    dataLogStruct.dataLogFileFormat.dataLogFileHeader.FormatVersion.all   = 1;
	dataLogStruct.dataLogFileFormat.dataLogFileHeader.SNMPVersion.byte[0] = SNMP_CARD_MAJOR_VERSION;
	dataLogStruct.dataLogFileFormat.dataLogFileHeader.SNMPVersion.byte[1] = SNMP_CARD_MINOR_VERSION;
	dataLogStruct.dataLogFileFormat.dataLogFileHeader.ProductVersion.all  = 100;
	dataLogStruct.dataLogFileFormat.dataLogFileHeader.logSize             = sizeof(LOG_HOLD_STRUCT);
	dataLogStruct.dataLogFileFormat.dataLogFileHeader.headerCRC           = COMCrcGenerator((uint8_t*)&dataLogStruct.dataLogFileFormat.dataLogFileHeader,
	                                                                                        (sizeof(DATALOG_FILE_HEADER)-2));
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_CreateAndOpenFile
//*
//* Return:   None
//*
//* Description:
//*   This function Creates and opens Datalog File.
//*
//****************************************************************************
//==============================================================================
FRESULT SPIFLASHDataLog_CreateAndOpenFile(char*FileName)
{
    uint32_t temp=0;
	FILINFO  FileInfo;
	FRESULT FileStatus = FR_NO_FILE;
	DATALOG_FILE_HEADER *pLogFileHeader;
	int bw;
	pLogFileHeader=&dataLogStruct.dataLogFileFormat.dataLogFileHeader;

	FileStatus = f_stat((const TCHAR *) FileName, (FILINFO *) &FileInfo);

	if(FileStatus == FR_NO_FILE)
	{
		 FileStatus = (FRESULT)(FatFS_fopen( &SPIFilesysDatalog, (const char*)FileName, FA_OPEN_ALWAYS | FA_WRITE));
		if(FileStatus == FR_OK)
		{
		   temp= f_size(&SPIFilesysDatalog);
		   FileStatus = (FRESULT)(FatFS_fseek( &SPIFilesysDatalog, temp));
			if(FileStatus == FR_OK)
			{

                SPIFLASHDataLog_GetSNMPCardLogFileInfo();
                FileStatus =  (FRESULT)(f_write(&SPIFilesysDatalog, (char*)&(pLogFileHeader->TravBuff[0]),
                                          sizeof(DATALOG_FILE_HEADER),(UINT *)&bw));
               f_close(&SPIFilesysDatalog);

			}
			else 
			{
				FileStatus = FR_INT_ERR;

			}
		}
		else
		{
			FileStatus = FR_NO_FILE;


		}
	}
	else
	{

		FileStatus = FR_EXIST;
	}
	
	return FileStatus;
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_CreateTodaysDirectory
//*
//* Return:   None
//*
//* Description:
//*   This function creates Todays Directory
//*
//****************************************************************************
//==============================================================================
static FRESULT SPIFLASHDataLog_CreateTodaysDirectory(void)
{
	FRESULT DirStatus  = FR_NO_PATH;
	FRESULT FileStatus 	= FR_OK;
	char CurrentDir[30]  = {0};
	char CurrentTemp[30]  = {0};
	uint16_t StationNumber = 0;
	char Filename[30];


	if(StationNumber != 0xFFFF)
	{
		sprintf(CurrentDir, "%s-%d/%02d/%02d", SYSTEM_ROOT,StationNumber,
				(GlobalDateTime.Year), GlobalDateTime.Month);
       if(GlobalDateTime.Year>=24)
       {
            DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));

            if((DirStatus != FR_OK))
            {
                memset(&CurrentDir,0x00,30); //clearning dir name

                //---------------------Creating SystemRoot------------------------------//
                //strcat( CurrentDir, SYSTEM_ROOT);
                sprintf(CurrentTemp,"%s-%d",SYSTEM_ROOT,StationNumber);
                strcat( CurrentDir, CurrentTemp);
                FileStatus = (FRESULT)(f_mkdir((const char*)CurrentDir));
                if((FileStatus == FR_OK) || (FileStatus == FR_EXIST))
                {
                //---------------------Creating Current Year Directory------------------//
                    sprintf(CurrentTemp, "/%02d", (GlobalDateTime.Year));
                    strcat( CurrentDir, CurrentTemp);
                    FileStatus = (FRESULT)f_mkdir((const char*)CurrentDir);
                    if((FileStatus == FR_OK) || (FileStatus == FR_EXIST))
                    {
                //---------------------Creating Current Month Directory------------------//
                        sprintf(CurrentTemp, "/%02d",GlobalDateTime.Month);
                        strcat( CurrentDir, CurrentTemp);
                        FileStatus = (FRESULT)f_mkdir((const char*)CurrentDir);
                        if((FileStatus == FR_OK) || (FileStatus == FR_EXIST))
                        {
                //---------------------Creating Today's Date Directory------------------//
                            DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
                            if(DirStatus == FR_OK)
                            {
                                sprintf( Filename, "/%02d.bin", GlobalDateTime.Date);
                                strcat(CurrentDir, Filename);
                                FileStatus = SPIFLASHDataLog_CreateAndOpenFile((char*)CurrentDir);
                            }
                        }
                    }
                }
            }
            else // Directory Already Created
            {
                sprintf( Filename, "/%02d.bin", GlobalDateTime.Date);
                strcat(CurrentDir, Filename);
                FileStatus = SPIFLASHDataLog_CreateAndOpenFile((char*)CurrentDir);
            }
        }

       return FileStatus;
	}
	else
	{
	    FileStatus = FR_NO_FILE;//RTC Not having correct Time
	    return FileStatus;
	}
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_OpenTodaysFile
//*
//* Return:   None
//*
//* Description:
//*   This function opens Todays date file
//*
//****************************************************************************
//==============================================================================
static FRESULT SPIFLASHDataLog_OpenTodaysFile()
{
	FRESULT FileStatus = FR_NO_FILE;
	char Filename[30]={0};
	uint16_t StationNumber = 0;
	

	if(StationNumber!= 0xFFFF)
	{

		sprintf( Filename, "%s-%d/%02d/%02d/%02d.bin", SYSTEM_ROOT,StationNumber,
        	                                          (GlobalDateTime.Year),GlobalDateTime.Month,
        	                                          GlobalDateTime.Date );

        FileStatus = (FRESULT)(f_open((FIL*)&SPIFilesysDatalog, Filename,
                          (FA_OPEN_ALWAYS | FA_WRITE)));

	}
	return FileStatus;
}

//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_WriteDataLogFile
//*
//* Return:   None
//*
//* Description:
//*   This function processes pending data logs
//*
//****************************************************************************
//==============================================================================
static void SPIFLASHDataLog_WriteDataLogFile(void)
{

    uint16_t LogSize=0;
    FRESULT DirCheck = FR_NO_FILESYSTEM;
    FRESULT FileCheck = FR_NO_FILE;
    uint16_t Cnt = 0,temp=0;
    uint16_t index = 0,bw;
    BOOL ValidDate = FALSE;
  //  if((WEB_SOCKET_TASK_NOT_PROCESSING==TRUE) && (HTTP_TASK_NOT_PROCESSING==TRUE)&& (FTP_TASK_NOT_PROCESSING==TRUE))
   {
//        if(SPI_FLASH_READY_TO_WRITE_NXT_RECORD == CalculatetotalDatalogFileSize())
        {
            LOG_HOLD_STRUCT *pLogHoldPtr;
            pLogHoldPtr=dataLogStruct.dataLogFileFormat.dataLogHoldStruct;
            ValidDate = SPIFLASHDataLog_GetTodaysDate();
            if(ValidDate == FALSE)
            {
                //Return from here because of unvalid data
                //USBHOST_printf("Date is invalid\r\n");
                return;
            }
            DirCheck = SPIFLASHDataLog_CreateTodaysDirectory();

            if((DirCheck == FR_OK) || (DirCheck == FR_EXIST))
            {
                LogSize = sizeof(LOG_HOLD_STRUCT);
                FileCheck = SPIFLASHDataLog_OpenTodaysFile();

                if(FileCheck == FR_OK)
                {
                   // temp= f_size(&spiflashFileSystemStruct.File);
                    //FileCheck = (FRESULT)(FatFS_fseek((FIL*)&spiflashFileSystemStruct.File,temp));//FatFS_fseek

                    temp= f_size(&SPIFilesysDatalog);
                    FileCheck = (FRESULT)(FatFS_fseek(&SPIFilesysDatalog,temp));//FatFS_fseek
                    if(FileCheck == FR_OK)
                    {
                        if(dataLogStruct.LogCounter > 0)
                        {
                          if(dataLogStruct.LogCounter >= (NO_OF_LATEST_LOGS))
                          {
                                index=dataLogStruct.fillIndex;
                          }
                          else
                          {
                                index=0;
                          }

                           for(Cnt=0; Cnt<dataLogStruct.LogCounter; Cnt++)
                            {

                                   FileCheck = (FRESULT)(FatFS_fwrite(&SPIFilesysDatalog,(uint8_t *)&pLogHoldPtr[index].TravBuff[0],(LogSize), (UINT *)&bw));

                                    memset(&pLogHoldPtr[index].TravBuff[0], 0x00,(sizeof(LOG_HOLD_STRUCT)));

                                    index++;
                                    if(index >= (NO_OF_LATEST_LOGS))
                                    {
                                        index=0;
                                    }
                                    if(FileCheck != FR_OK)
                                        break;
                                }
                                dataLogStruct.LogCounter=0;
                                dataLogStruct.fillIndex=0;
                            }
                        }
                    FatFS_fclose( &SPIFilesysDatalog);
                }
            }
      }
   }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_ProcessPendingLogs
//*
//* Return:   None
//*
//* Description:
//*   This function processes pending data logs
//* SPIFLASHDataLog_SaveDataLogInTemporaryHold() This function also called from
//* DetectAlarmsToLog() function. Minim 5 Sec Interval Logs written in the SPI flash
//* To avoid missing of any data, logs are stored in Hold and write after every 5 sec
//* Below Hold function called is after interval not after Alarm changed.
//****************************************************************************
//==============================================================================
void SPIFLASHDataLog_ProcessPendingLogs(void)
{
	BOOL CounterEndTimer = FALSE;
	BOOL CounterEndTimer_Emg = FALSE;	
	uint16_t Timestamp[6];

	CounterEndTimer = COMTimer_CheckTimerExpiry(dataLogStruct.StartTime_LogInterval,dataLogStruct.logInterval);
	CounterEndTimer_Emg = COMTimer_CheckTimerExpiry(dataLogStruct.StartTime_AlarmLogInterval,DEFAULT_INTERVAL);

	    DetectAlarmsToLog();
        if((CounterEndTimer) || ((dataLogStruct.hasAlarm == TRUE) && CounterEndTimer_Emg))
        {

            if((CounterEndTimer == TRUE)&&(dataLogStruct.hasAlarm==FALSE))
            {

                GetCurrentTimeStamp(Timestamp);
                if((Timestamp[0] <= (YEAR_OFFSET+0x3F)) && ((Timestamp[2] >= 0x01) && (Timestamp[2] <= 0x0C)) &&
                  ((Timestamp[1] >= 0x01) && (Timestamp[1] <= 0x1F)))
                {
                    SPIFLASHDataLog_SaveDataLogInTemporaryHold(&Timestamp[0]);
                }
            }

            SPIFLASHDataLog_WriteDataLogFile();
            dataLogStruct.hasAlarm=FALSE;
            dataLogStruct.StartTime_LogInterval = COMTimer_Get10MsecTick();//COMTimer_GetTickMsec();
            dataLogStruct.StartTime_AlarmLogInterval = COMTimer_StartTimerMsec();
        }

}
//==============================================================================
//****************************************************************************
//*
//* Function name: SPIFLASHDataLog_UpdateDataLogInterval
//*
//* Return:   None
//*
//* Description:
//*   This function updates data log interval
//*
//****************************************************************************
//==============================================================================
//void SPIFLASHDataLog_UpdateDataLogInterval(void)
//{
//	// Converting to msec
//	dataLogStruct.logInterval=10000;
//}
//==============================================================================
//****************************************************************************
/*
* Function name: DetectAlarmsToLog
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
REG_uint64_t LatestAlarms[4] = {0};
void DetectAlarmsToLog()
{
    uint16_t Timestamp[6];
    BOOL logAlarm=FALSE;
    if((LatestAlarms[0].all != UPSDb.warningFlags.warningAll)||
            ((LatestAlarms[1].all != UPSDb.upsStatus.statusAll)))
    {
        GetCurrentTimeStamp(Timestamp);
        LatestAlarms[0].all = UPSDb.warningFlags.warningAll;
        LatestAlarms[1].all = UPSDb.upsStatus.statusAll;
        logAlarm=TRUE;
    }

    if(logAlarm==TRUE)
    {
        SPIFLASHDataLog_SaveDataLogInTemporaryHold(&Timestamp[0]);
        dataLogStruct.hasAlarm=TRUE;
    }
}
//==============================================================================
//****************************************************************************
/*
* Function name: GetCurrentTimeStamp
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
void GetCurrentTimeStamp(uint16_t *Timestamp)
{
    Timestamp[0]= GlobalDateTime.Year;
    Timestamp[1]= GlobalDateTime.Date;
    Timestamp[2]= GlobalDateTime.Month;
    Timestamp[3]= GlobalDateTime.Minutes;
    Timestamp[4]= GlobalDateTime.Hours;
    Timestamp[5]= GlobalDateTime.Seconds;

}
//==============================================================================
//****************************************************************************
/*
* Function name: ChangeToDir
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static BOOL ChangeToDir(uint16_t RecType, char *DirPath, Time *ReqDate)
{
    BOOL Status = FALSE;
    DIR TempDirObj;

    //--------------------------------------------------------------------------

    sprintf(DirPath, "%s-%d/%02d/%02d", SYSTEM_ROOT,0,
                    (ReqDate->Year), ReqDate->Month);

    Status = !FatFS_opendir(&TempDirObj,DirPath);

    if(Status)
    {
        FatFS_closedir(&TempDirObj);
    }
    // Give some Time for other threads to execute
     return Status;
}
//==============================================================================
//****************************************************************************
/*
* Function name: fsizebypath
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
uint32_t fsizebypath(char *fpath)
{
    uint32_t size = 0;
    uint8_t fres=0;
    fres=FatFS_fopen( &spiflashFileSystemStruct.File, fpath, FA_READ);

    if(!fres)
    {
        size = f_size(&spiflashFileSystemStruct.File);
        FatFS_fclose( &spiflashFileSystemStruct.File);
    }

    return (size-10);
}
//==============================================================================
//****************************************************************************
/*
* Function name: SPIFlash_ReadData
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
BOOL SPIFlash_ReadData(Time *GivenDay, uint16_t Index, LOG_HOLD_STRUCT *Latest)
{
    BOOL Status = FALSE;
    BOOL fileOpened=FALSE;
    uint32_t Offset=0;

    if(Latest != NULL && (spiflashFileSystemStruct.SPIFlashMount==1))
    {
        char CurrentDir[20] = "";
        Status = ChangeToDir(0, CurrentDir, GivenDay);

        if(Status)
        {
            char Filename[20];
            sprintf( Filename, "%s/%02d.bin", CurrentDir, GivenDay->Date);
            Status = !FatFS_fopen( &spiflashFileSystemStruct.File, Filename, FA_READ);
        }

        if(Status)
        {

            Offset = (sizeof(LOG_HOLD_STRUCT)*Index)+10;//
            Status = !FatFS_fseek( &spiflashFileSystemStruct.File, Offset);
            fileOpened=TRUE;
        }

        if(Status)
        {
            Status = !FatFS_fread( &spiflashFileSystemStruct.File, Latest, sizeof(LOG_HOLD_STRUCT));
            //if(Status == 0)
            //{
            //SPIFlashStatus =0;
            //}
        }

        if(fileOpened==TRUE)
        {
            FatFS_fclose( &spiflashFileSystemStruct.File );
        }

        osDelay(1);
    }

    return Status;
}

//==============================================================================
//****************************************************************************
/*
* Function name: SPIFlash_GetRecordSize
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
uint16_t SPIFlash_GetRecordSize(uint8_t RecType, Time *FileDate)
{
    uint16_t RecordSize = 0;
    uint32_t size = 0;
    uint8_t fres=0;

    if(FileDate != NULL && (spiflashFileSystemStruct.SPIFlashMount==1))
    {
        char Filename[20];
        uint16_t RecSizeInBytes = 0;

         RecSizeInBytes = sizeof(LOG_HOLD_STRUCT);//LOG_HOLD_STRUCT

        char CurrentDir[20] = "";
        BOOL Status = ChangeToDir( RecType, CurrentDir, FileDate);

        if(Status)
        {
            sprintf( Filename, "%s/%02d.bin", CurrentDir, FileDate->Date);
           //uint32_t SizeInBytes = fsizebypath(Filename);

        fres=FatFS_fopen( &spiflashFileSystemStruct.File, Filename, FA_READ);

        if(!fres)
        {
           size = f_size(&spiflashFileSystemStruct.File);
           FatFS_fclose( &spiflashFileSystemStruct.File);
        }
          RecordSize = (long)(size)/RecSizeInBytes;
        }
    }
     return RecordSize;
}
///////////Find Files total size and Delete the oldest log and write new log
uint8_t CalculatetotalDatalogFileSize()
{
    uint16_t itr=0,itr1=0;
    FRESULT FileStatus  = FR_OK;
    uint32_t Flashfilessize=0;
    FRESULT DirStatus  = FR_NO_PATH;

    char CurrentDir[30]  = {0};
    char CurrentTemp[30]  = {0};
    uint16_t StationNumber = 0;
    char Filename[30];
    FILINFO  FileInfo;

    memset(&CurrentDir,0x00,30); //clearning dir name

    /////////////////////Check Current Year Files size

    sprintf(CurrentTemp, "%s-%d", SYSTEM_ROOT,StationNumber);
    strcat( CurrentDir, CurrentTemp);
    DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
    if(DirStatus == FR_OK)
    {
       sprintf(CurrentTemp, "/%02d", GlobalDateTime.Year);
       strcat( CurrentDir, CurrentTemp);
       DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
       if((DirStatus == FR_OK))
        {
           for(itr1=12;itr1>0;itr1--)
           {
             sprintf(CurrentTemp, "/%02d",itr1);
             CurrentDir[8]=CurrentTemp[0];
             CurrentDir[9]=CurrentTemp[1];
             CurrentDir[10]=CurrentTemp[2];
             memset(&CurrentDir[11],0x00,18); //clearning dir name
             DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
             if((DirStatus == FR_OK))
             {
                 for(itr=31;itr>=1;itr--)
                 {
                        sprintf(Filename, "/%02d.bin",itr);
                        CurrentDir[11]=Filename[0];
                        CurrentDir[12]=Filename[1];
                        CurrentDir[13]=Filename[2];
                        CurrentDir[14]=Filename[3];
                        CurrentDir[15]=Filename[4];
                        CurrentDir[16]=Filename[5];
                        CurrentDir[17]=Filename[6];
                        memset(&CurrentDir[18],0x00,12); //clearning dir name
                        FileStatus = f_stat((const TCHAR *) CurrentDir, (FILINFO *) &FileInfo);

                         if((FileStatus == FR_OK))
                         {
                            MEMCPY(Filename,CurrentDir,18);
                            memset(&Filename[18],0x00,12); //clearning dir name
                            FileStatus = f_open((FIL*)&SPIFilesysDatalog, (TCHAR*)Filename, FA_READ);
                             if(FileStatus==FR_OK)
                             {
                                Flashfilessize += f_size(&SPIFilesysDatalog);

                                if(Flashfilessize >= MAX_SIZE_SPI_FLASH_TO_WRITE)
                                 {
                                     FileStatus = f_unlink((TCHAR*)Filename);
                                     f_close(&SPIFilesysDatalog);
                                     return SPI_FLASH_READY_TO_WRITE_NXT_RECORD;
                                 }
                                else
                                {
                                    f_close(&SPIFilesysDatalog);
                                }
                             }
                     }
                }

             }
          }
        }
    }
#if 0
       else
       {
           if(GlobalDateTime.Year==00)
           {
               PreviuosYear=99;
           }
           else
           {
               PreviuosYear = GlobalDateTime.Year-1;
           }

         sprintf(CurrentTemp, "/%02d", PreviuosYear);
         strcat( CurrentDir, CurrentTemp);
         DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
         if((DirStatus == FR_OK))
          {
             for(itr1=12;itr1>0;itr1--)
             {
               sprintf(CurrentTemp, "/%02d",itr1);
               CurrentDir[8]=CurrentTemp[0];
               CurrentDir[9]=CurrentTemp[1];
               CurrentDir[10]=CurrentTemp[2];
               memset(&CurrentDir[11],0x00,18); //clearning dir name
               DirStatus = (FRESULT)(f_chdir((const char *) CurrentDir));
               if((DirStatus == FR_OK))
               {
                   for(itr=31;itr>=1;itr--)
                   {
                        sprintf(Filename, "/%02d.bin",itr);
                        CurrentDir[11]=Filename[0];
                        CurrentDir[12]=Filename[1];
                        CurrentDir[13]=Filename[2];
                        CurrentDir[14]=Filename[3];
                        CurrentDir[15]=Filename[4];
                        CurrentDir[16]=Filename[5];
                        CurrentDir[17]=Filename[6];
                        memset(&CurrentDir[18],0x00,12); //clearning dir name
                        FileStatus = f_stat((const TCHAR *) CurrentDir, (FILINFO *) &FileInfo);

                       if((FileStatus == FR_OK))
                       {
                          MEMCPY(Filename,CurrentDir,18);
                          memset(&Filename[18],0x00,12); //clearning dir name
                          FileStatus = f_open((FIL*)&SPIFilesysDatalog, (TCHAR*)Filename, FA_READ);
                           if(FileStatus==FR_OK)
                           {
                              Flashfilessize += f_size(&SPIFilesysDatalog);
                              if(Flashfilessize >= MAX_SIZE_SPI_FLASH_TO_WRITE)
                               {
                                   FileStatus = f_unlink((TCHAR*)Filename);
                                   f_close(&SPIFilesysDatalog);
                                   return SPI_FLASH_READY_TO_WRITE_NXT_RECORD;
                               }
                              else
                                {
                                    f_close(&SPIFilesysDatalog);
                                }
                           }
                     }
                  }
               }
            }
          }
   #endif
     return SPI_FLASH_READY_TO_WRITE_NXT_RECORD;
}
//==============================================================================
// END OF SPIFlash_Datalogger.c FIlE
//==============================================================================

