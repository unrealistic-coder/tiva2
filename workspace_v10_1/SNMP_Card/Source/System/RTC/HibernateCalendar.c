/*
 * HibernateCalendar.c
 *
 *  Created on: 01-Dec-2023
 *      Author: Rani Sonawane
 */

#include  "HibernateCalendar.h"
#include  "OnChipEEPROM.h"
#include  "SPIExtRTC.h"
//============================================================================
//                              > LOCAL VARIABLES <
//----------------------------------------------------------------------------
RTC_TIME_COMPENSATION_STRUCT RtcTimecompasatiomSt;


//*****************************************************************************
//
// Flag that informs that the user has requested hibernation.
//
//*****************************************************************************
volatile bool g_bHibernate;

//*****************************************************************************
//
// Flag that informs that date and time have to be set.
//
//*****************************************************************************
volatile bool g_bSetDate;
uint32_t g_ui32MonthIdx=0, g_ui32DayIdx=0, g_ui32YearIdx=0,g_u32secIdx=0;
uint32_t g_ui32HourIdx=0, g_ui32MinIdx=0;
uint32_t SEC_CNT=0;
//*****************************************************************************
//
// Buffers to store display information.
//
//*****************************************************************************
char g_pcWakeBuf[40], g_pcHibBuf[40], g_pcDateTimeBuf[40];


//*****************************************************************************
//
// Buffer to store user command line input.
//
//*****************************************************************************
char g_pcInputBuf[40];
//*****************************************************************************
//
// Variables that keep terminal position and status.
//
//*****************************************************************************
bool g_bFirstUpdate;
uint8_t g_ui8FirstLine;
//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//*****************************************************************************
//
// This function returns the number of days in a month including for a leap
// year.
//
//*****************************************************************************
uint32_t
GetDaysInMonth(uint32_t ui32Year, uint32_t ui32Mon)
{
    uint8_t LepaYearFlag=0;
    //
    // Return the number of days based on the month.
    //
    if(ui32Mon == 1)
    {
        //
        // For February return the number of days based on the year being a
        // leap year or not.
        //
        if((ui32Year % 4) == 0)
        {
            //
            // If leap year return 29.
            //
            LepaYearFlag=1;
            return 29;

          }
        else
        {
            //
            // If not leap year return 28.
            //
            return 28;
        }
    }
    else if((ui32Mon == 3) || (ui32Mon == 5) || (ui32Mon == 8) ||
            (ui32Mon == 10))
    {
        //
        // For April, June, September and November return 30.
        //
        return 30;
    }

    //
    // For all the other months return 31.
    //
    return 31;
}

//*****************************************************************************
//
// This function returns the date and time value that is written to the
// calendar match register.  5 seconds are added to the current time.  Any
// side-effects due to this addition are handled here.
//
//*****************************************************************************
void
GetCalendarMatchValue(struct tm* sTime)
{
    uint32_t ui32MonthDays;

    //
    // Get the current date and time and add 5 secs to it.
    //
   // HibernateCalendarGet(sTime);
    if(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable!=1)
    {
        sTime->tm_sec += 1;
    }


    //
    // Check if seconds is out of bounds.  If so subtract seconds by 60 and
    // increment minutes.
    //
    if(sTime->tm_sec > 59)
    {
        sTime->tm_sec -= 60;
        sTime->tm_min++;
    }

    //
    // Check if minutes is out of bounds.  If so subtract minutes by 60 and
    // increment hours.
    //
    if(sTime->tm_min > 59)
    {
        sTime->tm_min -= 60;
        sTime->tm_hour++;
    }

    //
    // Check if hours is out of bounds.  If so subtract minutes by 24 and
    // increment days.
    //
    if(sTime->tm_hour > 23)
    {
        sTime->tm_hour -= 24;
        sTime->tm_mday++;
    }

    //
    // Since different months have varying number of days, get the number of
    // days for the current month and year.
    //
    ui32MonthDays = GetDaysInMonth(sTime->tm_year, sTime->tm_mon);

    //
    // Check if days is out of bounds for the current month and year.  If so
    // subtract days by the number of days in the current month and increment
    // months.
    //
    if(sTime->tm_mday > ui32MonthDays)
    {
        sTime->tm_mday -= ui32MonthDays;
        sTime->tm_mon++;
    }

    //
    // Check if months is out of bounds.  If so subtract months by 11 and
    // increment years.
    //
    if(sTime->tm_mon > 11)
    {
        sTime->tm_mon -= 11;
        sTime->tm_year++;
    }

    //
    // Check if years is out of bounds.  If so subtract years by 100.
//    //
//    if(sTime->tm_year > 99)
//    {
//        sTime->tm_year -= 100;
//    }
}

