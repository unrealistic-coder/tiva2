//****************************************************************************
//*
//*
//* File: OnChipEEPROM.c
//*
//* Synopsis: FTP Server application over the top of LWIP library
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <OnChipEEPROM.h>
#include "ErrorHandler.h"
#include "SPIFlashTask.h"
#include "SMTPManager.h"
#include "stdio.h"
#include "string.h"
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

volatile EEPROM_MAPPED_STRUCT Device_Configurations;

// Table of CRC 16 values
const uint16_t CRC_Table[256] =
{
   0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
   0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
   0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0E40,
   0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
   0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
   0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
   0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
   0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
   0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
   0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
   0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
   0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
   0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
   0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
   0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
   0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
   0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
   0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
   0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
   0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
   0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
   0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
   0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
   0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
   0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
   0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
   0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
   0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
   0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
   0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
   0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
   0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//*****************************************************************
//*
//* Function name:   COMCrcGenerator
//*
//* Return:  uint16 crc : calculated CRC value
//*
//* Description: Returns 16bit calculated CRC
//*
//*****************************************************************
uint16_t COMCrcGenerator( const void* crcDataPtr, uint16_t crcDataLength)
{
   uint8_t * bPtr = (uint8_t *) crcDataPtr;
   uint16_t crc=0xFFFF;

   for ( ; crcDataLength--; )
   {
      crc = CRC_Table[(crc ^ *bPtr++) & 0x00FF] ^ ((crc >> 8) & 0x00FF);
   }

   return crc;
}

//****************************************************************************
//*
//* Function name: OnChipEEPROM_Read
//*
//* Return:  1 - Successfully read Data
//*			 0 - Fail to Read
//*
//*
//* Description:
//*   Read Data from Internal EEPROM and also generate CRC to check integrity 
//*   of Data.
//*
//****************************************************************************
//==============================================================================

uint8_t OnChipEEPROM_Read(void*Buffer, uint16_t *CRC_Check, uint32_t StartAddress, uint16_t DataLen)
{
    *CRC_Check = 0;
    uint8_t Status = 0;

    if(Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)//if(Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)
    {
        EEPROMRead((uint32_t *)Buffer, StartAddress, DataLen);
        *CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen - 2);
        Status = 1;
    }
    return Status;
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_CopyData
//*
//* Return: None
//*
//*
//* Description:
//*   Copy data from Source to Destination.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_CopyData(void*Dst, void*Src, uint16_t Len)
{
    uint8_t * SrcPtr = (uint8_t *) Src;
    uint8_t * DstPtr = (uint8_t *) Dst;
    uint16_t Cnt = 0;

    for(Cnt=0; Cnt < Len; Cnt++)
    {
        DstPtr[Cnt] = SrcPtr[Cnt];
    }

}

//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultUPSSchdShtDwnSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of RTC Compensate, if there is corruption in Memory
//*   EERPOM Fail, or fail to validate Data. Integrity of data validate by CRC
//*   check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultUPSSchdShtDwnSettings(void)
{
    uint16_t  CRC_Cal=0, cnt=0;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.UPSShutDownSt.TransBuffer);

    Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_HH = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_MM = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_SS = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByBattLow  = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByTime  = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_HH  = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_MM  = 0;
    Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_SS  = 0;

    sprintf((char *)Device_Configurations.EepromMapStruct.UPSShutDownSt.NTPServer, "pool.ntp.org");


   //MEMCPY((uint8_t*)&Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer,
    //        "pool.ntp.org", sizeof(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer));

    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE-2);
    Device_Configurations.EepromMapStruct.UPSShutDownSt.CRC_RTCCompensSett = CRC_Cal;

    //Save Default Setting to EEPROM:-
    //Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

    OnChipEEPROM_Write(RTC_COMPENSATORY_SAVE, (uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE);

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultRTCCompensateSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of RTC Compensate, if there is corruption in Memory
//*   EERPOM Fail, or fail to validate Data. Integrity of data validate by CRC
//*   check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultRTCCompensateSettings(void)
{
    uint16_t  CRC_Cal=0, cnt=0;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.RTCCompenConfig.TransBuffer);

    Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroyafterhourSet = 0;
    Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroySecondsSet   = 0;
    Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable     = 1;

    memset((uint8_t*)Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer,
           0x00, sizeof(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer));

    sprintf((char *)Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer, "pool.ntp.org");


   //MEMCPY((uint8_t*)&Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer,
    //        "pool.ntp.org", sizeof(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer));

    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE-2);
    Device_Configurations.EepromMapStruct.RTCCompenConfig.CRC_RTCCompensSett = CRC_Cal;

    //Save Default Setting to EEPROM:-
    //Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

    OnChipEEPROM_Write(RTC_COMPENSATORY_SAVE, (uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE);

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultADCSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of ADC Module, if there is corruption in Memory
//*   EERPOM Fail, or fail to validate Data. Integrity of data validate by CRC
//*   check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultSerialSettings(void)
{
    uint16_t  CRC_Cal=0, cnt=0;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SerialSettings.TransBuffer);

    for(cnt=0; cnt<SYSTEM_SERIAL_PORT_COUNT; cnt++)
    {
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[cnt].baudrate = SERIALPORT_BAUD_19200;
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[cnt].parity   = SERIALPORT_PARITY_NONE;
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[cnt].databit  = SERIALPORT_DATA_8BIT;
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[cnt].stopbit  = SERIALPORT_STOPBIT_1;
    }

    //Keeping Parity Even for Modbus Master

    Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[SERIAL_STDIO].protocolType
                                                                            = SERIAL_STDIO;
    Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[SERIAL_GSM_GPRS].protocolType
                                                                            = SERIAL_GSM_GPRS;


    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, SERIAL_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.Save_SerialSettings.CRC_Check_SerialSett = CRC_Cal;


    //Save Default Setting to EEPROM:-
    //Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

    OnChipEEPROM_Write(SERIAL_CONFIG_SAVE, (uint8_t*)Buffer, SERIAL_CONFIG_SIZE);

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ModuleUpdateCounter
//*
//* Return: None
//*
//*
//* Description:
//*   Counter for each module will update/increment for each indiviual module
//*   update.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ModuleUpdateCounter(EEPROM_MODULES ModuleNo, BOOL Counter)
{
	if(ModuleNo <= NO_OF_CONFIG_MODULE)
	{
	    Device_Configurations.EEPROMCurrentState.EEPROMWriteCounter[ModuleNo] += Counter;
	}
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultSNMPTrapSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of Ethernet/IP Module, if there is corruption in Memory
//*   EERPOM Fail, or fail to validate Data. Integrity of data validate by CRC
//*   check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultSNMPTrapSettings(void)
{
	uint16_t  CRC_Cal=0, cnt=0;
	uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TransBuffer);

	Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.all=0xFFFFFFFF;//0x5ED6820A;

	for (cnt =0; cnt < 64; cnt++)
	{
	   Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[cnt].Bits.risingedgetrap=1;
	}
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[0].Bits.typeofTrap=1;  //battery disconnected
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[7].Bits.typeofTrap=1;  //Battery Low
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[10].Bits.typeofTrap=1;  //EPO Enabled
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[13].Bits.typeofTrap=1;    //ChargerFail
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[14].Bits.typeofTrap=1;   //Remote ShutDown
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[37].Bits.typeofTrap=1;   //BatteryCellOvercharged
     Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[57].Bits.typeofTrap=1;   //MaintainSWOpen


	Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo=163;
	CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE-2);
	Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.CRC = CRC_Cal;


	//Save Default Setting to EEPROM:-
	//Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

	OnChipEEPROM_Write(SNMPTrap_CONFIG_SAVE, (uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE);

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultEthernetSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of Ethernet, 
//*	  if there is corruption in Memory EERPOM Fail, or fail to validate Data. 
//*   Integrity of data validate by CRC check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultEthernetSettings(void)
{
	uint16_t CRC_Cal = 0;
	uint8_t *Buffer = (uint8_t*) &(Device_Configurations.EepromMapStruct.Save_EthernetSettings.TransBuffer);

	//bit 0-> DHCP Enable
	//bit 1-> Modbus TCP Enable
	//bit 2-> Web server Enable
	//bit 3->BACNet Server Enable
    sprintf((char *)Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName, "FUJI UPS CARD");
//	MEMCPY((uint8_t*)&Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName,
	//		"FUJI UPS CARD", SIZE_OF_HOSTNAME);

	//Device_Configurations.Save_EthernetSettings.CONFIG_Enable.all       = 0x000E;//0x0006;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.dhcpEnable=1;

	//Device_Configurations.Save_EthernetSettings.CONFIG_Enable.bits.BACServerEnable = 1;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable=1;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.FTPServerEnable=0;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable=1;

	//-------------------------------------------------------------------
	//	DHCP:- Enabled
	//	Default IP address :- 0.0.0.0
	//	Default GW address :- 0.0.0.0
	//	Default Subnet address :- 0.0.0.0
	//--------------------------------------------------------------------

	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_IP_Address.all  = 0x00000000;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_GateWay.all     = 0x00000000;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Subnet.all      = 0x00000000;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Pri_DNS.all     = 0x00000000;
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Sec_DNS.all     = 0x00000000;
	

	CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, ETH_SETTS_CONFIG_SIZE-2);
	Device_Configurations.EepromMapStruct.Save_EthernetSettings.CRC = CRC_Cal;

	//Save Default Setting to EEPROM:- 
	//Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

	
	OnChipEEPROM_Write(ETH_SETTS_CONFIG_SAVE, (uint8_t*)Buffer, ETH_SETTS_CONFIG_SIZE);

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultMBTCPSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of Modbus TCP server, 
//*	  if there is corruption in Memory EERPOM Fail, or fail to validate Data. 
//*   Integrity of data validate by CRC check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultMBTCPSettings(void)
{
	uint16_t CRC_Cal = 0, cnt = 0;
	uint8_t *Buffer = (uint8_t*) &(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.TransBuffer);

	for(cnt = 0; cnt < MAX_MBTCP_IP; cnt++)
	{
	    Device_Configurations.EepromMapStruct.Save_MBTCPSettings.ModbusValidIP[cnt].all = 0x6400A8C0;
	}
	Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout = MBTCP_CONNECTION_TIMEOUT;
	Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port = MBTCP_DEFAULT_PORT;

	//Device_Configurations.Save_MBTCPSettings.EnableSuperViseTimer = 0;
	
	CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, MBTCP_CONFIG_SIZE-2);
	Device_Configurations.EepromMapStruct.Save_MBTCPSettings.CRC = CRC_Cal;

	//Save Default Setting to EEPROM:- 
	//Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

	OnChipEEPROM_Write(MBTCP_CONFIG_SAVE, (uint8_t*)Buffer, MBTCP_CONFIG_SIZE);

}

//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadADCConfig
//*
//* Return: None
//*
//*
//* Description:
//*   Read ADC Settings From EEPROM at the first execuation of Code. 
//*   CRC will decide integrity of Settings if all data is validated
//*   setting will apply to ADC Task otherwise default setting will apply.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadFaultRecord()
{
    uint16_t CRC_Cal = 0;

	if(OnChipEEPROM_Read((uint8_t*)&Device_Configurations.EepromMapStruct.faultRecordStruct,&CRC_Cal,
		FAULT_RECORD_START, sizeof(FAULT_STRUCT)))
	{
		if(CRC_Cal != Device_Configurations.EepromMapStruct.faultRecordStruct.crc)
		{
			memset((void *)&Device_Configurations.EepromMapStruct.faultRecordStruct,0xFF,sizeof(FAULT_STRUCT));
		}
	}
	else
	{
		memset((void *)&Device_Configurations.EepromMapStruct.faultRecordStruct,0xFF,sizeof(FAULT_STRUCT));
	}
}


//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultGSMSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of GSM and GPRS,
//*   if there is corruption in Memory EERPOM Fail, or fail to validate Data.
//*   Integrity of data validate by CRC check.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_DefaultGSMSettings(void)
{
    uint16_t CRC_Cal = 0;
    uint8_t *Buffer = (uint8_t*) &(Device_Configurations.EepromMapStruct.Save_GSMSettings.TransBuffer);

//    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_GPRS_IP_Address.all = 0xC0A80101;
//    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_GPRS_Port_Number = 27000;
//    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_Server_Connection_Interval = 3600;

    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].all = 910000000000;
    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].all = 910000000000;
    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].all = 910000000000;
    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].all = 910000000000;
    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].all = 910000000000;

    Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS =0;

    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, GSM_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.Save_GSMSettings.CRC_CheckGSMSett = CRC_Cal;

    //Save Default Setting to EEPROM:-
    //Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

    OnChipEEPROM_Write(GSM_CONFIG_SAVE, (uint8_t*)Buffer, GSM_CONFIG_SIZE);
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultHTTPSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of HTTP web server,
//*   if there is corruption in Memory EERPOM Fail, or fail to validate Data.
//*   Integrity of data validate by CRC check.
//*
//****************************************************************************
//==============================================================================
static void OnChipEEPROM_DefaultHTTPSettings(void)
{
    uint16_t CRC_Cal = 0;
    uint8_t *Buffer = (uint8_t*) &(Device_Configurations.EepromMapStruct.Save_HTTPSettings.TransBuffer);
    Device_Configurations.EepromMapStruct.Save_HTTPSettings.CONFIG_ADCStringVisibile = 36;
    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, HTTP_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.Save_HTTPSettings.CRC_CheckHTTPSett = CRC_Cal;

    //Save Default Setting to EEPROM:-
    //Consequences - 1. First time Save. 2. Memory Corrupt 3. Memory Change 4. Addon New Settings

    OnChipEEPROM_Write(HTTP_CONFIG_SAVE, (uint8_t*)Buffer, HTTP_CONFIG_SIZE);
}
//****************************************************************************
//*
//* Function name: OnChipEEPROM_DefaultEmailSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Default settings of HTTP web server,
//*   if there is corruption in Memory EERPOM Fail, or fail to validate Data.
//*   Integrity of data validate by CRC check.
//*
//****************************************************************************
//==============================================================================
static void OnChipEEPROM_DefaultEmailSettings(void)
{
    uint16_t CRC_Cal = 0;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.EmailConfig.TransBuffer);

    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom, 0x00, MAX_LEN);
    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add, 0x00, MAX_LEN);
    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add, 0x00, MAX_LEN);
    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add, 0x00, MAX_LEN);
    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword, 0x00, MAX_LEN);
    memset((uint8_t*) Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost, 0x00, MAX_LEN);

    Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent = 0;
    Device_Configurations.EepromMapStruct.EmailConfig.SSLEnable = 0;
    Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort = 587;

	Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled = 0;
	Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval =SMTP_CONNTIMEOUT;
    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, EMAIL_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.EmailConfig.CRC_EmailSett = CRC_Cal;

    OnChipEEPROM_Write(EMAIL_CONFIG_SAVE, (uint8_t*)Buffer, EMAIL_CONFIG_SIZE);
}

