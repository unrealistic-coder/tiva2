//****************************************************************************
//*
//*
//* File: OnChipEEPROM.h
//*
//* Synopsis: Internal EEPROM - related Structures and interfaces
//*
//*
//****************************************************************************


#ifndef ONCHIP_EEPROM_H_
#define ONCHIP_EEPROM_H_

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
#include "ModbusTCPServer.h"
#include "EthernetTask.h"
#include "ErrorHandler.h"
#include "GSM_Comm.h"
//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------
#if (_EEPROM_DEBUG == 1)
#define	EEPROM_printf(...)     UARTprintf(__VA_ARGS__)
#else
#define	EEPROM_printf(...)     {};
#endif


#define EEPROM_BUS 					(I2C6_BASE)

#define NO_OF_CONFIG_MODULE 		(12)//(11)
#define MAC_ADD_BYTES				(6)
#define SIZE_OF_HOSTNAME			(20ul)
#define MAX_MBTCP_IP				(5)

#define DEFAULT_DEVICE_NAME			"SNMP-Card"
#define DEFAULT_NETWORK_USERNAME	"root"
#define DEFAULT_NETWORK_PASSWORD	"fuji"

#define DEFAULT_SNMP_COMMUNITY_SIZE  (10ul)
#define NO_OF_TIMEOUT_FC_FAILSAFE	(10)
#define NO_OF_CONSUME_FC_ETHIP		(32)
#define TOTAL_ALARMS_CONFIG			(20)

#define MAX_LEN                     64
#define MAX_MSG_LEN                 128
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

typedef enum
{
    INVALID_OPERATOR   		= 0,
    GREATER_THAN            = 1,
    LESS_THAN   			= 2,
    NOT_EQUAL_TO         	= 3,
    GREATER_THAN_EQUAL_TO  	= 4,
    LESS_THAN_EQUAL_TO      = 5,
    EQUAL_TO          		= 6,
    END_OF_OPERATORS	//Keep At END
    
}LOGICAL_OPERATORS;


typedef enum
{
	INVALID_MODULE  		   = 0,
	SNMPTrap_CONFIG_SAVE       = 1,
    MBTCP_CONFIG_SAVE          = 2,
    ETH_SETTS_CONFIG_SAVE      = 3,
    SYSTEM_INFO_SAVE		   = 4,
    FAULT_RECORD_SAVE		   = 5,
    REBOOT_DEVICE			   = 6,
    SERIAL_CONFIG_SAVE         = 7,
    FLAG_CONFIG_SAVE           = 8,
    GSM_CONFIG_SAVE            = 9,
    HTTP_CONFIG_SAVE           = 10,
    EMAIL_CONFIG_SAVE          = 11,
    RTC_COMPENSATORY_SAVE      = 12,
    END_OF_MODULES				//Keep At END
    
}EEPROM_MODULES;

//----------------------------------------------------------------------------

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

//-----------------------------SNMP TRAP Settings---------------------------------

typedef union __attribute__((packed))
{
    uint8_t trapsetting;
    struct
    {

        uint8_t typeofTrap:1;
        uint8_t fallingedgetrap:1;
        uint8_t risingedgetrap:1;
        uint8_t trapSentFlag:1;
        uint8_t reserved:4;
    }Bits;
}UPS_SNMP_TRAP_SETTING;

typedef union
{
	uint8_t TransBuffer[1];
	struct __attribute__((packed))
	{
	    UPS_SNMP_TRAP_SETTING UpsSnmpTrapStruct[64];
	    REG_uint32_t TRAPIPAddress;
	    uint16_t TrapPortNo;
		uint16_t Reserved[6];
		uint16_t CRC;
	};
}CONFIG_SNMPTrapSettings;


//-----------------------------Modbus TCP Settings---------------------------------

typedef union
{
	uint8_t TransBuffer[1];
	struct __attribute__((packed))
	{
		uint16_t SocketTimeout;
		uint16_t Port;
		REG_uint32_t ModbusValidIP[MAX_MBTCP_IP];
		uint16_t Reserved[7];
		uint16_t CRC;
	}; 
	
}CONFIG_MBTCPSettings;

