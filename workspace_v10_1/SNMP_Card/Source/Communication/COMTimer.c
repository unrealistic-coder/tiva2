//****************************************************************************
//*
//*
//* File: COMTimer.c
//*
//* Synopsis: Communication Timer Library
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <COMTimer.h>

//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL ENUMERATED TYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL STRUCTURES <
//----------------------------------------------------------------------------


//============================================================================
//                              > LOCAL FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------
static COM_TIMER_STRUCT comTimerStruct;
static volatile COM_TIMER_TICK comTimerTick=0;

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------

//****************************************************************************
//*
//* Function name: COMTimer_Get10MsecTick
//*
//* Return:   returns COM_TIMER_TICK
//*
//* Description: returns timer tick
//*
//****************************************************************************

COM_TIMER_TICK  COMTimer_Get10MsecTick ( void )
{
    return comTimerTick;
}

//****************************************************************************
//*
//* Function name: COMTimer_IsExpired
//*
//* Return:   Success (TRUE) or Fail (FALSE)
//*
//* Description: checks timer expiry and returns True if expired
//*
//****************************************************************************

static BOOL COMTimer_IsExpired(COM_TIMER_ID id)
{
    // Just check if CurrentTick is within Tolerance zone.

    COM_TIMER_TICK currentTick=0;
	COM_TIMER_TICK nextTick=0;
	uint64_t totalTick=0;
	BOOL status=FALSE;

	if ( (id >= comTimerStruct.noOfTimersUsed))
        return status; // Invalid ID

	
	totalTick = comTimerStruct.comTimer[id].startTick + comTimerStruct.comTimer[id].duration;
	
	currentTick=COMTimer_Get10MsecTick();

	if(totalTick >= COM_TICK_MAX)
	{
	    if ( currentTick <= comTimerStruct.comTimer[id].startTick)
	    {   
	    	
	    	// rollover detected
	    	nextTick = (COM_TIMER_TICK) (totalTick - COM_TICK_MAX);
	        if (currentTick >= nextTick )
	            status= TRUE;

	    } 
	}
	else
	{
	    // Check if trigger tick is within tolerance. Tolerance for timer latency (late ISR)
	    // Upper limit: tick time + tolerance
	    // Lower limit: tick time
	    // Current time must be within Upper and Lower

	    // Check rollover
	    if ( currentTick >= comTimerStruct.comTimer[id].startTick)
	    {   
	    	// no rollover
	    	nextTick = (COM_TIMER_TICK) (comTimerStruct.comTimer[id].startTick + 
	    								comTimerStruct.comTimer[id].duration);
	        if (currentTick >= nextTick)
	            status= TRUE;

	    } 
		else  // if ( upper >= COM_TICK_MAX )
	    {   
	    	// rollover detected
	    	nextTick = (COM_TIMER_TICK) (comTimerStruct.comTimer[id].duration -
	    								( COM_TICK_MAX - comTimerStruct.comTimer[id].startTick));
	    	
	        if (currentTick >= nextTick)
	            status= TRUE;

	    }
	}

    // Else
    return status;

}
//****************************************************************************
//*
//* Function name: COMtimerISR_TimerHandler
//*
//* Return:   None
//*
//* Description: Timer ISR. Call timeout functions on expiry 
//*
//****************************************************************************

void COMtimerISR_TimerHandler ( void )
{
    COM_TIMER_ID    i;
    COM_TIMER_ARRAY *timerParam;
	
	MAP_IntMasterDisable();	

    //
    // Clear the timer interrupt flag.
    //
    MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  	comTimerTick++;

    // Service expired timeout
    for ( i=0 ; i < comTimerStruct.noOfTimersUsed ; i++ )
    {
        if ( comTimerStruct.comTimer[i].activeFlag == FALSE )
            continue;   // Inactive timer

        timerParam = &comTimerStruct.comTimer[i];

        if ( COMTimer_IsExpired (i))
        {   

            if ( timerParam->recurringFlag )
            {
			    // Register next trigger tick
			    timerParam->activeFlag = TRUE;
				timerParam->startTick = COMTimer_Get10MsecTick();
            } 
			else
            {   // To stop this timer
			    timerParam->activeFlag = FALSE;
				timerParam->startTick = 0;
				timerParam->duration=0;
            }

			// Trigger
            // ===============================================
            // Call Timer Handler
            if(timerParam->timeoutFunc != NULL)
            {
            	(*timerParam->timeoutFunc) (i, timerParam->param);
            }
            // ===============================================
        }

    }
 	MAP_IntMasterEnable();	
}

