//****************************************************************************
//*
//*
//* File: COMModbusTCPServer.c
//*
//* Synopsis: Modbus TCP Server application over the top of LWIP libarary
//*
//*
//****************************************************************************


//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <EthernetTask.h>
#include "ModbusTCPServer.h"
#include <ErrorHandler.h>
#include <UPSInterface.h>
#include "OnChipEEPROM.h"
#include "HibernateCalendar.h"
#include "Common.h"
#include "websocket.h"
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
MODBUS_TCP_RAM_STRUCT modbusTCPStruct;

MODBUS_TCP_RAM_STRUCT *pModbusStruct = (MODBUS_TCP_RAM_STRUCT *) &modbusTCPStruct;

MODBUSTCP_MAP_STRUCT ModbusMAPUPSDt[MODBUS_NO_OF_GROUPS]=
{
      {START_ADDRESS_OF_WARNING_ITEM,END_ADDRESS_OF_WARNING_ITEM,MAX_LENGHT_OF_WARNING_ITEM,FALSE},
      {START_ADDRESS_OF_CAPSET_CAPLIST,END_ADDRESS_OF_CAPSET_CAPLIST,MAX_LENGHT_OF_CAPSET_CAPLIST,FALSE},
      {START_ADDRESS_OF_CTRLITEM,END_ADDRESS_OF_CTRLITEM,MAX_LENGHT_OF_RESCTRL,FALSE},
      {START_ADDRESS_OF_RESCTRL,END_ADDRESS_OF_RESCTRL,MAX_LENGHT_OF_RESCTRL,FALSE},
      {START_ADDRESS_OF_SETTPARM1,END_ADDRESS_OF_SETTPARM1,MAX_LENGHT_OF_SETTPARM1,FALSE},
      {START_ADDRESS_OF_SETTPARM2,END_ADDRESS_OF_SETTPARM2,MAX_LENGHT_OF_SETTPARM2,FALSE},
      {START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT,END_ADDRESS_OF_UPSWORK_STAT1_UPS_STATUS,MAX_LENGHT_OF_UPSWORK_STAT1,FALSE},
      {START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS,END_ADDRESS_OF_UPSWORK_STAT2_INP_T_VOLT,MAX_LENGHT_OF_UPSWORK_STAT2,FALSE},
      {START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT,END_ADDRESS_OF_UPSBATTINFO_BATT_REMN_TIME,5,FALSE},
      {START_ADDRESS_OF_UPSWORK_MODE ,END_ADDRESS_OF_UPSWORK_MODE, MAX_LENGHT_OF_UPSWORK_MODE ,FALSE},
      {START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE,END_ADDRESS_OF_UPSFAULT_INFO_UPS_RUN_STAT_BEFORE_FAULT,MAX_LENGHT_OF_UPSFAULT_INFO,FALSE},
      {START_ADDRESS_OF_OUTPUTSOCK1,END_ADDRESS_OF_OUTPUTSOCK,MAX_LENGHT_OF_OUTPUTSOCK,FALSE},
      {START_ADDRESS_OF_OUTPUTSOCK2,END_ADDRESS_OF_OUTPUT_SOCK,MAX_LENGHT_OF_OUTPUT_SOCK,FALSE},
      {START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP,END_ADDRESS_OF_LOSSPNTDATA_BYPS_VOLT_LLP,MAX_LENGHT_OF_LOSSPNTDATA,FALSE},
      {START_ADDRESS_OF_SETTPARM_SUCC_FAIL,END_ADDRESS_OF_SETTPARM_SUCC_FAIL,MAX_LENGHT_OF_SETTPARM_SUCC_FAIL,FALSE},
      {START_ADDRESS_OF_REMOTESHUTDN,END_ADDRESS_OF_REMOTESHUTDN_RESTORE,MAX_LENGHT_OF_REMOTESHUTDN,FALSE},
      {START_ADDRESS_OF_REMOTESHUTDN_TEST,END_ADDRESS_OF_REMOTESHUTDN_TEST,MAX_LENGHT_OF_REMOTESHUTDN_TEST,FALSE},
      {START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ,END_ADDRESS_OF_CPUINFO,MAX_LENGHT_OF_CPUINFO,FALSE},
      {START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM,END_ADDRESS_OF_UPSMODEL,MAX_LENGHT_OF_UPSMODEL,FALSE},
      {START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT,END_ADDRESS_OF_UPSRATINGINFO_OUTP_VOLT,MAX_LENGHT_OF_UPSRATINGINFO,FALSE},
      {START_ADDRESS_OF_MODBUS_DATETIME,END_ADDRESS_OF_MODBUS_DATETIME,MAX_LENGTH_OF_MODBUS_DATETIME,FALSE},
};

//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------

/*************************************Function Definitions Of Reading The Modbus Data ****************************************/

//****************************************************************************
//*
//* Function name: ModbusTCPSever_UpdateConnectionTimeout
//*
//* Return: None
//*
//*
//* Description:
//*   Update timeout counter for Modbus TCP server connection with client through 
//*   webserver.
//*
//****************************************************************************
//==============================================================================

void ModbusTCPServer_UpdateConnectionTimeout(uint16_t Timout)
{
    uint8_t count = 0;
    for (count = 0; count < MBTCP_MAX_CONNECTION; count++)
    {
        modbusTCPStruct.modbusConnection[count].connectionTimeoutTime = Timout;
    }
}


