/*
 * SPI_Flash.h
 *
 *  Created on: 26-Apr-2023
 *      Author: Rani Sonawane
 */

#ifndef SOURCE_SYSTEM_MEMORY_EXTERNALSPIFLASH_SPI_FLASH_H_
#define SOURCE_SYSTEM_MEMORY_EXTERNALSPIFLASH_SPI_FLASH_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
//#include "usbhmsc.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usblibpriv.h"
#include "usblib/usbmsc.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdmsc.h"
#include "FatFS.h"
#include "fs.h"
#include "ff.h"         /* Declarations of FatFs API */
#include "diskio.h"     /* Declarations of disk I/O functions */
//


#define ON  1
#define OFF 0
#define USE_GETCHIPID           ON
#define USE_READ_STATUSREG      ON
#define USE_WRITE_STATUSREG     OFF
#define USE_WRITE_ENABLE        ON
#define USE_WRITE_DISABLE       OFF
#define USE_ERASE_CHIP          ON
#define USE_ERASE_SECTOR        ON
#define USE_ERASE_BLOCK         ON
#define USE_WAIT_BUSY           ON
#define USE_POWERDOWN           OFF
#define USE_RELEASEPOWERDOWN    OFF
#define USE_READ_BYTES          ON
#define USE_WRITE_BYTES         OFF
#define USE_READ_SECTOR         ON
#define USE_WRITE_SECTOR        ON

#define FLASH_PAGE_SIZE         256
#define FLASH_SECTOR_SIZE       4096//512
#define FLASH_SECTOR_COUNT      512
#define FLASH_BLOCK_SIZE        65536
#define FLASH_PAGES_PER_SECTOR  FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE

//****************************************************************************
//
// The variable g_ui32SysClock contains the system clock frequency in Hz.
//
//****************************************************************************

#define SCK1                GPIO_PIN_0
#define CS                  GPIO_PIN_1
#define SDO0                GPIO_PIN_2
#define SDI0                GPIO_PIN_3
#define SYSTEM_CLOCK       (120000000U)


#define       READ_COMMAND                                         0
#define       READ_DATA                                            1
#define       LENGTH_OF_DEVICE_ID                                  4
#define       COMMAND_TOREAD_DEVICEID                             0x90
#define       COMMAND_TOREAD_ERASE4K                              0x20
#define       REMOVE_PROTECTION_BY_CLEARING_ALL_STATUS_BITS       0x00
//#define       WRITE_AT_ZEROTHLOCATIONOK
//==============================================================================
// Pin Configurations
//==============================================================================
// Initialize chip select as Output
//#define INIT_CS()       ((MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ))\
//                          (MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3))\
//                          (MAP_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1)))


#define SDSPICSAssert()      (MAP_GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1,0))  //(LATBCLR = (unsigned int)(BIT_15))
#define SDSPICSDeAssert()    (MAP_GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1,GPIO_PIN_1))  //(LATBSET = (unsigned int)(BIT_15))

#define SST25_WRITE_BYTE(X)  (SPI_Write_Byte((X)))
#define SPI1_WriteByte(X)     (SPI_Write_Byte((X)))
#define SST25_READ_BYTE()    (SPI1_ReadByte())
#define SPI_Read_Byte()       (SPI1_ReadByte())


#define W25X32_CHIPID               0xBF2541//0xEF4016

#define W25X_WriteEnable            0x06
#define W25X_WriteDisable           0x04
#define W25X_ReadStatusReg          0x05
#define W25X_WriteStatusReg         0x01
#define W25X_ReadData               0x03
#define W25X_FastReadData           0x0B
#define W25X_FastReadDual           0x3B
#define W25X_PageProgram            0x02
#define W25X_BlockErase             0xD8
#define W25X_SectorErase            0x20
#define W25X_ChipErase              0xC7
#define W25X_SetPowerDown           0xB9
#define W25X_SetReleasePowerDown    0xAB
#define W25X_DeviceID               0xAB
#define W25X_ManufactDeviceID       0x90
#define W25X_JedecDeviceID          0x9F
#define W25X_Check()            ((SPI_Flash_GetChipID() == W25X32_CHIPID) ? 1 : 0)



