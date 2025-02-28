/*
 * WriteProtectionCmdsTry.c
 *
 *  Created on: 25-Apr-2023
 *      Author: Rani Sonawane
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/udma.h"
#include "usblib/usblib.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "SPI_Flash.h"
#include "FatFS.h"
#include <ErrorHandler.h>
#include "SPIFLash_Datalogger.h"
#include "SYSTask.h"
#include <Common.h>
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>SSI Master-to-Slave Transfer (spi_master_slave_xfer)</h1>
//!
//! This example demonstrates how to configure SSI0 as a SSI Master and SSI1
//! as a SSI slave.  The master will send four characters on the master to the
//! slave using the legacy mode.  In legacy mode, one bit is sent on each
//! SSI Clock pulse.  Once the SSI slave receives the four characters in the
//! receive FIFO it will generate an interrupt.
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SSI0 peripheral
//! - GPIO Port A peripheral (for SSI0 pins)
//! - SSI0Clk    - PA2
//! - SSI0Fss    - PA3
//! - SSI0TX     - PA4
//! - SSI0RX     - PA5
//!
//! - SSI1 peripheral
//! - GPIO Port B & E peripheral (for SSI1 pins)
//! - SSI1Clk    - PB5
//! - SSI1Fss    - PB4
//! - SSI1TX     - PE4
//! - SSI1RX     - PE5
//!
//! This example requires board level connection between SSI0 and SSI1.
//!
//! UART0, connected to the Virtual Serial Port and running at 115,200, 8-N-1,
//! is used to display messages from this application.
//
//*****************************************************************************

uint8_t SPIFlash_Initialized;
bool SPI_Flash_Erase(int32_t addr, int32_t len);
uint32_t g_ui32SysClock;
uint32_t DeviceInfo[8]={0};
//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************

//volatile tState g_eState;
//
//volatile tState g_eUIState;
int32_t res;
uint32_t IDDev;
uint8_t SPI1_Initialized=0,byt;

enum
{
    SPI_INIT_TRUE=0,
    SPI_INIT_FALSE=1
};

int32_t SPIFLASH_disk_initialize(void)
{
    BOOL status=SPI_INIT_TRUE;

    IDDev= SPI_Flash_GetChipID();
    SPI_Flash_Write_Enable();
    byt=SPI_Flash_Read_StatusReg();

    if(IDDev!=0xFFFFFF && byt==0x02)
        return SPI_INIT_TRUE;
    else
        return SPI_INIT_FALSE;
}

void SPI_Flash_Init(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);

    SDSPICSAssert();
    MAP_SysCtlDelay(ui32_TReset); // hold L
    SDSPICSDeAssert();

    SPI1_Init_HighSpeed();

    SPI_Flash_ResetWriteProtection();

}

/////////////////////

void SPI_Flash_ResetWriteProtection(void)
{
    // send write enable command
    SDSPICSAssert();
    SPI1_WriteByte(SST25_CMD_EWSR);
    SDSPICSDeAssert();

    // Delay for next I/O
    SPI1_WriteByte(0);

    SDSPICSAssert();
    SPI1_WriteByte(SST25_CMD_WRSR);
    SPI1_WriteByte(0);
    SDSPICSDeAssert();

    while(SPI_Flash_WriteBusy());
}
/////////////////////////
/************************************************************************
* Function: void SST25ReadArray(DWORD address, uint8_t* pData, nCount)
*
* Overview: this function reads data into buffer specified
*
* Input: flash memory address, pointer to the data buffer, data number
*
************************************************************************/

