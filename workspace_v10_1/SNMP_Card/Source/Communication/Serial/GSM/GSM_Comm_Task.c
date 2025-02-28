/*
 * GSM_Comm_Task.c
 *
 *  Created on: Oct 26, 2023
 *      Author: Rani Sonawane
 */
#include <string.h>
#include <stdio.h>
#include "ErrorHandler.h"
#include "GSM_Comm.h"
#include <string.h>
#include <stdio.h>
#include <UPSInterface.h>
#include <OnChipEEPROM.h>
#include <SMTPManager.h>
GSM_RAM_STRUCT   GsmStruct;
uint16_t		Prev_BaudRateGSM = 0;

//============================================================================
//								> STRUCTURES and TYPEDEFS <
//--------------------------------------------------------------------------x)



//============================================================================
//============================================================================
//								> FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//****************************************************************************
//*
//* Function name: void MX_UART5_Init(uint32_t BaudRate)
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void MX_UART2_Init(uint32_t baudrate)
{
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

    // Enable peripheral as per base address of UART1
     MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);
     // Wait for the UART module to be ready.
     while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UART2))
     {
     }

     MAP_GPIOPinConfigure(GPIO_PA6_U2RX);
     MAP_GPIOPinConfigure(GPIO_PA7_U2TX);
     MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);


     MAP_UARTConfigSetExpClk(UART2_BASE, SYSTEM_CLOCK, baudrate,
                              (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                               UART_CONFIG_PAR_NONE));

     MAP_UARTFIFODisable(UART2_BASE);


    MAP_IntMasterEnable();

    MAP_IntEnable(INT_UART2);
    UPS_UartRxIntEnable (PORT_UART2);
  //  MAP_UARTIntEnable(UART2_BASE, UART_INT_RX);
    UPS_UartTxIntDisable(PORT_UART2);

}
//============================================================================
//****************************************************************************
//*
//* Function name: void GSM_CommTimeoutInit()
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void GSM_CommTimeoutInit()
{
	COM_TIMER_PARAM comTimerParam;
	GSM_RAM_STRUCT *pGsmStruct = &GsmStruct;
	comTimerParam.param = (void *)pGsmStruct;
	comTimerParam.timeoutFunc = GsmCommon_Timeout_RxInterFrame;
	comTimerParam.recurringFlag = 0;
	pGsmStruct->rxInterFrameTimer = COMTimer_Create(&comTimerParam);

	if (COM_TIMER_ID_INVALID != pGsmStruct->rxInterFrameTimer)
		{
		}
	comTimerParam.param = (void *)pGsmStruct;
	comTimerParam.timeoutFunc = GsmCommon_Transaction_Timeout;
	comTimerParam.recurringFlag = 0;
	pGsmStruct->TransactionTimout = COMTimer_Create(&comTimerParam);

	if (COM_TIMER_ID_INVALID != pGsmStruct->TransactionTimout)
		{

		}
}
//============================================================================
//****************************************************************************
//*
//* Function name: void GSM_CommTimeoutInit()
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void InitGSM_Parameters()
{
	GSM_RAM_STRUCT *pGsmStruct = &GsmStruct;
	pGsmStruct->GsmDiag.ModemConnected =FALSE;
	memset(pGsmStruct->rxBuffer, 0, GSM_ATC_RXSIZE);
	memset(pGsmStruct->txBuffer, 0, MAX_GSM_BUFFER_SIZE*2);
	pGsmStruct->rxIndex = 0;
	pGsmStruct->txRemainingSize= 0;
	pGsmStruct->requestState = STATE_GSM_REQUEST_IDLE;
	pGsmStruct->rxState = STATE_GSM_RX_IDLE;
	pGsmStruct->txState = STATE_GSM_TX_IDLE;

}
//============================================================================
//****************************************************************************
//*
//* Function name: void GsmCommon_Timeout_RxInterFrame(COM_TIMER_ID id, void * param)
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void GsmCommon_Timeout_RxInterFrame(COM_TIMER_ID id, void * param)
{
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	if (pGsmStruct->rxIndex > 0)
	{
		pGsmStruct->rxState = STATE_GSM_RX_PROCESSING;

	}

}
//============================================================================
//****************************************************************************
//*
//* Function name: void GsmCommon_Transaction_Timeout(COM_TIMER_ID id, void * param)
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//==========================================================================
void GsmCommon_Transaction_Timeout(COM_TIMER_ID id, void * param)
{
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;
	pGsmStruct->requestState = STATE_GSM_REQUEST_IDLE;
	pGsmStruct->rxState = STATE_GSM_RX_IDLE;
	pGsmStruct->txState = STATE_GSM_TX_IDLE;
	COMTimer_Stop(pGsmStruct->TransactionTimout);
	COMTimer_Stop(pGsmStruct->rxInterFrameTimer);
	pGsmStruct->GsmDiag.ModemState=STATUS_TIMEOUT;
	pGsmStruct->GsmDiag.TransactionFailedCnt = (uint32_t) ((pGsmStruct->GsmDiag.TransactionFailedCnt + 1) & 0xFFFFFFFF);
}
//============================================================================
//****************************************************************************
//*
//* Function name: void GSM_Task(void * Parameters)
//*
//* Return:
//*
//* Description:
//*
//****************************************************************************
//==========================================================================
void GSM_Task(void * Parameters)
{
   	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	GSM_CommTimeoutInit();
	InitGSM_Parameters();

    if (Device_Configurations.EepromMapStruct.Save_GSMSettings.ConfigFlags.bits.GSMSMS_Enabled)
    {
        if((Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate == 0) || (Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate > 57600))
        {
            Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate=19200;
        }
        GSM_Modem_Init(UART2_BASE, Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate);
    }

	Prev_BaudRateGSM	=  Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate;
	while (1)
	{
	    Error_ResetTaskWatchdog(GSM_TASK_ID);	//GSM_Tasks

		if(Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate != Prev_BaudRateGSM)
		{
			InitGSM_Parameters();
			UPS_UartRxIntEnable (PORT_UART2);//MAP_UARTIntEnable(UART2_BASE, UART_INT_RX);
			UPS_UartTxIntDisable (PORT_UART2); //MAP_UARTIntDisable(UART2_BASE, UART_INT_TX);
			COMTimer_Stop(pGsmStruct->TransactionTimout);
			COMTimer_Stop(pGsmStruct->rxInterFrameTimer);

			if ((Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate == 0) || (Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate> 57600))
			{
			    Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate=19200;
			}
			GSM_Modem_Init(UART2_BASE, Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate);
			Prev_BaudRateGSM = Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate;
		}
		if (Device_Configurations.EepromMapStruct.Save_GSMSettings.ConfigFlags.bits.GSMSMS_Enabled)
		{
			if(pGsmStruct->GsmDiag.ModemConnected ==1)
			{
				GSM_Perform_IO(1);
			}
			else
			{
			    GSM_Modem_Init(UART2_BASE, Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate);
			}
		}

		if (Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate != Prev_BaudRateGSM)
		{
			if ((Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate == 0) || (Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate > 57600))
			{
			    Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate=19200;
			}

			GSM_Modem_Init(UART2_BASE, Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate);

			Prev_BaudRateGSM = Device_Configurations.EepromMapStruct.Save_GSMSettings.GSMBaudRate;
		}
		//=========================================================================
	//	if ((ModemTestCmd == 1) && (GPRSMON_Disable == 1))
	//	ModemTestRoutine();
    	vTaskDelay(10);
	}

}
//============================================================================
//****************************************************************************
//*
//* Function name: void GsmCommon_TxISR(void * param)
//*
//* Return:
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void GsmCommon_TxISR(void * param)
{
	uint8_t Index=0;
	GSM_RAM_STRUCT * pGsmStruct = (GSM_RAM_STRUCT *)&GsmStruct;

	//COMTimer_Start(pGsmStruct->TransactionTimout, 100);

	 switch(pGsmStruct->txState)
	    {
	         case STATE_GSM_TX_TRANSMITTING:
            if (pGsmStruct->txRemainingSize > 0  && (pGsmStruct->txRemainingSize < 20 ))
            {
                Index=pGsmStruct->txBufferIndex++;
                UPS_UartWriteChar (pGsmStruct->txBuffer[Index],PORT_UART2);
                pGsmStruct->txRemainingSize--;
                //Char Sent. Increment link counter
                pGsmStruct->GsmDiag.charSent = (uint32_t) (( pGsmStruct->GsmDiag.charSent + 1) & 0xFFFFFFFF);    // allow rollover
            }
            else
            {
                UPS_UartTxIntDisable(PORT_UART2);
                memset(pGsmStruct->rxBuffer,0,MAX_RX_BUFFER_SIZE);
              //  pGsmStruct->rxBufferSize=0;
                pGsmStruct->txState=STATE_GSM_TX_TRANSMITTED;
                pGsmStruct->GsmDiag.frameSent = (uint32_t) ((pGsmStruct->GsmDiag.frameSent + 1) & 0xFFFFFFFF);

            }

	             break;
	    }
//    while (pGsmStruct->txRemainingSize > 0)
//		{
//        Error_ResetTaskWatchdog(GSM_TASK_ID);   //GSM_Tasks
//
//			pGsmStruct->txState = STATE_GSM_TX_TRANSMITTING;
//			//MAP_UARTIntEnable(UART2_BASE, UART_INT_TX);
//		//	UPS_UartTxIntEnable(PORT_UART2);
//		//	UPS_UartWriteChar(pGsmStruct->txBuffer[dataByte],PORT_UART2);
//			//MAP_UARTIntDisable(UART2_BASE, UART_INT_TX);
//          //  UPS_UartWriteChar (pGsmStruct->txBuffer[dataByte],PORT_UART2);
//            pGsmStruct->txRemainingSize--;
//            dataByte++;
//            vTaskDelay(1);
//		//	HAL_UART_Transmit_IT(&huart5, pGsmStruct->txBuffer, pGsmStruct->txRemainingSize);
//
//		}
//	 UPS_UartTxIntDisable (PORT_UART2); //MAP_UARTIntDisable(UART2_BASE, UART_INT_TX);
//        pGsmStruct->txBufferIndex = 0;
//        pGsmStruct->txRemainingSize = 0;
//        memset(pGsmStruct->txBuffer, 0, (MAX_GSM_BUFFER_SIZE * 2));


}
//============================================================================
//****************************************************************************
//*
//* Function name: void GsmCommon_SendCommand(GSM_RAM_STRUCT * pGsmStruct, uint8_t * data, uint16_t len)
//*
//* Return:
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void GsmCommon_SendCommand(GSM_RAM_STRUCT * pGsmStruct, uint8_t * data, uint16_t len)
{
	uint16_t		i	= 0;

	if ((len > 0) && (len <= 512))
		{
		for (i = 0; i < len; i++)
			{
			pGsmStruct->txBuffer[i] = data[i];
			}

		pGsmStruct->txBufferIndex = 0;
		pGsmStruct->txRemainingSize = len;
		pGsmStruct->txState = STATE_GSM_TX_TRANSMITTING;
		GsmCommon_TxISR(pGsmStruct);
		}
}
//============================================================================
//****************************************************************************
//*
//* Function name: void GsmCommon_Found(char * found_str)
//*
//* Return:
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void GsmCommon_Found(char * found_str)
{
	char *			str;
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	str = strstr(found_str, "POWER DOWN\r\n");

	if (str != NULL)
		{
		pGsmStruct->status.power = 0;
		return;
		}

	str = strstr(found_str, "\r\n+CREG: ");

	if (str != NULL)
		{
			int16_t 		p1	= -1, p2 = -1;

			sscanf(str, "\r\n+CREG: %hd,%hd", &p1, &p2);

			if (p2 == 1)
				{
				pGsmStruct->status.netReg = 1;
				pGsmStruct->status.netChange = 1;
				return;
				}
			else
				{
				pGsmStruct->status.netReg = 0;
				pGsmStruct->status.netChange = 1;
				return;
				}
		}
}
//============================================================================
//****************************************************************************
//*
//* Function name: char * GsmCommon_SearchResponse(GSM_RAM_STRUCT * pGsmStruct, uint8_t * foundIndex,
//*                       char * str1, char * str2)
//*
//* Return:String pointer if found
//*
//* Description:This function searches string in recieved response and sets index accordingly.
//*
//****************************************************************************
//============================================================================
char * GsmCommon_SearchResponse(GSM_RAM_STRUCT * pGsmStruct, uint8_t * foundIndex,
	char * str1, char * str2)
{
	*foundIndex 		= 1;
	char * str			= NULL;
	str 				= strstr((char *) pGsmStruct->rxBuffer, (char *) str1);

	if (str == NULL)
		{
		str 				= strstr((char *) pGsmStruct->rxBuffer, (char *) str2);

		if (str != NULL)
			{
			*foundIndex 		= 2;
			}
		}
	else
		{
		*foundIndex 		= 1;
		}

	return str;
}
//============================================================================
//****************************************************************************
//*
//* Function name: GsmCommon_GSMcommand
//*
//* Return:int8_t
//*
//* Description:This function send commands to modem
//*
//****************************************************************************
//============================================================================
int8_t GsmCommon_GSMcommand(const char * command, uint32_t timeout_ms, char * answer, uint16_t answer_size, int items, char * str1, char * str2)
{
	uint8_t foundIndex	= 0;
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;
	if (answer != NULL)
		memset(answer, 0, answer_size);

	if (pGsmStruct->txState == STATE_GSM_TX_IDLE)
	{

		GsmCommon_SendCommand(pGsmStruct, (uint8_t *) command, strlen(command));
		pGsmStruct->requestState = STATE_GSM_REQUEST_WAIT_RSP;
		COMTimer_Start(pGsmStruct->TransactionTimout, timeout_ms);
		while (pGsmStruct->requestState == STATE_GSM_REQUEST_WAIT_RSP)
		{

		   	Error_ResetTaskWatchdog(GSM_TASK_ID);	//GSM_Tasks
		    vTaskDelay(10);
			if (pGsmStruct->rxState == STATE_GSM_RX_PROCESSING)
			{
			    vTaskDelay(10);
				char * found=GsmCommon_SearchResponse(pGsmStruct, &foundIndex, (char *) str1, (char *) str2);

				if (found != NULL && answer != NULL)
				{
					strncpy(answer, found, answer_size);
				}
				memset(pGsmStruct->rxBuffer, 0, GSM_ATC_RXSIZE);
				memset(pGsmStruct->txBuffer, 0, MAX_GSM_BUFFER_SIZE*2);
				pGsmStruct->rxIndex = 0;
				pGsmStruct->txRemainingSize=0;
				pGsmStruct->rxState = STATE_GSM_RX_IDLE;
				pGsmStruct->requestState = STATE_GSM_REQUEST_IDLE;
				pGsmStruct->txState = STATE_GSM_TX_IDLE;
				if(foundIndex==2)
				{
					pGsmStruct->GsmDiag.ModemState=STATUS_ERROR;
				}
				if (found != NULL)
				{
					break;
				}
			}
		}

	}
	
	return foundIndex;

	if (answer != NULL)
		memset(answer, 0, answer_size);

	if (pGsmStruct->txState == STATE_GSM_TX_IDLE)
	{
		COMTimer_Start(pGsmStruct->TransactionTimout, timeout_ms);
		GsmCommon_SendCommand(pGsmStruct, (uint8_t *) command, strlen(command)); //=======1
		pGsmStruct->requestState = STATE_GSM_REQUEST_WAIT_RSP;

		while (pGsmStruct->rxState != STATE_GSM_RX_PROCESSING)
		{
			strncpy((char *)str1, pGsmStruct->rxBuffer, answer_size);
			memset(pGsmStruct->rxBuffer, 0, GSM_ATC_RXSIZE);
            memset(pGsmStruct->txBuffer, 0, MAX_GSM_BUFFER_SIZE*2);
			pGsmStruct->rxIndex = 0;
			pGsmStruct->rxState = STATE_GSM_RX_IDLE;
			if(str1 !=NULL)
			{
	  			foundIndex=1;
			}
	  }
	}

	return foundIndex;

}

//==============================================================================

//==============================================================================

//==============================================================================
// END OF FILE
//==============================================================================