//*****************************************************************************
//
// This function does some application level cleanup and alerts the user
// before sending the hibernate request to the hardware.
//
//*****************************************************************************
void
AppHibernateEnter(void)
{
    uint32_t ui32Status;
    struct tm sTime;

    //
    // Print the buffer to the terminal.
    //
  //  UARTprintf("To wake, wait for 5 seconds or press WAKE or"
  //             "RESET\n");
  //  UARTprintf("See README.txt for additional wake sources.\n");

    //
    // Wait for UART transmit to complete before proceeding to
    // hibernate.
    //
    //UARTFlushTx(false);

    //
    // Get calendar match value to be 5 seconds from the current time.
    //
    GetCalendarMatchValue(&sTime);

    //
    // Set the calendar match register such that it wakes up from
    // hibernation in 5 seconds.
    //
    HibernateCalendarMatchSet(0, &sTime);

    //
    // Read and clear any status bits that might have been set since
    // last clearing them.
    //
    ui32Status = HibernateIntStatus(0);
    HibernateIntClear(ui32Status);

    //
    // Configure Hibernate wake sources.
    //
    HibernateWakeSet(HIBERNATE_WAKE_PIN | HIBERNATE_WAKE_GPIO |
                     HIBERNATE_WAKE_RESET | HIBERNATE_WAKE_RTC);

    //
    // Request Hibernation.
    //

    HibernateGPIORetentionEnable();
      HibernateRequest();

    //
    // Wait for a while for hibernate to activate.  It should never get
    // past this point.
    //
    //(100);

    //
    // If it ever gets here, store the text, that informs the user on
    // what to do, into the respective widget buffers.
    //
//    UARTprintf("The controller did not enter hibernate.  Press RESET"
//               "button to restart example.\n");

    //
    // Wait here.
    //
//    while(1)
//    {
//    }
}


