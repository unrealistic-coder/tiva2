//****************************************************************************
//*
//*
//* File: UPSIntrface.c
//*
//* Synopsis: UPS Interface serial communication implementation
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <SerialDriver.h>
#include <ErrorHandler.h>
#include <OnChipEEPROM.h>
#include <ModbusTCPServer.h>
#include <UPSInterface.h>
#include <SYSTask.h>
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

UPS_COMMAND_LIST_STRUCT commandListStruct[MAX_UPS_INQUIRY_CMDS]=
{{"QPI\r\n",QPI_CMD_SIZE,&QPI_ProcessResponse},      //Protocol ID
 {"QMD\r\n",QMD_CMD_SIZE,&QMD_ProcessResponse},     //Model Info
 {"QMOD\r\n",QMOD_CMD_SIZE,&QMOD_ProcessResponse},    //UPS Mode
 {"QRI\r\n",QRI_CMD_SIZE,&QRI_ProcessResponse},     //UPS Rating Information
 {"QGS\r\n",QGS_CMD_SIZE,&QGS_ProcessResponse},   //UPS General Status Parameter
 {"QBYV\r\n",QBYV_CMD_SIZE,&QBYV_ProcessResponse},    //Bypass Voltage Range
 {"QBYF\r\n",QBYF_CMD_SIZE,&QBYF_ProcessResponse},    //Bypass Frequency Range
 {"QFLAG\r\n",QFLAG_CMD_SIZE,&QFLAG_ProcessResponse},   //Setting Flag Status
 {"QWS\r\n",QWS_CMD_SIZE,&QWS_ProcessResponse},     //Warning Status
 {"QFS\r\n",QFS_CMD_SIZE,&QFS_ProcessResponse},   //Fault Status
 {"QVFW\r\n",QVFW_CMD_SIZE,&QVFW_ProcessResponse},   //Main CPU FW Version
 {"QID\r\n",QID_CMD_SIZE,&QID_ProcessResponse},   //UPS ID
 {"QBV\r\n",QBV_CMD_SIZE,&QBV_ProcessResponse},   //P Battery Information
 {"QHE\r\n",QHE_CMD_SIZE,&QHE_ProcessResponse},    //High Efficiency mode voltage range
 {"QSK1\r\n",QSK1_CMD_SIZE,&QSK1_ProcessResponse},   //Output 1 Socket Status
 {"QSK2\r\n",QSK2_CMD_SIZE,&QSK2_ProcessResponse},   //Output 2 Socket Status
 {"QSKT1\r\n",QSKT1_CMD_SIZE,&QSKT1_ProcessResponse},  //Output 1 Socket release time
 {"QSKT2\r\n",QSKT2_CMD_SIZE,&QSKT2_ProcessResponse},  //Output 2 Socket release time
 {"QSR\r\n",QSR_CMD_SIZE,&QSR_ProcessResponse}  //Output 2 Socket release time
 /*{"QYF\r\n",QYF_CMD_SIZE,&QYF_ProcessResponse},    //Bypass Frequency*/
 /*{"QMF\r\n",QMF_CMD_SIZE,&QMF_ProcessResponse},     //Manufacturer Info*/
};

UPS_INTERFACE_RAM_STRUCT  UPSInterfaceStruct;
UPS_DB_STRUCT UPSDb;


