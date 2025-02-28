//****************************************************************************
//*
//*
//* File: ErrorHandler.c
//*
//* Synopsis: Error Handler
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <EthernetTask.h>
#include <OnChipEEPROM.h>
#include <SNMPAgent.h>
#include <UPSInterface.h>
#include "ErrorHandler.h"
#include "SPIFlashTask.h"

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
//								> LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------

ERROR_STRUCT ErrorStruct;

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================


//****************************************************************************
//*
//* Function name: Error_DisableWatchdogReset
//*
//* Return: None
//*
//*
//* Description:
//*  This function disables Watchdog to reset controller
//*
//****************************************************************************

void Error_DisableWatchdogReset(void)
{
    //
    // Enable reset generation from the watchdog timer.
    //

	if(ErrorStruct.watchdogInit==TRUE)
    {
    	MAP_WatchdogResetDisable(WATCHDOG0_BASE);
	}
}

//****************************************************************************
//*
//* Function name: Error_EnableWatchdogReset
//*
//* Return: None
//*
//*
//* Description:
//*  This function enables Watchdog to reset controller
//*
//****************************************************************************

void Error_EnableWatchdogReset(void)
{
    //
    // Enable reset generation from the watchdog timer.
    //
	if(ErrorStruct.watchdogInit==TRUE)
    {
    	MAP_WatchdogResetEnable(WATCHDOG0_BASE);
	}
}

//****************************************************************************
//*
//* Function name: Error_GetRegistersFromStack
//*
//* Return: None
//*
//*
//* Description:
//*   gets registers from stack pointer.
//*
//****************************************************************************
//==============================================================================

void Error_GetRegistersFromStack(uint32_t *pulFaultStackAddress,uint16_t faultCode)
{
	uint8_t counter;
	uint16_t CRC_Cal=0;
	uint8_t *Buffer =(uint8_t*) &(Device_Configurations.EepromMapStruct.faultRecordStruct);

	 
	 if(Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionCounter >= 6)
	 {
		counter = 0;
		Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionCounter=1;
	 }
	 else
	 {
	 	counter = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionCounter;
	 	Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionCounter++;
	 }


     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Year=GlobalDateTime.Year;
     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Month=GlobalDateTime.Month;
     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Date=GlobalDateTime.Date;
     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Hours=GlobalDateTime.Hours;
     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Minutes=GlobalDateTime.Minutes;
     Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].timeStamp.Seconds=GlobalDateTime.Seconds;
	 Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].OSVersion=Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version;
	 Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].BootVersion=Device_Configurations.EepromMapStruct.Save_SystemInfo.BOOTVersion;
	 Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].reserved=0;
	 Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].faultCode=faultCode;
	 Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].faultAddress=pulFaultStackAddress[6];
	 memcpy((uint8_t*)Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[counter].stackPtr,(uint8_t*)pulFaultStackAddress,(sizeof(uint32_t)*64));

	 CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, (sizeof(FAULT_STRUCT)-2));
	 Device_Configurations.EepromMapStruct.faultRecordStruct.crc = CRC_Cal;
	 
	 OnChipEEPROM_Write(FAULT_RECORD_SAVE, (uint8_t *)&Device_Configurations.EepromMapStruct.faultRecordStruct, sizeof(FAULT_STRUCT));

	 ErrorStruct.FeedWatchdog=FALSE;
	 Error_EnableWatchdogReset();
	 
#if defined(PART_TM4C1292NCPDT)
	MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_2,GPIO_PIN_2);
#elif defined(PART_TM4C1294NCPDT)
	MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1,GPIO_PIN_1);
#endif  

	 while(1)
	 {
	 	// wait for watchdog to reset microcontroller
	 }	
}

//==============================================================================