uint32_t SPI1_ReadByte(void);
void SPI1_Init_HighSpeed(void);
void SPI_Flash_ResetWriteProtection(void);
uint8_t SPI_Flash_WriteBusy(void);
void Datalog_PerInterval();
void Datalog_Read();
void SPIFlash_Task();
void SPI1_DeInit(void);
uint32_t getFlashChipId(void);
void SPI_Flash_RawWrite(uint32_t address, uint8_t *pData, uint32_t nCount);
void SPI_Flash_RawRead(uint32_t address, uint8_t cmddata, uint8_t *pData, uint32_t nCount,uint8_t Cmdordata);
/************************************************************************
* SST25 Commands
************************************************************************/
#define SST25_CMD_WRSR  (unsigned)0x01  // Write Status Register
#define SST25_CMD_WRITE (unsigned)0x02
#define SST25_CMD_READ  (unsigned)0x03
#define SST25_CMD_RDSR  (unsigned)0x05  // Read Status Register
#define SST25_CMD_WREN  (unsigned)0x06
#define SST25_CMD_EWSR  (unsigned)0x50
#define SST25_CMD_ERASE (unsigned)0x60

//*****************************************************************************
//
// Number of bytes to send and receive.
//
//*****************************************************************************

extern uint32_t ui32_TReset;
void SPI_Flash_Init(void);

typedef enum
{
    COMMAND_REC_IDLE1 = 0,
    COMMAND_REC_RECEIVING2 = 1,
    COMMAND_REC_RECIVED3 = 2,
    COMMAND_REC_PROCESSING4= 3,
    COMMAND_REC_TX_COMPLETED5 = 4

}COMMOND_REC_STATE1;

extern uint8_t SPIFlash_Initialized;

void SPI_Flash_Init(void);
int32_t SPIFLASH_disk_initialize(void);
void    SPI1_Init(void);
;
void SPI_Write_Byte(uint8_t Byte);
//uint8_t SPI_Write_Byte(uint8_t data);
uint8_t SPI_Flash_RawWrite_new1(uint32_t address, uint8_t *pData, uint32_t nCount);
#if USE_GETCHIPID
int SPI_Flash_GetChipID(void);
#endif

#if USE_READ_STATUSREG
char   SPI_Flash_Read_StatusReg(void);
#endif

#if USE_WRITE_STATUSREG
void     SPI_Flash_Write_StatusReg(u8 reg);
#endif

#if USE_WRITE_ENABLE
void    SPI_Flash_Write_Enable(void);
#endif

#if USE_WRITE_DISABLE
void    SPI_Flash_Write_Disable(void);
#endif

#if USE_ERASE_CHIP
void    SPI_Flash_Erase_Chip(void);
#endif

#if USE_ERASE_SECTOR
void    SPI_Flash_Erase_Sector(uint32_t nDest);
#endif

#if USE_ERASE_BLOCK
void    SPI_Flash_Erase_Block(uint32_t nDest);
#endif

#if USE_WAIT_BUSY
void    SPI_Flash_Wait_Busy(void);
#endif

#if USE_POWERDOWN
void    SPI_Flash_PowerDown(void);
#endif

#if USE_RELEASEPOWERDOWN
void    SPI_Flash_ReleasePowerDown(void);
#endif

#if USE_READ_BYTES
void    SPI_Flash_Read_Bytes(uint32_t nDest, char* pBuffer, char nBytes);
#endif

#if USE_WRITE_BYTES
void    W25X_Write_Bytes(uint32_t nDest,char* pBuffer, u8 nBytes);
#endif

#if USE_READ_SECTOR
void SPI_Flash_Read_Sector(uint32_t nSector, BYTE* pBuffer);
#endif

#if USE_WRITE_SECTOR
void    SPI_Flash_Write_Sector(uint32_t nSector, char* pBuffer);
#endif

#endif