uint8_t firstchar=0;
uint32_t FrameRxComp=0,loopcnt=0;
uint8_t ctrrxbuf[10],Flag_CntrlCmd=0;
//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//****************************************************************************
//*
//* Function name: UPSInterface_GetWriteRequestStatus
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
uint8_t UPSInterface_GetWriteRequestStatus(UPS_INTERFACE_RAM_STRUCT *pDIStruct)
{
    uint8_t status=WRITE_ERROR;
    uint8_t loopBreak=FALSE,LoopCounter=0;

     while((loopBreak==FALSE) && (LoopCounter <= 100))    //100*5 //500 msec waiting to response of cntrl cmds
    {
         Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);

         switch (pDIStruct->UPSWriteRequestStruct.writeState)
        {
            case STATE_UPS_WRITE_SUCCESS:
                status=WRITE_SUCCESS;
                loopBreak=TRUE;
                break;
            case STATE_UPS_WRITE_ERROR:
                status=WRITE_ERROR;
                loopBreak=TRUE;
                break;
            default:
                break;
        }

        Error_ResetTaskWatchdog( pDIStruct->UPSWriteRequestStruct.taskID);
        LoopCounter++;
        vTaskDelay(5);

    }
    FrameRxComp=1;
    loopcnt=LoopCounter;
	if(status==WRITE_SUCCESS)
	{
		pDIStruct->uartDiag.cntrlCmdPassCntr = (uint32_t) ((pDIStruct->uartDiag.cntrlCmdPassCntr + 1) & 0xFFFFFFFF);
	}
	else
	{
		pDIStruct->uartDiag.cntrlCmdFailCntr = (uint32_t) ((pDIStruct->uartDiag.cntrlCmdFailCntr + 1) & 0xFFFFFFFF);
	}
    pDIStruct->txRemainingSize=0;
    pDIStruct->UPSWriteRequestStruct.writeState = STATE_UPS_WRITE_EMPTY;
    pDIStruct->UPSWriteRequestStruct.taskID=NO_TASK_ID;
    pDIStruct->txBufferIndex = 0;
    Flag_CntrlCmd=0;
    return status;
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_XmitNextCmd
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void UPSInterface_XmitNextCmd(UPS_INTERFACE_RAM_STRUCT *pDIStruct)
{
  uint16_t cmdIndex=0;
  int8_t itr;
  if(pDIStruct->UPSWriteRequestStruct.writeState==STATE_UPS_WRITE_WAIT)
  {

     Flag_CntrlCmd=1;
     memset(pDIStruct->rxBuffer,0,MAX_RX_BUFFER_SIZE);
     pDIStruct->requestState=STATE_UPS_REQUEST_XMITTING;
     pDIStruct->txRemainingSize=pDIStruct->UPSWriteRequestStruct.ctrlcmdSize;
     pDIStruct->txBufferIndex=0;
     for(itr=0;itr<pDIStruct->txRemainingSize;itr++)
     {
        pDIStruct->txBuffer[itr]=pDIStruct->UPSWriteRequestStruct.ctrlcommand[itr];
     }
     FrameRxComp=3;
     UPS_UartTxIntEnable(PORT_UART1);
     UPSInterface_TxISR();
     pDIStruct->UPSWriteRequestStruct.writeState=STATE_UPS_WRITE_CTRL_CMD_SENT;

  }
  else
  {
      pDIStruct->currCmdIndex++;
      if(pDIStruct->currCmdIndex >= MAX_UPS_INQUIRY_CMDS)
      {
          pDIStruct->currCmdIndex=0;
      }
      cmdIndex=pDIStruct->currCmdIndex;
      pDIStruct->requestState=STATE_UPS_REQUEST_XMITTING;
      pDIStruct->txRemainingSize=pDIStruct->commandListPtr[cmdIndex].cmdSize;
      for(itr=0;itr<pDIStruct->txRemainingSize;itr++)
      {
          pDIStruct->txBuffer[itr]=pDIStruct->commandListPtr[cmdIndex].command[itr];
      }

      pDIStruct->txBufferIndex=0;
      UPS_UartTxIntEnable(PORT_UART1);
      UPSInterface_TxISR();

  }
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_InitiateWriteRequest
//*
//* Return:   uint8_t
//*
//* Description:
//*
//*
//*
//****************************************************************************
//============================================================================
uint8_t UPSInterface_InitiateWriteRequest (UPS_WRITE_REQUEST *pReqParam)
{
    uint8_t status=WRITE_ERROR;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct = &UPSInterfaceStruct;
    int8_t itr=0;

    if (pDIStruct->UPSWriteRequestStruct.writeState == STATE_UPS_WRITE_EMPTY)
    {
       for(itr=0;itr<pReqParam->ctrlcmdSize;itr++)
       {
           pDIStruct->UPSWriteRequestStruct.ctrlcommand[itr]=pReqParam->ctrlcommand[itr];
       }
       pDIStruct->UPSWriteRequestStruct.taskID=pReqParam->taskID;
       pDIStruct->UPSWriteRequestStruct.ctrlcmdSize=pReqParam->ctrlcmdSize;
       pDIStruct->rxBufferSize=0;
       FrameRxComp=2;
	   pDIStruct->UPSWriteRequestStruct.writeState=STATE_UPS_WRITE_WAIT;
	   status= UPSInterface_GetWriteRequestStatus(pDIStruct);

    }

    return status;
}
//============================================================================
//****************************************************************************
//*
//* Function name: QPI_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* UPS: (PI <NN><cr>
//* N is an integer number ranging from 0 to 9.
//* Response contains UPS Protocol ID
//****************************************************************************
//============================================================================
void QPI_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=4)))
    {
        UPSDb.protocolID=atoi((const char*)&pDIStruct->rxBuffer[2]);
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.protocolID=0;
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QMD_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//  UPS Response in Buffer: (TTTTTTTTTTTTTTT WWWWWWW KK P/P MMM NNN RR BB.B <cr>
//  0-->  ( (DIDNT INCLUDE IN BUFFER)
//  0-->  TTTTTTTTTTTTTTT
//  16--> WWWWWWW
//  24--> KK
//  27--> P/P
//  31--> MMM
//  35--> NNN
//  39--> RR
//  42--> BB.B
//  <cr>
//****************************************************************************
//============================================================================
void QMD_ProcessResponse(void *ptr)
{
    uint16_t itr=0,itr1=0,itr2=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t paramIndex=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=46)))
    {

        for(itr1=0;itr1<15;itr1++)
        {
            if(pDIStruct->rxBuffer[itr1] != '#')
            {
                UPSDb.UPSModel[itr2++]=pDIStruct->rxBuffer[itr1];
            }
        }

        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if(pDIStruct->rxBuffer[itr]==SPACE_CHAR)
            {
                paramIndex++;
                switch(paramIndex)
                {
                    case 0:// will never come here
                        for(itr1=0;itr1<15;itr1++)
                        {
                            if(pDIStruct->rxBuffer[itr1] != '#')
                            {
                                UPSDb.UPSModel[itr2++]=pDIStruct->rxBuffer[itr1];
                            }
                        }
                        break;
                    case 1:
                        for(itr1=16;itr1<23;itr1++)
                        {
                            if(pDIStruct->rxBuffer[itr1] != '#')
                            {
                                UPSDb.RatedOutputVA=atoi((const char*)&pDIStruct->rxBuffer[itr1]);
                                break;
                            }
                        }
                        break;
                    case 2:
                        UPSDb.OutputPWRFactor=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 3:
                        UPSDb.supportedPhase.Bytes.InputPhase=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        UPSDb.supportedPhase.Bytes.OutputPhase=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 4:
                        UPSDb.NominalInputVolt=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 5:
                        UPSDb.NominalOutputVolt=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 6:
                        UPSDb.BattPieceNumber=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 7:
                        UPSDb.BattVoltPerUnit=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QMF_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer:QMF<cr>
//* Device: (MMMMMMMMMMMMMMMMMMMMMMMMMM<cr>
//* The whole length of manufacturer name is 26 ASCII characters, if the manufacturer name less than 26 ASCII character, please enter ‘#’ before the UPS manufacturer name.

//* For example:
//* Computer: QMF<cr>
//* Device: (###########<cr>
//****************************************************************************
//============================================================================
void QMF_ProcessResponse(void *ptr)
{
    uint16_t itr=0,itr1=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=26)))
    {
        for(itr=0;itr<26;itr++)
        {
            if(pDIStruct->rxBuffer[itr] != '#')
            {
                UPSDb.UPSManufacturer[itr1++]=pDIStruct->rxBuffer[itr];
            }
        }
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        memset(UPSDb.UPSManufacturer,0,26);
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QMOD_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* UPS: (M<cr>
//*
//****************************************************************************
//============================================================================
void QMOD_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=2)))
    {
        UPSDb.UPSMode[0]=pDIStruct->rxBuffer[0];
        UPSDb.UPSMode[1]=pDIStruct->rxBuffer[1];
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.UPSMode[0]=0;
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QRI_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer: QRI<cr>
//* UPS: (MMM.M QQQ SSS.S RR.R<cr>
//* This function makes the UPS answer the rating value of UPS. There should be a
//* space character between every field for separation. The UPS‘s response contains
//* the following information field:
//* a. Rating Output Voltage : MMM.M
//* b. Rating Output Current : QQQ
//* c. Battery Voltage: SSS.S.
//* d. Rating Output Frequency : RR.R
//****************************************************************************
//============================================================================
void QRI_ProcessResponse(void *ptr)
{
    uint16_t itr=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t paramIndex=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=20)))
    {
        UPSDb.RatedOuputVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);

        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if(pDIStruct->rxBuffer[itr]==SPACE_CHAR)
            {
                paramIndex++;
                switch(paramIndex)
                {
                    case 0:// will never come here
                        UPSDb.RatedOuputVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);
                        break;
                    case 1:
                        UPSDb.RatedOuputCurr=atoi((const char*)&pDIStruct->rxBuffer[itr+1]);
                        break;
                    case 2:
                        UPSDb.RatedBattVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 3:
                        UPSDb.RatedOuputFreq=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QGS_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*Computer:QGS<cr>
//*UPS: (MMM.M HH.H LLL.L NN.N QQQ.Q DDD KKK.K VVV.V SSS.S XXX.X TTT.T
//*b9b8b7b6b5b4b3b2b1b0<cr>
//*Data Description Notes
//*a ( Start byte
//*b MMM.M Input voltage M is an Integer number 0 to 9. The units is V.
//*c HH.H Input frequency H is an Integer number 0 to 9. The units is Hz.
//*d LLL.L Output voltage L is an Integer number 0 to 9. The units is V.
//*e NN.N Output frequency N is an Integer number from 0 to 9. The units is Hz.
//*g QQQ.Q Output current Q is an Integer number from 0 to 9. The units is A.
//*h DDD Output load percent For Off-line UPS: DDD is a percent of maximum VA,
//*not an absolute value.
//*For On-line UPS: DDD is Maximum of W% or VA%.
//*VA% is a percent of maximum VA.
//*W% is a percent of maximum real power.
//*j KKK.K Positive BUS voltage K is an Integer ranging from 0 to 9. The units is V.
//*k VVV.V Negative BUS voltage V is an Integer ranging from 0 to 9. The units is V.
//*l SSS.S P Battery voltage S is an Integer ranging from 0 to 9. The units is V.
//*m XXX.X N Battery voltage X is an Integer ranging from 0 to 9. The units is V.
//*n TTT.T Max Temperature of the
//*detecting pointers
//*T is an integer ranging from 0 to 9. The units is °C
//*o b9b8b7b6
//*b5b4b3b2
//*b1b0a0a1
//*Ups status B9,b8:
//*00: standy;
//*01: line-interactive;
//*10: on-line.
//*B7: Utility Fail
//*b6: Battery Low
//*b5: Bypass/Boost Active
//*b4: UPS Failed
//*b3: EPO
//*b2: Test in Progress
//*b1: Shutdown Active
//*b0: bat silence
//*a0: Bat test fail
//*a1: Bat test OK
//****************************************************************************
//============================================================================
void QGS_ProcessResponse(void *ptr)
{
    uint16_t itr=0,itr1=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t paramIndex=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=75)))
    {
        UPSDb.InputVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);

        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if(pDIStruct->rxBuffer[itr]==SPACE_CHAR)
            {
                paramIndex++;
                switch(paramIndex)
                {
                    case 0:
                        UPSDb.InputVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);
                        break;
                    case 1:
                        UPSDb.InputFreq=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 2:
                        UPSDb.OutputVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 3:
                        UPSDb.OutputFreq=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 4:
                        UPSDb.OutputCurr=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 5:
                        UPSDb.OutputLoadPercent=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    case 6:
                        UPSDb.PBusVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 7:
                        UPSDb.NBusVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 8:
                        UPSDb.PBattVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 9:
                        UPSDb.NBattVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 10:
                        UPSDb.MaxTemp=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 11:
                        UPSDb.upsStatus.statusAll=0;
                        for(itr1=0;itr1<12;itr1++)
                        {
                            if(pDIStruct->rxBuffer[itr+1+itr1]==0x31)
                            {
                                UPSDb.upsStatus.statusAll |= (1 << (15-itr1));
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }

}
//============================================================================
//****************************************************************************
//*
//* Function name: QBYV_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer: QBYV<cr>
//* UPS: (HHH LLL <cr>
//* Data Description Notes
//* a ( Start byte
//* b HHH Voltage high loss point H is an Integer number 0 to 9. The unit is V.
//* c LLL Voltage low loss point L is an Integer number 0 to 9. The unit is V.
//* The bypass voltage rang from 176 to 264, default 176V, the precision is 1 volt.
//****************************************************************************
//============================================================================
void QBYV_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=7)))
    {
        UPSDb.bypassVoltHighLossPt=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));
        UPSDb.bypassVoltLowLossPt=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[4]));
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.bypassVoltHighLossPt=0;
        UPSDb.bypassVoltLowLossPt=0;
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QBYF_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer: QBYF<cr>
//* UPS: (HH.H LL.L <cr>
//* Data Description Notes
//* a ( Start byte
//* b HH.H Freq high loss point H is an Integer number 0 to 9. The unit is Hz.
//* c LL.L Freq low loss point L is an Integer number 0 to 9. The unit is Hz.
//* The bypass frequency rang from 40.0 to 49.0, default 46.0Hz, the precision is 0.1Hz.
//*
//****************************************************************************
//============================================================================
void QBYF_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=9)))
    {
        UPSDb.freqHighLossPt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);
        UPSDb.freqLowLossPt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[5])*10.00);
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.freqHighLossPt=0;
        UPSDb.freqLowLossPt=0;

    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QYF_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer:QYF<cr>