//****************************************************************************
//*
//* Function name: Error_HardFault
//*
//* Return: None
//*
//*
//* Description:
//*  This is being called when Application has un recoverable error.
//*
//****************************************************************************
//==============================================================================
void Error_MajorFault(uint16_t faultCode)
{
	uint8_t counter;
	uint16_t CRC_Cal=0;
	uint8_t *Buffer =(uint8_t*) &(Device_Configurations.EepromMapStruct.faultRecordStruct);

	Error_DisableWatchdogReset();

	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;
	HWREG(NVIC_DIS2) = 0xffffffff;
	HWREG(NVIC_DIS3) = 0xffffffff;
	HWREG(NVIC_DIS4) = 0xffffffff;

	vTaskSuspendAll();
	vTaskEndScheduler();

	//MAP_IntMasterDisable(); 

	if(Device_Configurations.EepromMapStruct.faultRecordStruct.hardfaultCounter >= 10)
	{
		counter = 0;
		Device_Configurations.EepromMapStruct.faultRecordStruct.hardfaultCounter=1;
	}
	else
	{
		counter = Device_Configurations.EepromMapStruct.faultRecordStruct.hardfaultCounter;
		Device_Configurations.EepromMapStruct.faultRecordStruct.hardfaultCounter++;
	}

    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Year=GlobalDateTime.Year;
    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Month=GlobalDateTime.Month;
    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Date=GlobalDateTime.Date;
    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Hours=GlobalDateTime.Hours;
    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Minutes=GlobalDateTime.Minutes;
    Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].timeStamp.Seconds=GlobalDateTime.Seconds;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].OSVersion= Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].BootVersion=Device_Configurations.EepromMapStruct.Save_SystemInfo.BOOTVersion;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].reserved1=0;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].reserved2=0;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].reserved3=0;
	Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[counter].faultCode=faultCode;

	CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, (sizeof(FAULT_STRUCT)-2));
	Device_Configurations.EepromMapStruct.faultRecordStruct.crc = CRC_Cal;

	OnChipEEPROM_Write(FAULT_RECORD_SAVE, (uint8_t *)&Device_Configurations.EepromMapStruct.faultRecordStruct, sizeof(FAULT_STRUCT));

	ErrorStruct.FeedWatchdog=FALSE;
	Error_EnableWatchdogReset();
//	MAP_IntMasterEnable();  

#if defined(PART_TM4C1292NCPDT)
	MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_2,GPIO_PIN_2);
#elif defined(PART_TM4C1294NCPDT)
	MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1,GPIO_PIN_1);
#endif   


	while(1)
	{
		// wait for watchdog to reset microcontroller
	}		

}


//****************************************************************************
//*
//* Function name: Error_TimerInit
//*
//* Return: void
//*
//*
//* Description:
//*  Timer to monitor tasks 
//*
//****************************************************************************
void Error_TimerInit( void )
{

	 //
	 // Initialize the interrupt counter.
	 //
	 ErrorStruct.errTimerTick = 0;
	


    // Set up HAL
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	// Wait for the UART module to be ready.
	while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
	{
	}

	//
	  // Enable processor interrupts.
	//
	
	MAP_IntMasterEnable();	
	MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, SYSTEM_CLOCK/1000);

	// 120000000

	 //
	 // Enable the Timer0B interrupt on the processor (NVIC).
	 //
	 MAP_IntEnable(INT_TIMER1A);

	 //
	 // Configure the Timer0B interrupt for timer timeout.
	 //
	 MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	
	 //
	 // Enable Timer0B.
	 //
	 MAP_TimerEnable(TIMER1_BASE, TIMER_A);
	
}


//==============================================================================

//****************************************************************************
//*
//* Function name: Error_FaultISR
//*
//* Return: None
//*
//*
//* Description:
//*  This is being called when Application has un recoverable error.
//* This is the code that gets called when the processor receives a fault
//* interrupt.  This simply enters an infinite loop, preserving the system state
//* for examination by a debugger.
//*
//*****************************************************************************
void Error_FaultISR(void)
{
	uint32_t *pulFaultStackAddress;

	pulFaultStackAddress=(uint32_t *)HardFault_Handler();

	Error_DisableWatchdogReset();

    HWREG(NVIC_DIS0) = 0xffffffff;
    HWREG(NVIC_DIS1) = 0xffffffff;
    HWREG(NVIC_DIS2) = 0xffffffff;
    HWREG(NVIC_DIS3) = 0xffffffff;
    HWREG(NVIC_DIS4) = 0xffffffff;

	vTaskSuspendAll();
	vTaskEndScheduler();


	Error_GetRegistersFromStack(pulFaultStackAddress,ERR_EXCEPTION_FAULT);

/*    __asm volatile
    (   " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " bl Error_GetRegistersFromStack1    \n"
    );*/
}