//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadWarningStatus
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t  ModbusTCPServer_ReadWarningStatus(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus= MODBUS_EXCPT__SUCCESS ;
    uint8_t itr=0;
    uint8_t i=0;
    MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
    for(i=0;i<(pComRequest->length);i++)
    {
        switch(pComRequest->startAddress + i)
        {
            case START_ADDRESS_OF_WARNING_ITEM:

             modbus16_bit_datastruct.Bits.bit0=0;// Reserved bit
             modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit8=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit9=UPSDb.warningFlags.Bits.L1_IP_Fuse_Fail;
             modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit11=UPSDb.warningFlags.Bits.ChargerFail;

             modbus16_bit_datastruct.Bits.bit12= UPSDb.warningFlags.Bits.BattLow;
             modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit15=UPSDb.warningFlags.Bits.LinePhaseError;

             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
             break;
            case SECOND_ADDRESS_OF_WARNGITEM:

             modbus16_bit_datastruct.Bits.bit0=0;  // Reserved bit
             modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit7=UPSDb.warningFlags.Bits.BypassConnDiff;

             modbus16_bit_datastruct.Bits.bit8=UPSDb.warningFlags.Bits.LineConnDiff;
             modbus16_bit_datastruct.Bits.bit9= 0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit12=0; // Reserved data
             modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit15=0; // Reserved bit

             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
             break;
            case THIRD_ADDRESS_OF_WARNGITEM:

             modbus16_bit_datastruct.Bits.bit0=UPSDb.warningFlags.Bits.BatteryOpen;
             modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit3=UPSDb.warningFlags.Bits.INVCurrentUnbalance;

             modbus16_bit_datastruct.Bits.bit4=UPSDb.warningFlags.Bits.BatteryFuseBroken;
             modbus16_bit_datastruct.Bits.bit5=UPSDb.warningFlags.Bits.ThreePhaseCurrentUnbalance;
             modbus16_bit_datastruct.Bits.bit6=UPSDb.warningFlags.Bits.LockedInBypass;
             modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit8= 0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit9= 0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit12=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit15=0; // Reserved bit

             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
             break;
            case FOURTH_ADDRESS_OF_WARNGITEM:

             modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit6=UPSDb.warningFlags.Bits.CoverOfMaintainSWopen;
             modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

             modbus16_bit_datastruct.Bits.bit8=UPSDb.warningFlags.Bits.OverTemperature;
             modbus16_bit_datastruct.Bits.bit9= 0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit10=UPSDb.warningFlags.Bits.EPOActive;
             modbus16_bit_datastruct.Bits.bit11=UPSDb.warningFlags.Bits.FanLockWarning;

             modbus16_bit_datastruct.Bits.bit12=UPSDb.warningFlags.Bits.OverloadWarning;
             modbus16_bit_datastruct.Bits.bit13=UPSDb.warningFlags.Bits.BattOverCharge;
             modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
             modbus16_bit_datastruct.Bits.bit15=UPSDb.warningFlags.Bits.IP_N_Loss;

             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
             DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
             break;
            default :
             DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
             break;
         }
    }
    return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadCapabilitySetting_ListData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t  ModbusTCPServer_ReadCapabilitySetting_ListData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  uint8_t itr=0;
  uint8_t i=0;
  MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
  for(i=0;i<pComRequest->length;i++)
  {
    switch(pComRequest->startAddress + i)
    {
      case START_ADDRESS_OF_CAPSET_CAPLIST:
          modbus16_bit_datastruct.Bits.bit0=UPSDb.settingFlags.upsFlagsBits.energySaving;
          modbus16_bit_datastruct.Bits.bit1=UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
          modbus16_bit_datastruct.Bits.bit2=UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning;
          modbus16_bit_datastruct.Bits.bit3=UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff;

          modbus16_bit_datastruct.Bits.bit4=UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn;
          modbus16_bit_datastruct.Bits.bit5=UPSDb.settingFlags.upsFlagsBits.shortRestart3Times;
          modbus16_bit_datastruct.Bits.bit6=UPSDb.settingFlags.upsFlagsBits.bypassForbiding;
          modbus16_bit_datastruct.Bits.bit7=UPSDb.settingFlags.upsFlagsBits.codeStart;

          modbus16_bit_datastruct.Bits.bit8=UPSDb.settingFlags.upsFlagsBits.battLowProtect;
          modbus16_bit_datastruct.Bits.bit9= UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect;
          modbus16_bit_datastruct.Bits.bit10=UPSDb.settingFlags.upsFlagsBits.autoRestart;
          modbus16_bit_datastruct.Bits.bit11=UPSDb.settingFlags.upsFlagsBits.SetHotStandby;

          modbus16_bit_datastruct.Bits.bit12=UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect;
          modbus16_bit_datastruct.Bits.bit13=UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck;
          modbus16_bit_datastruct.Bits.bit14=UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning;
          modbus16_bit_datastruct.Bits.bit15=UPSDb.settingFlags.upsFlagsBits.audibleAlarm;

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
      case  SECOND_ADDRESS_OF_CAPSET_CAPLIST:

          modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit8=0; // Reserved data
          modbus16_bit_datastruct.Bits.bit9= 0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit12=UPSDb.settingFlags.upsFlagsBits.converterMode;
          modbus16_bit_datastruct.Bits.bit13=UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
          modbus16_bit_datastruct.Bits.bit14=UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode;
          modbus16_bit_datastruct.Bits.bit15=UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode;

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
      case  THIRD_ADDRESS_OF_CAPSET_CAPLIST:

          modbus16_bit_datastruct.Bits.bit0=UPSDb.settingFlags.upsFlagsBits.energySaving;
          modbus16_bit_datastruct.Bits.bit1=UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
          modbus16_bit_datastruct.Bits.bit2=UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning;
          modbus16_bit_datastruct.Bits.bit3=UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff;

          modbus16_bit_datastruct.Bits.bit4=UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn;
          modbus16_bit_datastruct.Bits.bit5=UPSDb.settingFlags.upsFlagsBits.shortRestart3Times;
          modbus16_bit_datastruct.Bits.bit6=UPSDb.settingFlags.upsFlagsBits.bypassForbiding;
          modbus16_bit_datastruct.Bits.bit7=UPSDb.settingFlags.upsFlagsBits.codeStart;

          modbus16_bit_datastruct.Bits.bit8=UPSDb.settingFlags.upsFlagsBits.battLowProtect;
          modbus16_bit_datastruct.Bits.bit9= UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect;
          modbus16_bit_datastruct.Bits.bit10=UPSDb.settingFlags.upsFlagsBits.autoRestart;
          modbus16_bit_datastruct.Bits.bit11=UPSDb.settingFlags.upsFlagsBits.SetHotStandby;

          modbus16_bit_datastruct.Bits.bit12=UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect;
          modbus16_bit_datastruct.Bits.bit13=UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck;
          modbus16_bit_datastruct.Bits.bit14=UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning;
          modbus16_bit_datastruct.Bits.bit15=UPSDb.settingFlags.upsFlagsBits.audibleAlarm;

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
      case  FOURTH_ADDRESS_OF_CAPSET_CAPLIST:

          modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit3=0; // Reserved  bit

          modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit8=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit9=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit12=UPSDb.settingFlags.upsFlagsBits.converterMode;
          modbus16_bit_datastruct.Bits.bit13=UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
          modbus16_bit_datastruct.Bits.bit14=UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode;
          modbus16_bit_datastruct.Bits.bit15=UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode;

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
      default :
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
     }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadControlItem
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadControlItem(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  uint8_t itr=0;
  uint8_t i=0;
  MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
  for(i=0;i<pComRequest->length;i++)
  {
    switch(pComRequest->startAddress + i)
    {
      case START_ADDRESS_OF_CTRLITEM:

          modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
        //  modbus16_bit_datastruct.Bits.bit5=0; // Data not supported
          modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
//          modbus16_bit_datastruct.Bits.bit7=0; // Data not supported
//
//          modbus16_bit_datastruct.Bits.bit8=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit9=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit10=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit11=0; // Data not supported
//
//          modbus16_bit_datastruct.Bits.bit12=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit13=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit14=0; // Data not supported
//          modbus16_bit_datastruct.Bits.bit15=0; // Data not supported
//
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
      default :
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
    }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name: ModbusTCPServer_ReadResultOfControlData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadResultOfControlData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
   for(i=0;i<pComRequest->length;i++)
   {
      switch(pComRequest->startAddress + i)
      {
         case START_ADDRESS_OF_RESCTRL:

           modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
           modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
           modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
           modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

           modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
          // modbus16_bit_datastruct.Bits.bit5=0; // Data not supported
           modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
//           modbus16_bit_datastruct.Bits.bit7=0; // Data not supported
//
//           modbus16_bit_datastruct.Bits.bit8=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit9=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit10=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit11=0; // Data not supported
//
//           modbus16_bit_datastruct.Bits.bit12=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit13=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit14=0; // Data not supported
//           modbus16_bit_datastruct.Bits.bit15=0; // Data not supported

           DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
           DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
           break;
         default:
           DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
       }
    }
   return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadSettingParamToDfltVal1
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadSettingParamToDfltVal1(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t  DBStatus= MODBUS_EXCPT__SUCCESS;
  uint8_t itr=0;
  uint8_t i=0;
  MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
  for(i=0;i<pComRequest->length;i++)
  {
    switch(pComRequest->startAddress + i)
    {
        case  START_ADDRESS_OF_SETTPARM1:

          modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit8=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit9=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit12=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
         // modbus16_bit_datastruct.Bits.bit15=0; //// Default values setting Bit

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
         default:
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
      }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name: ModbusTCPServer_ReadSettingParamToDfltVal2
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t   ModbusTCPServer_ReadSettingParamToDfltVal2(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  uint8_t itr=0;
  uint8_t i=0;
  MODBUS_16BIT_STRUCT modbus16_bit_datastruct;
  for(i=0;i<pComRequest->length;i++)
  {
    switch(pComRequest->startAddress + i)
    {
       case  START_ADDRESS_OF_SETTPARM2 :

          modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit8=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit9=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

          modbus16_bit_datastruct.Bits.bit12=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
          modbus16_bit_datastruct.Bits.bit14=0; // Reserved bit
        //  modbus16_bit_datastruct.Bits.bit15=0; // Default values setting Status

          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
          DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
          break;
       default:
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
     }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSWorkingStatus1
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSWorkingStatus1(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
       case START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT:

           DataBuffer[itr++]= highByte(UPSDb.InputVolt);
           DataBuffer[itr++]= lowByte(UPSDb.InputVolt);
           break;
       case ADDRESS_OF_INPUT_FREQUENCY :

           DataBuffer[itr++]= highByte(UPSDb.InputFreq);
           DataBuffer[itr++]= lowByte(UPSDb.InputFreq);
           break;

       case ADDRESS_OF_OUTPUT_VOLTAGE:

           DataBuffer[itr++]= highByte(UPSDb.OutputVolt);
           DataBuffer[itr++]= lowByte(UPSDb.OutputVolt);
           break;

       case ADDRESS_OF_OUTPUT_FREQUENCY:

           DataBuffer[itr++]= highByte(UPSDb.OutputFreq);
           DataBuffer[itr++]= lowByte(UPSDb.OutputFreq);
           break;

       case ADDRESS_OF_OUTPUT_CURRENT:

           DataBuffer[itr++]= highByte(UPSDb.OutputCurr);
           DataBuffer[itr++]= lowByte(UPSDb.OutputCurr);
           break;
       case ADDRESS_OF_OUTPUT_LOAD_PERCENT:

           DataBuffer[itr++]= highByte(UPSDb.OutputLoadPercent);
           DataBuffer[itr++]= lowByte(UPSDb.OutputLoadPercent);
           break;

       case ADDRESS_OF_P_BUS_VOLTAGE:

           DataBuffer[itr++]= highByte(UPSDb.PBusVolt);
           DataBuffer[itr++]= lowByte(UPSDb.PBusVolt);
           break;

       case  ADDRESS_OF_N_BUS_VOLTAGE:

           DataBuffer[itr++]= highByte(UPSDb.NBusVolt);
           DataBuffer[itr++]= lowByte(UPSDb.NBusVolt);
           break;

       case ADDRESS_OF_P_BATT_VOLTAGE:

           DataBuffer[itr++]= highByte(UPSDb.PBattVolt);
           DataBuffer[itr++]= lowByte(UPSDb.PBattVolt);
           break;

       case  ADDRESS_OF_N_BATT_VOLTAGE :

           DataBuffer[itr++]= highByte(UPSDb.NBattVolt);
           DataBuffer[itr++]= lowByte(UPSDb.NBattVolt);
           break;

       case  ADDRESS_OF_MAX_TEMP_DETECT_POINTERS :

           DataBuffer[itr++]= highByte(UPSDb.MaxTemp);
           DataBuffer[itr++]= lowByte(UPSDb.MaxTemp);
           break;

       case ADDRESS_OF_UPS_STATUS0 :

           DataBuffer[itr++]= highByte(UPSDb.upsStatus.statusAll);
           DataBuffer[itr++]= lowByte(UPSDb.upsStatus.statusAll);
           break;

       case ADDRESS_OF_UPS_STATUS1:
           DataBuffer[itr++]= 0;
           DataBuffer[itr++]= 0;
           break;

       default:
           DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
       }
    }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSWorkingStatus2
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSWorkingStatus2(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case  START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_LINE_VOLT_OF_RT:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_LINE_VOLT_OF_ST:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_INP_R_VOLT:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_INP_S_VOLT:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_INP_T_VOLT:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        default:
            DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
            break;
      }
    }
   return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSBattInfoDT
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSBattInfoDT(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT:

            DataBuffer[itr++]= highByte(UPSDb.BattVolt);
            DataBuffer[itr++]= lowByte(UPSDb.BattVolt);
            break;

        case ADDRESS_OF_BATT_PIECE_NUM :

            DataBuffer[itr++]= highByte(UPSDb.BattPieceNumber);
            DataBuffer[itr++]= lowByte(UPSDb.BattPieceNumber);
            break;

        case ADDRESS_OF_BATT_GROUP_NUM:

            DataBuffer[itr++]= highByte(UPSDb.BattgroupNumber);
            DataBuffer[itr++]= lowByte(UPSDb.BattgroupNumber);
            break;

        case ADDRESS_OF_BATT_CAPACITY :

            DataBuffer[itr++]= highByte(UPSDb.BattCapacity);
            DataBuffer[itr++]= lowByte(UPSDb.BattCapacity);
            break;

        case ADDRESS_OF_BATT_REMN_TIME :

            DataBuffer[itr++]= highByte(UPSDb.BattRemainTime);
            DataBuffer[itr++]= lowByte(UPSDb.BattRemainTime);
            break;

        default:
            DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
            break;
      }
    }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSWorkingModeDT
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSWorkingModeDT(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
      switch(pComRequest->startAddress + i)
      {
         case START_ADDRESS_OF_UPSWORK_MODE:

             DataBuffer[itr++]= UPSDb.UPSMode[1];//highByte(UPSDb.UPSMode);
             DataBuffer[itr++]= UPSDb.UPSMode[0];//lowByte(UPSDb.UPSMode);
             break;

          default:
             DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
             break;
       }
    }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSFaultInfo
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSFaultInfo(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;

   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.FaultType);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.FaultType);
            break;

        case  ADDRESS_OF_BATT_VOLT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.BattVoltBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.BattVoltBeforeFault);
            break;

        case  ADDRESS_OF_INP_FRQNCY_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.InputFreqBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.InputFreqBeforeFault);
            break;

        case  ADDRESS_OF_INP_VOLT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.InputVoltBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.InputVoltBeforeFault);
            break;

        case  ADDRESS_OF_INVTR_OUTP_FRQNCY_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.OutputFreqBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.OutputFreqBeforeFault);
            break;

        case  ADDRESS_OF_INVTR_OUTP_VOLT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.OutputVoltBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.OutputVoltBeforeFault);
            break;

        case  ADDRESS_OF_N_BUS_VOLT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.NBusVoltBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.NBusVoltBeforeFault);
            break;

        case  ADDRESS_OF_P_BUS_VOLT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.PBusVoltageBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.PBusVoltageBeforeFault);
            break;

        case  ADDRESS_OF_OUTP_LOAD_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.OutputLoadBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.OutputLoadBeforeFault);
            break;

        case  ADDRESS_OF_OUTP_CURRENT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.OutputCurrentBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.OutputCurrentBeforeFault);
            break;

        case  ADDRESS_OF_TEMP_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.TempBeforeFault);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.TempBeforeFault);
            break;

        case  ADDRESS_OF_UPS_RUN_STAT_BEFORE_FAULT:

            DataBuffer[itr++]= highByte(UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll);
            DataBuffer[itr++]= lowByte(UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll);
            break;

        default:
            DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
            break;
      }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadOutputSocketStat1
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadOutputSocketStat1(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t  DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   MODBUS_16BIT_STRUCT modbus16_bit_datastruct;

   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case START_ADDRESS_OF_OUTPUTSOCK1:

            modbus16_bit_datastruct.Bits.bit0=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit1=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit2=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit3=0; // Reserved bit

            modbus16_bit_datastruct.Bits.bit4=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit5=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit6=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit7=0; // Reserved bit

            modbus16_bit_datastruct.Bits.bit8=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit9=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit10=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit11=0; // Reserved bit

            modbus16_bit_datastruct.Bits.bit12=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit13=0; // Reserved bit
            modbus16_bit_datastruct.Bits.bit14=0; // Data not supported
            modbus16_bit_datastruct.Bits.bit15=0; // Data not supported

            DataBuffer[itr++]= modbus16_bit_datastruct.bytes[1];
            DataBuffer[itr++]= modbus16_bit_datastruct.bytes[0];
            break;
         default:
            DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
            break;
      }
    }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadOutputSocketStat2
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadOutputSocketStat2(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
  uint8_t itr=0;
  uint8_t i=0;
  for(i=0;i<pComRequest->length;i++)
  {
    switch(pComRequest->startAddress + i)
    {
      case START_ADDRESS_OF_OUTPUTSOCK2:

         DataBuffer[itr++]= 0; // Data not supported
         DataBuffer[itr++]= 0;
         break;

      default:
         DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
         break;
     }
   }
  return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSLossPtData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSLossPtData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP:

            DataBuffer[itr++]= highByte(UPSDb.voltHighLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.voltHighLossPt);
            break;

        case ADDRESS_OF_HIGH_EFF_MODE_VOLT_LLP:

            DataBuffer[itr++]= highByte(UPSDb.voltLowLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.voltLowLossPt);
            break;

        case THIRD_ADDRESS_OF_LOSSPNTDATA:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case FOURTH_ADDRESS_OF_LOSSPNTDATA:

            DataBuffer[itr++]= 0; // Data not supported
            DataBuffer[itr++]= 0;
            break;

        case ADDRESS_OF_BYPS_FREQ_HLP:

            DataBuffer[itr++]= highByte(UPSDb.freqHighLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.freqHighLossPt);
            break;

        case ADDRESS_OF_BYPS_FREQ_LLP:

            DataBuffer[itr++]= highByte(UPSDb.freqLowLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.freqLowLossPt);
            break;

        case ADDRESS_OF_BYPS_VOLT_HLP:

            DataBuffer[itr++]= highByte(UPSDb.bypassVoltHighLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.bypassVoltHighLossPt);
            break;

        case ADDRESS_OF_BYPS_VOLT_LLP:

            DataBuffer[itr++]= highByte(UPSDb.bypassVoltLowLossPt);
            DataBuffer[itr++]= lowByte(UPSDb.bypassVoltLowLossPt);
            break;

        default:
            DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
            break;
      }
   }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadSettingParamSuccFailDt
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadSettingParamSuccFailDt(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
       case START_ADDRESS_OF_SETTPARM_SUCC_FAIL:

           DataBuffer[itr++]= pModbusStruct->SettingParamSuccFailResults.bytes[1];
           DataBuffer[itr++]= pModbusStruct->SettingParamSuccFailResults.bytes[0];
           break;

       default:
           DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
      }
    }
    return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadRemoteShutdwDt
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadRemoteShutdwDt(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
      switch(pComRequest->startAddress + i)
      {
         case START_ADDRESS_OF_REMOTESHUTDN:

             DataBuffer[itr++]= 0;// Data not supported
             DataBuffer[itr++]= 0;
             break;

         case ADDRESS_OF_TEST_FOR_SPECIF_TIME:

             DataBuffer[itr++]= 0;// Data not supported
             DataBuffer[itr++]= 0;
             break;

         case ADDRESS_OF_SHUTDN_RESTORE_N:

             DataBuffer[itr++]= 0;// Data not supported
             DataBuffer[itr++]= 0;
             break;

         case ADDRESS_OF_SHUTDN_RESTORE_S:

             DataBuffer[itr++]= 0;// Data not supported
             DataBuffer[itr++]= 0;

             break;
         case ADDRESS_OF_SHUTDN_RESTORE_R:


             DataBuffer[itr++]= 0;// Data not supported
             DataBuffer[itr++]= 0;
             break;
         default:
             DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
             break;
       }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadRemoteShutdwTestDt
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadRemoteShutdwTestDt(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
      switch(pComRequest->startAddress + i)
      {
         case START_ADDRESS_OF_REMOTESHUTDN_TEST:

             DataBuffer[itr++]= pModbusStruct->shutdownRestoreResults.bytes[1];
             DataBuffer[itr++]= pModbusStruct->shutdownRestoreResults.bytes[0];
             break;

         default:
             DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
             break;
       }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadCPUInfo
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadCPUInfo(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
    uint8_t itr=0;
    uint8_t i=0;
    for(i=0;i<pComRequest->length;i++)
    {
        switch(pComRequest->startAddress + i)
        {
            case START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ:
                DataBuffer[itr++]= highByte(UPSDb.protocolID);
                DataBuffer[itr++]= lowByte(UPSDb.protocolID);
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER0:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[0];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[1];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER1:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[2];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[3];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER2:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[4];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[5];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER3:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[6];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[7];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER4:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[8];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[9];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER5:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[10];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[11];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER6:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[12];
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[13];
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER7:
                DataBuffer[itr++]=UPSDb.UPSCPUFwVersion[14];
                DataBuffer[itr++]=0;
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER8:
                DataBuffer[itr++]=0;
                DataBuffer[itr++]=0;
                break;
            case ADDRESS_OF_MAIN_CPU_FIRMWR_VER9:
                DataBuffer[itr++]=0;
                DataBuffer[itr++]=0;
                break;
            default:
                DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
                break;
        }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadCPUModelDt
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadCPUModelDt(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
    uint8_t itr=0;
    uint8_t i=0;

    for(i=0;i<pComRequest->length;i++)
    {
        switch(pComRequest->startAddress + i)
        {
            case START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM:
                DataBuffer[itr++]= highByte(UPSDb.BattPieceNumber);
                DataBuffer[itr++]= lowByte(UPSDb.BattPieceNumber);
                break;

            case ADDRESS_OF_BATT_STAND_VOLT_PER_UNIT:
                DataBuffer[itr++]= highByte(UPSDb.BattVoltPerUnit);
                DataBuffer[itr++]= lowByte(UPSDb.BattVoltPerUnit);
                break;

            case  ADDRESS_OF_INP_PHASE:
                DataBuffer[itr++]= highByte(UPSDb.supportedPhase.Bytes.InputPhase);
                DataBuffer[itr++]= lowByte(UPSDb.supportedPhase.Bytes.InputPhase);
                break;

            case  ADDRESS_OF_OUTP_PHASE:
                DataBuffer[itr++]= highByte(UPSDb.supportedPhase.Bytes.OutputPhase);
                DataBuffer[itr++]= lowByte(UPSDb.supportedPhase.Bytes.OutputPhase);
                break;

            case ADDRESS_OF_NOM_INP_VOLT:
                DataBuffer[itr++]= highByte(UPSDb.NominalInputVolt);
                DataBuffer[itr++]= lowByte(UPSDb.NominalInputVolt);
                break;

            case  ADDRESS_OF_NOM_OUTP_VOLT:
                DataBuffer[itr++]= highByte(UPSDb.NominalOutputVolt);
                DataBuffer[itr++]= lowByte(UPSDb.NominalOutputVolt);
                break;

            case  ADDRESS_OF_OUTP_PF:   // PF :- Power Factor
                DataBuffer[itr++]= highByte(UPSDb.OutputPWRFactor);
                DataBuffer[itr++]= lowByte(UPSDb.OutputPWRFactor);
                break;

            case  ADDRESS_OF_OUTP_RATED_VA_LOW:
                DataBuffer[itr++]= highByte((UPSDb.RatedOutputVA>>16)&0x0000FFFF);
                DataBuffer[itr++]= lowByte((UPSDb.RatedOutputVA>>16)&0x0000FFFF);
                break;

            case  ADDRESS_OF_OUTP_RATED_VA_HIGH:
                DataBuffer[itr++]= highByte(UPSDb.RatedOutputVA & 0x0000FFFF);
                DataBuffer[itr++]= lowByte(UPSDb.RatedOutputVA & 0x0000FFFF);
                break;

            case ADDRESS_OF_UPSMODEL0:
                DataBuffer[itr++]= UPSDb.UPSModel[0];
                DataBuffer[itr++]= UPSDb.UPSModel[1];
                break;

            case ADDRESS_OF_UPSMODEL1:
                DataBuffer[itr++]= UPSDb.UPSModel[2];
                DataBuffer[itr++]= UPSDb.UPSModel[3];
                break;

            case ADDRESS_OF_UPSMODEL2:
                DataBuffer[itr++]= UPSDb.UPSModel[4];
                DataBuffer[itr++]= UPSDb.UPSModel[5];
                break;

            case ADDRESS_OF_UPSMODEL3:
                DataBuffer[itr++]= UPSDb.UPSModel[6];
                DataBuffer[itr++]= UPSDb.UPSModel[7];
                break;

            case ADDRESS_OF_UPSMODEL4:
                DataBuffer[itr++]= UPSDb.UPSModel[8];
                DataBuffer[itr++]= UPSDb.UPSModel[9];
                break;

            case ADDRESS_OF_UPSMODEL5:
                DataBuffer[itr++]= UPSDb.UPSModel[10];
                DataBuffer[itr++]= UPSDb.UPSModel[11];
                break;

            case ADDRESS_OF_UPSMODEL6:
                DataBuffer[itr++]= UPSDb.UPSModel[12];
                DataBuffer[itr++]= UPSDb.UPSModel[13];
                break;

            case ADDRESS_OF_UPSMODEL7:
                DataBuffer[itr++]= UPSDb.UPSModel[14];
                DataBuffer[itr++]= UPSDb.UPSModel[15];
                break;

            default:
                DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
                break;
        }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadCurrentDateTime
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadCurrentDateTime(COM_Request*pComRequest,uint8_t *DataBuffer)
{

   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case MODBUS_YEAR_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Year);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Year);
           break;

        case MODBUS_MONTH_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Month);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Month);
           break;

        case MODBUS_DATE_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Date);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Date);
           break;

        case MODBUS_HOUR_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Hours);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Hours);
           break;

        case MODBUS_MINUTE_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Minutes);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Minutes);
           break;

        case MODBUS_SECONDS_ADDRESS:

           DataBuffer[itr++]= highByte(GlobalDateTime.Seconds);
           DataBuffer[itr++]= lowByte(GlobalDateTime.Seconds);
           break;

        default:
           DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
      }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadUPSRatingInfo
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadUPSRatingInfo(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
   uint8_t itr=0;
   uint8_t i=0;
   for(i=0;i<pComRequest->length;i++)
   {
     switch(pComRequest->startAddress + i)
     {
        case START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT:

           DataBuffer[itr++]= highByte(UPSDb.RatedBattVolt);
           DataBuffer[itr++]= lowByte(UPSDb.RatedBattVolt);
           break;

        case ADDRESS_OF_OUTP_CURRENT:

           DataBuffer[itr++]= highByte(UPSDb.RatedOuputCurr);
           DataBuffer[itr++]= lowByte(UPSDb.RatedOuputCurr);
           break;

        case ADDRESS_OF_OUTP_FREQ:

           DataBuffer[itr++]= highByte(UPSDb.RatedOuputFreq);
           DataBuffer[itr++]= lowByte(UPSDb.RatedOuputFreq);
           break;

        case ADDRESS_OF_OUTP_VOLT:

           DataBuffer[itr++]= highByte(UPSDb.RatedOuputVolt);
           DataBuffer[itr++]= lowByte(UPSDb.RatedOuputVolt);
           break;

        default:
           DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
      }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_EnableDisablFlagFn
//*
//* Return:  none
//*
//* Description: This function check UPS write request parameter are Enable/ Disable.
//*
//*
//**************************************************************************
static void ModbusTCPServer_EnableDisablFlagFn_2(uint8_t *dataBuff,uint8_t itr,uint8_t itr1)
{

    switch(itr1)
    {
        case 12:
            dataBuff[itr]= 'V';
         break;
         case 13:
             dataBuff[itr]= 'E';
         break;
         case 14:
             dataBuff[itr]= 'J';
         break;
         case 15:
             dataBuff[itr]= 'K';
         break;
        }

}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_EnableDisablFlagFn
//*
//* Return:  none
//*
//* Description: This function check UPS write request parameter are Enable/ Disable.
//*
//*
//**************************************************************************
static void ModbusTCPServer_EnableDisablFlagFn(uint8_t *dataBuff,uint8_t itr,uint8_t itr1)
{

	switch(itr1)
	{
		case 0:
			dataBuff[itr]= 'G';
			break;
		case 1:
			dataBuff[itr]= 'E';
			break;
		case 2:
			dataBuff[itr]= 'P';
			break;
		case 3:
			dataBuff[itr]= 'O';
			break;
		case 4:
			dataBuff[itr]= 'I';
			break;
		case 5:
			dataBuff[itr]= 'H';
			break;
		case 6:
			dataBuff[itr]= 'F';
			break;
		case 7:
			dataBuff[itr]= 'C';
			break;
		case 8:
			dataBuff[itr]= 'T';
			break;
		case 9:
			dataBuff[itr]= 'S';
			break;
		case 10:
			dataBuff[itr]= 'R';
			break;
		case 11:
			dataBuff[itr]= 'M';
			break;
		case 12:
			dataBuff[itr]= 'L';
			break;
		case 13:
			dataBuff[itr]= 'D';
			break;
		case 14:
			dataBuff[itr]= 'B';
			break;
		case 15:
			dataBuff[itr]= 'A';
			break;
	 }

}

/*************************************Function Definition Of Writing The Modbus Data ****************************************/

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteCapabilitySetting_ListData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
uint16_t Prvmodbus16_bit_datastruct1=0,Prvmodbus16_bit_datastruct2=0;
static uint8_t ModbusTCPServer_WriteCapabilitySetting_ListData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
	uint8_t status=WRITE_ERROR;
	uint8_t status1=WRITE_ERROR;
    uint16_t tempVar=0;
    uint16_t itr=0;
    uint16_t i=0;
    UPS_WRITE_REQUEST  pUpsWrtReqparamEn;
    UPS_WRITE_REQUEST  pUpsWrtReqparamDs;
    uint16_t modbus16_bit_datastruct1=0;


	if(pComRequest->length > 1)
	{
		return MODBUS_EXCPT__ACKNOWLEDGE;
	}
	
    pUpsWrtReqparamEn.taskID = MODBUS_TCP_TASK_ID;
	pUpsWrtReqparamEn.ctrlcommand[0]= 'P';
	pUpsWrtReqparamEn.ctrlcommand[1]= 'E';

	pUpsWrtReqparamDs.taskID = MODBUS_TCP_TASK_ID;
	pUpsWrtReqparamDs.ctrlcommand[0]= 'P';
	pUpsWrtReqparamDs.ctrlcommand[1]= 'D';

    for(i=0;i<pComRequest->length;i++)
    {
           modbus16_bit_datastruct1=((DataBuffer[(i*2)] << 8) | DataBuffer[(i*2)+1]);
           switch(pComRequest->startAddress + i)
           {
              case START_ADDRESS_OF_CAPSET_CAPLIST:
                  if(Prvmodbus16_bit_datastruct1 != modbus16_bit_datastruct1)
                  {
                      for(itr=0;itr<16;itr++)
                      {
                          tempVar = (0x0001 << itr);

                          if((modbus16_bit_datastruct1 & tempVar)==(Prvmodbus16_bit_datastruct1  & tempVar))
                          {
                              status1 =  WRITE_SUCCESS;
                              status  =  WRITE_SUCCESS;
                          }
                          else if(((modbus16_bit_datastruct1 & tempVar)!=0)&&((Prvmodbus16_bit_datastruct1  & tempVar)!=1))
                          {
                              ModbusTCPServer_EnableDisablFlagFn(pUpsWrtReqparamEn.ctrlcommand,2,itr);
                              pUpsWrtReqparamEn.ctrlcommand[3]= '\r';
                              pUpsWrtReqparamEn.ctrlcmdSize=4;
                              status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamEn);
                              if(status == WRITE_ERROR)
                               {
                                   return WRITE_ERROR;
                               }
                          }
                          else if(((modbus16_bit_datastruct1 & tempVar)!=1)&&((Prvmodbus16_bit_datastruct1  & tempVar)!=0))
                         {
                              ModbusTCPServer_EnableDisablFlagFn(pUpsWrtReqparamDs.ctrlcommand,2,itr);
                              pUpsWrtReqparamDs.ctrlcommand[3]= '\r';
                              pUpsWrtReqparamDs.ctrlcmdSize=4;
                              status1=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamDs);
                              if(status1 == WRITE_ERROR)
                               {
                                   return WRITE_ERROR;
                               }
                         }
                      }
                        if((status1 == WRITE_SUCCESS) &&(status == WRITE_SUCCESS))
                        {
                           DBStatus= MODBUS_EXCPT__SUCCESS;
                        }
                        else
                        {
                            DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
                        }
                        Prvmodbus16_bit_datastruct1 =modbus16_bit_datastruct1;
                  }
                  else
                  {
                      Prvmodbus16_bit_datastruct1 =modbus16_bit_datastruct1;
                      DBStatus= MODBUS_EXCPT__SUCCESS;
                  }
              break;
               case SECOND_ADDRESS_OF_CAPSET_CAPLIST:
                   if(Prvmodbus16_bit_datastruct2 != modbus16_bit_datastruct1)
                   {
                       for(itr=12;itr<16;itr++)
                       {
                           tempVar = (0x0001 << itr);

                            if((modbus16_bit_datastruct1 & tempVar)==(Prvmodbus16_bit_datastruct2  & tempVar))
                            {
                                status1 = WRITE_SUCCESS;
                                status = WRITE_SUCCESS;
                            }
                            else if(((modbus16_bit_datastruct1 & tempVar)!=0)&&((Prvmodbus16_bit_datastruct2  & tempVar)!=1))
                            {
                                ModbusTCPServer_EnableDisablFlagFn_2(pUpsWrtReqparamEn.ctrlcommand,2,itr);
                                pUpsWrtReqparamEn.ctrlcommand[3]= '\r';
                                pUpsWrtReqparamEn.ctrlcmdSize=4;
                                status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamEn);
                                if(status == WRITE_ERROR)
                                  {
                                      return WRITE_ERROR;
                                  }
                            }
                            else if(((modbus16_bit_datastruct1 & tempVar)!=1)&&((Prvmodbus16_bit_datastruct2  & tempVar)!=0))
                           {
                                ModbusTCPServer_EnableDisablFlagFn_2(pUpsWrtReqparamDs.ctrlcommand,2,itr);
                                pUpsWrtReqparamDs.ctrlcommand[3]= '\r';
                                pUpsWrtReqparamDs.ctrlcmdSize=4;
                                status1=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamDs);
                                if(status1 == WRITE_ERROR)
                                {
                                    return WRITE_ERROR;
                                }
                           }
                        }
                      Prvmodbus16_bit_datastruct2 =modbus16_bit_datastruct1;
                      if((status1 == WRITE_SUCCESS) &&(status == WRITE_SUCCESS))
                      {
                         DBStatus= MODBUS_EXCPT__SUCCESS;
                      }
                      else
                      {
                          DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
                      }

                   }
                   else
                   {
                       Prvmodbus16_bit_datastruct2 =modbus16_bit_datastruct1;
                       DBStatus= MODBUS_EXCPT__SUCCESS;
                   }

                 break;
               case THIRD_ADDRESS_OF_CAPSET_CAPLIST:
                    DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
                    break;
               case FOURTH_ADDRESS_OF_CAPSET_CAPLIST:
                    DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
                    break;
               default:
                    DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
                    break;
            }
       }

     return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteControlItem
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
uint16_t PrvModbusContrlItemWrite=0;
void ModbusTCPServer_WriteCommandofCntrilItem(UPS_WRITE_REQUEST *pUpsWrtReqparam, int  itr)
{

    switch(itr)
   {
     case 5:
         sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","SKOFF1\r");
         pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
         pUpsWrtReqparam->ctrlcmdSize=SKOFFN_CMD_SIZE;
         break;
//     case 6:
//         DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
//         break;
     case 7:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","SKON1\r");
          pUpsWrtReqparam->ctrlcmdSize=SKONN_CMD_SIZE;
          break;
     case 8:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","T\r");
          pUpsWrtReqparam->ctrlcmdSize=T_CMD_SIZE;
          break;
     case 9:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","CT\r");
          pUpsWrtReqparam->ctrlcmdSize=CT_CMD_SIZE;
          break;
     case 10:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","CS\r");
          pUpsWrtReqparam->ctrlcmdSize=CS_CMD_SIZE;
          break;
     case 11:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","SON\r");
          pUpsWrtReqparam->ctrlcmdSize=SON_CMD_SIZE;
          break;
     case 12:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","SOFF\r");
          pUpsWrtReqparam->ctrlcmdSize=SOFF_CMD_SIZE;
          break;
     case 13:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","TL\r");
          pUpsWrtReqparam->ctrlcmdSize=TL_CMD_SIZE;
          break;
     case 14:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","BZON\r");
          pUpsWrtReqparam->ctrlcmdSize=BZON_CMD_SIZE;
          break;
     case 15:
          pUpsWrtReqparam->taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam->ctrlcommand,"%s","BZOFF\r");
          pUpsWrtReqparam->ctrlcmdSize=BZOFF_CMD_SIZE;
          break;
      default:
        break;

    }

}