//* UPS: (FF.F <cr>
//*     Data    Description Notes
//* a   (   Start byte
//* b   FF.F    frequency of bypass F is an Integer number 0 to 9. The units is Hz.
//*
//****************************************************************************
//============================================================================
void QYF_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=4)))
    {
        UPSDb.bypassFreq=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.bypassFreq=0;
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QFLAG_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer: QFLAG<cr>
//* UPS: (ExxxDxxx <cr>
//* ExxxDxxx is the flag status. E means enable, D means disable
//* x Control setting
//* p Enable/disable bypass audible warning
//* b Enable/disable battery mode audible warning
//* r Enable/disable auto-Restart.
//* o Enable/disable bypass when UPS turn off.
//* a Enable/disable audible alarm
//* s Enable/disable battery deep discharge protect
//* v Enable/disable converter mode
//* e Enable/disable high efficiency mode
//* g Enable/disable energy saving
//* h Enable/disable short restart 3 times
//* c Enable/disable code start
//* f Enable/disable bypass forbiding
//* j Enable/disable Output socket1 when the delay release time is over in battery mode .
//* l Enable/disable Site fault detect
//* m Set hot standby master/slave, PEM means master, PDM means slave
//****************************************************************************
//============================================================================
void QFLAG_ProcessResponse(void *ptr)
{
    uint16_t itr=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t EnableFlag=TRUE;
    uint8_t tempFlag=TRUE;


    if(pDIStruct->rxBufferSize >=17)
    {
        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if((pDIStruct->rxBuffer[itr] == 'D') || (EnableFlag==FALSE))
            {
                EnableFlag=FALSE;
                tempFlag=FALSE;
            }
            else
            {
                tempFlag=TRUE;
            }

            if(pDIStruct->rxBuffer[itr] == 'a')
            {
                UPSDb.settingFlags.upsFlagsBits.audibleAlarm=tempFlag;
            }			
            else if(pDIStruct->rxBuffer[itr] == 'b')
            {
                UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'c')
            {
                UPSDb.settingFlags.upsFlagsBits.codeStart=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'd')
            {
                UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'e')
            {
                UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'f')
            {
                UPSDb.settingFlags.upsFlagsBits.bypassForbiding=tempFlag;
            }
	        else if(pDIStruct->rxBuffer[itr] == 'g')
            {
                UPSDb.settingFlags.upsFlagsBits.energySaving=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'h')
            {
                UPSDb.settingFlags.upsFlagsBits.shortRestart3Times=tempFlag;
            }	
            else if(pDIStruct->rxBuffer[itr] == 'i')
            {
                UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'j')
            {
                UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode=tempFlag;
            }

            else if(pDIStruct->rxBuffer[itr] == 'k')
            {
                UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode=tempFlag;
            }			
            else if(pDIStruct->rxBuffer[itr] == 'l')
            {
                UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'm')
            {
                UPSDb.settingFlags.upsFlagsBits.SetHotStandby=tempFlag;
            }			
            else if(pDIStruct->rxBuffer[itr] == 'n')
            {
                UPSDb.settingFlags.upsFlagsBits.deepHighEfficiencyMode=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'o')
            {
                UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'p')
            {
                UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'q')
            {
                UPSDb.settingFlags.upsFlagsBits.ConstantPhaseAngleFn=tempFlag;
            }			
            else if(pDIStruct->rxBuffer[itr] == 'r')
            {
                UPSDb.settingFlags.upsFlagsBits.autoRestart=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 's')
            {
                UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect=tempFlag;
            }
	        else if(pDIStruct->rxBuffer[itr] == 't')
            {
                UPSDb.settingFlags.upsFlagsBits.battLowProtect=tempFlag;
            }	
            else if(pDIStruct->rxBuffer[itr] == 'u')
            {
                UPSDb.settingFlags.upsFlagsBits.FreeRunFunction=tempFlag;
            }			
            else if(pDIStruct->rxBuffer[itr] == 'v')
            {
                UPSDb.settingFlags.upsFlagsBits.converterMode=tempFlag;
            }
	        else if(pDIStruct->rxBuffer[itr] == 'w')
            {
                UPSDb.settingFlags.upsFlagsBits.limitedRuntimeOnBatt=tempFlag;
            }
            else if(pDIStruct->rxBuffer[itr] == 'x')
            {
                UPSDb.settingFlags.upsFlagsBits.outputParallelFn=tempFlag;
            }
	        else if(pDIStruct->rxBuffer[itr] == 'y')
            {
                UPSDb.settingFlags.upsFlagsBits.phaseAutoAdapt=tempFlag;
            }
	        else if(pDIStruct->rxBuffer[itr] == 'z')
            {
                UPSDb.settingFlags.upsFlagsBits.periodSelfTest=tempFlag;
            }			
        }
    }

}
//============================================================================
//****************************************************************************
//*
//* Function name: QWS_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*Computer: QWS<cr>
//*UPS: (a0a1……a62a63<cr>
//*a0,…,a63 is the warning status. If the warning is happened, the relevant bit will set 1,
//*else the relevant bit will set 0. The following table is the warning code
//*
//*bit  Warning code    warning
//*a0   01  Battery open
//*a1   02  IP N loss
//*a2   03  IP site fail
//*a3   04  Line phase error
//*a4   05  Bypass phase error
//*a5   06  Bypass frequency unstable
//*a6   07  Battery over charge
//*a7   08  Battery low
//*a8   09  Overload warning
//*a9   0A  Fan lock warning
//*a10  0B  EPO active
//*a11  0C  Turn on abnormal
//*a12  0D  Over temperature
//*a13  0E  Charger fail
//*a14  0F  Remote shut down
//*a15  10  L1 IP fuse fail
//*a16  11  L2 IP fuse fail
//*a17  12  L3 IP fuse fail
//*a18  13  L1 PFC positive error
//*a19  14  L1 PFC negative error
//*a20  15  L2 PFC positive error
//*a21  16  L2 PFC negative error
//*a22  17  L3 PFC positive error
//*a23  18  L3 PFC negative error
//*a24  19  CAN communication error
//*a25  1A  Synchronization line error
//*a26  1B  Synchronization pulse error
//*a27  1C  Host line error
//*a28  1D  Male connection error
//*a29  1E  Female connection error
//*a30  1F  Parallel line connection error
//*a31  20  Battery connect different
//*a32  21  Line connect different
//*a33  22  Bypass connect different
//*a34  23  Mode type different
//*a35  24  Parallel inverter voltage setting different
//*a36  25  Parallel output frequency setting different
//*a37  26  Battery cell over charge
//*a38  27  Parallel output parallel setting different
//*a39  28  Parallel output phase setting different
//*a40  29  Parallel Bypass Forbidden setting different
//*a41  2A  Parallel Converter Enable setting different
//*a42  2B  Parallel Bypass Freq High loss setting different
//*a43  2C  Parallel Bypass Freq Low loss setting different
//*a44  2D  Parallel Bypass Volt High loss setting different
//*a45  2E  Parallel Bypass Volt Low Loss setting different
//*a46  2F  Parallel Line Freq High Loss setting different
//*a47  30  Parallel Line Freq Low Loss setting different
//*a48  31  Parallel Line Volt High Loss setting different
//*a49  32  Parallel Line Volt Low Loss setting different
//*a50  33  Locked in bypass after overload 3 times in 30min
//*a51  34  Warning for three-phase AC input current unbalance
//*a52  35  Battery fuse broken
//*a53  36  Inverter inter-current unbalance
//*a54  37  P1 cut off pre-alarm
//*a55  38  Warning for Battery replace
//*a56  39  Warning for input phase error for LV 6-10K UPS
//*a57  3A  Cover of maintain switch is open
//*a58  3B  Phase Auto Adapt Failed
//*a59  3C  Utility extremely unbalanced
//*A60  3D  Bypass unstable
//*A61  3E  EEPROM operation eeror
//*A62  3F  Parallel protect warning
//*A63  40  Discharger overly
//****************************************************************************
//============================================================================
void QWS_ProcessResponse(void *ptr)
{
    uint16_t itr1=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint32_t tempVar1=0,tempVar2=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=64)))
    {
          for(itr1=0;itr1<32;itr1++)
        {
              if(pDIStruct->rxBuffer[itr1]==0x31)
            {
                tempVar1 |= ((uint32_t)0x00000001 << itr1);

            }
        }
        for(itr1=0;itr1<32;itr1++)
       {

           if(pDIStruct->rxBuffer[itr1+32]==0x31)
           {
               tempVar2 |= ((uint64_t)0x00000001 << itr1);

           }
       }
        UPSDb.warningFlags.warningAll_word[0]=tempVar1;
        UPSDb.warningFlags.warningAll_word[1]=tempVar2;
        //UPSDb.warningFlags.warningAll=tempVar;
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QFS_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*This command shows the information of the last UPS fault recorded on non-volatile
//*memory.
//*If UPS never failed:
//*computer: QFS<cr>
//*UPS: (OK<cr> (no fault)
//*If there was UPS fail occur:
//*Computer: QFS<cr>
//*UPS: (KK PPP.P FF.F OOO.O EE.E LLL CCC.C HHH.H NNN.N BBB.B TTT.T
//*<b7b6b5b4b3b2b1b0><cr>

//*(a) Start byte: (
//*(b) Fault kind: KK
//*K is 2 bytes of ASCII code, define as following:
//*(c) I/P voltage before fault: PPP.P
//*P is an integer number ranging from 0 to 9. The unit is Volt.
//*(d) I/P frequency before fault: FF.F
//*F is an integer number ranging from 0 to 9. The unit is Hz.
//*(e) Inverter O/P voltage before fault: OOO.O
//*O is an integer number ranging from 0 to 9. The unit is Volt.
//*(f) Inverter O/P frequency before fault: EE.E
//*E is an integer number ranging from 0 to 9. The unit is Hz.
//*(g) O/P load before fault: LLL
//*LLL is the maximum of W% or VA%.
//*VA% is a percent of maximum VA.
//*W% is a percent of maximum real power.
//*(h) O/P current before fault: CCC.C
//*CCC is a percent of maximum current.
//*(i) Positive Bus voltage before fault: HHH.H
//*P is an integer number ranging from 0 to 9. The unit is volt.
//*(j) Negative Bus voltage before fault: NNN.N
//*N is an integer number ranging from 0 to 9. The unit is volt.
//*(k) Battery voltage before fault: BBB.B
//*B is an integer number ranging from 0 to 9. The unit is volt
//*(l) Temperature before fault: TTT.T
//*T is an integer number ranging from 0 to 9. The unit is degree of centigrade.
//*(m) UPS running status before fault: <b7b6b5b4b3b2b1b0>
//*<b7b6b5b4b3b2b1b0> is one byte of binary information.

//****************************************************************************
//============================================================================
void QFS_ProcessResponse(void *ptr)
{
    uint16_t itr=0,itr1=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t paramIndex=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=67)))
    {
        UPSDb.upsFaultStruct.FaultType=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));

        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if(pDIStruct->rxBuffer[itr]==SPACE_CHAR)
            {
                paramIndex++;
                switch(paramIndex)
                {
                    case 0:
                        UPSDb.upsFaultStruct.FaultType=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));
                        break;
                    case 1:
                        UPSDb.upsFaultStruct.InputVoltBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 2:
                        UPSDb.upsFaultStruct.InputFreqBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 3:
                        UPSDb.upsFaultStruct.OutputVoltBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 4:
                        UPSDb.upsFaultStruct.OutputFreqBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 5:
                        UPSDb.upsFaultStruct.OutputLoadBeforeFault=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    case 6:
                        UPSDb.upsFaultStruct.OutputCurrentBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 7:
                        UPSDb.upsFaultStruct.PBusVoltageBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 8:
                        UPSDb.upsFaultStruct.NBusVoltBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 9:
                        UPSDb.upsFaultStruct.BattVoltBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 10:
                        UPSDb.upsFaultStruct.TempBeforeFault=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);
                        break;
                    case 11:
                        UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll=0;
                        for(itr1=0;itr1<8;itr1++)
                        {
                            if(pDIStruct->rxBuffer[itr+1+itr1]==0x31)
                            {
                                UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll |= (1 << (7-itr1));

                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        memset(&UPSDb.upsFaultStruct,0,sizeof(UPS_FAULT_STRUCT));
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QVFW_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*Computer: QVFW<cr>
//*UPS: (VERFW: <NNNNN.NN><cr>
//*<n> is a HEX number from 0...9 or A…F.
//*Example:
//*Computer: QVFW<cr>
//*UPS: (VERFW: <00123.01><cr>
//*00123: firmware series number;01:version.
//****************************************************************************
//============================================================================
void QVFW_ProcessResponse(void *ptr)
{
    uint16_t itr=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=15)))
    {
        for(itr=0;itr<15;itr++)
        {
            UPSDb.UPSCPUFwVersion[itr]=pDIStruct->rxBuffer[itr];
        }
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        memset(UPSDb.UPSCPUFwVersion,0,15);
    }
}

