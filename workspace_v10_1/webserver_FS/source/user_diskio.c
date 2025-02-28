/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <source/user_diskio.h>		/* Declarations of disk functions */
#include "ff.h"			/* Obtains integer types */
#include "SST26VF032B.h"/* Driver for 4MB SPI FLASH - SST26VF032B/32BA*/

///* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_FLASH      0   /* SST26VF032B 4MB SPI FLASH */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/
#define disk_initialize user_disk_initialize
#define disk_status     user_disk_status
#define disk_read       user_disk_read
#define disk_write      user_disk_write
#define disk_ioctl      user_disk_ioctl

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	DSTATUS stat = 0;
//	int result;

	if (pdrv != 0) {
        stat |= STA_NOINIT; // Only one drive (pdrv 0) is supported
    }

    // Check the status of the SPI Flash or other storage
    if (!TEST_spi_init()) {
        stat |= STA_NOINIT; // Storage is not ready
    }


    BYTE config = ReadConfig();
    // check WP enabled, IOC = 0, WPEN = 1
    if (!(config & CONFIG_IOC)){
        if(config & CONFIG_WPEN)
            stat |= STA_PROTECT; // Drive is write-protected
    }
//    // check if any block is permanently WP
//    if(!(config & CONFIG_BPNV){
//        return STA_PROTECT;
//    }

    return 0; // Drive is initialized and ready
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv != 0) return STA_NOINIT; // Support only one drive
    // init
    SPI_Init();
    if (TEST_spi_init() != 1) return STA_NOINIT;
    return 0; // Successful initialization
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != 0) {
        return RES_PARERR; // Invalid drive number
    }

	if(sector > LAST_SECTOR){
	    return RES_PARERR;
	}

//    // Adjust `count` if it exceeds the valid range
//    if (sector + count - 1 > LAST_SECTOR) {
//        count = LAST_SECTOR - sector + 1; // Limit count to fit within the range
//    }
    if (count == 0 || (sector + count - 1 > LAST_SECTOR)) {
        return RES_PARERR; // Invalid count or range exceeds valid sectors
    }

    if (!TEST_spi_init()) {
        return RES_NOTRDY; // Storage is not initialized or ready
    }

    // Convert sector to byte address
    UINT i = 0;
    uint32_t address = sector * SECTOR_SIZE;
    for (i = 0; i < count; i++) {
        if (!ReadFlash(address + i * SECTOR_SIZE, SECTOR_SIZE, buff + i * SECTOR_SIZE, true)) {
            return RES_ERROR;
        }
    }

    return RES_OK; // Read completed successfully
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    if (pdrv != 0) {
        return RES_PARERR; // Invalid drive number
    }

    if(sector > LAST_SECTOR){
        return RES_PARERR;
    }

//    // Adjust `count` if it exceeds the valid range
//    if (sector + count - 1 > LAST_SECTOR) {
//        count = LAST_SECTOR - sector + 1; // Limit count to fit within the range
//    }
    if (count == 0 || (sector + count - 1 > LAST_SECTOR)) {
        return RES_PARERR; // Invalid count or range exceeds valid sectors
    }

    if (!TEST_spi_init()) {
        return RES_NOTRDY; // Storage is not initialized or ready
    }

    BYTE config = ReadConfig();
    // check WP enabled, IOC = 0, WPEN = 1
    if (!(config & CONFIG_IOC)){
        if(config & CONFIG_WPEN)
            return RES_WRPRT;
    }

    // Convert sector to byte address

    uint32_t sector_address = sector * SECTOR_SIZE;
    uint32_t i = 0, j = 0;
    uint32_t page_address = 0;

    // SECTOR WRITE
    for( i = 0; i < count; i++){

        sector_address = (sector_address + (i * SECTOR_SIZE));
        if(!SectorErase(sector_address)){
            return RES_ERROR;
        }
        //sector is divided into 16 pages
        for(j = 0; j < 16; j++){
            //add 256 byte to move to next page and do page program
            page_address = (sector_address + (j * 256));
            if(!PageProgram(page_address, 256, buff + (j*256))){
                return RES_ERROR;
            }
        }

    }

    return RES_OK; // Write completed successfully
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    if (pdrv != 0) {
        return RES_PARERR; // Invalid drive number
    }
    switch(cmd){
        case CTRL_SYNC:
            /* Complete pending write process (needed at FF_FS_READONLY == 0) */
            wait(); // loop until busy bit 0
            break;

        case GET_SECTOR_COUNT:
            /* Get media size (needed at FF_USE_MKFS == 1) */
            *(DWORD*)buff = SECTOR_COUNT;
            break;

        case GET_SECTOR_SIZE:
            /* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
            *(DWORD*)buff = SECTOR_SIZE;
            break;

        case GET_BLOCK_SIZE:
            /* Get erase block size (needed at FF_USE_MKFS == 1) */
            *(DWORD*)buff = 1;
            break;
        case CTRL_TRIM:
        { //TODO: error resolved here by adding braces to case
            /* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */
            LBA_t* range = (LBA_t*)buff;
            uint32_t start_address = range[0] * SECTOR_SIZE;
            uint32_t end_address = (range[1] * SECTOR_SIZE); // includes this
            DWORD i = 0;
            for(i = start_address; i <= end_address; i += SECTOR_SIZE){
                if(!SectorErase(i)){
                    return RES_ERROR;;
                }
            }
            break;
        }
    }

    return RES_OK;
}