static uint8_t ModbusTCPServer_WriteControlItem(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t status=WRITE_ERROR;
   UPS_WRITE_REQUEST  pUpsWrtReqparam;
   pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
   uint16_t itr=0;
   uint16_t tempVar=0;
   uint16_t modbus16_bit_datastruct1=0;
   uint8_t i=0;

   
   for(i=0;i<pComRequest->length;i++)
   {
    modbus16_bit_datastruct1=((DataBuffer[(i*2)] << 8) | DataBuffer[(i*2)+1]);
    if(PrvModbusContrlItemWrite!=modbus16_bit_datastruct1)
    {
     switch(pComRequest->startAddress + i)
     {
        case  START_ADDRESS_OF_CTRLITEM:
        {
            if(PrvModbusContrlItemWrite != modbus16_bit_datastruct1)
              {
                  for(itr=5;itr<16;itr++)
                  {
                      tempVar = (0x0001 << itr);
                      if((modbus16_bit_datastruct1 & tempVar)==(PrvModbusContrlItemWrite  & tempVar))
                      {

                          status  =  WRITE_SUCCESS;
                      }
                      else
                      {
                          ModbusTCPServer_WriteCommandofCntrilItem(&pUpsWrtReqparam,itr);
                          status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
                      }
                     }
                      PrvModbusContrlItemWrite=modbus16_bit_datastruct1;
                    if((status == WRITE_SUCCESS))
                    {
                      DBStatus= MODBUS_EXCPT__SUCCESS;
                    }
                    else
                    {
                      DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
                    }


              }

            }
            break;
        default:
            return MODBUS_EXCPT__ILLEGAL_DATA_ADDR;

     }

    }

    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteSettingParamToDfltVal1
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_WriteSettingParamToDfltVal1(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t status=WRITE_ERROR;
    uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
    UPS_WRITE_REQUEST  pUpsWrtReqparam;
    uint16_t modbus16_bit_datastruct1=0;
    uint8_t i=0;

    for(i=0;i<pComRequest->length;i++)
    {
       modbus16_bit_datastruct1=((DataBuffer[(i*2)] << 8) | DataBuffer[(i*2)+1]);
       switch(pComRequest->startAddress + i)
       {
          case  START_ADDRESS_OF_SETTPARM1:

            if(modbus16_bit_datastruct1 & 0x8000)
            {
               pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
               sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s","PF\r");
               pUpsWrtReqparam.ctrlcmdSize=PF_CMD_SIZE;
               status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
              if((status == WRITE_SUCCESS))
              {
                DBStatus= MODBUS_EXCPT__SUCCESS;
              }
              else
              {
                DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
              }

             }
              else
              {
			   DBStatus=MODBUS_EXCPT__ILLEGAL_DATA_VALUE;
			  }
              break;
          default:
              return MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
              break;
        }
    }
    return DBStatus;
}
//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteOutputSocketStat2
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t  ModbusTCPServer_WriteOutputSocketStat2(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t status=WRITE_ERROR;
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  UPS_WRITE_REQUEST  pUpsWrtReqparam;
  uint16_t sendDt=0;
  char dataconvt[10]={0};
  uint8_t i=0;

  for(i=0;i<pComRequest->length;i++)
  {
    sendDt=((DataBuffer[(i*2)] << 8) | DataBuffer[(i*2)+1]);
    switch(pComRequest->startAddress + i)
    {
      case  START_ADDRESS_OF_OUTPUTSOCK2:

       sprintf(dataconvt,"%03d",sendDt);
       pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
       sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%03d%c","PSK1",sendDt,'\r');
//       pUpsWrtReqparam.ctrlcommand[0]='P';
//       pUpsWrtReqparam.ctrlcommand[1]='S';
//       pUpsWrtReqparam.ctrlcommand[2]='K';
//       pUpsWrtReqparam.ctrlcommand[3]='1';
//       pUpsWrtReqparam.ctrlcommand[4]=dataconvt[0];
//       pUpsWrtReqparam.ctrlcommand[5]=dataconvt[1];
//       pUpsWrtReqparam.ctrlcommand[6]=dataconvt[2];
//       pUpsWrtReqparam.ctrlcommand[7]='\r';
       pUpsWrtReqparam.ctrlcmdSize=8;
       status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
       if((status == WRITE_SUCCESS))
       {
         DBStatus= MODBUS_EXCPT__SUCCESS;
       }
       else
       {
         DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
       }
       break;
      default:
         return MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
         break;
    }
  }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteUPSLossPtData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************

static uint8_t ModbusTCPServer_WriteUPSLossPtData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   uint8_t status=WRITE_ERROR;
   UPS_WRITE_REQUEST  pUpsWrtReqparam;
   uint16_t sendDt=0;
   float freq=0;
   char dataconvt[10]={0};
   uint8_t i=0;

   if(pComRequest->length > 1)
   {
     return MODBUS_EXCPT__ACKNOWLEDGE;
   }
   for(i=0;i<pComRequest->length;i++)
   {
     sendDt=((DataBuffer[(i*2)] << 8) | DataBuffer[(i*2)+1]);
     switch(pComRequest->startAddress + i)
     {
       case START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP:

         sprintf(dataconvt,"%d",sendDt);
         pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
         sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%03d%c","HEH",  sendDt/10,'\r');
//         pUpsWrtReqparam.ctrlcommand[0]='H';
//         pUpsWrtReqparam.ctrlcommand[1]='E';
//         pUpsWrtReqparam.ctrlcommand[2]='H';
//         pUpsWrtReqparam.ctrlcommand[3]= dataconvt[0];
//         pUpsWrtReqparam.ctrlcommand[4]= dataconvt[1];
//         pUpsWrtReqparam.ctrlcommand[5]= dataconvt[2];
//         pUpsWrtReqparam.ctrlcommand[6]='\r';
         pUpsWrtReqparam.ctrlcmdSize=HEH_CMD_SIZE;

         status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
         if((status == WRITE_SUCCESS))
         {
           DBStatus= MODBUS_EXCPT__SUCCESS;
           pModbusStruct->SettingParamSuccFailResults.Bits.bit15= 1;
         }
         else
         {
           pModbusStruct->SettingParamSuccFailResults.Bits.bit15= 0;
           DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
         }
         break;
       case ADDRESS_OF_HIGH_EFF_MODE_VOLT_LLP:

         sprintf(dataconvt,"%04d",sendDt);
         pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;

         sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%03d%c","HEL",  sendDt/10,'\r');
//         pUpsWrtReqparam.ctrlcommand[0]='H';
//         pUpsWrtReqparam.ctrlcommand[1]='E';
//         pUpsWrtReqparam.ctrlcommand[2]='L';
//         pUpsWrtReqparam.ctrlcommand[3]= dataconvt[0];
//         pUpsWrtReqparam.ctrlcommand[4]= dataconvt[1];
//         pUpsWrtReqparam.ctrlcommand[5]= dataconvt[2];
//         pUpsWrtReqparam.ctrlcommand[6]='\r';
         pUpsWrtReqparam.ctrlcmdSize=HEL_CMD_SIZE;
         status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
         if((status == WRITE_SUCCESS))
         {
           DBStatus= MODBUS_EXCPT__SUCCESS;
           pModbusStruct->SettingParamSuccFailResults.Bits.bit14= 1;
         }
         else
         {
           pModbusStruct->SettingParamSuccFailResults.Bits.bit14= 0;
           DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
         }
         break;
       case THIRD_ADDRESS_OF_LOSSPNTDATA:
           DBStatus=MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
           break;
       case FOURTH_ADDRESS_OF_LOSSPNTDATA:
           DBStatus=MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
           break;
       case ADDRESS_OF_BYPS_FREQ_HLP:
        pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
        sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%2.1f%c","PGF",(float)sendDt/10 ,'\r');
        pUpsWrtReqparam.ctrlcmdSize=PGF_CMD_SIZE;
        status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
        if((status == WRITE_SUCCESS))
        {
           DBStatus= MODBUS_EXCPT__SUCCESS;
           pModbusStruct->SettingParamSuccFailResults.Bits.bit11= 1;
        }
        else
        {
           pModbusStruct->SettingParamSuccFailResults.Bits.bit11= 0;
           DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
        }
        break;
       case ADDRESS_OF_BYPS_FREQ_LLP:

        pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
        sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%2.1f%c","PSF",(float)sendDt/10 ,'\r');
        pUpsWrtReqparam.ctrlcmdSize=PSF_CMD_SIZE;
        status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
        if((status == WRITE_SUCCESS))
        {
           DBStatus= MODBUS_EXCPT__SUCCESS;
           pModbusStruct->SettingParamSuccFailResults.Bits.bit10= 1;
        }
        else
        {
           pModbusStruct->SettingParamSuccFailResults.Bits.bit10= 0;
           DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
        }
        break;

       case ADDRESS_OF_BYPS_VOLT_HLP:
        pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
        sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%03d%c","PHV",  sendDt,'\r');
        pUpsWrtReqparam.ctrlcmdSize=PHV_CMD_SIZE;
        status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
        if((status == WRITE_SUCCESS))
        {
          DBStatus= MODBUS_EXCPT__SUCCESS;
          pModbusStruct->SettingParamSuccFailResults.Bits.bit9= 1;
        }
        else
        {
          pModbusStruct->SettingParamSuccFailResults.Bits.bit9= 0;
          DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
        }
        break;
        case ADDRESS_OF_BYPS_VOLT_LLP:

         pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
         sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%03d%c","PLV",  sendDt,'\r');
         pUpsWrtReqparam.ctrlcmdSize=PLV_CMD_SIZE;
         status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
         if((status == WRITE_SUCCESS))
         {
           DBStatus= MODBUS_EXCPT__SUCCESS;
           pModbusStruct->SettingParamSuccFailResults.Bits.bit8= 1;
         }
         else
         {
           pModbusStruct->SettingParamSuccFailResults.Bits.bit8= 0;
           DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
         }
         break;
         default:
           DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
           break;
          }
       }
    return DBStatus;
}

static uint8_t DateTime_Validation(uint8_t DateTime, uint8_t DateTimeIndex)
{
   switch(DateTimeIndex)
   {
       case SET_YEAR_INDEX:
           if((DateTime >=0) && (DateTime <=99))
            {
                return TRUE;
            }
           break;
       case SET_MONTH_INDEX:
           if((DateTime >=1) && (DateTime <=12))
              {
                  return TRUE;
              }
             break;
       case SET_DATE_INDEX:
           if((DateTime >=1) && (DateTime <=31))
         {
             return TRUE;
         }
             break;
       case SET_HOUR_INDEX:
           if((DateTime >=0) && (DateTime <=23))
                   {
                       return TRUE;
                   }
             break;

       case SET_MINUTE_INDEX:
       case SET_SECONDS_INDEX:
           if((DateTime >=0) && (DateTime <=59))
                   {
                       return TRUE;
                   }
             break;

       default:
             break;

   }
  return FALSE;
}
static void WriteRTCCompensateDataInOnChipEpprom()
{
     uint16_t  CRC_Cal=0;
     uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.RTCCompenConfig.TransBuffer);
     CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE-2);
     Device_Configurations.EepromMapStruct.RTCCompenConfig.CRC_RTCCompensSett= CRC_Cal;
     OnChipEEPROM_Write(RTC_COMPENSATORY_SAVE, (uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE);
}
static uint8_t ModbusTCPServer_WriteDateTimeGrpData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
    uint16_t sendDt=0;
    uint8_t i=0;
    Time SystemTimeSet;
    SystemTimeSet.Date=GlobalDateTime.Date;
    SystemTimeSet.Month=GlobalDateTime.Month;
    SystemTimeSet.Year=GlobalDateTime.Year;
    SystemTimeSet.Hours=GlobalDateTime.Hours;
    SystemTimeSet.Minutes=GlobalDateTime.Minutes;
    SystemTimeSet.Seconds=GlobalDateTime.Seconds;

  for(i=0;i<pComRequest->length;i++)
  {
    sendDt=((DataBuffer[i*2] << 8) | DataBuffer[(i*2)+1]);
    switch(pComRequest->startAddress + i)
    {
      case MODBUS_YEAR_ADDRESS:
          if(DateTime_Validation(sendDt, SET_YEAR_INDEX)==TRUE)
          {
                SystemTimeSet.Year=sendDt;
                WriteNewDateTimeToRTC(SystemTimeSet);
                DBStatus= MODBUS_EXCPT__SUCCESS;
          }
          break;
      case MODBUS_MONTH_ADDRESS:
          if(DateTime_Validation(sendDt, SET_MONTH_INDEX)==TRUE)
           {
                 SystemTimeSet.Month=sendDt;
                 WriteNewDateTimeToRTC(SystemTimeSet);
                 DBStatus= MODBUS_EXCPT__SUCCESS;
           }
          break;
      case MODBUS_DATE_ADDRESS:
          if(DateTime_Validation(sendDt, SET_DATE_INDEX)==TRUE)
           {
                 //write Date
                SystemTimeSet.Date=sendDt;
                WriteNewDateTimeToRTC(SystemTimeSet);
                DBStatus= MODBUS_EXCPT__SUCCESS;
           }
          break;
      case MODBUS_HOUR_ADDRESS:
          if(DateTime_Validation(sendDt, SET_HOUR_INDEX)==TRUE)
            {
                //write HOUR
              SystemTimeSet.Hours=sendDt;
              WriteNewDateTimeToRTC(SystemTimeSet);
              DBStatus= MODBUS_EXCPT__SUCCESS;
            }
          break;
      case MODBUS_MINUTE_ADDRESS:
          if(DateTime_Validation(sendDt, SET_MINUTE_INDEX)==TRUE)
          {
                //write MINUTE
              SystemTimeSet.Minutes=sendDt;
              WriteNewDateTimeToRTC(SystemTimeSet);
              DBStatus= MODBUS_EXCPT__SUCCESS;
          }
          break;
      case MODBUS_SECONDS_ADDRESS:
          if(DateTime_Validation(sendDt, SET_SECONDS_INDEX)==TRUE)
           {
                 //write Seconds
               SystemTimeSet.Seconds=sendDt;
               WriteNewDateTimeToRTC(SystemTimeSet);
               DBStatus= MODBUS_EXCPT__SUCCESS;
           }
          break;
      default:
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
    }
  }

  return DBStatus;
}