//============================================================================
//****************************************************************************
//*
//* Function name: QID_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* Computer:QID<cr>
//*UPS: (ABCDEEFFGXXXXX<cr>
//*Data Description Notes
//*a ( Start byte
//*b A Main Production type 8: UPS,9: NONE UPS
//*c B Sub Production type
//*d C VA type
//*e D H/LV type
//*f EE Year
//*g FF Month
//*h G Manufacturer ID
//*i XXXXX Serial number
//****************************************************************************
//============================================================================
void QID_ProcessResponse(void *ptr)
{
    uint16_t itr=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=19)))
    {
        UPSDb.upsID.type=pDIStruct->rxBuffer[0];//-0x30;
        UPSDb.upsID.subType=pDIStruct->rxBuffer[1];//-0x30;
        UPSDb.upsID.VAType=pDIStruct->rxBuffer[2];//-0x30;
        UPSDb.upsID.H_LV_Type=pDIStruct->rxBuffer[3];//-0x30;
        UPSDb.upsID.year[0]=pDIStruct->rxBuffer[4];//-0x30;;
        UPSDb.upsID.year[1]=pDIStruct->rxBuffer[5];//-0x30;;
        UPSDb.upsID.month[0]=pDIStruct->rxBuffer[6];//-0x30)
        UPSDb.upsID.month[1]=pDIStruct->rxBuffer[7];//-0x30)
        UPSDb.upsID.ManufacturerID=pDIStruct->rxBuffer[8];//-0x30;
        for(itr=0;itr<11;itr++)
        {
            UPSDb.upsID.serialNumber[itr]=pDIStruct->rxBuffer[9+itr];//-0x30;
        }
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.upsID.type=0x30;
        UPSDb.upsID.subType=0x30;
        UPSDb.upsID.VAType=0x30;
        UPSDb.upsID.H_LV_Type=0x30;
        UPSDb.upsID.year[0]=0x30;
        UPSDb.upsID.year[1]=0x30;
        UPSDb.upsID.month[0]=0x30;
        UPSDb.upsID.month[1]=0x30;
        UPSDb.upsID.ManufacturerID=0x30;
        memset(UPSDb.upsID.serialNumber,0x30,12);
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: QBV_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*Computer:QBV<cr>
//*UPS: (RRR.R NN MM CCC TTT<cr>
//*Data Description Notes
//*a ( Start byte
//*b RRR.R Battery voltage R is an Integer number 0 to 9. The units is V.
//*c NN Battery piece number NN is from 01 to 20.
//*d MM Battery group number MM is an Integer number 01 to 99.
//*e CCC Battery capacity CCC is an Integer number 000 to 100.
//*f TTT Battery remain time T is an Integer number 0 to 9. The units is minutes.
//****************************************************************************
//============================================================================
void QBV_ProcessResponse(void *ptr)
{
    uint16_t itr=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    uint8_t paramIndex=0;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=19)))
    {
        UPSDb.BattVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[itr+1])*10.00);

        for(itr=0;itr<pDIStruct->rxBufferSize;itr++)
        {
            if(pDIStruct->rxBuffer[itr]==SPACE_CHAR)
            {
                paramIndex++;
                switch(paramIndex)
                {
                    case 0:
                        UPSDb.BattVolt=(uint16_t)(atof((const char*)&pDIStruct->rxBuffer[0])*10.00);
                        break;
                    case 1:
                        UPSDb.BattPieceNumber=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    case 2:
                        UPSDb.BattgroupNumber=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    case 3:
                        UPSDb.BattCapacity=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    case 4:
                        UPSDb.BattRemainTime=(uint16_t)(atoi((const char*)&pDIStruct->rxBuffer[itr+1]));
                        break;
                    default:
                        break;
                }
            }
        }
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.BattVolt=0;
        UPSDb.BattPieceNumber=0;
        UPSDb.BattgroupNumber=0;
        UPSDb.BattCapacity=0;
        UPSDb.BattRemainTime=0;
    }

}
//============================================================================
//****************************************************************************
//*
//* Function name: QHE_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//*Computer: QHE<cr>
//*UPS: (HHH LLL <cr>
//*Data Description Notes
//*a ( Start byte
//*b HHH Voltage high loss point H is an Integer number 0 to 9. The unit is voltage.
//*c LLL Voltage low loss point L is an Integer number 0 to 9. The unit is voltage
//****************************************************************************
//============================================================================
void QHE_ProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        && ((pDIStruct->rxBufferSize >=7)))
    {
        UPSDb.voltHighLossPt=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));
        UPSDb.voltLowLossPt=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[4]));
    }
    else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
    {
        UPSDb.voltHighLossPt=0;
        UPSDb.voltLowLossPt=0;

    }
}

