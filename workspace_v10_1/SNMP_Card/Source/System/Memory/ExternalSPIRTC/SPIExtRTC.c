/*
 * SPIExtRTC.c
 *
 *  Created on: 14-Jun-2024
 *      Author: Rani Sonawane
 */
#include "SPIExtRTC.h"
#include "SPI_Flash.h"
#include "SYSTask.h"
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
#include "UPSInterface.h"
#include "HibernateCalendar.h"

//============================================================================
//                              > LOCAL VARIABLES <
//----------------------------------------------------------------------------
static int SPI0_Initialized=0;
static uint32_t u32blockinggetcounter=0,u32blockingputcounter=0;
BOOL Flag_Ext_SPI_RTC_Available=TRUE;
//============================================================================
//****************************************************************************
//* Function name
//****************************************************************************
//============================================================================
static bool _SSIBaseValid1(uint32_t ui32Base)
{
    return((ui32Base == SSI0_BASE) || (ui32Base == SSI1_BASE) ||
           (ui32Base == SSI2_BASE) || (ui32Base == SSI3_BASE));
}
void SPIRTC_SSIDataGet(uint32_t ui32Base, uint32_t *pui32Data)
{
    u32blockinggetcounter=0;
    u32blockingputcounter=0;
    //
      // Check the arguments.
      //
      ASSERT(_SSIBaseValid1(ui32Base));

      //
      // Wait until there is data to be read.
      //
      while(!(HWREG(ui32Base + SSI_O_SR) & SSI_SR_RNE))
      {
          u32blockinggetcounter++;
          if(u32blockinggetcounter>1000)
          {
               u32blockinggetcounter++;
               u32blockinggetcounter=0;
               break;
          }
      }
      //
      // Read data from SSI.
      //
      *pui32Data = HWREG(ui32Base + SSI_O_DR);
      u32blockinggetcounter=0;
}

void SPIRTC_SSIDataPut(uint32_t ui32Base, uint32_t ui32Data)
{
    //
    // Check the arguments.
    //
    ASSERT(_SSIBaseValid1(ui32Base));
    ASSERT((ui32Data & (0xfffffffe << (HWREG(ui32Base + SSI_O_CR0) &
                                       SSI_CR0_DSS_M))) == 0);
    //
    // Wait until there is space.
    //
    while(!(HWREG(ui32Base + SSI_O_SR) & SSI_SR_TNF))
    {
        u32blockingputcounter++;
        if(u32blockingputcounter>=1000)
        {
            u32blockingputcounter++;
            u32blockingputcounter=0;
            break;
        }
    }

    //
    // Write the data to the SSI.
    //
    HWREG(ui32Base + SSI_O_DR) = ui32Data;
    u32blockingputcounter=0;
}

void SPI0_WriteByte(uint8_t Byte)
{
    uint32_t dummy=0;
    if(SPI0_Initialized==1)
    {
        SPIRTC_SSIDataPut(SSI0_BASE, (Byte));
        SPIRTC_SSIDataGet(SSI0_BASE, (uint32_t *)(&dummy));
    }
}
uint32_t SPI0_ReadByte(void)
{
    uint32_t ui32Data=0;

    if(SPI0_Initialized==1)
    {
       SPIRTC_SSIDataPut(SSI0_BASE, (0xFF));
       SPIRTC_SSIDataGet(SSI0_BASE, (uint32_t *)(&ui32Data));

    }
    return ui32Data;
}
uint32_t pui32DataRx[10];
void SPI0_Init()
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Check if the peripheral access is enabled.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }

   MAP_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
   MAP_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
   MAP_GPIOPinConfigure(GPIO_PA4_SSI0XDAT0); //tx
   MAP_GPIOPinConfigure(GPIO_PA5_SSI0XDAT1); //rx

    // Configure the GPIO settings for the SSI pins.  This function also gives
    // control of these pins to the SSI hardware.  Consult the data sheet to
    // see which functions are allocated per pin.
    // The pins are assigned as follows:
    //      SSI0
    //      PA5 - SSI0RX
    //      PA4 - SSI0TX
    //      PA3 - SSI0Fss
    //      PA2 - SSI0CLK

   MAP_SysCtlDelay(ui32_TReset*1000); // hold L
   SPIRTCSDeAssert();
   MAP_SysCtlDelay(ui32_TReset*1000); // hold L



   GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 |
                  GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

   GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_INT_PIN_3);

   GPIOPinTypeSSI(GPIO_PORTA_BASE,  GPIO_PIN_2 |GPIO_PIN_4 | GPIO_PIN_5 );

  // IntEnable(INT_GPIOA);
  SSIConfigSetExpClk( SSI0_BASE, output_clock_rate_hz , SSI_FRF_MOTO_MODE_0,
                           SSI_MODE_MASTER,
                                (   100000),
                                    8);
   SSIEnable(SSI0_BASE);

   while(SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
    }
    SPI0_Initialized = 1;
}

static  uint8_t BcdToBin(uint8_t Bcd)
{
    return  (((Bcd & 0xF0) >> 4)*10 + (Bcd & 0x0F));
}

