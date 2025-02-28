//****************************************************************************
//*
//*
//* File: main.c
//*
//* Synopsis: main function program execution starts from this file.
//*			 
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <EthernetTask.h>
#include <SerialDriver.h>
#include <OnChipEEPROM.h>
#include "ModbusTCPServer.h"
#include "SYSTask.h"
#include <ErrorHandler.h>
#include <SPIExtRTC.h>
#include <SNMPAgent.h>
#include <UPSInterface.h>
#include "USBDeviceProtocol.h"
#include "SPI_Flash.h"
#include "fatfs.h"
#include "ff.h"
#include "diskio.h"
#include "SPIFlashTask.h"
#include "HibernateCalendar.h"
#include "FTPServer.h"
#include "websocket.h"
#include "GSM_Comm_Task.h"
//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------
uint32_t output_clock_rate_hz;
uint8_t stackOverflowCntr=0;
char FileName[30];
uint32_t ui32_TReset=0;
////============================================================================
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
//                              > LOCAL VARIABLES <
//----------------------------------------------------------------------------

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

//****************************************************************************
//*
//* Function name: main
//*
//* Return: return 0
//*
//*
//* Description:
//*   main function starts all inits and tasks of all modules and schedule all
//*   task using schedular.
//*
//****************************************************************************
//==============================================================================

int main(void){

    output_clock_rate_hz = MAP_SysCtlClockFreqSet(
                               (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240),
                               SYSTEM_CLOCK);
    ASSERT(output_clock_rate_hz == SYSTEM_CLOCK);

    ui32_TReset=output_clock_rate_hz*3*.0000002; // 200 ns delay

    //eNABLE STACKING FOR INTERRUPT HANDLER. tHIS ALLOWS FLOATING-POINTS
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage
    MAP_FPUEnable();
    MAP_FPULazyStackingEnable();
    SYSTask_PeripheralEnable();

#if !defined(DEBUG)//PART_TM4C1292NCPDT and PART_TM4C1294NCPDT
   Error_InitWatchdog(output_clock_rate_hz);
#endif

    COMTimer_Init();
	SYSTask_TimerInit();
	OnChipEEPROM_Init();
	USBDevice_init();
	UPSInterface_Init();
	Ethernet_init();
	ModbusTCPServer_Init();
	SNMPAgent_Init();
    FTPServer_SocketInit();
    SPIFlashFileSystem_Init();

    WS_Init(&WebSocketStruct);

    #if defined WEBSERVER_USING_SPI_FLASH
        HttpConnServer_Init(&HttpServerStruct);
    #endif

    RTC_Init();

    xTaskCreate(SPIFlashFileSystem_Task, "SPI Flash File System Task",3072, NULL,1, NULL); //Http_conn

	xTaskCreate(Ethernet_Task, "Ethernet Task", 1024, NULL, 1, NULL);

    xTaskCreate(SYS_Task, "SYS Task", 1024, NULL, 1, NULL);

//	if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable == 1)
	{
		xTaskCreate(SNMPAgent_Task, "SNMP Task", 2048, NULL, 2, NULL);
	}

//	if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable == 1)
	{
		xTaskCreate(ModbusTCPServer_Task, "Modbus TCP Task", 1024, NULL, 2, NULL);
	}
	
	xTaskCreate(USBDevice_Task, "USB Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

//	if (Device_Configurations.EepromMapStruct.Save_GSMSettings.ConfigFlags.bits.GSMSMS_Enabled == 1)
	{
	 xTaskCreate(GSM_Task,"GSM GPRS Task", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	}


#if !defined(DEBUG)//PART_TM4C1292NCPDT and PART_TM4C1294NCPDT

	Error_startTaskMonitoring();

#endif

     vTaskStartScheduler();

	while(1)
	{
		// Code should never reach this point
		//Not dnding lOOP
	}
   
    return 0;//ignore error
}
//============================================================================
//****************************************************************************
//*
//* Function name: vApplicationStackOverflowHook
//*
//* Return: return 0
//*
//*
//* Description:
//*   Application hook for stack overflow.
//*
//****************************************************************************
//==============================================================================
void vApplicationStackOverflowHook( TaskHandle_t xTask,	char * pcTaskName )
{
	stackOverflowCntr++;
	while(1);
}
//============================================================================
//****************************************************************************
//*
//* Function name: __error__
//*
//* Return: return 0
//*
//*
//* Description:
//*  Assert Errors with filename and line number
//*
//****************************************************************************
//==============================================================================

void __error__(char *pcFilename, uint32_t ui32Line)
{
    // Place a breakpoint here to capture errors until logging routine is finished
    Error_DisableWatchdogReset();
    strcpy(FileName, &pcFilename[0]);
    while (1)
    {
    }
}
//==============================================================================
// END OF main.c FIlE
//==============================================================================