//============================================================================
//****************************************************************************
//*
//* Function name: QSK1_ProcessResponse
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void QSK1_ProcessResponse(void *ptr)
{
    int socketnum=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

       if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
           && ((pDIStruct->rxBufferSize >=1)))
       {
           socketnum=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));

       }

}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_EndOfFrame
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void QSK2_ProcessResponse(void *ptr)
{
    int socketnum=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

       if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
           && ((pDIStruct->rxBufferSize >=1)))
       {
           socketnum=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));

       }

}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_EndOfFrame
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void QSKT1_ProcessResponse(void *ptr)
{
    int socketnum=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

   if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
       && ((pDIStruct->rxBufferSize >=1)))
   {
       socketnum=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));

   }
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_EndOfFrame
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void QSR_ProcessResponse(void *ptr)
{
    int ShutDownTime=0, ShutRestoreTime=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

   if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
       && ((pDIStruct->rxBufferSize >=9)))
   {
       ShutDownTime=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[3]));
       ShutRestoreTime=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[4]));

   }
}


//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_EndOfFrame
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================
void QSKT2_ProcessResponse(void *ptr)
{
    int socketnum=0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;

   if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
       && ((pDIStruct->rxBufferSize >=1)))
   {
       socketnum=(uint32_t)(atoi((const char*)&pDIStruct->rxBuffer[0]));

   }
}