static void OnChipEEPROM_UPSSchdShutDwnSettings()
{
    uint16_t CRC_Cal = 0;
    UPSSHUTDOWNSTRUCT TempHoldData;

      if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
          (OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                             &CRC_Cal, UPS_SCHDSHUTDWN_START, UPS_SCHEDULEDSHUTDWN_SIZE)))
      {
          if(CRC_Cal == TempHoldData.CRC_RTCCompensSett)
          {
              OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.UPSShutDownSt.TransBuffer,
                          (uint8_t*)&TempHoldData.TransBuffer,
                          UPS_SCHEDULEDSHUTDWN_SIZE);
          }
          else
          {
              OnChipEEPROM_DefaultUPSSchdShtDwnSettings();
          }
      }
      else
      {
          OnChipEEPROM_DefaultUPSSchdShtDwnSettings();
      }
}
static void OnChipEEPROM_RTCCompensateSettings()
{
    uint16_t CRC_Cal = 0;
    RTCCOMPENSTRUCT TempHoldData;

      if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
          (OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                             &CRC_Cal, RTC_COMPENASTORY_START, RTC_COMPENSORTORY_SIZE)))
      {
          if(CRC_Cal == TempHoldData.CRC_RTCCompensSett)
          {
              OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.RTCCompenConfig.TransBuffer,
                          (uint8_t*)&TempHoldData.TransBuffer,
                          RTC_COMPENSORTORY_SIZE);
          }
          else
          {
              OnChipEEPROM_DefaultRTCCompensateSettings();
          }
      }
      else
      {
          OnChipEEPROM_DefaultRTCCompensateSettings();
      }
}
static void OnChipEEPROM_ReadEmailSettings()
{
    uint16_t CRC_Cal = 0;
    EMAILSTRUCT TempHoldData;

      if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
          (OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                             &CRC_Cal, EMAIL_CONFIG_START, EMAIL_CONFIG_SIZE)))
      {
          if(CRC_Cal == TempHoldData.CRC_EmailSett)
          {
            OnChipEEPROM_CopyData(
                    (uint8_t*) &Device_Configurations.EepromMapStruct.EmailConfig.TransBuffer,
                          (uint8_t*)&TempHoldData.TransBuffer,
                          EMAIL_CONFIG_SIZE);
          }
          else
          {
              OnChipEEPROM_DefaultEmailSettings();
          }
      }
      else
      {
          OnChipEEPROM_DefaultEmailSettings();
      }
}
//============================================================================
//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadSNMPTrapSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read SNMP Trap Settings From EEPROM at the first execuation of Code.
//*   CRC will decide integrity of Settings if all data is validated
//*   setting will apply to Serial task otherwise default setting will apply.
//*
//****************************************************************************
//==============================================================================
static void OnChipEEPROM_ReadSNMPTrapSettings()
{
    uint16_t CRC_Cal = 0;
    CONFIG_SNMPTrapSettings TempHoldData;

	if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
		(OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
						   &CRC_Cal, SNMPTrap_CONFIG_START, SNMPTrap_CONFIG_SIZE)))
	{
		if(CRC_Cal == TempHoldData.CRC)
		{
			OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TransBuffer,
						(uint8_t*)&TempHoldData.TransBuffer,
						SNMPTrap_CONFIG_SIZE);
		}
		else
		{
			OnChipEEPROM_DefaultSNMPTrapSettings();
		}
	}
	else
	{
	    OnChipEEPROM_DefaultSNMPTrapSettings();
	}
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadSerialSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read Serial Settings From EEPROM at the first execuation of Code.
//*   CRC will decide integrity of Settings if all data is validated
//*   setting will apply to Serial task otherwise default setting will apply.
//*
//****************************************************************************
//==============================================================================
static void OnChipEEPROM_ReadSerialSettings()
{
    uint16_t CRC_Cal = 0;
    CONFIG_SerialSettings TempHoldData;

    if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
        (OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, SERIAL_CONFIG_START, SERIAL_CONFIG_SIZE)))
    {
        if(CRC_Cal == TempHoldData.CRC_Check_SerialSett)
        {
            OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_SerialSettings.TransBuffer,
                        (uint8_t*)&TempHoldData.TransBuffer,
                        SERIAL_CONFIG_SIZE);

        }
        else
        {
            OnChipEEPROM_DefaultSerialSettings();
        }
    }
    else
    {
        OnChipEEPROM_DefaultSerialSettings();
    }
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadEthernetSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read Ethernet Settings From EEPROM at the first execuation of Code. 
//*   CRC will decide integrity of Settings if all data is validated
//*   setting will apply to Ethernet task otherwise default setting will apply.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadEthernetSettings()
{
    uint16_t CRC_Cal = 0;
	CONFIG_EthernetSettings TempHoldData;
	
    if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
		(OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, ETH_SETTS_CONFIG_START, ETH_SETTS_CONFIG_SIZE)))
    {
        if(CRC_Cal == TempHoldData.CRC)
        {
            OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_EthernetSettings.TransBuffer,
                        (uint8_t*)&TempHoldData.TransBuffer,
                        ETH_SETTS_CONFIG_SIZE);
        }
        else
        {
            OnChipEEPROM_DefaultEthernetSettings();
        }
    }
    else
    {
        OnChipEEPROM_DefaultEthernetSettings();
    }

}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadGSMSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read GSM/GPRS Settings From EEPROM at the first execuation of Code.
//*   CRC will decide integrity of Settings if all data is validated
//*   setting will apply to GSM/GPRS task otherwise default setting will apply.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadGSMSettings()
{
    uint16_t CRC_Cal = 0;
    CONFIG_GSMSettings TempHoldData;
    Device_Configurations.EepromMapStruct.Save_GSMSettings.ConfigFlags.bits.GSMSMS_Enabled=1;
    if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
        (OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, GSM_CONFIG_START, GSM_CONFIG_SIZE)))
    {
        if(CRC_Cal == TempHoldData.CRC_CheckGSMSett)
        {
            OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_GSMSettings.TransBuffer,
                        (uint8_t*)&TempHoldData.TransBuffer,
                        GSM_CONFIG_SIZE);

        }
        else
        {
            OnChipEEPROM_DefaultGSMSettings();
        }
    }
    else
    {
        OnChipEEPROM_DefaultGSMSettings();
    }
  //  HTTP_UpdateSMSGroupConfig(0,Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS);
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadMBTCPSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read Modbus TCP server Settings From EEPROM at the first execuation of
//*   Code. CRC will decide integrity of Settings if all data is validated
//*   setting will apply to Modbus TCP server task otherwise default setting 
//*   will apply.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadMBTCPSettings()
{
    uint16_t CRC_Cal = 0;
	CONFIG_MBTCPSettings TempHoldData;
	
    if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)&&
		(OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, MBTCP_CONFIG_START, MBTCP_CONFIG_SIZE)))
    {
        if(CRC_Cal == TempHoldData.CRC)
        {
            OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_MBTCPSettings.TransBuffer,
                        (uint8_t*)&TempHoldData.TransBuffer,
                        MBTCP_CONFIG_SIZE);
        }
        else
        {
            OnChipEEPROM_DefaultMBTCPSettings();
        }
    }
    else
    {
        OnChipEEPROM_DefaultMBTCPSettings();
    }
}