//-----------------------------Ethernet Settings------------------------------



typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint8_t CONFIG_HostName[20];
        REG_uint32_t CONFIG_IP_Address;
        REG_uint32_t CONFIG_GateWay;
        REG_uint32_t CONFIG_Subnet;
        REG_uint32_t CONFIG_Pri_DNS;
        REG_uint32_t CONFIG_Sec_DNS;
		
		uint8_t MAC_Addr[6];
        ETH_SETTINGS_STRUCT CONFIG_Enable;
        uint16_t Reserved[5];
        uint16_t CRC;
    };
}CONFIG_EthernetSettings;
//-----------------------------GSM/GPRS Settings------------------------------

typedef union
{

    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint16_t CONFIG_ADCStringVisibile;
        uint16_t CONFIG_HTTPSettReserved[28];
        uint16_t CRC_CheckHTTPSett;

    };

}CONFIG_HTTPSettings;


//----------------------------------------------------------------------------
typedef union __attribute__((packed))
{
    uint8_t all;

    struct {
        uint8_t FactorySettingEnable : 1;
        uint8_t BootType : 2;
        uint8_t b1 : 1;
        uint8_t b2 : 1;
        uint8_t b3 : 1;
        uint8_t b4 : 1;
        uint8_t b5 : 1;
    } bits;
} SYS_GROUP_STRUCT;



//----------------------------System Info------------------------------------

typedef union
{

    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint16_t OS_Version;
		uint8_t BOOTVersion;
		SYS_GROUP_STRUCT SystemInfoGroup;
    };

}CONFIG_SystemInfo;
//----------------------------MAC Info------------------------------------

typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
		uint8_t Add_Bytes[MAC_ADD_BYTES];
		uint16_t CRC;
    };

}CONFIG_MACAddressInfo;

//------------------------------------------------------------------------------
typedef struct __attribute__((packed))
{

    SERIALPORT_BAUDRATE     baudrate;       // BAUDRATE
    SERIAL_PROTOCOL_TYPE    protocolType;   // Protocol Type
    SERIALPORT_PARITY       parity;         // PARITY
    SERIALPORT_DATABIT      databit;        // DATABIT
    SERIALPORT_STOPBIT      stopbit;        // STOPBIT

} CONFIG_SerialPorts;
//-----------------------------SerialSettings---------------------------------
typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        CONFIG_SerialPorts  CONFIG_SerialPort[SYSTEM_SERIAL_PORT_COUNT];//SERIALPORT_CONFIG
        uint16_t            CONFIG_SerialSettReserved[15]; //19 words reserved
        REG_uint32_t        DataLogInterval;
        uint16_t            CRC_Check_SerialSett;
    };

}CONFIG_SerialSettings;
typedef struct
{
    uint8_t EmailTo_Add[MAX_LEN];
}TO_EMAIL_ADDR_STRUCT;
typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint8_t EmailFrom[MAX_LEN];
        TO_EMAIL_ADDR_STRUCT  EmailTo[3];
        uint8_t EmailPassword[MAX_LEN];
        uint8_t SMTPHost[MAX_LEN];
        uint8_t SSLEnable;
        uint8_t WebEmail_Enabled;
        uint16_t EmailEvent;
        uint16_t SmtpPort;
        uint32_t SmtpMailInterval;
        uint16_t CRC_EmailSett;
    };

}EMAILSTRUCT;

typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint16_t RTCCompenstroyafterhourSet;
        uint16_t RTCCompenstroySecondsSet;
        uint8_t NTPServerEnableDisable;
        uint8_t NTPServer[32];
        uint8_t Dummy;
        uint16_t CRC_RTCCompensSett;
    };

}RTCCOMPENSTRUCT;

typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {
        uint8_t ShDnByTime;
        uint8_t ShDnByBattLow;
        uint8_t ShDnTime_SS;
        uint8_t ShDnTime_MM;
        uint8_t ShDnTime_HH;
        uint8_t RestartTime_SS;
        uint8_t RestartTime_MM;
        uint8_t RestartTime_HH;
        uint8_t MainsFailShDnDly_SS;
        uint8_t MainsFailShDnDly_MM;
        uint8_t MainsFailShDnDly_HH;
        uint8_t Dummy;
        uint16_t CRC_UPSShutDownSett;
        uint16_t CRC_RTCCompensSett;
        uint8_t NTPServer[32];
    } ;
}UPSSHUTDOWNSTRUCT;

//------------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
	QueueHandle_t xEEPROMSaveHandler;
	uint16_t EEPROMWriteCounter[NO_OF_CONFIG_MODULE];
	BOOL Internal_EEPROMActive;
	
}
EEPROM_RAM_STRUCT;

//----------------------------Save Configuration-------------------------------
typedef union
{
    uint8_t TransBuffer[1];
    struct __attribute__((packed))
    {                                                      //Data Size    Start Address   End Address
        CONFIG_MACAddressInfo Save_MACAddress;                   //8        0                 8
        CONFIG_SystemInfo Save_SystemInfo;                       //4        8                 12
        CONFIG_SNMPTrapSettings Save_SNMPTrapSettings;           //96       12                108
        CONFIG_MBTCPSettings Save_MBTCPSettings;                 //140      108               248
        CONFIG_EthernetSettings Save_EthernetSettings;           //60       248               308
        FAULT_STRUCT faultRecordStruct;                          //2048     308               2356
        CONFIG_HTTPSettings Save_HTTPSettings;                   //50       2356              2406
        CONFIG_SerialSettings Save_SerialSettings;               //50       2406              2456
        CONFIG_GSMSettings Save_GSMSettings;                     //200      2456              2656
        EMAILSTRUCT EmailConfig;                                 //389      2656              3045
        RTCCOMPENSTRUCT RTCCompenConfig;                         //6        3046              3052
        UPSSHUTDOWNSTRUCT UPSShutDownSt;                         //14       3052              3066
        uint8_t reserved[3039];                                  //3076     3066              6144
    };

}EEPROM_MAP_Struct;

typedef struct  _eeprom
{
    EEPROM_MAP_Struct EepromMapStruct;
    EEPROM_RAM_STRUCT EEPROMCurrentState;
}EEPROM_MAPPED_STRUCT;

typedef struct
{
    EEPROM_MODULES  ModuleNo;
    uint8_t         ModuleState;

}EEPROMReqQueue;

typedef enum
{
    OUT_OF_BOX_OS 		= 0,
    OUT_OF_BOX_SERIAL	= 1,
    OUT_OF_BOX_ETHERNET	= 2,
    OUT_OF_BOX_USB_FILE	= 3
}OUT_OF_BOX_TYPES;

//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------
extern volatile EEPROM_MAPPED_STRUCT Device_Configurations;
//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------

//===================================MODULE SIZE (in Bytes)===================
// All Module size is calculated and also alloted some extended for 
// future use so there will not any extra add on after this, but 
// in some case, any module is extended from its alloted size, 
// just calculate size of structure of respective module and change 
// size(in byte).
// it will implicated all though script.
//=============================================================================
#define SNMPTrap_CONFIG_SIZE        (sizeof(CONFIG_SNMPTrapSettings))
#define MBTCP_CONFIG_SIZE   		(sizeof(CONFIG_MBTCPSettings))
#define ETH_SETTS_CONFIG_SIZE       (sizeof(CONFIG_EthernetSettings))
#define SYSTEM_INFO_SIZE			(sizeof(CONFIG_SystemInfo))		
#define MAC_ADDRESS_SIZE			(sizeof(CONFIG_MACAddressInfo))
#define FAULT_ADDRESS_SIZE          (sizeof(FAULT_STRUCT))
#define HTTP_CONFIG_SIZE            (sizeof(CONFIG_HTTPSettings))
#define SERIAL_CONFIG_SIZE          (sizeof(CONFIG_SerialSettings))
#define GSM_CONFIG_SIZE             (sizeof(CONFIG_GSMSettings))
#define EMAIL_CONFIG_SIZE           (sizeof(EMAILSTRUCT))
#define RTC_COMPENSORTORY_SIZE      (sizeof(RTCCOMPENSTRUCT))
#define UPS_SCHEDULEDSHUTDWN_SIZE   (sizeof(UPSSHUTDOWNSTRUCT))
//===============================================================================
//==========================EEPROM CONFIG MODULES ADDRESS=======================
#define START_ADDRESS			   (0x00000000)

