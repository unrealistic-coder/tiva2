#include "websocket.h"
#include "EthernetTask.h"
#include "sha1.h"
#include "base64.h"
#include <string.h>
//#include <RTC.h>
#include <SMTPManager.h>
//#include "SD_DataLogger.h"
//#include "GlobalData.h"
#include "OnChipEEPROM.h"
#include "UPSInterface.h"
#include "lwip/inet_chksum.h"
#include "smtp_opts.h"
#include "smtp.h"
#include "SPIExtRTC.h"
#include "ErrorHandler.h"
#include "UPSInterface.h"
volatile REG_uint64_t EmailCurrentAlarms;
REG_uint64_t	CurrentEmailAlarms,PreviousEmailAlarms;

EMAILSTRUCT EmailConfig;
SMTPRUNBUFF SMTPConfig;


static void SMTP_ResultCallback(void *arg, u8_t smtp_result, u16_t srv_err, err_t err)
{
    printf("mail (%p) sent with results: 0x%02x, 0x%04x, 0x%08x\n", arg,
    smtp_result, srv_err, err);

    SMTPConfig.SMTP_result = smtp_result;
    SMTPConfig.SMTP_srv_err = srv_err;
    SMTPConfig.SMTP_err = err;

    if((SMTPConfig.SMTP_result == ERR_OK) &&
        (SMTPConfig.SMTP_srv_err == ERR_OK)&&
        (SMTPConfig.SMTP_err == ERR_OK))
    {
        SMTPConfig.EmailSent++;
    }
    else
    {
        SMTPConfig.EmailFailed++;
    }
}