static uint8_t ModbusTCPServer_WriteBatteryPieceNumGrpData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t status=WRITE_ERROR;
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  UPS_WRITE_REQUEST  pUpsWrtReqparam;
  uint16_t sendDt=0;
  uint8_t i=0;

  for(i=0;i<pComRequest->length;i++)
  {
    sendDt=((DataBuffer[i*2] << 8) | DataBuffer[(i*2)+1]);
    switch(pComRequest->startAddress + i)
    {
      case ADDRESS_OF_BATT_GROUP_NUM:

          pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%02d%c","BATGN" ,sendDt,'\r');
          pUpsWrtReqparam.ctrlcmdSize=BATGRPN_CMD_SIZE;
          status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
          if((status == WRITE_SUCCESS))
          {
            DBStatus= MODBUS_EXCPT__SUCCESS;
            pModbusStruct->shutdownRestoreResults.Bits.bit15= 1;
          }
          else
          {
            pModbusStruct->shutdownRestoreResults.Bits.bit15= 0;
            DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
          }
          break;
      default:
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
    }
  }
  return DBStatus;
}


//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteRemoteShutdwDt
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function Writes Modbus Registers
//* according to request (*pComRequest).
//*
//**************************************************************************

static uint8_t ModbusTCPServer_WriteRemoteShutdwDt(COM_Request*pComRequest,uint8_t *DataBuffer)
{
  uint8_t status=WRITE_ERROR;
  uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
  UPS_WRITE_REQUEST  pUpsWrtReqparam;
  uint16_t sendDt=0;
  char dataconvt[10]={0};
  uint8_t i=0;

  for(i=0;i<pComRequest->length;i++)
  {
    sendDt=((DataBuffer[i*2] << 8) | DataBuffer[(i*2)+1]);
    switch(pComRequest->startAddress + i)
    {
      case START_ADDRESS_OF_REMOTESHUTDN:

       //   sprintf(dataconvt,"%02d",sendDt);
          pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%c%02d%c",'S',  sendDt,'\r');
//          pUpsWrtReqparam.ctrlcommand[0]='S';
//          pUpsWrtReqparam.ctrlcommand[1]= dataconvt[0];
//          pUpsWrtReqparam.ctrlcommand[2]= dataconvt[1];
//          pUpsWrtReqparam.ctrlcommand[3]='\r';
          pUpsWrtReqparam.ctrlcmdSize=SN_CMD_SIZE;
          status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
          if((status == WRITE_SUCCESS))
          {
            DBStatus= MODBUS_EXCPT__SUCCESS;
            pModbusStruct->shutdownRestoreResults.Bits.bit15= 1;
          }
          else
          {
            pModbusStruct->shutdownRestoreResults.Bits.bit15= 0;
            DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
          }
          break;
      case ADDRESS_OF_TEST_FOR_SPECIF_TIME:

     //     sprintf(dataconvt,"%02d",sendDt);
          pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
          sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%c%02d%c",'T',  sendDt,'\r');
//          pUpsWrtReqparam.ctrlcommand[0]='T';
//          pUpsWrtReqparam.ctrlcommand[1]= dataconvt[0];
//          pUpsWrtReqparam.ctrlcommand[2]= dataconvt[1];
//          pUpsWrtReqparam.ctrlcommand[3]='\r';
          pUpsWrtReqparam.ctrlcmdSize=T_N_CMD_SIZE;
          status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
          if((status == WRITE_SUCCESS))
          {
            DBStatus= MODBUS_EXCPT__SUCCESS;
            pModbusStruct->shutdownRestoreResults.Bits.bit14= 1;
          }
          else
          {
            pModbusStruct->shutdownRestoreResults.Bits.bit14= 0;
            DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
          }
          break;
      case ADDRESS_OF_SHUTDN_RESTORE_N:
      case ADDRESS_OF_SHUTDN_RESTORE_S:
      case ADDRESS_OF_SHUTDN_RESTORE_R:
                
       if((pComRequest->startAddress + i) == ADDRESS_OF_SHUTDN_RESTORE_N)
       {
         if(sendDt >=0 && sendDt <=99 )
         {
        //   sprintf(dataconvt,"%02d",sendDt);
           pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
           sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%02d%c%02d%c",'S',sendDt,'R',sendDt,'\r');
//           pUpsWrtReqparam.ctrlcommand[0]='S';
//           pUpsWrtReqparam.ctrlcommand[1]= dataconvt[0];
//           pUpsWrtReqparam.ctrlcommand[2]= dataconvt[1];
//
//           pUpsWrtReqparam.ctrlcommand[3]= 'R';
//           pUpsWrtReqparam.ctrlcommand[4]= '0';
//           pUpsWrtReqparam.ctrlcommand[5]= '0';
//           pUpsWrtReqparam.ctrlcommand[6]= '0';
//           pUpsWrtReqparam.ctrlcommand[7]= '0';
//
//           pUpsWrtReqparam.ctrlcommand[8]='\r';
           pUpsWrtReqparam.ctrlcmdSize=SNR_CMD_SIZE;
          }
       }
         else
         {
           sprintf(dataconvt,"%04d",sendDt);
           pUpsWrtReqparam.taskID = MODBUS_TCP_TASK_ID;
           sprintf((char *)pUpsWrtReqparam.ctrlcommand,"%s%02d%c%02d%c",'S',sendDt,'R',sendDt,'\r');
//           pUpsWrtReqparam.ctrlcommand[0]= 'S';
//           pUpsWrtReqparam.ctrlcommand[1]= '0';
//           pUpsWrtReqparam.ctrlcommand[2]= '0';
//
//           pUpsWrtReqparam.ctrlcommand[3]='R';
//           pUpsWrtReqparam.ctrlcommand[4]= dataconvt[0];
//           pUpsWrtReqparam.ctrlcommand[5]= dataconvt[1];
//           pUpsWrtReqparam.ctrlcommand[6]= dataconvt[2];
//           pUpsWrtReqparam.ctrlcommand[7]= dataconvt[3];
//
//           pUpsWrtReqparam.ctrlcommand[8]='\r';
           pUpsWrtReqparam.ctrlcmdSize=SNR_CMD_SIZE;
         }
          status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparam);
          if((status == WRITE_SUCCESS))
          {
              DBStatus= MODBUS_EXCPT__SUCCESS;
              pModbusStruct->shutdownRestoreResults.Bits.bit13= 1;
          }
          else
          {
              pModbusStruct->shutdownRestoreResults.Bits.bit13= 0;
              DBStatus= MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE;
          }
          break;
      default:
          DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_ADDR;
          break;
    }
  }
  return DBStatus;
}