//============================================================================
//****************************************************************************
//*
//* Function name: CtrlCmdProcessResponse
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================
void CtrlCmdProcessResponse(void *ptr)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)ptr;
    memcpy(ctrrxbuf,pDIStruct->rxBuffer,5);
    if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED) && (pDIStruct->rxBufferSize >=4))
    {
        if((pDIStruct->rxBuffer[0]=='A') && (pDIStruct->rxBuffer[1]=='C') && (pDIStruct->rxBuffer[2]=='K'))
        {
            pDIStruct->UPSWriteRequestStruct.writeState = STATE_UPS_WRITE_SUCCESS;
			pDIStruct->uartDiag.goodTransaction = (uint32_t) ((pDIStruct->uartDiag.goodTransaction + 1) & 0xFFFFFFFF);
        }
        else
	    {
	        pDIStruct->UPSWriteRequestStruct.writeState = STATE_UPS_WRITE_ERROR;
			pDIStruct->uartDiag.badTransaction = (uint32_t) ((pDIStruct->uartDiag.badTransaction + 1) & 0xFFFFFFFF);
			
	    }		
    }
    else
    {
        pDIStruct->UPSWriteRequestStruct.writeState = STATE_UPS_WRITE_ERROR;
		pDIStruct->uartDiag.badTransaction = (uint32_t) ((pDIStruct->uartDiag.badTransaction + 1) & 0xFFFFFFFF);
    }

}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_RxISR
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================

