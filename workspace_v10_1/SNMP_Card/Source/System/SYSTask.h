//****************************************************************************
//*
//*
//* File: SYSTask.h
//*
//* Synopsis: System Task define file - related Structures and interfaces
//*
//*
//****************************************************************************

#ifndef _SYS_TASK_H_
#define _SYS_TASK_H_

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
#include "utils/swupdate.h"
#include "lwip/netdb.h"
#include "lwip/udp.h"
#include "lwip/timeouts.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "SPIExtRTC.h"
#include "ErrorHandler.h"
#include "SPIFLash_Datalogger.h"
#include "HibernateCalendar.h"
#include "EthernetTask.h"
#include "OnChipEEPROM.h"


//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------
#define SYS_MICROSECOND_TICK      (SYSTEM_CLOCK/1000000)
#define SYS_MILLISECOND_TICK      (SYSTEM_CLOCK/1000)
#define NTP_PORT 123                     // NTP uses port 123
#define NTP_SERVER_NAME "pool.ntp.org"   // Example NTP server
#define NTP_TIMEOUT_MS 10000             // 10 seconds timeout
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

uint32_t SYS_GetTickMicroSecond( void );
void SYSTask_TimerInit(void);
void SYS_Task();
void SYSTask_PeripheralEnable(void);
// Function prototypes
void (*NTP_Init_Send_Funptr)(void);
void NTP_send_request(const ip_addr_t *ntp_server_ip);
void NTP_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
void NTP_timeout(void *arg);
void NTP_process_response(uint8_t *ntp_data);
void NTP_start_dns(void);
void NTP_Send_Request(void);

typedef struct __attribute__ ((packed))
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Date;
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
}NTP_Time;

// Function prototypes
//void NTP_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
//void NTP_timeout(void *arg);
//void NTP_process_response(uint8_t *ntp_data);

//----------------------------------------------------------------------------


#endif//_SYS_TASK_H_
//==============================================================================
// END OF PWM_HANDLER_H
//==============================================================================