static  uint8_t BinToBcd(uint8_t Bin)
{
    return ((Bin/10) << 4) | (Bin % 10);
}
void RTC_Init(void)
{
    uint8_t ControlReg=0;
    SPI0_Init();
    SPIRTCCSAssert();
    SPI0_WriteByte(0x8d);
    SPI0_WriteByte(0x18);
    SPIRTCSDeAssert();

    SPI0_WriteByte(0xFF); /////////All read write byte are non blocking, to read data need write FF
    SPIRTCCSAssert();
    SPI0_WriteByte(0x0D);
    ControlReg = SPI0_ReadByte();
    SPIRTCSDeAssert();

    if(ControlReg==0x18)           //for testing Internal RTC
    {
        Flag_Ext_SPI_RTC_Available=TRUE;
    }
    else
    {
        Flag_Ext_SPI_RTC_Available=FALSE;
    }

    if(Flag_Ext_SPI_RTC_Available==FALSE)
    {
        InitHibernateCalender();
    }
}

static void GetCalendarMatchValueExtRTC(uint8_t *datetime, Time SystemTimeSet)
{
    uint32_t ui32MonthDays;

    //
    // Get the current date and time and add 5 secs to it.
    //
   // HibernateCalendarGet(sTime);
    SystemTimeSet.Seconds += 5;

    //
    // Check if seconds is out of bounds.  If so subtract seconds by 60 and
    // increment minutes.
    //
    if(SystemTimeSet.Seconds > 59)
    {
        SystemTimeSet.Seconds -= 60;
        SystemTimeSet.Minutes++;
    }

    //
    // Check if minutes is out of bounds.  If so subtract minutes by 60 and
    // increment hours.
    //
    if(SystemTimeSet.Minutes > 59)
    {
        SystemTimeSet.Minutes -= 60;
        SystemTimeSet.Hours++;
    }

    //
    // Check if hours is out of bounds.  If so subtract minutes by 24 and
    // increment days.
    //
    if(SystemTimeSet.Hours > 23)
    {
        SystemTimeSet.Hours -= 24;
        SystemTimeSet.Date++;
    }

    //
    // Since different months have varying number of days, get the number of
    // days for the current month and year.
    //
    ui32MonthDays = GetDaysInMonth(SystemTimeSet.Year, SystemTimeSet.Month);

    //
    // Check if days is out of bounds for the current month and year.  If so
    // subtract days by the number of days in the current month and increment
    // months.
    //
    if(SystemTimeSet.Date > ui32MonthDays)
    {
        SystemTimeSet.Date -= ui32MonthDays;
        SystemTimeSet.Month++;
    }

    //
    // Check if months is out of bounds.  If so subtract months by 11 and
    // increment years.
    //
    if(SystemTimeSet.Month > 11)
    {
        SystemTimeSet.Month -= 11;
        SystemTimeSet.Year++;
    }

    //
    // Check if years is out of bounds.  If so subtract years by 100.
//    //
//    if(sTime->tm_year > 99)
//    {
//        sTime->tm_year -= 100;
//    }
}
void RTC_WriteNewDateTime(Time SystemTimeSet)
{
    uint8_t Itr[8], i = 0;
    Itr[0]=0;//msec
    Itr[1] = SystemTimeSet.Seconds;
    Itr[2] = SystemTimeSet.Minutes;
    Itr[3] = SystemTimeSet.Hours;
    Itr[4] = 0;//Day
    Itr[5] = SystemTimeSet.Date;
    Itr[6] = SystemTimeSet.Month;
    Itr[7] = SystemTimeSet.Year;

    GetCalendarMatchValueExtRTC(Itr,SystemTimeSet);
    Itr[0]=0;//msec
    Itr[1] = SystemTimeSet.Seconds;
    Itr[2] = SystemTimeSet.Minutes;
    Itr[3] = SystemTimeSet.Hours;
    Itr[4] = 0;
    Itr[5] = SystemTimeSet.Date;
    Itr[6] = SystemTimeSet.Month;
    Itr[7] = SystemTimeSet.Year;


    SPI0_WriteByte(0xFF);
    SPIRTCCSAssert();
    SPI0_WriteByte(WRITE_TIME);
    for(;i <8; i++)
    {
        SPI0_WriteByte(BinToBcd(Itr[i]));
    }
    SPIRTCSDeAssert();
}

void RTC_ReadDateTime()
{
    uint8_t Datetime[8]={0},itr=0;
   SPI0_WriteByte(0xFF); /////////All read write byte are non blocking, to read data need write FF

   SPIRTCCSAssert();

   SPI0_WriteByte(READ_TIME);

   for(itr=0;itr<8;itr++)
   {
       Datetime[itr]=BcdToBin(SPI0_ReadByte());
   }
//   if(Datetime[7]>=24)
   {
       GlobalDateTime.Seconds = Datetime[1];
       GlobalDateTime.Minutes = Datetime[2];
       GlobalDateTime.Hours   = Datetime[3];
       GlobalDateTime.Date    = Datetime[5];
       GlobalDateTime.Month   = Datetime[6];
       GlobalDateTime.Year    = Datetime[7];
   }

   SPIRTCSDeAssert();
}
//==============================================================================
// END OF SPIRTCExt.c FIlE
//==============================================================================
