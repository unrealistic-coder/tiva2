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

#ifndef SNMP_AGENT_H
#define SNMP_AGENT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lwip/netif.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmpv3.h"
#include "lwip/apps/snmp_snmpv2_framework.h"
#include "lwip/apps/snmp_snmpv2_usm.h"
#include "snmpv3_dummy.h"
#include "private_mib.h"
#include "upsMIB2.h"
#include "upsPrivateMIB.h"
#include "Common.h"
#include "OnChipEEPROM.h"
typedef struct __attribute__((packed))
{
	struct snmp_mib *mibs[2];
	bool Continous_TrapFlag;
	bool SNMPEnable;

}SNMP_COMMON_STRUCT;

void SNMPAgent_Task();
void SNMPAgent_Init(void);


extern SNMP_COMMON_STRUCT snmpStruct;

//SNMP Traps and OIDs
#define     cmpChargerFailure                       4
#define     cmpbatteryfusebeingoc                   8
#define     cmpLowbattery                           9
#define     cmpSitefault                           11
#define     cmpPhasesequenceincorrect              12
#define     cmpPhasesequenceincorrectbypass        13
#define     cmpFanalarm                            14
#define     cmpEPOenabled                          15
#define     cmpUnabletotrunonUPS                   16
#define     cmpOvertemperaturealarm                17
#define     cmpInputfrequnstablebypass             18
#define     cmpL1inputfusenotwork                  20
#define     cmpNeutralnotConnected                 21
#define     cmpL2inputfusenotwork                  22
#define     cmpL3inputfusenotwork                  23
#define     cmpPositivePFCabnormalL1               24
#define     cmpNegativePFCabnormalL1               25
#define     cmpPositivePFCabnormalL2               26
#define     cmpNegativePFCabnormalL2               27
#define     cmpPositivePFCabnormalL3               28
#define     cmpNegativePFCabnormalL3               29
#define     cmpInvertervoloutofrange               36
#define     cmpInoutwiresoppositelyconnected       52
#define     cmpabnormalcanbuscommunication         53
#define     cmpabnormalsyncsignalcircuit           55
#define     cmpabnormalsyncpulsesignalcircuit      56
#define     cmpcurrent3punbalancedetected          57
#define     cmpintercurrentunbalance               59
#define     cmpbatterydisconnected                 60
#define     cmpbatteryovercharged                  62
#define     cmpfemaleconnectornotconnwell          65
#define     cmpmaleconnectornotconnwell            66
#define     cmplockingbypassA3COW30M               67
#define     cmpparallelcabledisconnected           68
#define     cmpsyncpulsecircuitfault               69
#define     cmpsyncsignalcircuitfalt               70
#define     cmphostsignalcircuitfault              71
#define     cmplowlosspointforvolinACmode          73
#define     cmphighlosspointforvolinACmode         74
#define     cmplowlosspointforfreqinACmode         75
#define     cmphighlosspointforfreqinACmode        76
#define     cmplowlosspointforvolinbypassmode      77
#define     cmphighlosspointforvolinbypassmode     78
#define     cmplowlosspointforfreqinbypassmode     79
#define     cmphighlosspointforfreqinbypassmode    80
#define     cmploadunbalanced                      81
#define     cmpoverloadalarm                       82
#define     cmpparallelnotconnectedwell            83
#define     cmpbatteryconnnotsonsistent            85
#define     cmpconverternotconsisstent             86
#define     cmpbypassnotallownotconsistent         87
#define     cmpinput3pcurrentunbalance             89
#define     cmpbypassconnnotconsistent             90
#define     cmpupsmodeltypesnotconsistent          93
#define     cmpbypassnotconsistent                 94
#define     cmpautorestartnotconsistent            96
#define     cmpBatteryReplace                      97
#define     cmpUPSShutAlarm                       106
#define     cmpUnknowStatus                       109
#define     cmpBatteryDischarging                 122
#define     cmpP1cutoffprealarm                   127
#define     cmpInputPhaseError                    128
#define     cmpMaintainSwitchOpenalarm            129

