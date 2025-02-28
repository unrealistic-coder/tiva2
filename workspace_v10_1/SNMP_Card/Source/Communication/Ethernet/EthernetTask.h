//****************************************************************************
//*
//*
//* File: EthernetTask.h
//*
//* Synopsis: Ethernet Tasks define file - related Structures and interfaces
//*
//*
//****************************************************************************


#ifndef ETHERNET_TASK_H
#define ETHERNET_TASK_H 

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>

//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <


//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
#define ETH_MAC_ADDR0   0xB6//2U
#define ETH_MAC_ADDR1   0x1A//0U
#define ETH_MAC_ADDR2   0x00//0U
#define ETH_MAC_ADDR3   0xCC//0U
#define ETH_MAC_ADDR4   0x19//0U
#define ETH_MAC_ADDR5   0xa//0U
typedef enum
{
	STATE_POWERUP = 0,
	STATE_LINK_DETECT = 1,
	STATE_LINK_BREAK  = 2,
	STATE_IP_CONFIG	  = 3,
	STATE_LWIP_WORKING,
	
}
LWIP_OP_STATE;

//----------------------------------------------------------------------------

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------


typedef union
{
    uint16_t all;
    uint8_t byte[2];

    struct
    {
         uint8_t CfgAppliedFlag:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t dhcpEnable:1;
         uint8_t modBusTCPEnable:1;
         uint8_t SNMPEnable:1;
         //uint8_t webServerEnable:1;
         uint8_t FTPServerEnable:1; //b3:1;BACServerEnable:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
    } bits;
} ETH_SETTINGS_STRUCT;

typedef struct __attribute__((packed))
{
	ip4_addr_t ui32IPAddr;
	ip4_addr_t ui32NetMask;
	ip4_addr_t ui32GWAddr;
	ETH_SETTINGS_STRUCT protocolConfig;

	ip4_addr_t ui32IPAddr_NEW;
	ip4_addr_t ui32NetMask_NEW;
	ip4_addr_t ui32GWAddr_NEW;

	struct netif sNetIF;
	uint8_t MAC_ADDRESS[6];
	LWIP_OP_STATE CurrentState;
	LWIP_OP_STATE OldState;

}NETWORK_SETTINGS_STRUCT;

//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------

extern NETWORK_SETTINGS_STRUCT networkConfig;
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------
void Ethernet_init(void);
uint32_t EthernetTask_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset);
void Ethernet_Task();
const char* EthernetTask_GetHostName(void);
uint8_t EthernetTask_CheckDHCPEnable(void);
uint32_t EthernetTask_GetDNSServerName(uint8_t Index);

void EthernetTask_SetHostName(char* HostName);

void EthernetTask_SetNetworkSettings(uint32_t ui32IPAddr, uint32_t ui32NetMask,
                        uint32_t ui32GWAddr, BOOL dhcpEnable);
BOOL EthernetTask_ValidateNetworkSettings(uint32_t ui32IPAddr, uint32_t ui32NetMask, uint32_t ui32GWAddr,
							 BOOL dhcpEnable, BOOL validateIP);
void EthernetTask_SaveNetworkChangeInConfig();
BOOL EthernetTask_SubNetCalculator(REG_uint32_t GatewayIP, REG_uint32_t SubnetMask,REG_uint32_t IPAddress);
//----------------------------------------------------------------------------


#endif /* ETHERNET_TASK_H */
//==============================================================================
// END OF ETHERNET_TASK_H
//==============================================================================