//****************************************************************************
//*
//* Function name: Error_MPUFaultISR
//*
//* Return: None
//*
//*
//* Description:
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
void Error_MPUFaultISR(void)
{
	uint32_t *pulFaultStackAddress;

	Error_DisableWatchdogReset();

	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;
	HWREG(NVIC_DIS2) = 0xffffffff;
	HWREG(NVIC_DIS3) = 0xffffffff;
	HWREG(NVIC_DIS4) = 0xffffffff;

	vTaskSuspendAll();
	vTaskEndScheduler();

	pulFaultStackAddress=(uint32_t *)HardFault_Handler();
	Error_GetRegistersFromStack(pulFaultStackAddress,ERR_MPU_FAULT);

	/*	 __asm volatile
	(   " tst lr, #4 											   \n"
	   " ite eq 												   \n"
	   " mrseq r0, msp											   \n"
	   " mrsne r0, psp											   \n"
	   " ldr r1, [r0, #24]										   \n"
	   " bl Error_GetRegistersFromStack    \n"
	);*/
}


//****************************************************************************
//*
//* Function name: Error_BUSFaultISR
//*
//* Return: None
//*
//*
//* Description:
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
void Error_BUSFaultISR(void)
{
	uint32_t *pulFaultStackAddress;

	Error_DisableWatchdogReset();

	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;
	HWREG(NVIC_DIS2) = 0xffffffff;
	HWREG(NVIC_DIS3) = 0xffffffff;
	HWREG(NVIC_DIS4) = 0xffffffff;

	vTaskSuspendAll();
	vTaskEndScheduler();

	pulFaultStackAddress=(uint32_t *)HardFault_Handler();
	Error_GetRegistersFromStack(pulFaultStackAddress,ERR_BUS_FAULT);

/*	  __asm volatile
	(	" tst lr, #4												\n"
		" ite eq													\n"
		" mrseq r0, msp 											\n"
		" mrsne r0, psp 											\n"
		" ldr r1, [r0, #24] 										\n"
		" bl Error_GetRegistersFromStack	\n"
	);*/

}


//****************************************************************************
//*
//* Function name: Error_USAGEFaultISR
//*
//* Return: None
//*
//*
//* Description:
// This is the code that gets called when the processor receives a fault
// interrupt.  This simply enters an infinite loop, preserving the system state
// for examination by a debugger.
//
//*****************************************************************************
void Error_USAGEFaultISR(void)
{
	uint32_t *pulFaultStackAddress;

	Error_DisableWatchdogReset();

	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;
	HWREG(NVIC_DIS2) = 0xffffffff;
	HWREG(NVIC_DIS3) = 0xffffffff;
	HWREG(NVIC_DIS4) = 0xffffffff;

	vTaskSuspendAll();
	vTaskEndScheduler();

	pulFaultStackAddress=(uint32_t *)HardFault_Handler();
	Error_GetRegistersFromStack(pulFaultStackAddress,ERR_USAGE_FAULT);

/*	  __asm volatile
	(	" tst lr, #4												\n"
		" ite eq													\n"
		" mrseq r0, msp 											\n"
		" mrsne r0, psp 											\n"
		" ldr r1, [r0, #24] 										\n"
		" bl Error_GetRegistersFromStack	\n"
	);*/
}


//****************************************************************************
//*
//* Function name: Error_WatchdogIntISR
//*
//* Return: None
//*
//*
//* Description:
//*  Watch dog ISR.  Kicks the Watchdog and toggle  LED.
//*
//****************************************************************************
//==============================================================================

