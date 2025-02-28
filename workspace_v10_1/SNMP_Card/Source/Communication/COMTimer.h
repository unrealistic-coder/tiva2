//****************************************************************************
//*
//*
//* File: COMTimer.h
//*
//* Synopsis: Communication Timer Header
//*
//*
//****************************************************************************

#ifndef COMTIMER_H
#define COMTIMER_H

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------

#if (_COMTIMER_DEBUG == 1)
#define	COMTIMER_printf(...)     UARTprintf(__VA_ARGS__)
#else
#define	COMTIMER_printf(...)     {};
#endif


#define COM_TIMER_ID_INVALID            0xFF
#define COM_TICK_MAX                    ( 0xFFFFFFFF)    
#define COM_TIMER_COUNT                 ((4*3)) // Max timer count. 
//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------
typedef uint8_t   COM_TIMER_ID;
typedef uint32_t  COM_TIMER_TICK; // 32bit resolution
typedef void (*COM_TIMER_FUNC) ( COM_TIMER_ID id, void* param );

typedef struct __attribute__((packed))
{
    COM_TIMER_FUNC  timeoutFunc;    // NULL = available
    void*        	param;          // Callback parameter
    BOOL  recurringFlag;  // 0 for 1-shot timer
} COM_TIMER_PARAM, *PCOM_TIMER_PARAM;

typedef struct __attribute__((packed))
{
    COM_TIMER_FUNC  timeoutFunc;    // NULL = available
    void*  param;                  // Callback parameter
    COM_TIMER_TICK  startTick;
    COM_TIMER_TICK  duration;  // 0 for 1-shot timer
    BOOL			recurringFlag;  // 0 for 1-shot timer
    BOOL            activeFlag;
} COM_TIMER_ARRAY;

typedef struct __attribute__((packed))
{
    COM_TIMER_ARRAY comTimer[COM_TIMER_COUNT];
    COM_TIMER_ID    noOfTimersUsed; // Largest Timer ID
} COM_TIMER_STRUCT;

//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

// Timer Setup
void COMTimer_Init( void );
COM_TIMER_ID COMTimer_Create(PCOM_TIMER_PARAM timerParam);
void COMTimer_Delete(const COM_TIMER_ID id);
COM_TIMER_TICK  COMTimer_Get10MsecTick ( void );
void COMTimer_Start(COM_TIMER_ID id, COM_TIMER_TICK tickDuration);
void COMTimer_Stop(COM_TIMER_ID id);
BOOL COMTimer_IsRunning(COM_TIMER_ID id);
uint32_t COMTimer_StartTimerMsec();
BOOL COMTimer_CheckTimerExpiry(uint32_t startTime,uint32_t duration);
void COMtimerISR_TimerHandler ( void );


//============================================================================
//                              > INLINE FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------

#endif