//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadHTTPSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read HTTP Web Server Settings From EEPROM at the first execuation of
//*   Code. CRC will decide integrity of Settings if all data is validated
//*   setting will apply to HTTP web server otherwise default setting will
//*   apply.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadHTTPSettings()
{
    uint16_t CRC_Cal = 0;
    CONFIG_HTTPSettings TempHoldData;
    if((Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)
        &&(OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, HTTP_CONFIG_START, HTTP_CONFIG_SIZE)))
    {
        if(CRC_Cal == TempHoldData.CRC_CheckHTTPSett)
        {
            OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_HTTPSettings.TransBuffer,
                        (uint8_t*)&TempHoldData.TransBuffer,
                        HTTP_CONFIG_SIZE);

        }
        else
        {
            OnChipEEPROM_DefaultHTTPSettings();
        }
    }
    else
    {
        OnChipEEPROM_DefaultHTTPSettings();
    }
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadInfoSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Read System Information stored by Device in OS mode or in Boot mode.
//*   NO CRC Check will pay roll here. OS version is added by write function.
//*    
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadInfoSettings()
{
	CONFIG_SystemInfo TempHoldData;
	//uint8_t *Buffer =(uint8_t*) &(Device_Configurations.Save_SystemInfo.TransBuffer);
	
	EEPROMRead((uint32_t *)&TempHoldData.TransBuffer, SYSTEM_INFO_START, SYSTEM_INFO_SIZE);

    OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_SystemInfo.TransBuffer,
            (uint8_t*)&TempHoldData.TransBuffer, SYSTEM_INFO_SIZE);
            	
	Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version = (((SNMP_CARD_MAJOR_VERSION<<8) & 0xFF00)|
													(SNMP_CARD_MINOR_VERSION & 0x00FF));
	
	//write Directly in EEPROM
	//OnChipEEPROM_Write(SYSTEM_INFO_SAVE, (uint8_t*)Buffer, SYSTEM_INFO_SIZE);
}