void Error_WatchdogIntISR(void)
{
   // Error_KickWatchdog();
   // UARTprintf("I am in Watchdog ISR!\r\n");
    //
    // If we have been told to stop blinking the LED, return immediately
    // without blinking the interrupt.
    //
    if(ErrorStruct.FeedWatchdog == FALSE)
    {
        //
        // Invert the GPIO PN0 value.
        //
#if defined(PART_TM4C1292NCPDT)
		MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_2,GPIO_PIN_2);
#elif defined(PART_TM4C1294NCPDT)
		MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1,GPIO_PIN_1);
#endif        

	}
	else
	{
	    //
	    // Clear the watchdog interrupt.
	    //
	    MAP_WatchdogIntClear(WATCHDOG0_BASE);

	    //
	    // Invert the GPIO value.
	    //
#if defined(PART_TM4C1292NCPDT)	    
		MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_2,
							 (MAP_GPIOPinRead(GPIO_PORTP_BASE, GPIO_PIN_2) ^
											 GPIO_PIN_2));
#elif defined(PART_TM4C1294NCPDT)
		MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1,
						 (MAP_GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) ^
											 GPIO_PIN_1));
#endif  

	}   	
}

//****************************************************************************
//*
//* Function name: Error_TimerHandlerISR
//*
//* Return: None
//*
//*
//* Description:
//*  Timer 1 ISR.
//*
//****************************************************************************
//==============================================================================
void Error_TimerHandlerISR ( void )
{
	uint16_t faultCode=ERR_NO_FAULT;

	MAP_IntMasterDisable();	
    //
    // Clear the timer interrupt flag.
    //
    MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	if(ErrorStruct.tasksInit == FALSE)
	{
		MAP_IntMasterEnable();	
		return;
	}
  	if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable == 1)
  	{
		ErrorStruct.SNMPTaskCounter++;
	}
	else
	{
		ErrorStruct.SNMPTaskCounter = 0;
	}
	
  	if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable == 1)
  	{
		ErrorStruct.ModbusTCPTaskCounter++;
	}
	else
	{
		ErrorStruct.ModbusTCPTaskCounter = 0;
	}
	if(spiflashFileSystemStruct.SPIFlashMount == 1)
    {
        ErrorStruct.SPIFlashFSTaskCounter++;
    }
    else
    {
        ErrorStruct.SPIFlashFSTaskCounter = 0;
    }
    if(Device_Configurations.EepromMapStruct.Save_GSMSettings.ConfigFlags.bits.GSMSMS_Enabled == 1)
    {
        ErrorStruct.GSMSTaskCounter++;
    }
    else
    {
        ErrorStruct.GSMSTaskCounter=0;
    }
    ErrorStruct.errTimerTick++;
	ErrorStruct.USBAppTaskCounter++;
	ErrorStruct.SysTaskCounter++;
	//ErrorStruct.WebserTaskCounter++;
	ErrorStruct.EthernetTaskCounter++;

	if(ErrorStruct.ModbusTCPTaskCounter > MODBUS_TCP_TASK_LIMIT)
	{
		faultCode = MODBUS_TCP_TASK_FAULT;
	}
	else if(ErrorStruct.SysTaskCounter > SYS_TASK_LIMIT)
	{
		faultCode = SYS_TASK_FAULT;
	}
	else if(ErrorStruct.EthernetTaskCounter > ETHERNET_TASK_LIMIT)
	{
		faultCode = ETHERNET_TASK_FAULT;
	}
	else if(ErrorStruct.USBAppTaskCounter > USB_APP_TASK_LIMIT)
	{
		faultCode = USB_APP_TASK_FAULT;
	}
	else if (ErrorStruct.SNMPTaskCounter > SNMP_TASK_LIMIT)
	{
	    faultCode =SNMP_TASK_FAULT;
	}
	else if (ErrorStruct.GSMSTaskCounter > GSM_TASK_LIMIT)
    {
        faultCode =GSM_COMM_FAULT;
    }
	else if (ErrorStruct.SPIFlashFSTaskCounter > SPI_FS_TASK_LIMIT)
	    {
	        faultCode =SPI_FS_TASK_FAULT;
	    }
	else
	{
		faultCode = ERR_NO_FAULT;
	}
	if(faultCode != ERR_NO_FAULT)
	{
		Error_MajorFault(faultCode);
	}
 	MAP_IntMasterEnable();	
}
//****************************************************************************
//*
//* Function name: Error_ResetTaskWatchdog
//*
//* Return: void
//*
//*
//* Description:
//*  Resets Task watchdog counter.
//*
//****************************************************************************