//*****************************************************************************
//
// This function reads the current date and time from the calendar logic of the
// hibernate module.  Return status indicates the validity of the data read.
// If the received data is valid, the 24-hour time format is converted to
// 12-hour format.
//
//*****************************************************************************
bool
Read_DateTime()
{
    struct tm sTime;
    //
    // Get the latest time.
    //
    HibernateCalendarGet(&sTime);

    //
    // Is valid data read?
    //
    if(((sTime.tm_sec < 0) || (sTime.tm_sec > 59)) ||
       ((sTime.tm_min < 0) || (sTime.tm_min > 59)) ||
       ((sTime.tm_hour < 0) || (sTime.tm_hour > 23)) ||
       ((sTime.tm_mday < 1) || (sTime.tm_mday > 31)) ||
       ((sTime.tm_mon < 0) || (sTime.tm_mon > 12)) ||
       ((sTime.tm_year < 100) || (sTime.tm_year > 199)))
    {
        //
        // No - Let the application know the same by returning relevant
        // message.
        //
        return false;
    }

    GlobalDateTime.Year=(sTime.tm_year-100);
    GlobalDateTime.Month=sTime.tm_mon+1;
    GlobalDateTime.Date=sTime.tm_mday;
    GlobalDateTime.Hours=sTime.tm_hour;
    GlobalDateTime.Minutes=sTime.tm_min;
    GlobalDateTime.Seconds=sTime.tm_sec;
    //
    // Return that new data is available so that it can be displayed.
    //
    return true;
}
//*****************************************************************************
//
// This function writes the requested date and time to the calendar logic of
// hibernation module.
//
//*****************************************************************************
void WriteNewDateTime(void)
{
    struct tm sTime;

    //
    // Get the latest date and time.  This is done here so that unchanged
    // parts of date and time can be written back as is.
    //
    HibernateCalendarGet(&sTime);
    //
    // Set the date and time values that are to be updated.
    //
    sTime.tm_hour = g_ui32HourIdx;
    sTime.tm_min = g_ui32MinIdx;
    sTime.tm_mon =  g_ui32MonthIdx;
    sTime.tm_mday =  g_ui32DayIdx;
    sTime.tm_year = 100 + g_ui32YearIdx;
    sTime.tm_sec  = g_u32secIdx;
    GetCalendarMatchValue(&sTime);
    //
    // Update the calendar logic of hibernation module with the requested data.
    //
    HibernateCalendarSet(&sTime);
}
void ReadCurrentDateTime()
{
    if(Flag_Ext_SPI_RTC_Available==TRUE)
    {
        RTC_ReadDateTime(); //External RTC Read Date Time
    }
    else
    {
        if(Read_DateTime() == false)
           {
              //InternalRTCRead Date Time
           }
    }
}
//=======================================================================================================
//* Function Name: WriteNewDateTimeToRTC
//*
//=======================================================================================================
void WriteNewDateTimeToRTC(Time Systemtime)
{
    if(Flag_Ext_SPI_RTC_Available==TRUE)
    {
        RTC_WriteNewDateTime(Systemtime);
    }
    else
    {
        if(Systemtime.Month>=1)
        {
            g_ui32MonthIdx = Systemtime.Month-1;  //SPI based RT
        }
         g_ui32DayIdx = Systemtime.Date;
         g_ui32YearIdx = Systemtime.Year;
         g_ui32HourIdx = Systemtime.Hours;
         g_ui32MinIdx = Systemtime.Minutes;
         g_u32secIdx= Systemtime.Seconds;
         WriteNewDateTime();
    }
}
//=======================================================================================================
//* Function Name: RTC_TimeCompensation_RequestTimeout
//*
//=======================================================================================================
static void RTC_TimeCompensation_RequestTimeout(COM_TIMER_ID id, void* param)
{

    RTC_TIME_COMPENSATION_STRUCT  *pRtcTimecompasatiomSt;
    pRtcTimecompasatiomSt=&RtcTimecompasatiomSt;
    uint32_t RTCCompensateInterval=0;
    uint16_t RTCCompesateTime=0;
    Time SystemTimeSet;
    RTCCompensateInterval = (Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroyafterhourSet * 60 * 60);
    RTCCompesateTime = Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroySecondsSet;
    COMTimer_Stop(pRtcTimecompasatiomSt->reqTimeoutTimer);

     if(RTCCompensateInterval != 0 && RTCCompesateTime != 0)
     {
          if(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable==0)
          {
             if(++SEC_CNT >= RTCCompensateInterval) //  60*60*5(hours) = 18000, 60*60*3(hours) = 10800,  60*60*6(hours)= 21600
               {
                   SEC_CNT=0;
                   SystemTimeSet.Seconds = GlobalDateTime.Seconds;
                   SystemTimeSet.Minutes = GlobalDateTime.Minutes;
                   SystemTimeSet.Hours = GlobalDateTime.Hours;
                   SystemTimeSet.Date = GlobalDateTime.Date;
                   SystemTimeSet.Month = GlobalDateTime.Month;
                   SystemTimeSet.Year = GlobalDateTime.Year;

                   SystemTimeSet.Seconds = SystemTimeSet.Seconds + RTCCompesateTime; //increment sec by 1 every 2 hours

                   if(SystemTimeSet.Seconds < 60)
                       WriteNewDateTimeToRTC(SystemTimeSet);
               }
          }
     }
     COMTimer_Start(pRtcTimecompasatiomSt->reqTimeoutTimer,100);
}
//=======================================================================================================
//* Function Name: InitOneSecondTimeout
//*
//=======================================================================================================
static void InitOneSecondTimeout()
{
    RTC_TIME_COMPENSATION_STRUCT  *pRtcTimecompasatiomSt;
    pRtcTimecompasatiomSt=&RtcTimecompasatiomSt;
    COM_TIMER_PARAM     reqTimerParam;

    reqTimerParam.param = (void*) pRtcTimecompasatiomSt;
    reqTimerParam.timeoutFunc = RTC_TimeCompensation_RequestTimeout;
    reqTimerParam.recurringFlag = 0;
    pRtcTimecompasatiomSt->reqTimeoutTimer = COMTimer_Create(&reqTimerParam);


    if ( COM_TIMER_ID_INVALID == pRtcTimecompasatiomSt->reqTimeoutTimer)
      {

      }

    pRtcTimecompasatiomSt->reqTimeoutTime=((COM_TIMER_TICK)RTC_COMPENSATION_TIMEOUT);;
    COMTimer_Start(pRtcTimecompasatiomSt->reqTimeoutTimer,pRtcTimecompasatiomSt->reqTimeoutTime);

 }