//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadMACAddress
//*
//* Return: None
//*
//*
//* Description:
//*   Read MAC Address stored by Device in Boot mode.
//*   CRC Check will conduct as per other settings
//*    
//*
//****************************************************************************
//==============================================================================
static void OnChipEEPROM_ReadMACAddress()
{
	CONFIG_MACAddressInfo TempHoldData;
	uint16_t CRC_Cal = 0;

	memset((uint8_t*)&Device_Configurations.EepromMapStruct.Save_MACAddress.TransBuffer[0], 0xFF, MAC_ADDRESS_SIZE);

    if(OnChipEEPROM_Read((uint8_t*)&TempHoldData.TransBuffer,
                           &CRC_Cal, MAC_ADD_INFO_START, MAC_ADDRESS_SIZE))
    {
        if(CRC_Cal == TempHoldData.CRC)
        {
                    OnChipEEPROM_CopyData((uint8_t*)&Device_Configurations.EepromMapStruct.Save_MACAddress.TransBuffer,
                    (uint8_t*)&TempHoldData.TransBuffer,
                    MAC_ADDRESS_SIZE);
        }
		else
		{
#if defined(DEBUG)////PART_TM4C129XNCZAD
	//For testing dummy mac address is given in debug mode
	
	//	ui32User0 = 0x00001AB6;
	//	ui32User1 = 0x000C18CC; //B6:1A:00:CC:18:0A

        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[0]=ETH_MAC_ADDR0;
        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[1]=ETH_MAC_ADDR1;
        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[2]=ETH_MAC_ADDR2;
        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[3]=ETH_MAC_ADDR3;
        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[4]=ETH_MAC_ADDR4;
        Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[5]=ETH_MAC_ADDR5;
#else
		Error_MajorFault(ETHERNET_MAC_FAULT);//Call an Error function here
#endif
		}
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: OnChipEEPROM_CheckFactorySetting
//*
//* Return: None
//*
//*
//* Description:
//*   Checking Factory Settings is enable/disable, to apply default settings and save in EEPROM
//*   Read Functions.
//*
//****************************************************************************
//==============================================================================
static BOOL OnChipEEPROM_CheckFactorySetting()
{
	BOOL FactoryReset = FALSE;
	uint8_t *Buffer =(uint8_t*) &(Device_Configurations.EepromMapStruct.Save_SystemInfo.TransBuffer);
	
	OnChipEEPROM_ReadInfoSettings();
	if(Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.FactorySettingEnable == 1)
	{
	    Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.FactorySettingEnable = 0;
		FactoryReset = TRUE;
	}

	if ((Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.BootType == OUT_OF_BOX_SERIAL) ||
		 (Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.BootType == OUT_OF_BOX_USB_FILE) ||
		 (Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.BootType == OUT_OF_BOX_ETHERNET))
	{
		//Here is Boot Type We received from Boot loader
		// Currently it is clearing after OS update
	    Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.BootType = OUT_OF_BOX_OS;
	}
	OnChipEEPROM_Write(SYSTEM_INFO_SAVE, (uint8_t*)Buffer, SYSTEM_INFO_SIZE);
	return FactoryReset;
}
//****************************************************************************
//*
//* Function name: OnChipEEPROM_ReadAllConfig
//*
//* Return: None
//*
//*
//* Description:
//*   Read all Module settings from EEPROM, function will call all module/task
//*   Read Functions.
//*
//****************************************************************************
//==============================================================================

static void OnChipEEPROM_ReadAllConfig(void)
{
	BOOL FactoryReset = FALSE;
	FactoryReset = OnChipEEPROM_CheckFactorySetting();
	if(FactoryReset == TRUE)
	{
		//if Factory Settings is enable the apply default settings and save in EEPROM
	    OnChipEEPROM_DefaultSNMPTrapSettings();
		OnChipEEPROM_DefaultMBTCPSettings();
		OnChipEEPROM_DefaultEthernetSettings();

		OnChipEEPROM_DefaultGSMSettings();
		OnChipEEPROM_DefaultSerialSettings();
		OnChipEEPROM_DefaultHTTPSettings();
		OnChipEEPROM_DefaultEmailSettings();
		OnChipEEPROM_DefaultRTCCompensateSettings();

	}
	else
	{
	    OnChipEEPROM_ReadSNMPTrapSettings();
		OnChipEEPROM_ReadMBTCPSettings();
		OnChipEEPROM_ReadEthernetSettings();
		OnChipEEPROM_ReadSerialSettings();
		OnChipEEPROM_DefaultHTTPSettings();
		OnChipEEPROM_ReadGSMSettings();
		OnChipEEPROM_ReadHTTPSettings();
		OnChipEEPROM_ReadEmailSettings();
		OnChipEEPROM_RTCCompensateSettings();
	}
	OnChipEEPROM_ReadMACAddress();
	OnChipEEPROM_ReadFaultRecord();
}
//============================================================================

//****************************************************************************
//*
//* Function name: IsInternalEEPROMActive
//*
//* Return: TRUE:- EEPROM Active
//*			FALSE:- EEPROM Fail/Inactive
//*
//*
//* Description:
//*   Function to check Internal EEPROM is active.
//*
//****************************************************************************
//==============================================================================

BOOL IsInternalEEPROMActive()
{
	return (Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive);
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_Init
//*
//* Return: None
//*
//*
//* Description:
//*   Initialize Internal EEPROM with its peripheral also create RTOS queue 
//*   Length of 20 for write operation in EEPROM.
//*
//****************************************************************************
//==============================================================================

void OnChipEEPROM_Init(void)
{
   
    // Saving configurations in 6K internal EERPROM
    uint32_t returnCode = 0;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); // EEPROM activate
    do
    {
        returnCode=EEPROMInit(); // EEPROM start
    }
    while(returnCode!=EEPROM_INIT_OK);
    Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive = TRUE;

	if(Device_Configurations.EEPROMCurrentState.Internal_EEPROMActive)
	{	
	    Device_Configurations.EEPROMCurrentState.xEEPROMSaveHandler =
						xQueueCreate(20, sizeof(EEPROMReqQueue));

		if(Device_Configurations.EEPROMCurrentState.xEEPROMSaveHandler!= NULL)
		{
			OnChipEEPROM_ReadAllConfig();
		}
		else
		{
			//Ideally should not received here
			EEPROM_printf("Error In Creating EEPROM Queue Operation\r\n");
			Error_MajorFault(ERR_FREERTOS_QUEUE_FAULT);//Call an Error function here
		}
	}
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_SendEEPROMSaveEvent
//*
//* Return: None
//*
//*
//* Description:
//*   Send queue request for saving any request in EEPROM from any module.
//*   event is only generated by HTTP web server.
//*
//****************************************************************************
//==============================================================================

void OnChipEEPROM_SendEEPROMSaveEvent(EEPROM_MODULES Event, uint8_t Status)
{
	EEPROMReqQueue EnqueueStatus;

	EnqueueStatus.ModuleNo 		= Event;
	EnqueueStatus.ModuleState 	= Status;
		xQueueSend(Device_Configurations.EEPROMCurrentState.xEEPROMSaveHandler,
				(EEPROMReqQueue*) &EnqueueStatus, (uint32_t)10);
	
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_SaveConfiguration
//*
//* Return: None
//*
//*
//* Description:
//*   Save request module settings in RCD Configuration with calculating new 
//*   CRC and wait to save in EEPROM.
//*
//****************************************************************************
//==============================================================================

void OnChipEEPROM_SaveConfiguration(EEPROM_MODULES Module)
{
	uint8_t *Buffer;
	uint16_t DataLen = 0;
	uint16_t CRC_Check = 0;
	switch(Module)
    {
	    case SNMPTrap_CONFIG_SAVE:
	        DataLen = SNMPTrap_CONFIG_SIZE;
			Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TransBuffer);
			CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
			Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.CRC = CRC_Check;
	        break;
	    case ETH_SETTS_CONFIG_SAVE:
			Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_EthernetSettings.TransBuffer);
	        DataLen = ETH_SETTS_CONFIG_SIZE;
			CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
			Device_Configurations.EepromMapStruct.Save_EthernetSettings.CRC = CRC_Check;
	        break;
			
	    case MBTCP_CONFIG_SAVE:
			Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.TransBuffer);
	        DataLen = MBTCP_CONFIG_SIZE;
			CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
			Device_Configurations.EepromMapStruct.Save_MBTCPSettings.CRC = CRC_Check;
	        break;

		case SYSTEM_INFO_SAVE:
			Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SystemInfo.TransBuffer);
			DataLen = SYSTEM_INFO_SIZE;
			//No need to check CRC in system info
			break;	

		case SERIAL_CONFIG_SAVE:
            DataLen = SERIAL_CONFIG_SIZE;
            Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SerialSettings.TransBuffer);
            CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CRC_Check_SerialSett = CRC_Check;
            break;

        case GSM_CONFIG_SAVE:
            Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_GSMSettings.TransBuffer);
            DataLen = GSM_CONFIG_SIZE;
            CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
            Device_Configurations.EepromMapStruct.Save_GSMSettings.CRC_CheckGSMSett = CRC_Check;
            break;

        case HTTP_CONFIG_SAVE:
            Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_HTTPSettings.TransBuffer);
            DataLen = HTTP_CONFIG_SIZE;
            CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
            Device_Configurations.EepromMapStruct.Save_HTTPSettings.CRC_CheckHTTPSett = CRC_Check;
            break;

        case EMAIL_CONFIG_SAVE:
            Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.EmailConfig.TransBuffer);
            DataLen = EMAIL_CONFIG_SIZE;
            CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
            Device_Configurations.EepromMapStruct.EmailConfig.CRC_EmailSett = CRC_Check;
            break;

        case  RTC_COMPENSATORY_SAVE:
               Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.RTCCompenConfig.TransBuffer);
               DataLen = RTC_COMPENSORTORY_SIZE;
               CRC_Check = COMCrcGenerator((uint8_t*)Buffer, DataLen-2);
               Device_Configurations.EepromMapStruct.RTCCompenConfig.CRC_RTCCompensSett = CRC_Check;
               break;
	    default:
			return;	
    }

	OnChipEEPROM_Write(Module, (uint8_t*)Buffer, DataLen);
	
}
//============================================================================