void Error_ResetTaskWatchdog(uint8_t taskID)
{

	switch(taskID)
	{
		case MODBUS_TCP_TASK_ID:
			ErrorStruct.ModbusTCPTaskCounter = 0;
			break;
		case SNMP_TASK_ID:
			ErrorStruct.SNMPTaskCounter = 0;
			break;		
		case WEBSERVER_TASK_ID:
			ErrorStruct.WebserTaskCounter = 0;
			break;	
		case SYS_TASK_ID:
			ErrorStruct.SysTaskCounter = 0;
			break;
		case UPS_SERIAL_TASK_ID:
			ErrorStruct.DriveSPITaskCounter = 0;
			break;			
		case ETHERNET_TASK_ID:
			ErrorStruct.EthernetTaskCounter = 0;
			break;
		case USB_APP_TASK_ID:
			ErrorStruct.USBAppTaskCounter = 0;
			break;
		case SPI_FLASH_FS_TASK_ID:
		    ErrorStruct.SPIFlashFSTaskCounter = 0;
            break;
		case GSM_TASK_ID:
		     ErrorStruct.GSMSTaskCounter = 0;
		     break;
	}
	
}

//****************************************************************************
//*
//* Function name: Error_InitWatchdog
//*
//* Return: uint32_t as Watchdof timeout (1 sec)
//*
//*
//* Description:
//*  Watch dog ISR.
//*
//****************************************************************************

void Error_InitWatchdog(uint32_t l_clock)
{

	ErrorStruct.FeedWatchdog=TRUE;
	ErrorStruct.ModbusTCPTaskCounter = 0;
	ErrorStruct.SNMPTaskCounter=0;
//	ErrorStruct.BacNetTaskCounter = 0;
	ErrorStruct.WebserTaskCounter = 0;
	ErrorStruct.SysTaskCounter = 0;
	ErrorStruct.DriveSPITaskCounter = 0;
	ErrorStruct.EthernetTaskCounter = 0;
	ErrorStruct.USBAppTaskCounter = 0;
	ErrorStruct.watchdogInit=FALSE;
	Error_stopTaskMonitoring();

	Error_TimerInit();

	/*MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
	//
	// Set GPIO PN0 as an output.  This drives an LED on the board that will
	// toggle when a watchdog interrupt is processed.
	//
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0);
	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_1);
	MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_0, 0);
	MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1, 0);*/

	// Enable the peripherals used by this example.
	//
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	//
	// Enable processor interrupts.
	//
	MAP_IntMasterEnable();

	//
	// Enable the watchdog interrupt.
	//
	MAP_IntEnable(INT_WATCHDOG);

	//
	// Set the period of the watchdog timer to 1 second.
	//
	MAP_WatchdogReloadSet(WATCHDOG0_BASE, l_clock);

	//
	// Enable reset generation from the watchdog timer.
	//
	MAP_WatchdogResetEnable(WATCHDOG0_BASE);

	//
	// Enable the watchdog timer.
	//
	MAP_WatchdogEnable(WATCHDOG0_BASE);
	
	ErrorStruct.watchdogInit=TRUE;
}

//****************************************************************************
//*
//* Function name: Error_InitWatchdog
//*
//* Return: uint32_t as Watchdof timeout (1 sec)
//*
//*
//* Description:
//*  Watch dog ISR.
//*
//****************************************************************************

void Error_startTaskMonitoring()
{
	ErrorStruct.tasksInit=TRUE;
}

//****************************************************************************
//*
//* Function name: Error_InitWatchdog
//*
//* Return: uint32_t as Watchdof timeout (1 sec)
//*
//*
//* Description:
//*  Watch dog ISR.
//*
//****************************************************************************

void Error_stopTaskMonitoring()
{
	ErrorStruct.tasksInit=FALSE;
}

//==============================================================================
// END OF SYSTask.c FIlE
//==============================================================================