void SPI_Flash_RawRead(uint32_t address, uint8_t cmddata, uint8_t *pData, uint32_t nCount,uint8_t Cmdordata)
{
    uint32_t dummy;
    union
    {
        uint32_t addr;
        uint8_t addrbyt[4];
    }un;
    un.addr=0;
    un.addrbyt[0]=0;
    un.addrbyt[1]=0;
    un.addrbyt[2]=0;
    un.addrbyt[3]=0;

  while(MAP_SSIDataGetNonBlocking(SSI3_BASE,&dummy));

   // un.addr=address&0x0000FFFF;
    un.addr=address;

    SDSPICSAssert();

    if(Cmdordata == READ_DATA)
    {

        SPI1_WriteByte(SST25_CMD_READ);
        SPI1_WriteByte(un.addrbyt[2]);
        SPI1_WriteByte(un.addrbyt[1]);
        SPI1_WriteByte(un.addrbyt[0]);

    }
    else
    {
        SPI1_WriteByte(cmddata);
        SPI1_WriteByte(un.addrbyt[0]);
        SPI1_WriteByte(un.addrbyt[1]);
        SPI1_WriteByte(un.addrbyt[2]);

    }

    while(nCount--)
    {
        *pData++ = SPI1_ReadByte();
    }

    SDSPICSDeAssert();

}
/************************************************************************
* Function: SST25WriteEnable()
*
* Overview: this function allows write/erase SST25. Must be called
* before every write/erase command.
*
* Input: none
*
* Output: none
*
************************************************************************/
void SPI_Flash_WriteEnable(void)
{

    SDSPICSAssert();
    SPI1_WriteByte(SST25_CMD_WREN);
    SDSPICSDeAssert();
}

/////////////
/************************************************************************
* Function: uint8_t SST25IsWriteBusy(void)
*
* Overview: this function reads status register and chek BUSY bit for write operation
*
* Input: none
*
* Output: non zero if busy
*
************************************************************************/
uint8_t SPI_Flash_WriteBusy(void)
{
    uint8_t Status;
    uint32_t dummy;
    while(MAP_SSIDataGetNonBlocking(SSI3_BASE,&dummy));

    SDSPICSAssert();

    SPI1_WriteByte(SST25_CMD_RDSR);

    Status = SPI1_ReadByte();

    SDSPICSDeAssert();

    return (Status & 0x01);
}

////////////////////////
void SPI_Flash_RawErase(void)
{
    SPI1_Init_HighSpeed();

    SPI_Flash_WriteEnable();

    SDSPICSAssert();

    SPI1_WriteByte(SST25_CMD_ERASE);
    SDSPICSDeAssert();

    // Wait for write end
    while(SPI_Flash_WriteBusy());
}

//////////
void SPI1_Init_HighSpeed(void)
{
    if(SPI1_Initialized)
       SPI1_DeInit();
   {

       MAP_GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
       MAP_GPIOPinConfigure(GPIO_PQ1_SSI3FSS);
       MAP_GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0); //tx
       MAP_GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1); //rx

        /*MAP_SysCtlClockGet()*/
       MAP_SSIConfigSetExpClk( SSI3_BASE, output_clock_rate_hz , SSI_FRF_MOTO_MODE_0,
                                    SSI_MODE_MASTER,
                                    (40000000),
                                        8);
       MAP_SSIAdvModeSet(SSI3_BASE,SSI_ADV_MODE_READ_WRITE);//SSI_ADV_MODE_LEGACY);//SSI_ADV_MODE_READ_WRITE);//SSI_ADV_MODE_READ_WRITE);/SSI_ADV_MODE_READ_WRITE);//SSI_ADV_MODE_LEGACY);// SSI_ADV_MODE_READ_WRITE);//SSI_ADV_MODE_QUAD_WRITE);//SSI_ADV_MODE_READ_WRITE);//SSI_ADV_MODE_QUAD_WRITE);


       MAP_GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
       MAP_GPIOPinTypeSSI(GPIO_PORTQ_BASE,  GPIO_PIN_0 |GPIO_PIN_2 | GPIO_PIN_3 );

       MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);

       MAP_SSIEnable(SSI3_BASE);

       SPI1_Initialized = 1;
   }
}
//////////
/************************************************************************
* Function: void SST25WriteByte(BYTE data, DWORD address)
*
* Overview: this function writes a byte to the address specified
*
* Input: data to be written and address
*
* Output: none
*
************************************************************************/
void SPI_Flash_WriteByte(uint8_t DatatoWrite, uint32_t address)
{
    uint32_t dummy=0;
       union
       {
           uint32_t addr;
           uint8_t addrbyt[4];
       }un;
       un.addr=0;
       un.addrbyt[0]=0;
       un.addrbyt[1]=0;
       un.addrbyt[2]=0;
       un.addrbyt[3]=0;
       un.addr=address;

       while(MAP_SSIDataGetNonBlocking(SSI3_BASE,&dummy));
       SPI_Flash_WriteEnable();
       SDSPICSAssert();

       SPI1_WriteByte(SST25_CMD_WRITE);
       SPI1_WriteByte(un.addrbyt[2]);
       SPI1_WriteByte(un.addrbyt[1]);
       SPI1_WriteByte(un.addrbyt[0]);

       SPI1_WriteByte(DatatoWrite&0xFF);

       SDSPICSDeAssert();
       while(SPI_Flash_WriteBusy());
}
/************************************************************************
* Function: BYTE SST25WriteArray(DWORD address, BYTE* pData, nCount)
*
* Overview: this function writes a data array at the address specified
*
* Input: flash memory address, pointer to the data array, data number
*
* Output: return 1 if the operation was successfull
*
************************************************************************/
void SPI_Flash_RawWrite(uint32_t address, uint8_t *pData, uint32_t nCount)
{
    uint32_t   addr;
    uint8_t    *pD;
    uint32_t    counter;

    addr = address;
    pD = pData;

 //   SPI1_Init_HighSpeed();
  //Write
    for(counter = 0; counter < nCount; counter++)
    {
        SPI_Flash_WriteByte(*pD++, addr++);
    }
}