void UPSInterface_ValidateAndProcess(UPS_INTERFACE_RAM_STRUCT *pDIStruct)
{
    uint16_t cmdIndex=0;
    cmdIndex=pDIStruct->currCmdIndex;

	if(pDIStruct->UPSWriteRequestStruct.writeState==STATE_UPS_WRITE_CTRL_CMD_SENT)
	{
	   CtrlCmdProcessResponse(pDIStruct);
	}	
    else if(cmdIndex < MAX_UPS_INQUIRY_CMDS)
    {
        if((pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
             && ((pDIStruct->rxBufferSize >=4)))
        {
            if((pDIStruct->rxBuffer[0]=='N') && (pDIStruct->rxBuffer[1]=='A') && (pDIStruct->rxBuffer[2]=='K'))
            {
                pDIStruct->uartDiag.badTransaction = (uint32_t) ((pDIStruct->uartDiag.badTransaction + 1) & 0xFFFFFFFF);
            }
            else
            {
                pDIStruct->uartDiag.goodTransaction = (uint32_t) ((pDIStruct->uartDiag.goodTransaction + 1) & 0xFFFFFFFF);
                pDIStruct->commandListPtr[cmdIndex].cmdResponseFn(pDIStruct);
            }
        }
        else if(pDIStruct->requestState==STATE_UPS_REQUEST_RECEIVED)
        {
           pDIStruct->commandListPtr[cmdIndex].cmdResponseFn(pDIStruct);
        }
    }
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_RxISR
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master RX ISR function. This function recieves Modbus Rtu packets
//* and starts Rx interframe delay timer on every byte recieved.
//****************************************************************************
//============================================================================

void UPSInterface_RxISR(uint8_t character)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct;
    uint32_t Index=0;

    pDIStruct=&UPSInterfaceStruct;

    COMTimer_Start(pDIStruct->reqTimeoutTimer,pDIStruct->reqTimeoutTime);

    pDIStruct->uartDiag.charRecv = (uint32_t) ((pDIStruct->uartDiag.charRecv + 1) & 0xFFFFFFFF);

    switch(pDIStruct->requestState)
    {
        case STATE_UPS_REQUEST_XMITTED:
            if((pDIStruct->rxBufferSize==0) && (character==START_OF_FRAME)) // 0x28 == '(' --> is start of frame
            {
                pDIStruct->requestState=STATE_UPS_REQUEST_RECEIVING;
             }
            else
            {
                pDIStruct->uartDiag.DumpChars = (uint32_t) ((pDIStruct->uartDiag.DumpChars + 1) & 0xFFFFFFFF);
            }
        break;
         case STATE_UPS_REQUEST_RECEIVING:
             Index=pDIStruct->rxBufferSize++;
             if (Index < MAX_RX_BUFFER_SIZE)
             {
                 pDIStruct->rxBuffer[Index] = character;


                if (pDIStruct->rxBuffer[Index] == END_OF_FRAME)// 0x0D == '\r' --> end of frame
                {
                    pDIStruct->requestState=STATE_UPS_REQUEST_RECEIVED;
                    pDIStruct->uartDiag.framesRecv = (uint32_t) ((pDIStruct->uartDiag.framesRecv + 1) & 0xFFFFFFFF);
                    UPSInterface_ValidateAndProcess(pDIStruct);


                  //  UPSInterface_XmitNextCmd(pDIStruct);
                }
                else if (pDIStruct->rxBuffer[Index] == SPACE_CHAR)// 0x20 == 'SPACE' --> Space
                {
                    //UPSInterface_ValidateAndProcess(pDIStruct);Dosnt need to call. If progressive processing is needed then Call
                }
             }
             else
             {
                 pDIStruct->uartDiag.DumpChars = (uint32_t) ((pDIStruct->uartDiag.DumpChars + 1) & 0xFFFFFFFF);
             }			 
            break;
        default:
            pDIStruct->uartDiag.DumpChars = (uint32_t) ((pDIStruct->uartDiag.DumpChars + 1) & 0xFFFFFFFF);
            UPS_UartTxIntDisable(PORT_UART1);

            break;
    }

}

//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_TxISR
//*
//* Return:   None
//*
//* Description:
//* This is Modbus Rtu Master Tx ISR function. This function transmits Modbus Rtu packets
//* and starts Tx interframe delay timer after last byte is transmitted.
//****************************************************************************
//============================================================================
void UPSInterface_TxISR()
{
    uint32_t Index=0;

    UPS_INTERFACE_RAM_STRUCT *pDIStruct=&UPSInterfaceStruct;

    COMTimer_Start(pDIStruct->reqTimeoutTimer,pDIStruct->reqTimeoutTime);

    switch(pDIStruct->requestState)
    {
        case STATE_UPS_REQUEST_XMITTING:

            if (pDIStruct->txRemainingSize > 0)
            {
                Index=pDIStruct->txBufferIndex++;
                UPS_UartWriteChar (pDIStruct->txBuffer[Index],PORT_UART1);
                pDIStruct->txRemainingSize--;
                //Char Sent. Increment link counter
                pDIStruct->uartDiag.charSent = (uint32_t) ((pDIStruct->uartDiag.charSent + 1) & 0xFFFFFFFF);    // allow rollover

             }
            else
            {
                UPS_UartTxIntDisable(PORT_UART1);
                memset(pDIStruct->rxBuffer,0,MAX_RX_BUFFER_SIZE);
                pDIStruct->rxBufferSize=0;
                pDIStruct->requestState=STATE_UPS_REQUEST_XMITTED;
                pDIStruct->uartDiag.frameSent = (uint32_t) ((pDIStruct->uartDiag.frameSent + 1) & 0xFFFFFFFF);

            }
        break;

        default:
            UPS_UartTxIntDisable(PORT_UART1);
            memset(pDIStruct->rxBuffer,0,MAX_RX_BUFFER_SIZE);
            pDIStruct->rxBufferSize=0;
            break;
    }
}


//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_RequestTimeout
//*
//* Return:   None
//*
//* Description:
//*
//*
//============================================================================
//****************************************************************************
void UPSInterface_RequestTimeout(COM_TIMER_ID id, void* param)
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=(UPS_INTERFACE_RAM_STRUCT *)param;
   // uint16_t cmdIndex=0;

    COMTimer_Stop(pDIStruct->reqTimeoutTimer);

    switch(pDIStruct->requestState)
    {
        case STATE_UPS_REQUEST_IDLE:

            pDIStruct->currCmdIndex=MAX_UPS_INQUIRY_CMDS;//So that it shall start with 0 index
            if((pDIStruct->UPSWriteRequestStruct.writeState!=STATE_UPS_WRITE_EMPTY))
            {
                FrameRxComp=4;
                pDIStruct->UPSWriteRequestStruct.writeState=STATE_UPS_WRITE_ERROR;
            }
			//UPSInterface_XmitNextCmd(pDIStruct);
         break;
        case STATE_UPS_REQUEST_RECEIVED:
			//Transmit next command
         break;			
		
        default:
            pDIStruct->uartDiag.ReqTimedout = (uint32_t) ((pDIStruct->uartDiag.ReqTimedout + 1) & 0xFFFFFFFF);  // allow rollover
//            if((pDIStruct->UPSWriteRequestStruct.writeState!=STATE_UPS_WRITE_EMPTY))
//            {
//                FrameRxComp=5;
//                pDIStruct->UPSWriteRequestStruct.writeState=STATE_UPS_WRITE_ERROR;
//            }
//
        break;
    }

	UPSInterface_XmitNextCmd(pDIStruct);
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_StartUp
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description:
//* This function is being called when Modbus Rtu Master is initialised at powerup or
//* when protocol settings are changed. This function initialises all  timers and structures
//* related to Modbus rtu Master.
//*
//****************************************************************************
//============================================================================
BOOL UPSInterface_Init()
{
    UPS_INTERFACE_RAM_STRUCT *pDIStruct;

    COM_TIMER_PARAM     reqTimerParam;

    BOOL status=TRUE;

    pDIStruct= &UPSInterfaceStruct;

    memset(&pDIStruct->uartDiag,0x00,sizeof(UART_DIAG_COUNTER));

    reqTimerParam.param = (void*) pDIStruct;
    reqTimerParam.timeoutFunc = UPSInterface_RequestTimeout;
    reqTimerParam.recurringFlag = 0;
    pDIStruct->reqTimeoutTimer = COMTimer_Create(&reqTimerParam);

    if ( COM_TIMER_ID_INVALID == pDIStruct->reqTimeoutTimer)
    {
        status=FALSE;
    }

    pDIStruct->reqTimeoutTime=((COM_TIMER_TICK)SERIAL_INTERFACE_TIMEOUT);

  	memset(&pDIStruct->UPSWriteRequestStruct,0x00,sizeof(UPS_WRITE_REQUEST));

    pDIStruct->currCmdIndex=MAX_UPS_INQUIRY_CMDS;//So that it shall start with 0 index
    pDIStruct->requestState=STATE_UPS_REQUEST_IDLE;
    pDIStruct->rxBufferSize=0;
    pDIStruct->txBufferIndex=0;
    pDIStruct->txRemainingSize=0;

    pDIStruct->commandListPtr=commandListStruct;
    UPS_UartInit();
    COMTimer_Start(pDIStruct->reqTimeoutTimer,pDIStruct->reqTimeoutTime);

    return status;
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_DiagnosticParameters
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description:
//* This function is being called when Modbus Rtu Master is initialised at powerup or
//* when protocol settings are changed. This function initialises all  timers and structures
//* related to Modbus rtu Master.
//*
//****************************************************************************
//============================================================================
uint32_t UPSInterface_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset)
{
    uint32_t paramValue = 0;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=&UPSInterfaceStruct;

    if(Reset == TRUE)
    {
        memset(&pDIStruct->uartDiag,0x00,sizeof(UART_DIAG_COUNTER));
    }
    else
    {
        switch(ParamIndex)
        {
            case 0:
            {
                paramValue = pDIStruct->uartDiag.framesRecv;
                break;
            }
            case 1:
            {
                paramValue = pDIStruct->uartDiag.frameSent;
                break;
            }
            case 2:
            {
                paramValue = pDIStruct->uartDiag.ReqTimedout;
                break;
            }
            case 3:
            {
                paramValue = pDIStruct->uartDiag.charRecv;
                break;
            }
            case 4:
            {
                //Comm fail
                paramValue = pDIStruct->uartDiag.charSent;
                break;
            }
            case 5:
            {
                paramValue = pDIStruct->uartDiag.goodTransaction;
                break;
            }
            default:
                break;
        }
    }
    return paramValue;
}
//============================================================================
//****************************************************************************
//*
//* Function name: UPSInterface_CheckPendingReqeust
//*
//* Return:   None
//*
//* Description: This function manages pending request to be processed
//* This is not used now as all the processing is handled by GPIO (CS) ISRs and SPI ISR.
//*
//****************************************************************************
//============================================================================
void UPSInterface_Task()
{
    //UPS_INTERFACE_RAM_STRUCT *pDIStruct=&UPSInterfaceStruct;

    while(1)
    {
        Error_ResetTaskWatchdog(UPS_SERIAL_TASK_ID);
        //UPSInterface_ProcessRequest(pDIStruct,pDIStruct->pCurrRXRequest);
        vTaskDelay(100);//dont change delay time
    }

}


//============================================================================
//****************************************************************************
//*
//* Function name: HTTP_ReadSerialDiagnostic
//*
//* Return: None
//*
//*
//* Description:
//*  Http web server page shows the UPS interface diagnosis data
//*
//*
//*
//****************************************************************************
//==============================================================================

uint32_t HTTP_ReadSerialDiagnostic(uint16_t Port, uint16_t Index)
{
    uint32_t Counter = 0;
	UPS_INTERFACE_RAM_STRUCT *pDIStruct;
	pDIStruct=&UPSInterfaceStruct;

    if(Index == 0)
        Counter = pDIStruct->uartDiag.charRecv;
    else if(Index == 1)
        Counter = pDIStruct->uartDiag.charSent;
    else if(Index == 2)
        Counter = pDIStruct->uartDiag.framesRecv;
    else if(Index == 3)
        Counter = pDIStruct->uartDiag.frameSent;
    else if(Index == 4)
        Counter = pDIStruct->uartDiag.goodTransaction;
    else if(Index == 5)
        Counter = 0;
    else if(Index == 6)
        Counter = 0;
    else if(Index == 7)
        Counter = 0;
    else if(Index == 8)
        Counter = pDIStruct->uartDiag.ReqTimedout;
    else if(Index == 9)
        Counter = pDIStruct->uartDiag.badTransaction;			
	return Counter;
}

//==============================================================================

//==============================================================================
// END OF UPSIntrface.c FIlE
//==============================================================================