//****************************************************************************
//*
//* Function name: COMTimer_Init
//*
//* Return:   None
//*
//* Description: Initialises HW timer and COM_TIMER_STRUCT structure 
//*
//****************************************************************************

void COMTimer_Init( void )
{
    COM_TIMER_ID    i;

    // Init variables
    memset ( comTimerStruct.comTimer, 0, sizeof(comTimerStruct.comTimer) );
    for ( i=0 ; i<COM_TIMER_COUNT ; i++ )
    {
		// Release timer
		comTimerStruct.comTimer[i].timeoutFunc = NULL;
		comTimerStruct.comTimer[i].startTick = 0;  // Set timer to inactive
		comTimerStruct.comTimer[i].param = 0;
		comTimerStruct.comTimer[i].recurringFlag = 0;
		comTimerStruct.comTimer[i].activeFlag = FALSE;
		comTimerStruct.comTimer[i].duration = 0;
    }

    comTimerStruct.noOfTimersUsed = 0;

	 //
	 // Initialize the interrupt counter.
	 //
	 comTimerTick = 0;
	


    // Set up HAL
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	// Wait for the UART module to be ready.
	while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
	{
	}

	//
	  // Enable processor interrupts.
	//
	
	MAP_IntMasterEnable();	
	MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, SYSTEM_CLOCK/100);

	//SYSTEM_CLOCK/100 = 1200000 --> 10 msec
	// 120000 for 1 msec--> 1200000 (120000 * 10) for 10 msec 

	// 120000000

	 //
	 // Enable the Timer0B interrupt on the processor (NVIC).
	 //
	 MAP_IntEnable(INT_TIMER0A);

	 //
	 // Configure the Timer0B interrupt for timer timeout.
	 //
	 MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	
	 //
	 // Enable Timer0B.
	 //
	 MAP_TimerEnable(TIMER0_BASE, TIMER_A);

}

//****************************************************************************
//*
//* Function name: COMTimer_Create
//*
//* Return:   None
//*
//* Description: Creats new timer and assigns its callback function
//*
//****************************************************************************

COM_TIMER_ID COMTimer_Create(PCOM_TIMER_PARAM timerParam)
{
    COM_TIMER_ID    i, id = COM_TIMER_ID_INVALID;


	MAP_IntMasterDisable(); 

    // Find available timer slot
    for ( i=0 ; i<COM_TIMER_COUNT ; i++ )
    {
        if ( comTimerStruct.comTimer[i].timeoutFunc == NULL )
        {   // Found
            // Copy parameters
            comTimerStruct.comTimer[i].timeoutFunc    = timerParam->timeoutFunc;
            comTimerStruct.comTimer[i].param          = timerParam->param;
            comTimerStruct.comTimer[i].recurringFlag  = timerParam->recurringFlag;
            comTimerStruct.comTimer[i].activeFlag   = FALSE;   // Create disabled
	         // Success
            id = i;
			comTimerStruct.noOfTimersUsed++;
            break;
        }
    }
	MAP_IntMasterEnable(); 

    return id;
}

//****************************************************************************
//*
//* Function name: COMTimer_Delete
//*
//* Return:   None
//*
//* Description: Deletes Timer
//*
//****************************************************************************

void COMTimer_Delete(const COM_TIMER_ID id)
{
    if ( (id >= comTimerStruct.noOfTimersUsed))
        return; // Invalid ID

	MAP_IntMasterDisable();	

    // Release timer
    comTimerStruct.comTimer[id].timeoutFunc = NULL;
    comTimerStruct.comTimer[id].startTick = 0;  // Set timer to inactive
    comTimerStruct.comTimer[id].param = 0;
    comTimerStruct.comTimer[id].recurringFlag = 0;
    comTimerStruct.comTimer[id].activeFlag = FALSE;
	comTimerStruct.comTimer[id].duration = 0;
    // Set to 0 if none found.
    comTimerStruct.noOfTimersUsed--;
	
	MAP_IntMasterEnable();	
}