void SPI1_DeInit(void)
{
    MAP_SSIDisable(SSI3_BASE);//SpiChnClose(SPI_CHANNEL1);
    SPI1_Initialized = 0;
}
////////////
void  DataGet_SSI3(uint32_t ui32Base, uint32_t *pui32Data)
{
    //uint16_t waitcnt=100;
    //
    // Check the arguments.
    //
  //  ASSERT(_SSIBaseValid(ui32Base));

#if 1
    //Method 2 for timeout from whileloop
    while(!(HWREG(ui32Base + SSI_O_SR) & SSI_SR_RNE))
    {
        MAP_SysCtlDelay(ui32_TReset*10);
        break;
    }
#endif
    //
    // Read data from SSI.
    //
    *pui32Data = HWREG(ui32Base + SSI_O_DR);
}



void DataPut_SSI3(uint32_t ui32Base, uint32_t ui32Data)
{
 //   uint16_t waitcnt=100;
    //
    // Check the arguments.
 //   //
 //   ASSERT(_SSIBaseValid(ui32Base));
    ASSERT((ui32Data & (0xfffffffe << (HWREG(ui32Base + SSI_O_CR0) &
                                       SSI_CR0_DSS_M))) == 0);

#if 1
    //Method 1 for timeout to break while loop
    while(!(HWREG(ui32Base + SSI_O_SR) & SSI_SR_TNF))
    {
            MAP_SysCtlDelay(ui32_TReset*10);

        break;
    }
#endif

    HWREG(ui32Base + SSI_O_DR) = ui32Data;
}

///////////////
/**********************************************************************************************
 *  Function Name:
 * ********************************************************************************************/

void SPI_Write_Byte(uint8_t Byte)
{
    uint32_t dummy;
    if(SPI1_Initialized)
     {
        while(MAP_SSIDataGetNonBlocking(SSI3_BASE,&dummy));
        DataPut_SSI3(SSI3_BASE,Byte);
        DataGet_SSI3(SSI3_BASE,&dummy);

     }
}
/**********************************************************************************************
 *  Function Name:
 * ********************************************************************************************/
uint32_t SPI1_ReadByte(void)
{
    uint8_t data=0;
    uint32_t dummy;
    while(MAP_SSIDataGetNonBlocking(SSI3_BASE,&dummy));
    if(SPI1_Initialized)
    {

       DataPut_SSI3(SSI3_BASE,0xFF);
       DataGet_SSI3(SSI3_BASE,(uint32_t *) &data);
       return data;
    }
    else
    return 0;
}


#if USE_GETCHIPID
int SPI_Flash_GetChipID(void)
{
    int nID = 0;

    SDSPICSAssert();//FLASH_CS_0();

    SPI_Write_Byte(W25X_JedecDeviceID);
    nID = SPI_Read_Byte();
    nID <<= 8;
    nID |= SPI_Read_Byte();
    nID <<= 8;
    nID |= SPI_Read_Byte();

    SDSPICSDeAssert();// FLASH_CS_1();

    return nID;
}
#endif

#if USE_READ_STATUSREG
char SPI_Flash_Read_StatusReg(void)
{
    char u8 = 0;
    SDSPICSAssert();//FLASH_CS_0();
    SPI_Write_Byte(W25X_ReadStatusReg);
    u8 = SPI_Read_Byte();
    SDSPICSDeAssert();// FLASH_CS_1();
    return u8;
}
#endif