//Mapped OIDs
#define     TRAP_BATTERY_OPEN                              cmpbatterydisconnected                 //0
#define     TRAP_IP_N_LOSS                                 cmpNeutralnotConnected                 //1
#define     TRAP_IP_SITE_FAIL                              cmpSitefault                           //2
#define     TRAP_LINEPHASEERROR                            cmpPhasesequenceincorrect              //3
#define     TRAP_BYPASSPHASEERROR                          cmpPhasesequenceincorrectbypass        //4
#define     TRAP_BYPASSFREQUENCYUNSTABLE                   cmpInputfrequnstablebypass             //5
#define     TRAP_BATT_OVERCHARGE                           cmpbatteryovercharged                  //6
#define     TRAP_BATTLOW                                   cmpLowbattery                          //7
#define     TRAP_OVER_LOADWARNING                          cmpoverloadalarm                       //8
#define     TRAP_FAN_LOCKWARNING                           cmpFanalarm                            //9
#define     TRAP_EPO_ACTIVE                                cmpEPOenabled                           //10
#define     TRAP_TURN_ON_ABNORMAL                          cmpUnabletotrunonUPS                    //11
#define     TRAP_OVER_TEMPERATURE                          cmpOvertemperaturealarm                //12
#define     TRAP_CHARGER_FAIL                              cmpChargerFailure                       //13
#define     TRAP_REMOTE_SHUTDOWN                           cmpUPSShutAlarm                         //14
#define     TRAP_L1_IP_FUSE_FAIL                           cmpL1inputfusenotwork                    //15
#define     TRAP_L2_IP_FUSE_FAIL                           cmpL2inputfusenotwork                    //16
#define     TRAP_L3_IP_FUSE_FAIL                           cmpL3inputfusenotwork                    //17
#define     TRAP_L1_PFC_PERROR                             cmpPositivePFCabnormalL1                 //18
#define     TRAP_L1_PFC_NERROR                             cmpNegativePFCabnormalL1                 //19
#define     TRAP_L2_PFC_PERROR                             cmpPositivePFCabnormalL2                 //20
#define     TRAP_L2_PFC_NERROR                             cmpNegativePFCabnormalL2                 //21
#define     TRAP_L3_PFC_PERROR                             cmpPositivePFCabnormalL3                 //22
#define     TRAP_L3_PFC_NERROR                             cmpNegativePFCabnormalL3                 //23
#define     TRAP_CAN_COMM_ERROR                            cmpabnormalcanbuscommunication           //24
#define     TRAP_SYNCH_LINE_ERROR                          cmpsyncsignalcircuitfalt                 //25
#define     TRAP_SYNCH_PULSE_ERROR                         cmpsyncpulsecircuitfault                 //26
#define     TRAP_HOST_LINE_ERROR                           cmphostsignalcircuitfault                //27
#define     TRAP_MALE_CONN_ERROR                           cmpmaleconnectornotconnwell              //28
#define     TRAP_FEMALE_CONN_ERROR                         cmpfemaleconnectornotconnwell            //29
#define     TRAP_PARALLEL_LINE_CONNERROR                   cmpparallelcabledisconnected             //30
#define     TRAP_BATT_CONNDIFF                             cmpbatteryconnnotsonsistent              //31
#define     TRAP_LINE_CONNDIFF                             cmpInoutwiresoppositelyconnected         //32
#define     TRAP_BYPASS_CONNDIFF                           cmpbypassconnnotconsistent               //33
#define     TRAP_MODE_TYPEDIFF                             cmpupsmodeltypesnotconsistent            //34
#define     TRAP_PARALLEL_INV_VOLTDIFF                     cmpInvertervoloutofrange                 //35
#define     TRAP_PARALLEL_OUTPUT_FREQDIFF                  cmpabnormalsyncpulsesignalcircuit        //36
#define     TRAP_BATT_CELL_OVERCHARGE                      cmpbatteryovercharged                    //37
#define     TRAP_PARALLEL_OUTPUT_PARALLELDIFF              cmploadunbalanced                        //38
#define     TRAP_PARALLEL_OUTPUT_PHASEDIFF                 cmpabnormalsyncsignalcircuit             //39
#define     TRAP_PARALLEL_BYPASS_FORBIDDENDIFF             cmpbypassnotallownotconsistent           //40
#define     TRAP_PARALLEL_CONVERTER_ENABLEDIFF             cmpconverternotconsisstent               //41
#define     TRAP_PARALLEL_BYPASS_FREQ_HIGHLOSSDIFF         cmphighlosspointforfreqinbypassmode      //42
#define     TRAP_PARALLEL_BYPASS_FREQ_LOWLOSSDIFF          cmplowlosspointforfreqinbypassmode       //43
#define     TRAP_PARALLEL_BYPASS_VOLT_HIGHLOSSDIFF         cmphighlosspointforvolinbypassmode       //44
#define     TRAP_PARALLEL_BYPASS_VOLT_LOW_LOSSDIFF         cmplowlosspointforvolinbypassmode        //45
#define     TRAP_PARALLEL_LINE_FREQ_HIGH_LOSSDIFF          cmphighlosspointforfreqinACmode          //46
#define     TRAP_TRAP_PARALLEL_LINE_FREQ_LOW_LOSSDIFF      cmplowlosspointforfreqinACmode           //47
#define     TRAP_PARALLEL_LINE_VOLT_HIGH_LOSSDIFF          cmphighlosspointforvolinACmode           //48
#define     TRAP_PARALLEL_LINE_VOLT_LOW_LOSSDIFF           cmplowlosspointforvolinACmode            //49
#define     TRAP_LOCKED_IN_BYPASS                          cmplockingbypassA3COW30M                 //50
#define     TRAP_THREE_PHASE_CURRENT_UNBALANCE             cmpcurrent3punbalancedetected            //51
#define     TRAP_BATTERY_FUSE_BROKEN                       cmpbatteryfusebeingoc                    //52
#define     TRAP_INV_CURRENT_UNBALANCE                     cmpintercurrentunbalance                 //53
#define     TRAP_P1_CUTOFF_PREALARM                        cmpP1cutoffprealarm                      //54
#define     TRAP_BATTERY_REPLACE                           cmpBatteryReplace                        //55
#define     TRAP_INPUT_PHAS_EERROR                         cmpInputPhaseError                       //56
#define     TRAP_COVER_OF_MAINTAIN_SW_OPEN                 cmpMaintainSwitchOpenalarm               //57
#define     TRAP_PHASE_AUTO_ADAPTFAILED                    cmpautorestartnotconsistent              //58
#define     TRAP_UTILITY_EXTREMELY_UNBALANCED              cmpinput3pcurrentunbalance               //59
#define     TRAP_BYPASS_UNSTABLE                           cmpbypassnotconsistent                   //60
#define     TRAP_EEPROM_ERROR                              cmpUnknowStatus                          //61
#define     TRAP_PARALLEL_PROTECT_WARNING                  cmpparallelnotconnectedwell              //62
#define     TRAP_DISCHARGER_OVERLY                         cmpBatteryDischarging                    //63


#define SNMP_TRAP_TIMEOUT                               6000
#define UPS_TRAP_COUNT                                  64
#define SNMP_DEVICE_ENTERPRISE_TRAPOID                  {1, 3, 6, 1, 4, 1, 21111,1,1,12}
#define SNMP_DEVICE_TRAP_OID_LEN                        10

#define  TRAP_ISDISABLED                                0
#define  TYPE_TRAP_ENABLED                              1
#define  FALLING_EDGE_TRAP_ONCE                         2
#define  FALLIING_EDGE_INTRVAL_TRAP                     3
#define  RISING_EDGE_TRAP_ONCE                          4
#define  RISING_EDGE_INTRVAL_TRAP                       5
#define  RISING_FALLING_EDGE_TRAP_ONCE                  6
#define  TRAP_ENABLED                                   7

typedef struct __attribute__((packed))
{
     uint16_t mappedOID;
    UPS_SNMP_TRAP_SETTING trapsetting;
    uint32_t startTimeConnectionTimeout;
}SNMP_TRAP_STRUCT;

#ifdef __cplusplus
}
#endif

#endif /* SNMP_AGENT_H */