/************************************************************************
**    FUNCTION NAME   : Hibernate_init
**
**    DESCRIPTION     : Initialize the hibernation module.
**
**    ARGUMENTS       : None
**
**    RETURN TYPE     : None
**
***************************************************************************/

void Hibernate_init(void)
{
    uint32_t ui32Status = 0;

 //   Types_FreqHz cpuFreq;

    //Get the CPU clock frequency(120MHz) set in BIOS
   // BIOS_getCpuFreq(&cpuFreq);

    //Extract the 32-bit frequency value
   // ui32SysClock = cpuFreq.lo;

    // Enable the hibernate module.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);

    // Initialize these variables before they are used.
    ui32Status = 0;

    // Check if Hibernation module is already active, which could mean
    // that the processor is waking from a hibernation.
    if(HibernateIsActive())
    {
        // Read the status bits to see what caused the wake.  Clear the wake
        // source so that the device can be put into hibernation again.
        ui32Status = HibernateIntStatus(0);
        HibernateIntClear(ui32Status);

        // System console prints to see which caused wakeup.
       // System_printf("Wake Due To : \n");

        // Wake was due to the External Wake pin.
        if(ui32Status & HIBERNATE_INT_PIN_WAKE)
        {
         //   System_printf(" Wakeup Pin\n");
        }
    }

    // Configure Hibernate module clock(120MHz).
    HibernateEnableExpClk(output_clock_rate_hz);

    // If the wake was not due to the above sources, then it was a system reset.
    if(!(ui32Status & (HIBERNATE_INT_PIN_WAKE)))
    {
        // Configure the module clock source.
        HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);

        // Print that this was a system restart, not wake from hibernation.
        //System_printf(" System restart\n");
    }
    // Enable processor interrupts.
    IntMasterEnable();
}

void InitHibernateCalender()
{
    uint32_t ui32Status = 0,ui32Len=0;
    uint32_t ui32HibernateCount = 0,ui32SysClock=0;
    ////Hibernate RTC

    SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);

     //
     // Initialize these variables before they are used.
     //
     ui32Status = 0;
     ui32HibernateCount = 0;

     //
     // Check to see if Hibernation module is already active, which could mean
     // that the processor is waking from a hibernation.
     //
     if(HibernateIsActive())
     {
         //
         // Read the status bits to see what caused the wake.  Clear the wake
         // source so that the device can be put into hibernation again.
         //
         ui32Status = HibernateIntStatus(0);
         HibernateIntClear(ui32Status);

            //
         // If the wake is due to any of the configured wake sources, then read
         // the first location from the battery-backed memory, as the
         // hibernation count.
         //
         if(ui32Status & (HIBERNATE_INT_PIN_WAKE | HIBERNATE_INT_RTC_MATCH_0 |
                                  HIBERNATE_INT_GPIO_WAKE | HIBERNATE_INT_RESET_WAKE))
             {
                 HibernateDataGet(&ui32HibernateCount, 1);
             }
     }

     //
     // Configure Hibernate module clock.
     //
     HibernateEnableExpClk(output_clock_rate_hz);

     //
     // If the wake was not due to the above sources, then it was a system
     // reset.
     //
     if(!(ui32Status & (HIBERNATE_INT_PIN_WAKE | HIBERNATE_INT_RTC_MATCH_0 |
                        HIBERNATE_INT_GPIO_WAKE | HIBERNATE_INT_RESET_WAKE)))
     {
         //
         // Configure the module clock source.
         //
         HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);

         g_bSetDate = true;
     }

     //

     //
     HibernateRTCEnable();

     //
     // Configure the hibernate module counter to 24-hour calendar mode.
     //
     HibernateCounterMode(HIBERNATE_COUNTER_24HR);


     InitOneSecondTimeout();

    //*****************************************************************************
    //
    // This function sets the time to the default system time.
    //
    //*****************************************************************************
     g_bHibernate = false;
     if(g_bSetDate)
            {
                //
                // Clear the flag.
                //
                g_bSetDate = false;

                }


}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