err_t SMTP_ConfigureAndSendEmail()
{
    err_t SMTPError=ERR_OK;
    uint16_t TimeOutCounter =0;
    int *some_argument = 0;
    uint8_t itr=0;

    SMTPError = smtp_set_server_addr((char *)Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost);
    if(SMTPError == ERR_OK)
    {
        smtp_set_server_port(Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort);
        SMTPError = smtp_set_auth((char *)Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom, (char *)Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword);
        if(SMTPError == ERR_OK)
        {
            for(itr=0;itr<3;itr++)
            {
                if ((Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[itr].EmailTo_Add[0] != '\0') )
                {
                    SMTPError = smtp_send_mail((char *)Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom, (char *)Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[itr].EmailTo_Add,
                                (char *)SMTPConfig.Subject,(char *)SMTPConfig.EmailMSG , SMTP_ResultCallback, &itr);
                    if(SMTPError != ERR_OK)
                    {
                        break;
                    }
                }
          }
		}
	}

	 return SMTPError;
}
static BOOL SMTPEmail_PrepareEmail()
{
    uint16_t StringLen = 0;
    BOOL  Status=FALSE;
    static uint8_t         MainsFail_Flag = 0;
    memset((uint8_t *)SMTPConfig.Subject, 0x00, MAX_LEN);
    memset((uint8_t *)SMTPConfig.EmailMSG, 0x00, MAX_MSG_LEN);

    MEMCPY((char *)&SMTPConfig.EmailMSG[StringLen], "Serial No: \n",11);

    StringLen = strlen((char *)SMTPConfig.EmailMSG);

    MEMCPY((char *)&SMTPConfig.EmailMSG[StringLen], UPSDb.upsID.serialNumber,11);

    StringLen = strlen((char *)SMTPConfig.EmailMSG);
    if(StringLen==11)
    {
        MEMCPY((char *)&SMTPConfig.EmailMSG[StringLen], "0000000000\n",11);
    }

    StringLen = strlen((char *)SMTPConfig.EmailMSG);
    sprintf((char *)&SMTPConfig.EmailMSG[StringLen], "Date & time: %02d-%02d-20%02d %02d:%02d:%02d\nAlarms:\n",
                GlobalDateTime.Date,GlobalDateTime.Month,GlobalDateTime.Year,
                GlobalDateTime.Hours, GlobalDateTime.Minutes, GlobalDateTime.Seconds);

    StringLen = strlen((char *)SMTPConfig.EmailMSG);
    if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 1) != 0)
      {
          if (UPSDb.warningFlags.Bits.BattLow)
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Batt Low Alarm\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 2) != 0)
      {
          if (UPSDb.warningFlags.Bits.ParallelINV_VoltDiff || (UPSDb.warningFlags.Bits.EPOActive==1)
                  ||UPSDb.warningFlags.Bits.RemoteShutdown )
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Inverter Trip\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 4) != 0)
      {
          if (UPSDb.warningFlags.Bits.BatteryOpen)
            {
                sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Battery SW Open\n");
                StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
                Status              = TRUE;

            }
          else if (UPSDb.warningFlags.Bits.BattLow || UPSDb.warningFlags.Bits.BattOverCharge
                   || UPSDb.warningFlags.Bits.BattCellOverCharge)
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Battery Cell Alarm\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;

          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 8) != 0)
      {
          if (UPSDb.warningFlags.Bits.IP_N_Loss || UPSDb.warningFlags.Bits.IP_Site_Fail
                || UPSDb.warningFlags.Bits.LinePhaseError ||
                UPSDb.warningFlags.Bits.L1_IP_Fuse_Fail ||
                UPSDb.warningFlags.Bits.L2_IP_Fuse_Fail ||
                UPSDb.warningFlags.Bits.L3_IP_Fuse_Fail ||
                UPSDb.warningFlags.Bits.LineConnDiff )
          {
              MainsFail_Flag     = 1;
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Mains Fail\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 16) != 0)
      {
          if(MainsFail_Flag==1)
          {
              MainsFail_Flag=0;
              if (!UPSDb.warningFlags.Bits.InputPhaseError && !UPSDb.warningFlags.Bits.BattLow
                      &&(!UPSDb.warningFlags.Bits.UtilityExtremelyUnbalanced))
              {
                  sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Mains OK\n");
                  StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
                  Status              = TRUE;
              }
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 32) != 0)
      {
          if (UPSDb.warningFlags.Bits.INVCurrentUnbalance)
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "INV Curr Alarm\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 64) != 0)
      {
          if (UPSDb.warningFlags.Bits.BypassUnstable || UPSDb.warningFlags.Bits.BypassConnDiff
                  ||UPSDb.warningFlags.Bits.BypassFrequencyUnstable ||UPSDb.warningFlags.Bits.BypassConnDiff)
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Bypass Unstable\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

      if((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 128) != 0)
      {
          if (UPSDb.warningFlags.Bits.ChargerFail)
          {
              sprintf((char *) &SMTPConfig.EmailMSG[StringLen], "Charger Fail\n");
              StringLen           = strlen((const char *)SMTPConfig.EmailMSG);
              Status              = TRUE;
          }
      }

    StringLen = 0;
    sprintf((char *)&SMTPConfig.Subject[StringLen], "Fuji Notification (Sr No.%s)\n", UPSDb.upsID.serialNumber);
    StringLen = 0;
    return STATUS;
}

void SMTP_ProcessPendingEmail()
{
    switch(SMTPConfig.SMTP_State)
    {
        case SMTP_DETECT_CHANGE:

            if(Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled==1)
             {
               CurrentEmailAlarms.dword[0]  = UPSDb.warningFlags.warningAll_word[0] & 0xB0003CFB;
               CurrentEmailAlarms.dword[1]  = UPSDb.warningFlags.warningAll_word[1] & 0x99200003;
                if(((PreviousEmailAlarms.dword[0] &  0xB0003CFB) != (CurrentEmailAlarms.dword[0] &  0xB0003CFB))
                        ||((PreviousEmailAlarms.dword[1] & 0x99200003) != (CurrentEmailAlarms.dword[1] & 0x99200003)))
                {
                   PreviousEmailAlarms.dword[0] = CurrentEmailAlarms.dword[0];
                   PreviousEmailAlarms.dword[1] = CurrentEmailAlarms.dword[1];
                   EmailCurrentAlarms.all = CurrentEmailAlarms.all;
                   memset((uint8_t *)SMTPConfig.Subject, 0x00, MAX_LEN);
                   memset((uint8_t *)SMTPConfig.EmailMSG, 0x00, MAX_MSG_LEN);
                   SMTPConfig.feedbackTimeoutCntr=0;
                   SMTPConfig.To_Index=0;
                   SMTPConfig.SMTP_State=SMTP_PREPARE_EMAIL;
               }

             }
            break;
        case SMTP_PREPARE_EMAIL:
            if ((Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add[0] != '\0') ||
                (Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add[0] != '\0') ||
                (Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add[0] != '\0'))
            {

                if(SMTPEmail_PrepareEmail()==TRUE)
                {
                    SMTPConfig.SMTP_State=SMTP_CHECK_INTERVAL;
                }
                else
                {
                    SMTPConfig.SMTP_State=SMTP_DETECT_CHANGE;
                }
            }
            else
            {
                SMTPConfig.SMTP_State=SMTP_DETECT_CHANGE;
            }
            break;
        case SMTP_CHECK_INTERVAL:
            if(SMTP_CheckConnectionTimeout() == TRUE)
            {
                SMTP_StartConnectionTimer();
                SMTPConfig.SMTP_State=SMTP_SEND_EMAIL;
            }
            break;
        case SMTP_SEND_EMAIL:
            // Timeout

            if (SMTP_ConfigureAndSendEmail() != ERR_OK)
            {
                SMTPConfig.EmailFailed++;
            }
            SMTPConfig.SMTP_State=SMTP_DETECT_CHANGE;
            break;

    }
}

void SMTP_Init()
{
    SMTPConfig.IntervalTime = Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval *10;
    SMTPConfig.startIntervalTimeTimeout = 0;
    SMTPConfig.EmailSent = 0;
    SMTPConfig.EmailFailed = 0;
    SMTPConfig.SMTP_err = 0;
    SMTPConfig.SMTP_srv_err = 0;
    SMTPConfig.SMTP_result = 0;
    SMTPConfig.To_Index=0;
    SMTPConfig.SMTP_State = SMTP_DETECT_CHANGE;
    memset((uint8_t *)SMTPConfig.EmailMSG,0,MAX_MSG_LEN);
    memset((uint8_t *)SMTPConfig.Subject,0,MAX_LEN);

    SMTP_StartConnectionTimer();
}
//==========================================================================
//****************************************************************************
//* Function name: SMTP_StartConnectionTimer
//*
//* Return: None
//*
//* Description:
//*   Start WebSocket Server Timer after Connection established and request
//*   recived.
//****************************************************************************
//==========================================================================

void SMTP_StartConnectionTimer()
{
    SMTPConfig.startIntervalTimeTimeout =  COMTimer_Get10MsecTick();
}
//=============================================================================
//****************************************************************************
//* Function name: SMTP_CheckConnectionTimeout
//*
//* Return:  TRUE- For Timeout
//*          FALSE- Timer is still running.
//*
//* Description:
//*   WebSocket sever timer sends keep alive message after connection timer
//*   Timeout call by WebSocketServer_SocketOperation.
//*
//****************************************************************************
//==============================================================================
BOOL SMTP_CheckConnectionTimeout()
{
    uint32_t currentTick=0;
    uint32_t nextTick=0;
    uint64_t totalTick=0;
    BOOL status=FALSE;

    totalTick = SMTPConfig.startIntervalTimeTimeout + SMTPConfig.IntervalTime;

    currentTick=COMTimer_Get10MsecTick();

    if(totalTick >= COM_TICK_MAX)
    {
        if ( currentTick <= SMTPConfig.startIntervalTimeTimeout)
        {

            // rollover detected
            nextTick = (uint32_t)(totalTick - COM_TICK_MAX);
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
        if ( currentTick >= SMTPConfig.startIntervalTimeTimeout)
        {
            // no rollover
            nextTick = (uint32_t) (SMTPConfig.startIntervalTimeTimeout + SMTPConfig.IntervalTime);
            if (currentTick >= nextTick)
                status= TRUE;

        }
        else  // if ( upper >= COM_TICK_MAX )
        {
            // rollover detected
            nextTick = (uint32_t) (SMTPConfig.IntervalTime - ( COM_TICK_MAX - SMTPConfig.startIntervalTimeTimeout));

            if (currentTick >= nextTick)
                status= TRUE;
        }
    }
    return status;
}

