//****************************************************************************
/*
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Dirk Ziegelmeier <dziegel@gmx.de>
 *
 */
//****************************************************************************
//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <COMTimer.h>
#include <EthernetTask.h>
#include <SNMPAgent.h>
//#include "DatabaseHandler.h"
#include "OnChipEEPROM.h"
#include <UPSInterface.h>
#include "ErrorHandler.h"
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
//============================================================================
SNMP_COMMON_STRUCT snmpStruct;

SNMP_TRAP_STRUCT trapStructArray[64]=
{
    {TRAP_BATTERY_OPEN                                       ,{0},0},
    {TRAP_IP_N_LOSS                                          ,{0},0},
    {TRAP_IP_SITE_FAIL                                       ,{0},0},
    {TRAP_LINEPHASEERROR                                     ,{0},0},
    {TRAP_BYPASSPHASEERROR                                   ,{0},0},
    {TRAP_BYPASSFREQUENCYUNSTABLE                            ,{0},0},
    {TRAP_BATT_OVERCHARGE                                    ,{0},0},
    {TRAP_BATTLOW                                            ,{0},0},
    {TRAP_OVER_LOADWARNING                                   ,{0},0},
    {TRAP_FAN_LOCKWARNING                                    ,{0},0},
    {TRAP_EPO_ACTIVE                                         ,{0},0},
    {TRAP_TURN_ON_ABNORMAL                                   ,{0},0},
    {TRAP_OVER_TEMPERATURE                                   ,{0},0},
    {TRAP_CHARGER_FAIL                                       ,{0},0},
    {TRAP_REMOTE_SHUTDOWN                                    ,{0},0},
    {TRAP_L1_IP_FUSE_FAIL                                    ,{0},0},
    {TRAP_L2_IP_FUSE_FAIL                                    ,{0},0},
    {TRAP_L3_IP_FUSE_FAIL                                    ,{0},0},
    {TRAP_L1_PFC_PERROR                                      ,{0},0},
    {TRAP_L1_PFC_NERROR                                      ,{0},0},
    {TRAP_L2_PFC_PERROR                                      ,{0},0},
    {TRAP_L2_PFC_NERROR                                      ,{0},0},
    {TRAP_L3_PFC_PERROR                                      ,{0},0},
    {TRAP_L3_PFC_NERROR                                      ,{0},0},
    {TRAP_CAN_COMM_ERROR                                     ,{0},0},
    {TRAP_SYNCH_LINE_ERROR                                   ,{0},0},
    {TRAP_SYNCH_PULSE_ERROR                                  ,{0},0},
    {TRAP_HOST_LINE_ERROR                                    ,{0},0},
    {TRAP_MALE_CONN_ERROR                                    ,{0},0},
    {TRAP_FEMALE_CONN_ERROR                                  ,{0},0},
    {TRAP_PARALLEL_LINE_CONNERROR                            ,{0},0},
    {TRAP_BATT_CONNDIFF                                      ,{0},0},
    {TRAP_LINE_CONNDIFF                                      ,{0},0},
    {TRAP_BYPASS_CONNDIFF                                    ,{0},0},
    {TRAP_MODE_TYPEDIFF                                      ,{0},0},
    {TRAP_PARALLEL_INV_VOLTDIFF                              ,{0},0},
    {TRAP_PARALLEL_OUTPUT_FREQDIFF                           ,{0},0},
    {TRAP_BATT_CELL_OVERCHARGE                               ,{0},0},
    {TRAP_PARALLEL_OUTPUT_PARALLELDIFF                       ,{0},0},
    {TRAP_PARALLEL_OUTPUT_PHASEDIFF                          ,{0},0},
    {TRAP_PARALLEL_BYPASS_FORBIDDENDIFF                      ,{0},0},
    {TRAP_PARALLEL_CONVERTER_ENABLEDIFF                      ,{0},0},
    {TRAP_PARALLEL_BYPASS_FREQ_HIGHLOSSDIFF                  ,{0},0},
    {TRAP_PARALLEL_BYPASS_FREQ_LOWLOSSDIFF                   ,{0},0},
    {TRAP_PARALLEL_BYPASS_VOLT_HIGHLOSSDIFF                  ,{0},0},
    {TRAP_PARALLEL_BYPASS_VOLT_LOW_LOSSDIFF                  ,{0},0},
    {TRAP_PARALLEL_LINE_FREQ_HIGH_LOSSDIFF                   ,{0},0},
    {TRAP_TRAP_PARALLEL_LINE_FREQ_LOW_LOSSDIFF               ,{0},0},
    {TRAP_PARALLEL_LINE_VOLT_HIGH_LOSSDIFF                   ,{0},0},
    {TRAP_PARALLEL_LINE_VOLT_LOW_LOSSDIFF                    ,{0},0},
    {TRAP_LOCKED_IN_BYPASS                                   ,{0},0},
    {TRAP_THREE_PHASE_CURRENT_UNBALANCE                      ,{0},0},
    {TRAP_BATTERY_FUSE_BROKEN                                ,{0},0},
    {TRAP_INV_CURRENT_UNBALANCE                              ,{0},0},
    {TRAP_P1_CUTOFF_PREALARM                                 ,{0},0},
    {TRAP_BATTERY_REPLACE                                    ,{0},0},
    {TRAP_INPUT_PHAS_EERROR                                  ,{0},0},
    {TRAP_COVER_OF_MAINTAIN_SW_OPEN                          ,{0},0},
    {TRAP_PHASE_AUTO_ADAPTFAILED                             ,{0},0},
    {TRAP_UTILITY_EXTREMELY_UNBALANCED                       ,{0},0},
    {TRAP_BYPASS_UNSTABLE                                    ,{0},0},
    {TRAP_EEPROM_ERROR                                       ,{0},0},
    {TRAP_PARALLEL_PROTECT_WARNING                           ,{0},0},
    {TRAP_DISCHARGER_OVERLY                                  ,{0},0},
  };