//****************************************************************************
//*
//* Function name: COMTimer_Start
//*
//* Return:   None
//*
//* Description: Starts Timer with parameters passed
//*
//****************************************************************************

void COMTimer_Start(COM_TIMER_ID id, COM_TIMER_TICK tickDuration)
{
  	COM_TIMER_TICK currentTick=0;

    if ( (id >= comTimerStruct.noOfTimersUsed))
        return; // Invalid ID

	MAP_IntMasterDisable();	
	currentTick=COMTimer_Get10MsecTick();

    // Register next trigger tick
    comTimerStruct.comTimer[id].activeFlag = TRUE;
	comTimerStruct.comTimer[id].startTick = currentTick;
	comTimerStruct.comTimer[id].duration = tickDuration;

	MAP_IntMasterEnable();	

}

//****************************************************************************
//*
//* Function name: COMTimer_Stop
//*
//* Return:   None
//*
//* Description: Stops Timer
//*
//****************************************************************************

void COMTimer_Stop(COM_TIMER_ID id)
{
    if ( (id >= comTimerStruct.noOfTimersUsed))
        return; // Invalid ID

	MAP_IntMasterDisable();	

    // Disable timer
    comTimerStruct.comTimer[id].startTick = 0;  // Set timer to inactive
    comTimerStruct.comTimer[id].activeFlag = FALSE;
	comTimerStruct.comTimer[id].duration = 0;
	
	MAP_IntMasterEnable();	

}

//****************************************************************************
//*
//* Function name: COMTimer_IsRunning
//*
//* Return:  Timer Running (True) not running (False)
//*
//* Description: Returns True if timer is running
//*
//****************************************************************************

BOOL COMTimer_IsRunning(COM_TIMER_ID id)
{
	BOOL status=FALSE;
	MAP_IntMasterDisable(); 

    if ( (id < comTimerStruct.noOfTimersUsed))
    {    	
    	status= comTimerStruct.comTimer[id].activeFlag; // Invalid ID
    }
	MAP_IntMasterEnable();	

	COMTIMER_printf("comTimerStruct.comTimer[id].activeFlag %d %d\n",comTimerStruct.comTimer[id].activeFlag, id );
	return status;

}

//****************************************************************************
//*
//* Function name: COMTimer_StartTimerMsec
//*
//* Return:   returns timer tick
//*
//* Description: Returns timer tick to start new timer
//*
//****************************************************************************

uint32_t COMTimer_StartTimerMsec()
{
	uint32_t startTime=COMTimer_Get10MsecTick();

	return startTime;
}

//****************************************************************************
//*
//* Function name: COMTimer_CheckTimerExpiry
//*
//* Return:   Returns (True) if expired or (False)
//*
//* Description: Returns (True) if expired or (False)
//*
//****************************************************************************

BOOL COMTimer_CheckTimerExpiry(uint32_t startTime,uint32_t duration)
{
	
    COM_TIMER_TICK currentTick=0;
	COM_TIMER_TICK nextTick=0;
	uint64_t totalTick=0;
	BOOL status=FALSE;

	
	totalTick = startTime + duration;
	
	currentTick=COMTimer_Get10MsecTick();

	if(totalTick >= COM_TICK_MAX)
	{
	    if ( currentTick <= startTime)
	    {   
	    	// rollover detected
	    	nextTick = (COM_TIMER_TICK)(totalTick - COM_TICK_MAX);
	        if (currentTick >= nextTick )
	            status= TRUE;
	    } 
	}
	else
	{
	    // Check if trigger tick is within tolerance. Tolerance for timer latency (late ISR)
	    // Upper limit: tick time + tolerance
	    // Lower limit: tick time
	    // Current time must be within Upper and Lower

	    // Check rollover
	    if ( currentTick >= startTime)
	    {   
	    	// no rollover
	    	nextTick = (COM_TIMER_TICK) (startTime + duration);
	        if (currentTick >= nextTick)
	            status= TRUE;

	    } 
		else  // if ( upper >= COM_TICK_MAX )
	    {   
	    	// rollover detected
	    	nextTick = (COM_TIMER_TICK) (duration -	( COM_TICK_MAX - startTime));
	    	
	        if (currentTick >= nextTick)
	            status= TRUE;

	    }
	}
	return status;
}


// #define TIMER_PROFILING