#if USE_WRITE_STATUSREG
void SPI_Flash_Write_StatusReg(u8 reg)
{   FLASH_CS_0();
    SPI_Write_Byte(W25X_WriteStatusReg);
    SPI_Write_Byte(reg);
    FLASH_CS_1();
}
#endif

#if USE_WRITE_ENABLE
void SPI_Flash_Write_Enable(void)
{   SDSPICSAssert();//FLASH_CS_0();
    SPI_Write_Byte(W25X_WriteEnable);
    SDSPICSDeAssert();//FLASH_CS_1();
}
#endif

#if USE_WRITE_DISABLE
void SPI_Flash_Write_Disable(void)
{   FLASH_CS_0();
    SPI_Write_Byte(W25X_WriteDisable);
    FLASH_CS_1();
}
#endif

#if USE_WAIT_BUSY
void SPI_Flash_Wait_Busy(void)
{   
		while(SPI_Flash_Read_StatusReg() == 0x03)
      		 SPI_Flash_Read_StatusReg();
}
#endif

#if USE_ERASE_SECTOR
void SPI_Flash_Erase_Sector(uint32_t nDest)
{
    nDest *= FLASH_SECTOR_SIZE;

    SPI_Flash_WriteEnable();
    SDSPICSAssert();
    SPI_Write_Byte(W25X_SectorErase);
    SPI_Write_Byte((char)((nDest & 0xFFFFFF) >> 16));
    SPI_Write_Byte((char)((nDest & 0xFFFF) >> 8));
    SPI_Write_Byte((char)nDest & 0xFF);
    SDSPICSDeAssert();
    while(SPI_Flash_WriteBusy());

}
#endif

#if USE_ERASE_BLOCK
void SPI_Flash_Erase_Block(uint32_t nDest)
{
    nDest *= FLASH_BLOCK_SIZE;

    nDest *= FLASH_BLOCK_SIZE;
    SPI_Flash_WriteEnable();
    SDSPICSAssert();
    SPI_Write_Byte(W25X_SectorErase);
    SPI_Write_Byte((char)((nDest & 0xFFFFFF) >> 16));
    SPI_Write_Byte((char)((nDest & 0xFFFF) >> 8));
    SPI_Write_Byte((char)nDest & 0xFF);
    SDSPICSDeAssert();
    while(SPI_Flash_WriteBusy());

}
#endif

#if USE_ERASE_CHIP
void SPI_Flash_Erase_Chip(void)
{
    SPI_Flash_WriteEnable();
    while(SPI_Flash_WriteBusy());
    SDSPICSAssert();
    SPI_Write_Byte(W25X_ChipErase);
    SDSPICSDeAssert();
    while(SPI_Flash_WriteBusy());
}
#endif

void SPI_Flash_Read_Bytes(uint32_t nDest, char* pBuffer, char nBytes)
{
   // uint16_t i;
    SPI_Flash_RawRead(nDest, 0, pBuffer, nBytes,READ_DATA);

}



void SPI_Flash_Read_Sector(uint32_t nSector, BYTE* pBuffer)
{
    //uint16_t i;
    nSector *= FLASH_SECTOR_SIZE;
    SPI_Flash_RawRead(nSector, 0, pBuffer,FLASH_SECTOR_SIZE,1);
    /*
    SDSPICSAssert();
    SPI_Write_Byte(W25X_ReadData);
    SPI_Write_Byte((char)(nSector >> 16));
    SPI_Write_Byte((char)(nSector>> 8));
    SPI_Write_Byte((char) nSector);

    for(i=0;i<FLASH_SECTOR_SIZE;i++)
    {
        pBuffer[i] = SPI_Read_Byte();
    }
    SDSPICSDeAssert();
    W25X_Wait_Busy();
    */
}
void SPI_Flash_Write_Sector(uint32_t nSector, char* pBuffer)
{
  //  int i,j;

    nSector *= FLASH_SECTOR_SIZE;
    SPI_Flash_RawWrite(nSector,pBuffer,FLASH_SECTOR_SIZE);

}

/**********************************************************************************************
 *  End of Main File
 * ********************************************************************************************/