#define MAC_ADD_INFO_START         (START_ADDRESS)
#define MAC_ADD_INFO_END           (MAC_ADD_INFO_START + MAC_ADDRESS_SIZE)

#define SYSTEM_INFO_START          (MAC_ADD_INFO_END)
#define SYSTEM_INFO_END            (SYSTEM_INFO_START + SYSTEM_INFO_SIZE)

#define SNMPTrap_CONFIG_START      (SYSTEM_INFO_END)
#define SNMPTrap_CONFIG_END        (SNMPTrap_CONFIG_START + SNMPTrap_CONFIG_SIZE)

#define MBTCP_CONFIG_START         (SNMPTrap_CONFIG_END)
#define MBTCP_CONFIG_END		   (MBTCP_CONFIG_START + MBTCP_CONFIG_SIZE)

#define ETH_SETTS_CONFIG_START     (MBTCP_CONFIG_END)
#define ETH_SETTS_CONFIG_END	   (ETH_SETTS_CONFIG_START + ETH_SETTS_CONFIG_SIZE)

#define FAULT_RECORD_START         (ETH_SETTS_CONFIG_END)
#define FAULT_RECORD_END           (FAULT_RECORD_START+FAULT_ADDRESS_SIZE)

#define HTTP_CONFIG_START          (FAULT_RECORD_END)
#define HTTP_CONFIG_END            (HTTP_CONFIG_START + HTTP_CONFIG_SIZE)

#define SERIAL_CONFIG_START        (HTTP_CONFIG_END)
#define SERIAL_CONFIG_END          (SERIAL_CONFIG_START + SERIAL_CONFIG_SIZE)

#define GSM_CONFIG_START           (SERIAL_CONFIG_END)
#define GSM_CONFIG_END             (GSM_CONFIG_START + GSM_CONFIG_SIZE)

#define EMAIL_CONFIG_START         (GSM_CONFIG_END)
#define EMAIL_CONFIG_END           (EMAIL_CONFIG_START + EMAIL_CONFIG_SIZE)


#define RTC_COMPENASTORY_START     (EMAIL_CONFIG_END)
#define RTC_COMPENASTORY_END       (RTC_COMPENASTORY_START + RTC_COMPENSORTORY_SIZE)

#define UPS_SCHDSHUTDWN_START      (RTC_COMPENASTORY_END)
#define UPS_SCHDSHUTDWN_END        (UPS_SCHDSHUTDWN_START + UPS_SCHEDULEDSHUTDWN_SIZE)

#define MAX_SIZE_OF_ONCHIP_EEPROM   6144

#define RESERVED_MEM_START        UPS_SCHDSHUTDWN_END
#define RESERVED_MEM_END          (RESERVED_MEM1_START + (MAX_SIZE_OF_ONCHIP_EEPROM - RESERVED_MEM1_START))
//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------
uint16_t COMCrcGenerator( const void* crcDataPtr, uint16_t crcDataLength);
void OnChipEEPROM_Init(void);
BOOL IsInternalEEPROMActive();
uint8_t OnChipEEPROM_Read(void*Buffer, uint16_t *CRC_Check, uint32_t StartAddress, uint16_t DataLen);
void OnChipEEPROM_SaveConfiguration(EEPROM_MODULES Module);
void OnChipEEPROM_Write(EEPROM_MODULES Module, uint8_t *Value, uint16_t DataLen);
void OnChipEEPROM_SendEEPROMSaveEvent(EEPROM_MODULES Event, uint8_t Status);
//=======================================================================================//

#endif /* ONCHIP_EEPROM_H_ */

//==============================================================================
// END OF ONCHIP_EEPROM_H_
//==============================================================================

