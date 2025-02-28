

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */
#include "SPI_Flash.h"

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "user_diskio.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
 void Init_Fs();
/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};


#include "SPIFLash_Datalogger.h"
//#include "Common.h"
#include "user_diskio.h" /* defines USER_Driver as external */
//uint8_t retUSER;    /* Return value for USER */
//char USERPath[4];   /* USER logical drive path */
//FATFS USERFatFS;    /* File system object for USER logical drive */
//FIL USERFile;       /* File object for USER */
FIL SFLASHPath[];
FATFS fatfs[_VOLUMES];    // file system object


uint8_t retATA;    /* Return value for USER */
char ATAPath[4];   /* USER logical drive path */

uint8_t retMMCDisk;    /* Return value for USER */
char MMCDiskPath[4];   /* USER logical drive path */

uint8_t retUSBDisk;    /* Return value for USER */
char USBDiskPath[4];   /* USER logical drive path */


/* USER CODE BEGIN Variables */

/* USER CODE END Variables */


/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "fatfs.h"
#include "user_diskio.h" /* defines USER_Driver as external */
uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */
FIL SFLASHPath[];
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);
  FATFS_LinkDriver(&USER_Driver, (char *)SFLASHPath);
  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
    return    ((2007UL-1980) << 25)   // Year = 2007
               | (6UL << 21)           // Month = June
               | (5UL << 16)           // Day = 5
               | (11U << 11)           // Hour = 11
               | (38U << 5)            // Min = 38
               | (0U >> 1)             // Sec = 0
               ;

  /* USER CODE BEGIN get_fattime */
 // return 0;
  /* USER CODE END get_fattime */
}

BOOL FatFS_Init(void)
{

    if (!(disk_status(SPI_FLASH) & STA_NODISK))
    {
        // disk inserted so initialise it
        if (disk_initialize(SPI_FLASH) == 0)
        {
         //   f_mount(&spiflashFileSystemStruct.g_sFatFs, "0", 1);
            if(f_mount(&fatfs[SPI_FLASH], "0", 1) == FR_OK)
           // if (f_mount(MMC_DRIVE, &fatfs[MMC_DRIVE],1) == FR_OK)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{

	DSTATUS status;
	/* USER CODE BEGIN INIT */
	status=SPIFLASH_disk_initialize();

	return status;

  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
	return 0;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
	int i;
	for(i=0;i<count;i++)
	{
		SPI_Flash_Read_Sector(sector,buff);
		sector ++;
		buff += FLASH_SECTOR_SIZE;
	}

	return RES_OK;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */
	int i;
	for(i=0;i<count;i++)
	{
	    SPI_Flash_Erase_Sector(sector);
	    SPI_Flash_Write_Sector(sector,(char*)buff);
		sector ++;
		buff += FLASH_SECTOR_SIZE;
	}

	return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
	DRESULT res = RES_OK;

	switch(cmd)
		{
			case CTRL_SYNC :
				break;

			//扇区擦除
	//		case CTRL_ERASE_SECTOR:
	//			nFrom = *((DWORD*)buff);
	//			nTo = *(((DWORD*)buff)+1);
	//			for(i=nFrom;i<=nTo;i++)
	//				W25X_Erase_Sector(i);
	//
	//			break;

			case GET_BLOCK_SIZE:
				*(DWORD*)buff = FLASH_BLOCK_SIZE;
			//	buf[1] = (u8)(FLASH_BLOCK_SIZE & 0xFF);
			//	buf[0] = (u8)(FLASH_BLOCK_SIZE >> 8);
				break;


			case GET_SECTOR_SIZE:
				*(DWORD*)buff = FLASH_SECTOR_SIZE;
			//	buf[0] = (u8)(FLASH_SECTOR_SIZE & 0xFF);
			//	buf[1] = (u8)(FLASH_SECTOR_SIZE >> 8);
				break;

			case GET_SECTOR_COUNT:
				*(DWORD*)buff = FLASH_SECTOR_COUNT;
			//	buf[0] = (u8)(FLASH_SECTOR_COUNT & 0xFF);
			//	buf[1] = (u8)(FLASH_SECTOR_COUNT >> 8);
				break;

			default:
				//res = RES_PARERR;
				break;
		}
		return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