//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//============================================================================
//****************************************************************************
//*
//* Function name: void SNMPAgent_Init(void)
//*
//****************************************************************************
//============================================================================
void SNMPAgent_Init(void)
{

  snmpStruct.SNMPEnable = 1;// Read from EEPROM

  snmpStruct.mibs[0] =(struct snmp_mib*) &upsmib;
  snmpStruct.mibs[1] = (struct snmp_mib*)&companymib;

  snmp_set_mibs((const struct snmp_mib **)snmpStruct.mibs, LWIP_ARRAYSIZE(snmpStruct.mibs));

}
//============================================================================
//****************************************************************************
//*
//* Function name: void SNMPAgent_StackInit(void)
//*
//****************************************************************************
//============================================================================
void SNMPAgent_StackInit(void)
{
    int itr=0;
    ip_addr_t dst;
    s32_t req_nr;
    UPS_INTERFACE_RAM_STRUCT *pDIStruct=&UPSInterfaceStruct;
    //dst.addr=0x01D6820A;

    dst.addr= Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.all;

    snmp_init();

    snmp_trap_dst_ip_set(0, &dst);
    snmp_trap_dst_enable(0, 1);

    snmp_send_inform_generic(SNMP_GENTRAP_COLDSTART, NULL, &req_nr);
    snmp_send_trap_generic(SNMP_GENTRAP_COLDSTART);

    snmpStruct.Continous_TrapFlag=FALSE;

    pDIStruct->Prev_WarningFlags =0x00000000;

    for(itr=0;itr<64;itr++)
    {
       trapStructArray[itr].trapsetting=Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[itr];
    }
}
//==============================================================================
//******************************************************************************
//*
//* Function name: static void SNMP_StartTrapTimer(uint8_t trapnum)
//*
//* Return: None
//*
//*
//* Description:
//*   Timer after Connection established and request received
//*
//*
//******************************************************************************
//==============================================================================
static void SNMPAgent_StartTrapTimer(uint8_t trapnum)
{
    trapStructArray[trapnum].startTimeConnectionTimeout= COMTimer_Get10MsecTick();
}
//==============================================================================
//******************************************************************************
//*
//* Function name: static BOOL SNMPTrap_CheckSentTimeout(uint8_t trapnum)
//*
//* Return:  TRUE- For Timeout
//*          FALSE- Timer is still running.
//*
//* Description:
//*   Timeout checking for each "Type of Trap" to send Trap Info
//*
//*
//******************************************************************************
//==============================================================================
static BOOL SNMPAgent_CheckTrapSentTimeout(uint8_t trapnum)
{
    // Just check if CurrentTick is within Tolerance zone.

    COM_TIMER_TICK currentTick=0;
    COM_TIMER_TICK nextTick=0;
    uint64_t totalTick=0;
    BOOL status=FALSE;

    totalTick =   trapStructArray[trapnum].startTimeConnectionTimeout + SNMP_TRAP_TIMEOUT;

    currentTick=COMTimer_Get10MsecTick();

    if(totalTick >= COM_TICK_MAX)
    {
        if ( currentTick <= trapStructArray[trapnum].startTimeConnectionTimeout)
        {
            // rollover detected
            nextTick = (COM_TIMER_TICK)(totalTick - SNMP_TRAP_TIMEOUT);
            if (currentTick >= nextTick )
            {
                status= TRUE;
            }
        }
    }
    else
    {
        // Check if trigger tick is within tolerance. Tolerance for timer latency (late ISR)
        // Upper limit: tick time + tolerance
        // Lower limit: tick time
        // Current time must be within Upper and Lower
        // Check rollover

        if ( currentTick >= trapStructArray[trapnum].startTimeConnectionTimeout)
        {
            // no rollover
            nextTick = (COM_TIMER_TICK) (trapStructArray[trapnum].startTimeConnectionTimeout+ SNMP_TRAP_TIMEOUT);

            if (currentTick >= nextTick)
            {
                status= TRUE;
            }

        }
        else  // if ( upper >= COM_TICK_MAX )
        {
            // rollover detected
          nextTick = (COM_TIMER_TICK) (SNMP_TRAP_TIMEOUT-(COM_TICK_MAX-trapStructArray[trapnum].startTimeConnectionTimeout)) ;

          if (currentTick >= nextTick)
           {
               status= TRUE;

           }

        }
    }
    return status;
}
//==============================================================================
//******************************************************************************
//*
//* Function name: SNMPAgent_CheckForTraps
//*
//* Return:  NONE
//*
//*
//* Description:
//*   Checking for traps, type of trap and timer configure for each type of trap
//*
//*
//******************************************************************************
//==============================================================================
static void SNMPAgent_CheckForTraps(UPS_INTERFACE_RAM_STRUCT  *pDIStruct)
{
	uint16_t itr=0;
    uint64_t currentcnt=0;
    bool tempTrapFlag=false;
    struct snmp_obj_id  trapoid = {SNMP_DEVICE_TRAP_OID_LEN, SNMP_DEVICE_ENTERPRISE_TRAPOID};
 	tempTrapFlag=false;

   if((pDIStruct->Prev_WarningFlags != UPSDb.warningFlags.warningAll)||(snmpStruct.Continous_TrapFlag==TRUE))
    {
       pDIStruct->Prev_WarningFlags = UPSDb.warningFlags.warningAll;

       for(itr=0;itr<UPS_TRAP_COUNT;itr++)
       {
              switch(((trapStructArray[itr].trapsetting.trapsetting) & (0x07)))
              {

                 case FALLING_EDGE_TRAP_ONCE:
                 {
                    currentcnt= (uint64_t)(0x00000001)<< itr;
                    if((UPSDb.warningFlags.warningAll & (currentcnt))==0)
                    {
                      if(trapStructArray[itr].trapsetting.Bits.trapSentFlag==FALSE)
                        {
                            snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                            SNMPAgent_StartTrapTimer(itr);
                            trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                        }

                    }
                 }
                 break;
                 case FALLIING_EDGE_INTRVAL_TRAP:
                 {
                     currentcnt= (uint64_t)(0x00000001)<< itr;
                     if((UPSDb.warningFlags.warningAll & (currentcnt))==0)
                     {
                         tempTrapFlag=TRUE;
                         if(((SNMPAgent_CheckTrapSentTimeout(itr))==TRUE) || (trapStructArray[itr].trapsetting.Bits.trapSentFlag==FALSE))
                         {
                             snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                             SNMPAgent_StartTrapTimer(itr);
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }
                     }
                 }
                 break;
                 case RISING_EDGE_TRAP_ONCE:
                 {
                    currentcnt= (uint64_t)(0x00000001)<< itr;
                    if(UPSDb.warningFlags.warningAll & (currentcnt))
                    {
                       if(trapStructArray[itr].trapsetting.Bits.trapSentFlag==FALSE)
                        {
                            snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                            SNMPAgent_StartTrapTimer(itr);
                            trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                        }
                        else
                        {
                            trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                        }
                    }
                 }
                 break;
                 case RISING_EDGE_INTRVAL_TRAP:
                 {
                     currentcnt= (uint64_t)(0x00000001)<< itr;
                     if(UPSDb.warningFlags.warningAll & (currentcnt))
                     {
                         tempTrapFlag=TRUE;
                         if(((SNMPAgent_CheckTrapSentTimeout(itr))==TRUE) || (trapStructArray[itr].trapsetting.Bits.trapSentFlag ==FALSE))
                         {
                             snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                             SNMPAgent_StartTrapTimer(itr);
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }

                     }
                 }
                 break;
                 case RISING_FALLING_EDGE_TRAP_ONCE:
                 {
                     currentcnt= (uint64_t)(0x00000001)<< itr;
                     if(UPSDb.warningFlags.warningAll & (currentcnt))
                     {
                         if(trapStructArray[itr].trapsetting.Bits.trapSentFlag==FALSE)
                         {
                             snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                             SNMPAgent_StartTrapTimer(itr);
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }
                         else
                         {
                             snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }
                     }
                     else
                     {
                         if(trapStructArray[itr].trapsetting.Bits.trapSentFlag==FALSE)
                         {
                             snmp_send_trap(&trapoid,SNMP_GENTRAP_ENTERPRISE_SPECIFIC,trapStructArray[itr].mappedOID,NULL);
                             SNMPAgent_StartTrapTimer(itr);
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }
                         else
                         {
                             trapStructArray[itr].trapsetting.Bits.trapSentFlag=TRUE;
                         }
                     }
                 }
                 break;
                 default:
                 //case TRAP_ISDISABLED:
                 //case TYPE_TRAP_ENABLED:
                 //case TRAP_ENABLED:
                 break;

            }
       }
    }
    if(tempTrapFlag==TRUE)
    {
        snmpStruct.Continous_TrapFlag=TRUE;
    }
    else
    {
        snmpStruct.Continous_TrapFlag=FALSE;
    }
}
//==============================================================================
//******************************************************************************
//*
//* Function name: void SNMP_Task()
//*
//* Return:   None
//*
//* Description:
//*
//*
//******************************************************************************
//==============================================================================
void SNMPAgent_Task()
{
    UPS_INTERFACE_RAM_STRUCT  *pDIStruct=&UPSInterfaceStruct;
    
	while(networkConfig.CurrentState != STATE_LWIP_WORKING)
	{
		Error_ResetTaskWatchdog(SNMP_TASK_ID);
		vTaskDelay(10);
	}
	
	SNMPAgent_StackInit();

    while(1)
    {
		Error_ResetTaskWatchdog(SNMP_TASK_ID);
		if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable == 1)
		{
		    SNMPAgent_CheckForTraps(pDIStruct);
		}
        vTaskDelay(10);

     }
}

//==============================================================================
// END OF SNMPAgent.c FIlE
//==============================================================================





