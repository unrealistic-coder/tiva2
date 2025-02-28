/*
 * SPIExtRTC.h
 *
 *  Created on: 30-May-2024
 *      Author: Rani Sonawane
 */

#ifndef SOURCE_SYSTEM_MEMORY_RTC_RTC_H_
#define SOURCE_SYSTEM_MEMORY_RTC_RTC_H_
#include <Common.h>
#include "SPIFlashTask.h"
#include "COMTimer.h"
#include "UPSInterface.h"

#define    EXT_RTC_H

#define YEAR_BASE   2000

// RTC Register Addresses
#define MAX_YEAR    99
#define MIN_YEAR    0
#define MAX_MONTH   12
#define MIN_MONTH   1
#define MAX_DATE    31
#define MIN_DATE    1

// RTC Register Addresses
#define MILLISECS            0x00
#define SECONDS              0x01
#define MINUTES              0x02
#define HOURS                0x03
#define DAY                  0x04
#define DATE                 0x05
#define MONTH_CENT           0x06
#define YEAR                 0x07

#define ALARM_MILLISECS      0x08
#define ALARM_SECONDS        0x09
#define ALARM_MINUTES        0x0A
#define ALARM_HOURS          0x0B
#define ALARM_DAY_DATE       0x0C

#define CONTROL              0x0D
#define STATUS               0x0E
#define TRICKLE              0x0F

//==============================================================================
// RTC Commands
#define READ_TIME   (0x00)
#define WRITE_TIME  (0x80)

// Chip select Macro functions. RTC CS is Pin RA3
#define SPIRTCCSAssert()       (MAP_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0))
#define SPIRTCSDeAssert()      (MAP_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,GPIO_PIN_3))

void RTC_Init(void);
void RTC_ReadDateTime();
void RTC_WriteNewDateTime();

extern BOOL Flag_Ext_SPI_RTC_Available;
#endif /* SOURCE_SYSTEM_MEMORY_RTC_RTC_H_ */


