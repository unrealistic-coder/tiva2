/*
 * HibernateCalendar.h
 *
 *  Created on: 01-Dec-2023
 *      Author: Rani Sonawane
 */

#ifndef SOURCE_SYSTEM_RTC_HIBERNATECALENDAR_H_
#define SOURCE_SYSTEM_RTC_HIBERNATECALENDAR_H_

#include  "COMTimer.h"
#include  <Common.h>

#define RTC_COMPENSATION_TIMEOUT  (1000)

typedef struct __attribute__((packed))
{
    COM_TIMER_ID        reqTimeoutTimer;    // Request Timeout Timer
    COM_TIMER_TICK      reqTimeoutTime;
} RTC_TIME_COMPENSATION_STRUCT;
extern RTC_TIME_COMPENSATION_STRUCT RtcTimecompasatiomSt;


void InitHibernateCalender();
bool Read_DateTime();
void ReadCurrentDateTime();
void WriteNewDateTimeToRTC(Time SystimeToSet);
uint32_t GetDaysInMonth(uint32_t ui32Year, uint32_t ui32Mon);
void InitOneSecondTimeout();



#endif /* SOURCE_SYSTEM_RTC_HIBERNATECALENDAR_H_ */