//**************************************************************************
//*
//* Function name:  ModbusTCPServer_ReadData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function call the all read data function according to group index.
//
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
   uint8_t DBStatus= MODBUS_EXCPT__SUCCESS;
   switch(pComRequest->groupindex)
   {
     case MODBUSTCP_WARNING_ITEM_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadWarningStatus(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_CAPSET_CAPLIST_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadCapabilitySetting_ListData(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_CTRLITEM_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadControlItem(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_RESCTRL_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadResultOfControlData(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_SETTPARM1_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadSettingParamToDfltVal1(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_SETTPARM2_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadSettingParamToDfltVal2(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSWORK_STAT1_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadUPSWorkingStatus1(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSWORK_STAT2_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadUPSWorkingStatus2(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSBATTINFO_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadUPSBattInfoDT(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSWORK_MODE_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadUPSWorkingModeDT(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSFAULT_INFO_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadUPSFaultInfo(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_OUTPUTSOCK1_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadOutputSocketStat1(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_OUTPUTSOCK2_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadOutputSocketStat2(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_LOSSPNTDATA_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadUPSLossPtData(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_SETTPARM_SUCC_FAIL_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadSettingParamSuccFailDt(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_REMOTESHUTDN_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadRemoteShutdwDt(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_REMOTESHUTDN_TEST_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadRemoteShutdwTestDt(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_CPUINFO_GROUPINDX:
           DBStatus=ModbusTCPServer_ReadCPUInfo(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSMODEL_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadCPUModelDt(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_UPSRATINGINFO_GROUPINDX:
           DBStatus= ModbusTCPServer_ReadUPSRatingInfo(pComRequest,DataBuffer);
           break;
     case MODBUSTCP_DATETIMESET_GROUPINDX:
          DBStatus= ModbusTCPServer_ReadCurrentDateTime(pComRequest,DataBuffer);
         break;
     default:
           DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_VALUE;
           break;
    }
   return DBStatus;
}


//**************************************************************************
//*
//* Function name:  ModbusTCPServer_WriteData
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function call the all write data function according to group index.
//
//*
//**************************************************************************
static uint8_t ModbusTCPServer_WriteData(COM_Request*pComRequest,uint8_t *DataBuffer)
{
    uint8_t DBStatus=MODBUS_EXCPT__SUCCESS;
    switch(pComRequest->groupindex)
    {
      case MODBUSTCP_CAPSET_CAPLIST_GROUPINDX:
         DBStatus= ModbusTCPServer_WriteCapabilitySetting_ListData(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_CTRLITEM_GROUPINDX:
         DBStatus=ModbusTCPServer_WriteControlItem(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_SETTPARM1_GROUPINDX:
         DBStatus=ModbusTCPServer_WriteSettingParamToDfltVal1(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_OUTPUTSOCK2_GROUPINDX:
         DBStatus= ModbusTCPServer_WriteOutputSocketStat2(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_LOSSPNTDATA_GROUPINDX:
         DBStatus=ModbusTCPServer_WriteUPSLossPtData(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_REMOTESHUTDN_GROUPINDX:
         DBStatus= ModbusTCPServer_WriteRemoteShutdwDt(pComRequest,DataBuffer);
         break;

      case MODBUSTCP_UPSBATTINFO_GROUPINDX:
           DBStatus= ModbusTCPServer_WriteBatteryPieceNumGrpData(pComRequest,DataBuffer);
           break;

      case MODBUSTCP_DATETIMESET_GROUPINDX:
          DBStatus= ModbusTCPServer_WriteDateTimeGrpData(pComRequest,DataBuffer);
          break;
      default:
         DBStatus= MODBUS_EXCPT__ILLEGAL_DATA_VALUE;
         break;
     }
   return DBStatus;
}
//**************************************************************************
//*
//* Function name:   ModbusTCPServer_ReadWriteMappedReg
//*
//* Return:   TRUE: Success
//*           FALSE: Failure
//*
//* Description: This function reads and writes Modbus Registers to database
//* according to request (*pComRequest).
//*
//**************************************************************************
static uint8_t ModbusTCPServer_ReadWriteMappedReg(COM_Request *pComRequest, uint8_t*DataBuffer)
{
    uint8_t validRequest=FALSE;
    uint16_t qty=0;
    uint8_t i=0;
    uint8_t  DBStatus=MODBUS_EXCPT__ILLEGAL_FNC;
    for(i=0;i < MODBUS_NO_OF_GROUPS;i++)
    {
        if((pComRequest->startAddress >=ModbusMAPUPSDt[i].startadd)
            && (pComRequest->startAddress <= ModbusMAPUPSDt[i].endadd))
        {
            qty=( ModbusMAPUPSDt[i].endadd - pComRequest->startAddress) + 1;

            if(pComRequest->length <= qty)
            {
                validRequest = TRUE;
                pComRequest->groupindex=i;
                break;
            }
        }
     }
   if(validRequest == TRUE)
   {

    switch(pComRequest->functionCode)
    {
        case MODBUS_FNC_READ_HOLDING_REG:
        {
            if(pComRequest->length > 0)
            {

                DBStatus=ModbusTCPServer_ReadData(pComRequest,DataBuffer);//Call a function
            }
        }
            break;
        case MODBUS_FNC_WRITE_MULTI_REG:
        {
            if(pComRequest->length > 0)
            {
                DBStatus= ModbusTCPServer_WriteData(pComRequest,DataBuffer); //Call a function
            }
        }
            break;
        case MODBUS_FNC_WRITE_SINGLE_REG:
        {
            if(pComRequest->length == 1)
            {
                DBStatus= ModbusTCPServer_WriteData(pComRequest,DataBuffer); //Call a function
            }
        }
              break;
        default:
            DBStatus=MODBUS_EXCPT__ILLEGAL_FNC;
              break;
     }
   }
      return DBStatus;
}

//****************************************************************************
//*
//* Function name: ModbusTCPServer_Init
//*
//* Return: None
//*
//*
//* Description:
//*   Modbus TCP server initialization with clearing all respective buffers of 
//*   socket.
//*
//****************************************************************************
//==============================================================================

void ModbusTCPServer_Init(void)
{
    uint8_t count = 0;

    for (count = 0; count <MBTCP_MAX_CONNECTION; count++)
    {
        pModbusStruct->modbusConnection[count].sockfd = -1;
        pModbusStruct->modbusConnection[count].startTimeConnectionTimeout=0;
        if(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout>10)
        {
            pModbusStruct->modbusConnection[count].connectionTimeoutTime =
               Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout/10;
            WebSocketStruct.ClientConnection[count].connectionTimeoutTime = pModbusStruct->modbusConnection[count].connectionTimeoutTime;
        }

        pModbusStruct->modbusConnection[count].txRemainingSize=0;
        pModbusStruct->modbusConnection[count].rxBufferSize=0;
        memset(&pModbusStruct->modbusConnection[count].remote,0,sizeof(struct sockaddr_in));
        memset(pModbusStruct->modbusConnection[count].txBuffer,0,MBTCP_BUF_SIZE);
        memset(pModbusStruct->modbusConnection[count].rxBuffer,0,MBTCP_BUF_SIZE);
    }

    pModbusStruct->listenSockfd = -1;


    pModbusStruct->mb_clientIndex = 0;

   //Check MOdbus TCP is enable or not
    pModbusStruct->modbusTCPEnable =
            Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable;

    //clear All diagnostic Parameteres on initialization
    memset(&pModbusStruct->modbusDiag, 0x00, sizeof(MODBUS_TCP_DIAG_COUNTER));


}
//****************************************************************************
//*
//* Function name: ModbusTCPServer_DiagnosticParameters
//*
//* Return: paramValue __uint32_t
//*
//*
//* Description:
//*   Modbus TCP server Counters for connection integrity with Mobus tcp client
//*
//****************************************************************************
//==============================================================================

uint32_t ModbusTCPServer_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset)
{
    uint32_t paramValue = 0;
    if(Reset == TRUE)
    {
        //Reset all parameters
        memset(&pModbusStruct->modbusDiag, 0x00, sizeof(MODBUS_TCP_DIAG_COUNTER));
    }
    else
    {
        switch(ParamIndex)
        {
            case 0:
            {
                paramValue = pModbusStruct->modbusDiag.framesRecv;
                break;
            }
            case 1:
            {
                paramValue = pModbusStruct->modbusDiag.frameSent;
                break;
            }
            case 2:
            {
                paramValue = pModbusStruct->modbusDiag.goodTransaction;
                break;
            }
            case 3:
            {
                paramValue = pModbusStruct->modbusDiag.goodException;
                break;
            }
            case 4:
            {
                paramValue = pModbusStruct->modbusDiag.clientConnections;
                break;
            }
            case 5:
            {
                paramValue = pModbusStruct->modbusDiag.otherError;
                break;
            }
            default:
                break;
        }
    }
    return paramValue;
}

//****************************************************************************
//*
//* Function name: ModbusTCPSever_UpdateConnectionTimeout
//*
//* Return: None
//*
//*
//* Description:
//*   Update timeout counter for Modbus TCP server connection with client through 
//*   webserver.
//*
//****************************************************************************
//==============================================================================

void ModbusTCPSever_UpdateConnectionTimeout(uint16_t Timout)
{
    uint8_t count = 0;
    if(Timout>10)
    {
        Timout=Timout/10;//As Time base of time is 10msec
    }
    else
    {
        Timout=300;
    }
    for (count = 0; count < MBTCP_MAX_CONNECTION; count++)
    {
        pModbusStruct->modbusConnection[count].connectionTimeoutTime = Timout;
        WebSocketStruct.ClientConnection[count].connectionTimeoutTime = Timout;
    }
}
//****************************************************************************
//*
//* Function name: ModbusTCPServer_StartConnectionTimer
//*
//* Return: None
//*
//*
//* Description:
//*   Start Modbus TCP server Timer after Connection established and request
//*   recived.
//*
//****************************************************************************


static void ModbusTCPServer_StartConnectionTimer(MODBUS_TCP_RAM_STRUCT *pModbusStruct, uint8_t Client)
{
    pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout = COMTimer_Get10MsecTick();
}
//==============================================================================

//****************************************************************************
//*
//* Function name: ModbusTCPServer_CheckConnectionTimeout
//*
//* Return:  TRUE- For Timeout
//*          FALSE- Timer is still running.
//*
//* Description:
//*   Modbus TCP sever timer sends keep alive message after connection timer
//*   Timeout call by ModbusTCPSever_SocketOperation.
//*
//****************************************************************************
//==============================================================================

static BOOL ModbusTCPServer_CheckConnectionTimeout(MODBUS_TCP_RAM_STRUCT *pModbusStruct,uint8_t Client)
{
    // Just check if CurrentTick is within Tolerance zone.

    COM_TIMER_TICK currentTick=0;
    COM_TIMER_TICK nextTick=0;
    uint64_t totalTick=0;
    BOOL status=FALSE;


    totalTick = pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout + 
                pModbusStruct->modbusConnection[Client].connectionTimeoutTime;

    currentTick=COMTimer_Get10MsecTick();

    if(totalTick >= COM_TICK_MAX)
    {
        if ( currentTick <= pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout)
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
        if ( currentTick >= pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout)
        {
            // no rollover
            nextTick = (COM_TIMER_TICK) (pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout +
                                        pModbusStruct->modbusConnection[Client].connectionTimeoutTime);
            if (currentTick >= nextTick)
                status= TRUE;

        }
        else  // if ( upper >= COM_TICK_MAX )
        {
            // rollover detected
            nextTick = (COM_TIMER_TICK) (pModbusStruct->modbusConnection[Client].connectionTimeoutTime -
                                        ( COM_TICK_MAX - pModbusStruct->modbusConnection[Client].startTimeConnectionTimeout));

            if (currentTick >= nextTick)
                status= TRUE;

        }
    }

    return status;

}
//==============================================================================

//****************************************************************************
//*
//* Function name: ModbusTCPServer_SendResponse
//*
//* Return:  TRUE - for all condition.
//*
//* Description:
//*   Send respond over the socket for request modbus ID call by 
//*   ModbusTCPSever_SocketOperation.
//*
//****************************************************************************
//==============================================================================

static BOOL ModbusTCPServer_SendResponse( MODBUS_TCP_RAM_STRUCT *pModbusStruct, uint8_t ClientConn)
{
    uint8_t * mbTCPFrame;
    uint16_t mbTCPLength;
    uint8_t state = FALSE;


    if(ClientConn < MBTCP_MAX_CONNECTION)
    {
        mbTCPFrame=pModbusStruct->modbusConnection[ClientConn].txBuffer;//pModbusStruct->txBuffer;
        mbTCPLength=pModbusStruct->modbusConnection[ClientConn].txRemainingSize;//mbTCPLength=pModbusStruct->txRemainingSize;
        send(pModbusStruct->modbusConnection[ClientConn].sockfd, mbTCPFrame, mbTCPLength, NETCONN_NOCOPY);
        state = TRUE;
        pModbusStruct->modbusDiag.frameSent = (uint32_t) ((pModbusStruct->modbusDiag.frameSent + 1) & 0xFFFFFFFF);  // allow rollover
    }
    return state;
}

//****************************************************************************
//*
//* Function name: ModbusTCPServer_ProcessRequest
//*
//* Return: None
//*
//* Description:
//*   Process all Modbus request and fill with data as response and send over
//*   socket, call by ModbusTCPSever_SocketOperation.
//*
//****************************************************************************
//==============================================================================

static void ModbusTCPServer_ProcessRequest(MODBUS_TCP_RAM_STRUCT *pModbusStruct, uint8_t ClientConn)
{

    uint8_t u8ModbusADUSize = 0;
    uint8_t *u8ModbusADU;
    uint8_t *u8rxBuffer;
    COM_Request COMRequest;
    uint8_t ret=MODBUS_EXCPT__ILLEGAL_DATA_ADDR;;

    COMRequest.functionCode = (uint8_t) pModbusStruct->modbusConnection[ClientConn].rxBuffer[MBTCP_FUNC];//(uint8_t) pModbusStruct->rxBuffer[MBTCP_FUNC];
    u8ModbusADU=pModbusStruct->modbusConnection[ClientConn].txBuffer;//u8ModbusADU=pModbusStruct->txBuffer;
    u8rxBuffer=pModbusStruct->modbusConnection[ClientConn].rxBuffer;//pModbusStruct->rxBuffer;

     // assemble Modbus Request Application Data Unit
     u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[0];
     u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[1];
     u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[2];
     u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[3];

     u8ModbusADUSize = u8ModbusADUSize + 2;
     u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[6];
     u8ModbusADU[u8ModbusADUSize++] = COMRequest.functionCode;

     switch(COMRequest.functionCode)
     {
       case MODBUS_FNC_READ_HOLDING_REG:
             COMRequest.startAddress =(pModbusStruct->modbusConnection[ClientConn].rxBuffer[8] << 8) | pModbusStruct->modbusConnection[ClientConn].rxBuffer[9];

             COMRequest.length = (pModbusStruct->modbusConnection[ClientConn].rxBuffer[10] << 8) | pModbusStruct->modbusConnection[ClientConn].rxBuffer[11];
             u8ModbusADU[u8ModbusADUSize++] = (lowByte(COMRequest.length) << 1);
             ret= ModbusTCPServer_ReadWriteMappedReg(&COMRequest, &u8ModbusADU[u8ModbusADUSize]);
                if(ret == SUCCESS)
                {
                    u8ModbusADUSize += (COMRequest.length << 1);
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodTransaction = (uint32_t) ((pModbusStruct->modbusDiag.goodTransaction + 1) & 0xFFFFFFFF);  // allow rollover
                }
                else
                {
                    u8ModbusADUSize=MBTCP_FUNC;
                    u8ModbusADU[u8ModbusADUSize++] = 0x80 | COMRequest.functionCode;
                    u8ModbusADU[u8ModbusADUSize++] = ret;
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodException = (uint32_t) ((pModbusStruct->modbusDiag.goodException + 1) & 0xFFFFFFFF);
                }
        break;
       case MODBUS_FNC_WRITE_MULTI_REG:
             COMRequest.startAddress = (pModbusStruct->modbusConnection[ClientConn].rxBuffer[8] << 8) | 
                                        pModbusStruct->modbusConnection[ClientConn].rxBuffer[9];
             
            COMRequest.length = (pModbusStruct->modbusConnection[ClientConn].rxBuffer[10] << 8) |
                                pModbusStruct->modbusConnection[ClientConn].rxBuffer[11];

            ret=(ModbusTCPServer_ReadWriteMappedReg(&COMRequest, &u8rxBuffer[13]));

                if(ret == SUCCESS)
                {
                    u8ModbusADU[u8ModbusADUSize++] = highByte(COMRequest.startAddress);
                    u8ModbusADU[u8ModbusADUSize++] = lowByte(COMRequest.startAddress);
                    u8ModbusADU[u8ModbusADUSize++] = highByte(COMRequest.length);
                    u8ModbusADU[u8ModbusADUSize++] = lowByte(COMRequest.length);
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodTransaction = (uint32_t) ((pModbusStruct->modbusDiag.goodTransaction + 1) & 0xFFFFFFFF);  // allow rollover
                }
                else
                {
                    u8ModbusADUSize=MBTCP_FUNC;
                    u8ModbusADU[u8ModbusADUSize++] = 0x80 | COMRequest.functionCode;
                    u8ModbusADU[u8ModbusADUSize++] =ret;
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodException = (uint32_t) ((pModbusStruct->modbusDiag.goodException + 1) & 0xFFFFFFFF);
                }

         break;
       case MODBUS_FNC_WRITE_SINGLE_REG:
            COMRequest.startAddress = (pModbusStruct->modbusConnection[ClientConn].rxBuffer[8] << 8) |
                                        pModbusStruct->modbusConnection[ClientConn].rxBuffer[9];
            COMRequest.length = 1;

            ret=(ModbusTCPServer_ReadWriteMappedReg(&COMRequest, &u8rxBuffer[10]));

                if(ret ==SUCCESS)
                {
                    u8ModbusADU[u8ModbusADUSize++] = highByte(COMRequest.startAddress);
                    u8ModbusADU[u8ModbusADUSize++] = lowByte(COMRequest.startAddress);
                    u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[10];
                    u8ModbusADU[u8ModbusADUSize++] = u8rxBuffer[11];
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodTransaction = (uint32_t) ((pModbusStruct->modbusDiag.goodTransaction + 1) & 0xFFFFFFFF);  // allow rollover
                }
                else
                {
                    u8ModbusADUSize=MBTCP_FUNC;
                    u8ModbusADU[u8ModbusADUSize++] = 0x80 | COMRequest.functionCode;
                    u8ModbusADU[u8ModbusADUSize++] = ret;
                    u8ModbusADU[MBTCP_LEN] = 0;
                    u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
                    pModbusStruct->modbusDiag.goodException = (uint32_t) ((pModbusStruct->modbusDiag.goodException + 1) & 0xFFFFFFFF);
                }
         break;
       default:
            COMRequest.startAddress = 0;
            COMRequest.length = 0;
            u8ModbusADUSize=MBTCP_FUNC;
            u8ModbusADU[u8ModbusADUSize++] = 0x80 | COMRequest.functionCode;
            u8ModbusADU[u8ModbusADUSize++] = MODBUS_EXCPT__ILLEGAL_FNC;
            u8ModbusADU[MBTCP_LEN] = 0;
            u8ModbusADU[MBTCP_LEN + 1] = (u8ModbusADUSize - 6);
            pModbusStruct->modbusDiag.goodException = (uint32_t) ((pModbusStruct->modbusDiag.goodException + 1) & 0xFFFFFFFF);  // allow rollover
         break;
     }
     //pModbusStruct->txBufferIndex=0;
     pModbusStruct->modbusConnection[ClientConn].txRemainingSize=u8ModbusADUSize;//pModbusStruct->txRemainingSize=u8ModbusADUSize;
     pModbusStruct->modbusConnection[ClientConn].rxBufferSize=0;//pModbusStruct->rxBufferSize=0;
     ModbusTCPServer_SendResponse(pModbusStruct,ClientConn);

     memset(pModbusStruct->modbusConnection[ClientConn].txBuffer,0,MBTCP_BUF_SIZE);
     memset(pModbusStruct->modbusConnection[ClientConn].rxBuffer,0,MBTCP_BUF_SIZE);
     pModbusStruct->modbusConnection[ClientConn].txRemainingSize=0;
     pModbusStruct->modbusConnection[ClientConn].rxBufferSize=0;
}
//****************************************************************************
//*
//* Function name: ModbusTCPServer_CreateSocket
//*
//* Return: None
//*
//* Description:
//*   Create Modbus TCP Listen Socket
//*
//****************************************************************************

static uint8_t ModbusTCPServer_CreateSocket(MODBUS_TCP_RAM_STRUCT *pModbusStruct)
{
    int tempSockfd=-1;
    int option=1,rc=0;
    struct sockaddr_in local;
    uint32_t addr_len = sizeof(struct sockaddr);



    if((networkConfig.CurrentState != STATE_LWIP_WORKING) && (pModbusStruct->listenSockfd < 0))
    {
        return 0;
    }

    if((pModbusStruct->listenSockfd < 0) && (pModbusStruct->mb_clientIndex < MBTCP_MAX_CONNECTION))
    {
        local.sin_port = htons(MBTCP_DEFAULT_PORT);
        local.sin_family = PF_INET;
        local.sin_addr.s_addr = INADDR_ANY;

        tempSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (tempSockfd < 0)
        {

            MBTCP_printf("MBTCP:ERROR: Create socket\r\n");
            return 0;
        }
        MBTCP_printf("MBTCP: SUCCESS: Create socket\r\n");


        /*************************************************************/
        /* Allow socket descriptor to be reuseable                   */
        /*************************************************************/
        rc = setsockopt(tempSockfd, SOL_SOCKET, SO_REUSEADDR,
                        (char *)&option, sizeof(option));
        if (rc < 0)
        {

           MBTCP_printf("MBTCP:setsockopt() failed\r\n");
           close(tempSockfd);
           return 0;
        }

        /*************************************************************/
        /* Set socket to be nonblocking. All of the sockets for      */
        /* the incoming connections will also be nonblocking since   */
        /* they will inherit that state from the listening socket.   */
        /*************************************************************/
        rc = ioctl(tempSockfd, FIONBIO, (char *)&option);
        if (rc < 0)
        {

           MBTCP_printf("MBTCP:ioctl() failed\r\n");
           close(tempSockfd);
           return 0;
        }

        /*             21      */
        if (bind(tempSockfd, (struct sockaddr *) &local, addr_len) < 0)
        {

            MBTCP_printf("MBTCP:ERROR: Bind socket\r\n");
            close(tempSockfd);
            return 0;
        }
        MBTCP_printf("MBTCP:SUCCESS: Bind socket\r\n");

        if (listen(tempSockfd, 1) < 0)//MBTCP_MAX_CONNECTION
        {

            MBTCP_printf("MBTCP:ERROR: Listen %d socket connections\r\n", MBTCP_MAX_CONNECTION);
            close(tempSockfd);
            return 0;
        }
        MBTCP_printf("MBTCP:SUCCESS: Listen socket\r\n");

        pModbusStruct->listenSockfd = tempSockfd;
        //pModbusStruct->maxfd = tempSockfd;
    }

    return 1;
}



//****************************************************************************
//*
//* Function name: ModbusTCPServer_LookForNewConnection
//*
//* Return: None
//*
//* Description:
//*   Accepts New Modbus TCP connections  
//*
//****************************************************************************
//==============================================================================

static void ModbusTCPServer_LookForNewConnection(MODBUS_TCP_RAM_STRUCT *pModbusStruct)
{
    int tempSockfd=-1;
    struct sockaddr_in remote;
    uint32_t addr_len = sizeof(struct sockaddr);
    fd_set readFD;
    struct timeval timeout;
    uint8_t i=0;

    timeout.tv_sec  = 0;
    timeout.tv_usec = 2000;

    if(pModbusStruct->mb_clientIndex < MBTCP_MAX_CONNECTION)
    {

        FD_ZERO(&readFD);
        FD_SET(pModbusStruct->listenSockfd, &readFD);//maxfd

        if ((select(pModbusStruct->listenSockfd + 1, &readFD, 0, 0, &timeout) > 0))
        {
            if (FD_ISSET(pModbusStruct->listenSockfd, &readFD))
            {

                FD_CLR(pModbusStruct->listenSockfd, &readFD);

                tempSockfd = accept(pModbusStruct->listenSockfd, (struct sockaddr *) &remote,
                                (socklen_t *) & addr_len);
                if (tempSockfd < 0)
                {
                    //handle this aituation later
                }
                else
                {
                    MBTCP_printf("MBTCP: Got connection from %s maxfd = %d\r\n", inet_ntoa(remote.sin_addr),tempSockfd);
                    for(i=0;i<MBTCP_MAX_CONNECTION; i++)
                    {

                        if(pModbusStruct->modbusConnection[i].sockfd < 0)//pModbusStruct->sockfd[i]
                        {

                            //pModbusStruct->ConnFD[i].connFlag=TRUE;
                            pModbusStruct->modbusConnection[i].sockfd = tempSockfd;//pModbusStruct->sockfd[i]
                            pModbusStruct->modbusConnection[i].remote = remote;//pModbusStruct->remote[pModbusStruct->mb_clientIndex]
                            pModbusStruct->modbusConnection[i].rxBufferSize = 0;//pModbusStruct->rxBufferSize = 0;
                            ModbusTCPServer_StartConnectionTimer(pModbusStruct,i);
                            pModbusStruct->mb_clientIndex++;
                            //pModbusStruct->modbusDiag.clientConnections = pModbusStruct->mb_clientIndex;
                            break;
                        }
                    }
                    if(pModbusStruct->mb_clientIndex >= MBTCP_MAX_CONNECTION)
                    {

                        //FD_CLR(pModbusStruct->listenSockfd, &readFD);
                        close(pModbusStruct->listenSockfd);
                        pModbusStruct->listenSockfd = -1;
                       // pModbusStruct->mb_clientIndex--;
                    }

                }
            }
            else
            {
                FD_CLR(pModbusStruct->listenSockfd, &readFD);
            }
        }
    }
}


//****************************************************************************
//*
//* Function name: ModbusTCPServer_LookForNewRequest
//*
//* Return: None
//*
//* Description:
//*   Recieves new packets from different clients and processes it. Also it sends replies
//*
//****************************************************************************
//==============================================================================

static void ModbusTCPServer_LookForNewRequest(MODBUS_TCP_RAM_STRUCT *pModbusStruct)
{
    int numbytes;
    fd_set readFD;
    struct timeval timeout;
    uint16_t usLength;
    uint8_t i=0;


    timeout.tv_sec  = 0;
    timeout.tv_usec = 10000;


    if(pModbusStruct->mb_clientIndex > 0)
    {
        for(i=0;i<MBTCP_MAX_CONNECTION;i++)
        {
            if(pModbusStruct->modbusConnection[i].sockfd >= 0)//pModbusStruct->ConnFD[i].sockfd != -1
            {

                FD_ZERO(&readFD);

                FD_SET(pModbusStruct->modbusConnection[i].sockfd, &readFD);

                if (select((pModbusStruct->modbusConnection[i].sockfd + 1), &readFD, 0, 0, &timeout) > 0)//pModbusStruct->sockfd[i]
                {

                    if (FD_ISSET(pModbusStruct->modbusConnection[i].sockfd, &readFD)) //pModbusStruct->sockfd[i]
                    {
                        FD_CLR(pModbusStruct->modbusConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                        numbytes = recv(pModbusStruct->modbusConnection[i].sockfd,
                                        &pModbusStruct->modbusConnection[i].rxBuffer[pModbusStruct->modbusConnection[i].rxBufferSize],
                                        (MBTCP_BUF_SIZE - pModbusStruct->modbusConnection[i].rxBufferSize), 0);

                        if (numbytes <= 0)
                        {
                            // ErrnoIndex = errno;
                             if (errno != EWOULDBLOCK)
                             {
                                MBTCP_printf("MBTCP:recv() failed-> listenSockfd= %d\r\n",pModbusStruct->modbusConnection[i].sockfd);
                                MBTCP_printf("MBTCP: Client %s disconnected\r\n", inet_ntoa(pModbusStruct->modbusConnection[i].remote.sin_addr));
                                //FD_CLR(pModbusStruct->modbusConnection[i].sockfd, &readFD);//pModbusStruct->sockfd[i]
                                close(pModbusStruct->modbusConnection[i].sockfd);//pModbusStruct->sockfd[i]
                                pModbusStruct->modbusConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                                pModbusStruct->mb_clientIndex--;
                            //  ErrnoIndex = 0;
                             }
                        }
                        else
                        {
                            pModbusStruct->modbusConnection[i].rxBufferSize +=numbytes;//pModbusStruct->rxBufferSize +=numbytes;
                            //MBTCP_printf("MBTCP:recv()--> %d Total %d\n",numbytes, pModbusStruct->rxBufferSize);

                            /* If we have received the MBAP header we can analyze it and calculate
                             * the number of bytes left to complete the current request. If complete
                             * notify the protocol stack.
                             */
                            if( pModbusStruct->modbusConnection[i].rxBufferSize >= MBTCP_FUNC )
                            {
                                /* Length is a byte count of Modbus PDU (function code + data) and the
                                 * unit identifier. */
                                usLength = pModbusStruct->modbusConnection[i].rxBuffer[MBTCP_LEN] << 8U;
                                usLength |= pModbusStruct->modbusConnection[i].rxBuffer[MBTCP_LEN + 1];

                                /* Is the frame already complete. */
                                if( pModbusStruct->modbusConnection[i].rxBufferSize < ( MBTCP_UID + usLength ) )
                                {
                                }
                                else if( pModbusStruct->modbusConnection[i].rxBufferSize == ( MBTCP_UID + usLength ) )
                                {
                                    pModbusStruct->modbusDiag.framesRecv = (uint32_t) ((pModbusStruct->modbusDiag.framesRecv + 1) & 0xFFFFFFFF);    // allow rollover
                                    ModbusTCPServer_ProcessRequest(pModbusStruct, i);
                                    ModbusTCPServer_StartConnectionTimer(pModbusStruct,i);
                                }
                                else
                                {
                                    /* This should not happen. We can't deal with such a client and
                                     * drop the connection for security reasons.
                                     */
                                    pModbusStruct->modbusConnection[i].rxBufferSize=0;
                                    pModbusStruct->modbusDiag.otherError = (uint32_t) ((pModbusStruct->modbusDiag.otherError + 1) & 0xFFFFFFFF);    // allow rollover
                                }
                            }
                        }
                    }
                    else
                    {
                        FD_CLR(pModbusStruct->modbusConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                    }
                }
                else if((ModbusTCPServer_CheckConnectionTimeout(pModbusStruct,i) == TRUE))
                {
                    MBTCP_printf("MBTCP:recv() keep Alive Error\n");
                    MBTCP_printf("MBTCP: Client %s disconnected\r\n", inet_ntoa(pModbusStruct->modbusConnection[i].remote.sin_addr));
                    //FD_CLR(pModbusStruct->modbusConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                    close(pModbusStruct->modbusConnection[i].sockfd); //pModbusStruct->sockfd[i]
                    pModbusStruct->modbusConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                    pModbusStruct->mb_clientIndex--;
                }

            }
        }
    }

}


//==============================================================================


//==============================================================================

//****************************************************************************
//*
//* Function name: ModbusTCPServer_SocketOperation
//*
//* Return: None
//*
//* Description:
//*   Initializing Socket as server, check binding, accept the socket and send
//*   data over the socket, call by ModbusTCPSever_Task.
//*
//****************************************************************************
//==============================================================================

static void ModbusTCPServer_SocketOperation(MODBUS_TCP_RAM_STRUCT *pModbusStruct)
{

    if(ModbusTCPServer_CreateSocket(pModbusStruct))
    {
        ModbusTCPServer_LookForNewConnection(pModbusStruct);

        ModbusTCPServer_LookForNewRequest(pModbusStruct);

        pModbusStruct->modbusDiag.clientConnections = pModbusStruct->mb_clientIndex;

    }
}

//==============================================================================

//****************************************************************************
//*
//* Function name: ModbusTCPServer_Task
//*
//* Return: None
//*
//* Description:
//*   Modbus server task with initializing it socket and all operations.
//*
//****************************************************************************
//==============================================================================

void ModbusTCPServer_Task()
{

    while(1)
    {
        Error_ResetTaskWatchdog(MODBUS_TCP_TASK_ID);
		if(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable ==1)
		{
        	ModbusTCPServer_SocketOperation(pModbusStruct);
		}
        vTaskDelay(10);

    }

}
//==============================================================================

//==============================================================================
// END OF COMModbusTCPServer.c FIlE
//==============================================================================