//****************************************************************************
//*
//* Function name: OnChipEEPROM_Write
//*
//* Return: None
//*
//*
//* Description:
//*   Write RCD_Configuration reques module structure in EEPROM set up in 
//*   OnChipEEPROM_SaveConfiguration it is called only by event.
//*   
//*
//****************************************************************************
//==============================================================================

void OnChipEEPROM_Write(EEPROM_MODULES Module, uint8_t *Value, uint16_t DataLen)
{
    BOOL Status = FALSE;
    uint32_t DataAddress = 0;
    uint32_t EEPROMSize = 0;
    switch(Module)
    {
        case SNMPTrap_CONFIG_SAVE:
            DataAddress = SNMPTrap_CONFIG_START;
            break;
			
	    case ETH_SETTS_CONFIG_SAVE:
	        DataAddress = ETH_SETTS_CONFIG_START;
	        break;
			
	    case MBTCP_CONFIG_SAVE:
	        DataAddress = MBTCP_CONFIG_START;
	        break;

	    case SYSTEM_INFO_SAVE:
			DataAddress = SYSTEM_INFO_START;
			break;
		
		case FAULT_RECORD_SAVE:
			DataAddress = FAULT_RECORD_START;
			break;

        case SERIAL_CONFIG_SAVE:
            DataAddress = SERIAL_CONFIG_START;
            break;

        case GSM_CONFIG_SAVE:
            DataAddress = GSM_CONFIG_START;
            break;

        case HTTP_CONFIG_SAVE:
            DataAddress = HTTP_CONFIG_START;
            break;
        case EMAIL_CONFIG_SAVE:
            DataAddress = EMAIL_CONFIG_START;
            break;
        case RTC_COMPENSATORY_SAVE:
              DataAddress = RTC_COMPENASTORY_START;
              break;

        default:
            return;// Highest Data Address
    }
    EEPROMSize = EEPROMSizeGet();
    if((DataAddress + DataLen) <= EEPROMSize)
    {
        EEPROMProgram((uint32_t *)Value, DataAddress, DataLen);
        Status = TRUE;
    }
   //=====================================================================//
   //Counter to Copy Data in EEPROM//
   //====================================================================//
    OnChipEEPROM_ModuleUpdateCounter(Module, Status);
}
//==============================================================================

//==============================================================================
// END OF OnChipEEPROM.c FIlE
//==============================================================================

