//****************************************************************************
//*
//*
//* File: GSM.c
//*
//* Synopsis: UPS Interface serial communication implementation
//*
//*
//****************************************************************************
//============================================================================
//								> INCLUDE FILES <
//----------------------------------------------------------------------------
#include "GSM_Comm.h"
#include "stdio.h"
#include "string.h"
#include "ErrorHandler.h"
#include <UPSInterface.h>
bool GsmMsg_TextMode();

typedef union __attribute__((packed))
{
	Time			ThisTime;
	uint8_t 		buffer[1];
} ModbusBuff;

ModbusBuff SMSbuff;

static volatile REG_uint64_t CurrentFaults, CurrentStatus;
uint64_t		currentfaults,currentstatus,PreviousAlarmsGSM, PreviousStatusGSM;
char			SendDatabuff[45];
uint8_t 		Mains_Fail_Flag = 0,Mains_OK_SMS=0;

BOOL SetModemBaudRate(uint32_t UartBaudRate)
{
	BOOL STATUS=FALSE;
	uint32_t ModemBaudRate=0;
	char GSMbaudStr[20]={'0'};
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	while(1)
	{
	    Error_ResetTaskWatchdog(GSM_TASK_ID);
		MX_UART2_Init(9600);
		if((GsmCommon_GSMcommand("AT+IPR=9600\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			ModemBaudRate=9600;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GSMBaudRateSet=ModemBaudRate;
			break;
		}
		MX_UART2_Init(19200);
		if((GsmCommon_GSMcommand("AT+IPR=19200\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			ModemBaudRate=19200;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GSMBaudRateSet=ModemBaudRate;
			break;
		}

		MX_UART2_Init(38400);
		if((GsmCommon_GSMcommand("AT+IPR=38400\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			ModemBaudRate=38400;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GSMBaudRateSet=ModemBaudRate;
			break;
		}
		MX_UART2_Init(57600);
		if((GsmCommon_GSMcommand("AT+IPR=57600\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			ModemBaudRate=57600;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GSMBaudRateSet=ModemBaudRate;
			break;
		}
		MX_UART2_Init(115200);
		if((GsmCommon_GSMcommand("AT+IPR=115200\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			ModemBaudRate=115200;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GSMBaudRateSet=ModemBaudRate;
			break;
		}
		if(ModemBaudRate==0)
		{
		    MX_UART2_Init(19200);
			pGsmStruct->GsmDiag.ModemState=DEFAULT_BAUD_RATE_SET;
			pGsmStruct->GSMBaudRateSet=19200;
			pGsmStruct->GsmDiag.ModemConnected = FALSE;
		    return STATUS;

		}
	    vTaskDelay(10);

	}
	if(UartBaudRate!=ModemBaudRate)
	{
		if(UartBaudRate==9600)
		{
			sprintf(GSMbaudStr, "AT+IPR=%s\r\n", "9600");
		}
		else if(UartBaudRate==19200)
		{
			sprintf(GSMbaudStr, "AT+IPR=%s\r\n", "19200");
		}
		else if(UartBaudRate==38400)
		{
			sprintf(GSMbaudStr, "AT+IPR=%s\r\n", "38400");
		}
		else if(UartBaudRate==57600)
		{
			sprintf(GSMbaudStr, "AT+IPR=%s\r\n", "57600");
		}
		else if(UartBaudRate==115200)
		{
			sprintf(GSMbaudStr, "AT+IPR=%s\r\n", "115200");
		}
		if((GsmCommon_GSMcommand(GSMbaudStr, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			MX_UART2_Init(UartBaudRate);
			ModemBaudRate=UartBaudRate;
			pGsmStruct->GSMBaudRateSet=UartBaudRate;
			STATUS= TRUE;
			pGsmStruct->GsmDiag.ModemState=SYSTEM_BAUD_RATE_SET;

		}
		else
		{
			return STATUS;
		}
	}
	else
	{
		STATUS= TRUE;
	}
	if(ModemBaudRate!=0 && STATUS==TRUE)
	{
		if(GSMModemDiagnosticsAtPowerUp(UartBaudRate,ModemBaudRate)==TRUE)
		{
			pGsmStruct->GsmDiag.ModemState=MODEM_BAUD_RATE_SET;
			STATUS= TRUE;
		}
	}

	return STATUS;
}
//****************************************************************************
//*
//* Function name: GSM_Modem_Init
//*
//* Return:   None
//*
//* Description:
//* Init of UART5 and set Modem baud rate
//****************************************************************************
void GSM_Modem_Init(uint32_t ui32Base, uint32_t BaudRate)
{
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	if(SetModemBaudRate(BaudRate)==TRUE)
	{
		pGsmStruct->GsmDiag.ModemConnected = TRUE;
	}
	else
	{
		pGsmStruct->GsmDiag.ModemConnected = FALSE;
	}
}

void SendGSMmodemTestcommand(uint8_t Port)
{
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;
	PreviousAlarmsGSM	= UPSDb.warningFlags.warningAll;
	PreviousStatusGSM	= UPSDb.upsStatus.statusAll;

	memcpy(& (SMSbuff.ThisTime), (void *) &GlobalDateTime, sizeof(GlobalDateTime));

	if(pGsmStruct->GsmDiag.ModemConnected == TRUE)
	{
		currentfaults		= PreviousAlarmsGSM;
		currentstatus		= PreviousStatusGSM;
	}
}

BOOL GSMModemDiagnosticsAtPowerUp(uint32_t systemsetBaud, uint32_t ModemBaudRate)
{
	uint8_t Status		= FALSE;
	GSM_RAM_STRUCT * pGsmStruct = &GsmStruct;

	if ((GsmCommon_GSMcommand("ATE0\r\n", 1000, NULL, 0,6, "\r\nOK\r\n", "\r\nERROR\r\n")) ==1)
	{
		pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
		pGsmStruct->GsmDiag.ModemState			= ATE0_CMD_RESP_OK;
		Status		= TRUE;
	}
	else
	{
		pGsmStruct->GsmDiag.ModemState =STATUS_ERROR;
		Status		= FALSE;
	}
	if ((GsmCommon_GSMcommand("AT\r\n", 1000, NULL, 0, 6, "\r\nOK\r\n", "\r\nERROR\r\n")) ==1)
	{
			pGsmStruct->GsmDiag.framesRecv =(uint32_t)((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			pGsmStruct->GsmDiag.ModemState			= AT_CMD_RESP_OK;
			Status		= TRUE;
	}
	else
	{
		pGsmStruct->GsmDiag.ModemState =STATUS_ERROR;
		Status		= FALSE;
	}
	if (GsmCommon_GSMcommand("AT+CMGF=1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1)
	    {
	    	pGsmStruct->status.textMode = 1;
	    	pGsmStruct->GsmDiag.ModemState	= MODEM_CONFIG_IN_TEXT_MODE;
	    	Status		= TRUE;
	    }
	else
		{
			pGsmStruct->GsmDiag.ModemState =STATUS_ERROR;
			Status		= FALSE;
		}

	if(Status==FALSE && ModemBaudRate!=0)
	{
		if(ModemBaudRate==9600)
		{
			sprintf((char *) pGsmStruct->txBuffer, "AT+IPR=%s\r\n", "9600");
		}
		else if(ModemBaudRate==19200)
		{
			sprintf((char *) pGsmStruct->txBuffer, "AT+IPR=%s\r\n", "19200");
		}
		else if(ModemBaudRate==38400)
		{
			sprintf((char *) pGsmStruct->txBuffer, "AT+IPR=%s\r\n", "38400");
		}
		else if(ModemBaudRate==57600)
		{
			sprintf((char *) pGsmStruct->txBuffer, "AT+IPR=%s\r\n", "57600");
		}
		else if(ModemBaudRate==115200)
		{
			sprintf((char *) pGsmStruct->txBuffer, "AT+IPR=%s\r\n", "115200");
		}
		MX_UART2_Init(ModemBaudRate);
		if((GsmCommon_GSMcommand((char *)pGsmStruct->txBuffer, 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{

			Status	= TRUE;
			pGsmStruct->GsmDiag.framesRecv =(uint32_t) ((pGsmStruct->GsmDiag.framesRecv + 1) & 0xFFFFFFFF);
			//SystemVars.Save_GsmSettings.baudrate=ModemBaudRate;
		}
		else
		{
			return Status;
		}
	}
	return Status;
}

//****************************************************************************
//*
//* Function name: GsmMsg_SendMsg
//*
//* Return:   Success (True) or Failure (False)
//*
//* Description:
//*   This function sends SMS.
//*   
//****************************************************************************

static bool GsmMsg_SendMsg(GSM_RAM_STRUCT *pGsmStruct,char *number, char *msg)
{ 
	static int16_t ModemChkConnCnt=0;
  if ((number == NULL) || (msg == NULL))
  {
    return false;
  }

  if (pGsmStruct->status.textMode)
  {
    char str[32];
    sprintf(str, "AT+CMGS=\"%s\"\r\n", number);
    if (GsmCommon_GSMcommand(str, 5000, NULL, 0, 2, "\r\r\n> ", "\r\nERROR\r\n") != 1)
    {
      sprintf(str, "%c", 27);

      GsmCommon_GSMcommand(str, 1000, NULL, 0, 0,"","");

		if((GsmCommon_GSMcommand("AT\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n"))==1)
		{
			 ModemChkConnCnt=0;
		}
		else
		{
			ModemChkConnCnt++;
			if(ModemChkConnCnt>=5)
			{
				ModemChkConnCnt=0;
				 pGsmStruct->GsmDiag.ModemConnected=0;
			}
		}
      return false;
    }
    pGsmStruct->GsmDiag.ModemState=SENDING_SMS_INFO;
    ModemChkConnCnt=0;
    if (GsmCommon_GSMcommand((char*)msg, 80000 , NULL, 0, 2, "\r\n+CMGS:", "\r\nERROR\r\n") != 1)
    {

     pGsmStruct->GsmDiag.TransactionFailedCnt =(uint32_t) ((pGsmStruct->GsmDiag.TransactionFailedCnt + 1) & 0xFFFFFFFF);
     pGsmStruct->GsmDiag.NumberofSMSNotsent =(uint32_t) ((pGsmStruct->GsmDiag.NumberofSMSNotsent + 1) & 0xFFFFFFFF);
      return false;
    }
    ModemChkConnCnt=0;
    pGsmStruct->GsmDiag.NumberofSMSsent =(uint32_t) ((pGsmStruct->GsmDiag.NumberofSMSsent + 1) & 0xFFFFFFFF);
	
    return true;
  }
  else
  {
	  pGsmStruct->GsmDiag.ModemConnected=0;
	  pGsmStruct->GsmDiag.ModemState=STATUS_CONN_CLOSED;
     return false;
  }
}

//****************************************************************************
//*
//* Function name: GsmMsg_BuildMsg
//*
//* Return:   Success (True) or Failure (False)
//*
//* Description:
//*   This function builds SMS.
//*   
//****************************************************************************

static BOOL GsmMsg_BuildMsg(GSM_RAM_STRUCT *pGsmStruct,char *SMSContent)
{

	uint16_t StringLen = 0;
	uint8_t status=TRUE;
	uint16_t jax=0;
	REG_uint64_t VerfiyMobilNO;
	char MobilNumber[13]={0};

	
	StringLen = strlen(SMSContent);
	sprintf(&SMSContent[StringLen],"%c",26);//26=ctrl-z
	for(jax=0; jax<5; jax++)
	{
		//VerfiyMobilNO.all =SystemVars.Save_GsmSettings.CONFIG_MobilNumbers[jax].all;
	
		if(((VerfiyMobilNO.byte[5] == 0) || (VerfiyMobilNO.byte[5] == 0x91))
			&&((VerfiyMobilNO.byte[4] >= 0x60) && (VerfiyMobilNO.byte[4] <= 0x99)))
		{
			pGsmStruct->GsmDiag.ModemState=SENDING_MOBILE_NUM;
			if(VerfiyMobilNO.byte[5] == 0)
				sprintf(&MobilNumber[0], "%x%08x", VerfiyMobilNO.dword[1],VerfiyMobilNO.dword[0]);
			else
				sprintf(&MobilNumber[0], "+%x%08x", VerfiyMobilNO.dword[1],VerfiyMobilNO.dword[0]);
			if((GsmMsg_SendMsg(pGsmStruct,MobilNumber, SMSContent) != TRUE))
			{
				pGsmStruct->GsmDiag.NumberofSMSNotsent =(uint32_t) ((pGsmStruct->GsmDiag.NumberofSMSNotsent + 1) & 0xFFFFFFFF);
				status=FALSE;
				break;
			}

			/*else
			{
				memset(SMSContent, 0x00, 155);
			}*/
		}
		else
		{
			pGsmStruct->GsmDiag.ModemState=MOBILE_NUM_NOT_VALID;
		}
	}

	return status;

}

//****************************************************************************
//*
//* Function name: GsmMsg_SendPendingMsg
//*
//* Return:   None
//*
//* Description:
//*   This function sends pending SMS it.
//*   
//****************************************************************************
static void GsmMsg_SendPendingMsg(GSM_RAM_STRUCT *pGsmStruct)
{
	char SMSContent[155]={0};
	uint16_t StringLen = 0;
	BOOL  Status=FALSE ;
	StringLen = strlen(SMSContent);
    char system[]={"this is test"};
	sprintf(&SMSContent[StringLen], "Serial No: %s\r\n",system );
	StringLen = strlen(SMSContent);
	sprintf(&SMSContent[StringLen], "%02d-%02d-20%02d %02d:%02d:%02d\r\n", SMSbuff.ThisTime.Date, SMSbuff.ThisTime.Month,
	SMSbuff.ThisTime.Year, SMSbuff.ThisTime.Hours, SMSbuff.ThisTime.Minutes, SMSbuff.ThisTime.Seconds);

	StringLen = strlen(SMSContent);

	if (UPSDb.warningFlags.Bits.BattLow)
	{
		sprintf((char *) &SMSContent[StringLen], "Batt Low Alarm\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}
	if (UPSDb.warningFlags.Bits.ParallelINV_VoltDiff)
	{
		sprintf((char *) &SMSContent[StringLen], "Inverter Trip\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}

	if (UPSDb.warningFlags.Bits.BattLow || UPSDb.warningFlags.Bits.BatteryOpen || UPSDb.warningFlags.Bits.BattOverCharge
	        || UPSDb.warningFlags.Bits.BattOverCharge || UPSDb.warningFlags.Bits.BattCellOverCharge)
	{
		sprintf((char *) &SMSContent[StringLen], "Battery Cell Alarm\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;

	}
	if (UPSDb.warningFlags.Bits.BattLow) //added on 12/05/2016
	{
		Mains_Fail_Flag 	= 1;
		sprintf((char *) &SMSContent[StringLen], "Mains Fail\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}
	if (Mains_OK_SMS && Mains_Fail_Flag)
	{
		sprintf((char *) &SMSContent[StringLen], "Mains OK\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}
	if (UPSDb.warningFlags.Bits.INVCurrentUnbalance)
	{
		sprintf((char *) &SMSContent[StringLen], "INV Curr Alarm\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}

	if (UPSDb.warningFlags.Bits.BypassUnstable)
	{
		sprintf((char *) &SMSContent[StringLen], "Bypass Unstable\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}
	if (UPSDb.warningFlags.Bits.ChargerFail)
	{
		sprintf((char *) &SMSContent[StringLen], "Charger Fail\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}
	if (Status == FALSE)
	{
		sprintf((char *) &SMSContent[StringLen], "No Alarm\r\n");
		StringLen			= strlen((const char *)SMSContent);
		Status				= TRUE;
	}

	if(GsmMsg_BuildMsg(pGsmStruct, SMSContent))
	{
		 pGsmStruct->GsmDiag.TransctionmPassCnt =(uint32_t) ((pGsmStruct->GsmDiag.TransctionmPassCnt + 1) & 0xFFFFFFFF);
	}
	StringLen = 0;
	memset(SMSContent, 0x00, 155);
}



//****************************************************************************
//*
//* Function name: GsmMsg_ProcessPendingMsg
//*
//* Return:   None
//*
//* Description:
//*   This function process pending SMS and sends it.
//*   
//****************************************************************************
void GsmMsg_ProcessPendingMsg (void)
{
	GSM_RAM_STRUCT *pGsmStruct=&GsmStruct;
	GsmMsg_SendPendingMsg(pGsmStruct);
}

//****************************************************************************
//*
//* Function name: GsmMsg_TextMode
//*
//* Return:   Success (True) or Failure (False)
//*
//* Description:
//*   This function configures modem in Text Mode.
//*   
//****************************************************************************

bool GsmMsg_TextMode()
{
	GSM_RAM_STRUCT *pGsmStruct=&GsmStruct;

    if (GsmCommon_GSMcommand("AT+CMGF=1\r\n", 1000, NULL, 0, 2, "\r\nOK\r\n", "\r\nERROR\r\n") == 1)
    {
    	pGsmStruct->status.textMode = 1;
     
      return true;
    }
  
  return false;
}


//****************************************************************************
//*
//* Function name: GSM_Perform_IO
//*
//* Return:   None
//*
//* Description:
//* This function send commands to modem and receive response from modem
//****************************************************************************
void GSM_Perform_IO(uint8_t Port)
{
	if(((PreviousAlarmsGSM & 0xFFFFFFF) != (CurrentFaults.all & 0xFFFFFFF)) || ((PreviousStatusGSM & 0xFFFFFFF) != (CurrentStatus.all & 0xFFFFFFF)))
	{
		CurrentFaults.all= PreviousAlarmsGSM;
		CurrentStatus.all= PreviousStatusGSM;
		GsmMsg_ProcessPendingMsg();
	}
}
void GSMRxCallback(uint8_t character)
{
	GSM_RAM_STRUCT *pGsmStruct= &GsmStruct;

	COMTimer_Start(pGsmStruct->rxInterFrameTimer,20);

	  pGsmStruct->GsmDiag.charRecv=(uint32_t)((pGsmStruct->GsmDiag.charRecv + 1) & 0xFFFFFFFF);

        if(pGsmStruct->rxIndex < (GSM_ATC_RXSIZE - 1))
        {
            pGsmStruct->rxBuffer[pGsmStruct->rxIndex] = pGsmStruct->recvbyte;
            pGsmStruct->rxIndex++;
        }

}

//==============================================================================
// END OF FILE
//==============================================================================
