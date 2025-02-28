//****************************************************************************
//*
//*
//* File: EthernetTask.c
//*
//* Synopsis: Ethernet Tasks as application over the top of LWIP libarary
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <EthernetTask.h>
#include <UPSInterface.h>
#include <OnChipEEPROM.h>
#include "ModbusTCPServer.h"
#include "SYSTask.h"
//#include "DatabaseHandler.h"
#include "utils/swupdate.h"
#include "utils/locator.h"
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


/*
NETWORK_SETTINGS_STRUCT networkConfig={
		{0}, //IP address
		{0}, // subnet
		{0}, // gateway
		{0}, // protocol Config
		{0}, // Ip new
		{0}, // subnet new
		{0}, // gateway new
		{0},			//Netif
		{0,0,0,0,0,0}, // Mac Address
		STATE_POWERUP,STATE_POWERUP}; //Network Status
*/
NETWORK_SETTINGS_STRUCT networkConfig={
		{0}, //IP address
		{0}, // subnet
		{0}, // gateway
		{0}, // Ip new
		{0}, // subnet new
		{0}, // gateway new	
		{0}, // Netif	
		{ETH_MAC_ADDR0,ETH_MAC_ADDR1,ETH_MAC_ADDR2,ETH_MAC_ADDR3,ETH_MAC_ADDR4,ETH_MAC_ADDR5}, // Mac Address
		STATE_POWERUP,
		STATE_POWERUP}; //Network Status
			
static volatile bool g_bFirmwareUpdate = false;
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//****************************************************************************
//*
//* Function name: EthenetTask_SaveNetworkSettingsinDB
//*
//* Return: None
//*
//*
//* Description:
//*   Save current/changed network Settings into Database.
//*
//****************************************************************************
//==============================================================================


static void EthenetTask_SaveNetworkSettingsinDB(uint32_t ui32IPAddr, uint32_t ui32NetMask,
                        uint32_t ui32GWAddr, BOOL dhcpEnable)
{
	CONFIG_EthernetSettings ethNetSetts;
	BOOL status = FALSE;
	uint16_t CRC_Check = 0;

	
	networkConfig.ui32IPAddr.addr = ui32IPAddr;
	networkConfig.ui32NetMask.addr = ui32NetMask;
	networkConfig.ui32GWAddr.addr = ui32GWAddr;
	networkConfig.protocolConfig.bits.dhcpEnable= dhcpEnable;	
		
	status = EthernetTask_ValidateNetworkSettings(networkConfig.ui32IPAddr.addr, 
												networkConfig.ui32NetMask.addr,
												networkConfig.ui32GWAddr.addr,
												networkConfig.protocolConfig.bits.dhcpEnable, 
												TRUE);

	
	if(status == TRUE)
    {
    	//Read EEPROM Event and check its valid or not.
    	networkConfig.protocolConfig.bits.CfgAppliedFlag = TRUE;
		OnChipEEPROM_Read((uint8_t*)&ethNetSetts.TransBuffer, &CRC_Check, 
			              ETH_SETTS_CONFIG_START, ETH_SETTS_CONFIG_SIZE);
		
		if((networkConfig.ui32IPAddr.addr!= ethNetSetts.CONFIG_IP_Address.all) ||
		    (networkConfig.ui32NetMask.addr != ethNetSetts.CONFIG_Subnet.all) ||
			(networkConfig.ui32GWAddr.addr != ethNetSetts.CONFIG_GateWay.all) ||
			(networkConfig.protocolConfig.bits.dhcpEnable != ethNetSetts.CONFIG_Enable.bits.dhcpEnable))
        {
			OnChipEEPROM_SendEEPROMSaveEvent(ETH_SETTS_CONFIG_SAVE, TRUE);
        }	
	}
}

//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_SetNetworkSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Apply Network settings over the top of LWIP libraries calling
//*   lwIPNetworkConfigChange to apply changes.
//*
//****************************************************************************
//==============================================================================

void EthernetTask_SetNetworkSettings(uint32_t ui32IPAddr, uint32_t ui32NetMask,
                        uint32_t ui32GWAddr, BOOL dhcpEnable)
{
    //SaveNetworkSettingsinDB();
	//uint32_t ui32IPMode;
	if(networkConfig.protocolConfig.bits.CfgAppliedFlag == TRUE)
	{
		if((networkConfig.protocolConfig.bits.dhcpEnable != dhcpEnable) ||
				(((networkConfig.ui32IPAddr.addr != htonl(ui32IPAddr)) || 
				(networkConfig.ui32NetMask.addr != htonl(ui32NetMask)) ||
				(networkConfig.ui32GWAddr.addr != htonl(ui32GWAddr))) && 
				(dhcpEnable == IPADDR_USE_STATIC)))	
		{
			networkConfig.protocolConfig.bits.CfgAppliedFlag = FALSE;
			networkConfig.protocolConfig.bits.dhcpEnable = dhcpEnable;
			networkConfig.ui32IPAddr_NEW.addr  = ui32IPAddr;
			networkConfig.ui32NetMask_NEW.addr = ui32NetMask;
			networkConfig.ui32GWAddr_NEW.addr  = ui32GWAddr;
			
		}
	}
}
//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_ValidateNetworkSettings
//*
//* Return: None
//*
//*
//* Description:
//*   Before applying Network setting check device IP type and also call 
//*   Subnet calculator to validate IP if it is static, for Dynamic it is 
//*   it will only apply 0 to IP address and apply validate changed to network
//*   function is called only when event is generated for change in network 
//*   configuration.
//*
//****************************************************************************
//==============================================================================
BOOL EthernetTask_ValidateNetworkSettings(uint32_t ui32IPAddr, uint32_t ui32NetMask, uint32_t ui32GWAddr,
                             BOOL dhcpEnable, BOOL validateIP)
{
    BOOL Status = FALSE;
    REG_uint32_t Gateway, NetMask, IPAddress;

    if((validateIP == TRUE) || (dhcpEnable == FALSE))
    {
        Gateway.all = ui32GWAddr;
        NetMask.all = ui32NetMask;
        IPAddress.all = ui32IPAddr;
        Status = EthernetTask_SubNetCalculator((REG_uint32_t)Gateway.all, (REG_uint32_t)NetMask.all, (REG_uint32_t)IPAddress.all);
    }
    else if(validateIP == FALSE)
    {
        Status=TRUE;
    }

    return Status;


}

//=============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_IsNetworkSettingChanged
//*
//* Return: None
//*
//*
//* Description:
//*   Timer handler call continouly to check if there is any change in Dynamic 
//*   IP address, if yes then save in data base.
//*  
//*
//****************************************************************************
//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_IsNetworkSettingChanged
//*
//* Return: None
//*
//*
//* Description:
//*   Timer handler call continouly to check if there is any change in Dynamic 
//*   IP address, if yes then save in data base.
//*  
//*
//****************************************************************************
//==============================================================================
BOOL EthernetTask_IsNetworkSettingChanged(BOOL applyChange)
{
	BOOL status=FALSE;
	uint32_t ui32IPAddr;
	uint32_t ui32NetMask;
	uint32_t ui32GWAddr;
	BOOL dhcpEnable;

	ui32IPAddr = (lwIPLocalIPAddrGet());
	ui32NetMask = (lwIPLocalNetMaskGet());
	ui32GWAddr = (lwIPLocalGWAddrGet());
	dhcpEnable= EthernetTask_CheckDHCPEnable();	

	if((networkConfig.protocolConfig.bits.dhcpEnable != dhcpEnable) ||
			(networkConfig.ui32IPAddr.addr != ui32IPAddr) ||
			(networkConfig.ui32NetMask.addr != ui32NetMask) ||
			(networkConfig.ui32GWAddr.addr != ui32GWAddr))

	{
		if(applyChange == TRUE)
		{
			EthenetTask_SaveNetworkSettingsinDB(ui32IPAddr,ui32NetMask,ui32GWAddr,dhcpEnable);
		}
		status=TRUE;
	}
			
	return status;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_SoftwareUpdateRequestCallback
//*
//* Return: None
//*
//*
//* Description:
//*   Callby function for swupdate_SoftwareUpdateInit to check if there is any
//*   software update.
//*
//****************************************************************************
//==============================================================================

void EthernetTask_SoftwareUpdateRequestCallback(void)
{
    g_bFirmwareUpdate = true;
}

#if defined(PART_TM4C1292NCPDT)
static void Configure_gpiopins_for_RMII()
{
	//-----Alternate GPIO pin Selection for RMII--------//
	//GPIOPinConfigure(GPIO_PG2_EN0TXCK);
	GPIOPinConfigure(GPIO_PG3_EN0TXEN);
	
	GPIOPinConfigure(GPIO_PG4_EN0TXD0);	
	GPIOPinConfigure(GPIO_PG5_EN0TXD1);
	
	//GPIOPinConfigure(GPIO_PG6_EN0RXER);
	GPIOPinConfigure(GPIO_PG7_EN0RXDV);

	GPIOPinConfigure(GPIO_PQ5_EN0RXD0);
	GPIOPinConfigure(GPIO_PQ6_EN0RXD1);

	
//	GPIOPinConfigure(GPIO_PK4_EN0RXD3);
 // 	GPIOPinConfigure(GPIO_PK5_EN0RXD2);
//	GPIOPinConfigure(GPIO_PK6_EN0TXD2);
//	GPIOPinConfigure(GPIO_PK7_EN0TXD3);

	GPIOPinConfigure(GPIO_PF3_EN0MDIO);
	GPIOPinConfigure(GPIO_PF2_EN0MDC);
	
	GPIOPinConfigure(GPIO_PM4_EN0RREF_CLK);
	//GPIOPinConfigure(GPIO_PM6_EN0CRS);
	//------------GPIO ETHERNET PIN CONFIGURATION---//
	GPIOPinTypeEthernetMII(GPIO_PORTG_BASE,0xbC);//0xbc
	GPIOPinTypeEthernetMII(GPIO_PORTQ_BASE,0x60);
	GPIOPinTypeEthernetMII(GPIO_PORTM_BASE,0x10);
	
	GPIOPinTypeEthernetMII(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinTypeEthernetMII(GPIO_PORTF_BASE, GPIO_PIN_2);
	
	//GPIOPadConfigSet (GPIO_PORTG_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	//GPIOPadConfigSet (GPIO_PORTQ_BASE, GPIO_PIN_6, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	
	GPIOPadConfigSet (GPIO_PORTG_BASE, 0xBC, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet (GPIO_PORTQ_BASE, 0x60, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet (GPIO_PORTF_BASE, 0x0C, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
	//GPIOPadConfigSet (GPIO_PORTK_BASE, 0xF0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet (GPIO_PORTM_BASE, 0x10, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
	//GPIODirModeSet (GPIO_PORTG_BASE, 0xB8, GPIO_DIR_MODE_HW);
	//GPIODirModeSet (GPIO_PORTQ_BASE, 0x60, GPIO_DIR_MODE_HW);
	//GPIODirModeSet (GPIO_PORTF_BASE, 0x0C, GPIO_DIR_MODE_HW);
//	GPIODirModeSet (GPIO_PORTK_BASE, 0x10, GPIO_DIR_MODE_HW);
//	GPIODirModeSet (GPIO_PORTM_BASE, 0x10, GPIO_DIR_MODE_HW);
	//GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE,GPIO_PIN_3);
//	GPIOPadConfigSet (GPIO_PORTK_BASE, GPIO_PIN_7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
//	GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_3, 0);
//	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_7, 0);

}

//==============================================================================
#endif

//****************************************************************************
//*
//* Function name: Ethernet_init
//*
//* Return: None
//*
//*
//* Description:
//*   Derived MAC address and Init Ethernet port LEDS for further ethernet 
//*   Operation.
//*
//****************************************************************************
//==============================================================================

void Ethernet_init(void)
{
	uint32_t ui32User0 = 0;
	uint32_t ui32User1 = 0;
	//uint8_t pui8MACArray[6];
	
#if defined(PART_TM4C129XNCZAD)

    ROM_GPIOPinConfigure(GPIO_PK5_EN0LED2);
    ROM_GPIOPinConfigure(GPIO_PK6_EN0LED1);
    GPIOPinTypeEthernetLED(GPIO_PORTK_BASE, GPIO_PIN_5 | GPIO_PIN_6);

#elif defined(PART_TM4C1294NCPDT)
	ROM_GPIOPinConfigure(GPIO_PF0_EN0LED0);
    ROM_GPIOPinConfigure(GPIO_PF4_EN0LED1);

    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);
#else
	Configure_gpiopins_for_RMII();
#endif

    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.

	//ui32User0 = 0x00001AB6;
    //ui32User1 = 0x000C18CC; //B6:1A:00:CC:18:0A
    
	ui32User0 = Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[0] << 0;
	ui32User0 |= (Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[1] << 8);
	ui32User0 |= (Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[2] << 16);

	ui32User1 = Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[3] << 0;
	ui32User1 |= (Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[4] << 8);
	ui32User1 |= (Device_Configurations.EepromMapStruct.Save_MACAddress.Add_Bytes[5] << 16);

	//MAP_FlashUserSet(ui32User0, ui32User1);
	//MAP_FlashUserGet(&ui32User0, &ui32User1);

    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    networkConfig.MAC_ADDRESS[0] = ((uint8_t)(ui32User0 >>  0) & 0xff);
    networkConfig.MAC_ADDRESS[1] = ((uint8_t)(ui32User0 >>  8) & 0xff);
    networkConfig.MAC_ADDRESS[2] = ((uint8_t)(ui32User0 >> 16) & 0xff);
    networkConfig.MAC_ADDRESS[3] = ((uint8_t)(ui32User1 >>  0) & 0xff);
    networkConfig.MAC_ADDRESS[4] = ((uint8_t)(ui32User1 >>  8) & 0xff);
    networkConfig.MAC_ADDRESS[5] = ((uint8_t)(ui32User1 >> 16) & 0xff);	
}

//****************************************************************************
//*
//* Function name: Ethernet_DiagnosticParameters
//*
//* Return: None
//*
//*
//* Description:
//*   Derived MAC address and Init Ethernet port LEDS for further ethernet 
//*   Operation.
//*
//****************************************************************************
//==============================================================================

uint32_t EthernetTask_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset)
{
	uint32_t paramValue = 0;
	
	if(Reset == TRUE)
	{
		//memset(&networkConfig.networkDiagnostic,0x00, sizeof(NETWORK_DIAG_COUNTER));
		lwip_stats.link.xmit=0;
		lwip_stats.link.recv=0;
		lwip_stats.tcp.xmit=0;
		lwip_stats.tcp.recv=0;
		lwip_stats.udp.xmit=0;
		lwip_stats.udp.recv=0;
	}
	else
	{
		switch(ParamIndex)
		{
			case 0:
			{
				paramValue = lwip_stats.link.xmit;
				break;
			}
			case 1:
			{
				paramValue = lwip_stats.link.recv;
				break;
			}
			case 2:
			{
				paramValue = lwip_stats.tcp.xmit;
				break;
			}
			case 3:
			{
				paramValue = lwip_stats.tcp.recv;
				break;
			}
			case 4:
			{
				paramValue =lwip_stats.udp.xmit;
				break;
			}
			case 5:
			{
				paramValue = lwip_stats.udp.recv;
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
//* Function name: Ethernet_Task
//*
//* Return: None
//*
//*
//* Description:
//*   Ethernet Task initialze LEDs GPIO, Setting MAC Address to Device and Powering
//*   UP IP Traker Task with IP initialization on EEPROM Save (where it is static or
//*   dynamic). Initializing HTTP inits with it SSI and CGI Handlers to start HTTP
//*   web server and Continous traking of LINK with it detect to device and IP 
//*   state.
//*
//****************************************************************************
//==============================================================================

void Ethernet_Task()
{
	uint32_t IP_Address = 0, IP_Mask_Address=0, GateWay_IPAddress =0;
	BOOL status = FALSE;
	uint32_t IPSetAttemptCounter=0;

    // Loop forever.  All the work is done in the created tasks
	CONFIG_EthernetSettings *ethSettings=(CONFIG_EthernetSettings*)&(Device_Configurations.EepromMapStruct.Save_EthernetSettings.TransBuffer);
    while(1)
    {
    	Error_ResetTaskWatchdog(ETHERNET_TASK_ID);
        // Delay ou pode inclusive apagar a tarefa
        switch(networkConfig.CurrentState)
        {
        	case STATE_POWERUP:
			{
				networkConfig.OldState = networkConfig.CurrentState;
				networkConfig.CurrentState = STATE_LINK_DETECT;

				status = EthernetTask_ValidateNetworkSettings(ethSettings->CONFIG_IP_Address.all, 
											  				  ethSettings->CONFIG_Subnet.all,
											                  ethSettings->CONFIG_GateWay.all,
							 				                  /*ethSettings->CONFIG_Pri_DNS.all,
							 				                  ethSettings->CONFIG_Sec_DNS.all,*/ //Secondary DNS
							 				                  ethSettings->CONFIG_Enable.bits.dhcpEnable,
							 				                  FALSE);
					
				if((ethSettings->CONFIG_Enable.bits.dhcpEnable == 1) ||
					(status == FALSE))
				{
					//Apply Dynamic Ip address allocation
					lwIPInit(configCPU_CLOCK_HZ, &networkConfig.MAC_ADDRESS[0], 0, 0, 0, IPADDR_USE_DHCP,
			    		&networkConfig.sNetIF);
	
				}
				else
				{
					//Apply Static IP address allocation if IP is validate
					IP_Address = htonl(ethSettings->CONFIG_IP_Address.all);
					IP_Mask_Address = htonl(ethSettings->CONFIG_Subnet.all);
					GateWay_IPAddress = htonl(ethSettings->CONFIG_GateWay.all);
					
					lwIPInit(configCPU_CLOCK_HZ, &networkConfig.MAC_ADDRESS[0], IP_Address,
								IP_Mask_Address, GateWay_IPAddress, IPADDR_USE_STATIC, &networkConfig.sNetIF);									   
				}
				
			 	//
	    		// Setup the device locator service.
	    		//
	    		LocatorInit();
	    		LocatorMACAddrSet(&networkConfig.MAC_ADDRESS[0]);
	    		LocatorAppTitleSet(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName);//"UPS-CARD"

				//Disable HTTP web server as requested setting
				//if(httpserverConfig.HTTPServerEnable == 1)
				{
#if !defined WEBSERVER_USING_SPI_FLASH
				    httpd_init();
#endif
//					HTTPApp_RCDSSIinit();
//					HTTPApp_WriteCGIInit();
				}
        	}
			break;
			case STATE_LINK_DETECT:
			{
				if(CheckLinkDetect())
				{
					networkConfig.OldState = networkConfig.CurrentState;
					networkConfig.CurrentState = STATE_IP_CONFIG;
					IPSetAttemptCounter=0;
				}
			}
			break;
			
			case STATE_IP_CONFIG:
			{	
				if(CheckLinkDetect())
				{
					EthernetTask_IsNetworkSettingChanged(TRUE);
					if(networkConfig.protocolConfig.bits.CfgAppliedFlag == TRUE)
					{
						networkConfig.OldState = networkConfig.CurrentState;
					//	swupdate_SoftwareUpdateInit(EthernetTask_SoftwareUpdateRequestCallback);
						networkConfig.CurrentState = STATE_LWIP_WORKING;
						IPSetAttemptCounter=0;
					}
					else if((networkConfig.protocolConfig.bits.dhcpEnable == FALSE) &&
						(IPSetAttemptCounter >= 600))// 600*100msec sleep=60000msec=60sec=1 min
					{
						//Changing to DHCP from static since static IP has some problem.
//						networkConfig.protocolConfig.bits.dhcpEnable=TRUE;
//						lwIPNetworkConfigChange(htonl(networkConfig.ui32IPAddr_NEW.addr),
//												htonl(networkConfig.ui32NetMask_NEW.addr),
//												htonl(networkConfig.ui32GWAddr_NEW.addr),
//												networkConfig.protocolConfig.bits.dhcpEnable);
//						IPSetAttemptCounter=0;


                        //Changing to DHCP from static since static IP has some problem.
                        networkConfig.protocolConfig.bits.dhcpEnable=TRUE;
                        lwIPNetworkConfigChange(0,0,0,networkConfig.protocolConfig.bits.dhcpEnable);
                        IPSetAttemptCounter=0;
					}
					else if(networkConfig.protocolConfig.bits.dhcpEnable == FALSE)
					{
						IPSetAttemptCounter++;
					}
				}
				else
				{
					networkConfig.OldState     = networkConfig.CurrentState;
					networkConfig.CurrentState = STATE_LINK_BREAK;	
				}
			}
			break;

			case STATE_LINK_BREAK:
			{
				networkConfig.OldState = networkConfig.CurrentState;
				networkConfig.CurrentState = STATE_LINK_DETECT;
				networkConfig.protocolConfig.bits.CfgAppliedFlag = FALSE;
			}
			break;
			case STATE_LWIP_WORKING:
			{
				if(CheckLinkDetect() == 0)
				{
					networkConfig.OldState = networkConfig.CurrentState;
					networkConfig.CurrentState = STATE_LINK_BREAK;
				}
				else
				{					
				//	if(g_bFirmwareUpdate == true)
					{
				//		Error_DisableWatchdogReset();
				//		swupdate_SoftwareUpdateBegin(SYSTEM_CLOCK);
					}
					
				   if(networkConfig.protocolConfig.bits.CfgAppliedFlag == FALSE)
					{
						//networkConfig.protocolConfig.bits.CfgAppliedFlag = TRUE;
						lwIPNetworkConfigChange(htonl(networkConfig.ui32IPAddr_NEW.addr),
												htonl(networkConfig.ui32NetMask_NEW.addr),
												htonl(networkConfig.ui32GWAddr_NEW.addr),
												networkConfig.protocolConfig.bits.dhcpEnable);
						networkConfig.OldState = networkConfig.CurrentState;
						networkConfig.CurrentState = STATE_LINK_DETECT;	
					}
					else if(networkConfig.protocolConfig.bits.dhcpEnable==TRUE)
					{
						if(EthernetTask_IsNetworkSettingChanged(FALSE)==TRUE)
						{
							networkConfig.protocolConfig.bits.CfgAppliedFlag = FALSE;
							networkConfig.OldState = networkConfig.CurrentState;
							networkConfig.CurrentState = STATE_LINK_DETECT;
						}
					}
				}
			}
			break;
        }
        vTaskDelay(100);
    }

}
//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_SaveNetworkChangeInEEPROM
//*
//* Return: None
//*
//*
//* Description:
//*   Save Network Changes in EEPROM. 
//*
//****************************************************************************
//==============================================================================

void EthernetTask_SaveNetworkChangeInConfig()
{

	/*Device_Configurations.Save_EthernetSettings.CONFIG_Enable = (0x0006 |
								networkConfig.protocolConfig.bits.dhcpEnable);*/
    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.dhcpEnable =
											networkConfig.protocolConfig.bits.dhcpEnable;

    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_IP_Address.all = (networkConfig.ui32IPAddr.addr);
    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_GateWay.all    = (networkConfig.ui32GWAddr.addr);
    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Subnet.all     = (networkConfig.ui32NetMask.addr);

    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Pri_DNS.all = EthernetTask_GetDNSServerName(0);
    Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Sec_DNS.all = EthernetTask_GetDNSServerName(1);
}

//==============================================================================

//=============================HTTP CALLS=======================================

//****************************************************************************
//*
//* Function name: HTTP_GetHostName
//*
//* Return: char* - Host Name string pointer
//*
//*
//* Description:
//*   Get Host Name of Device.
//*
//****************************************************************************
//==============================================================================

const char* EthernetTask_GetHostName(void)
{
    //return netif_get_hostname(&networkConfig.sNetIF);
	return (const char*)Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: HTTP_CheckDHCPEnable
//*
//* Return: TRUE  - If DHCP Enable
//*			FALSE - IF DHCP Disable
//*
//* Description:
//*   Check DHCP of device is enable or not.
//*
//****************************************************************************
//==============================================================================

uint8_t EthernetTask_CheckDHCPEnable(void)
{
    uint8_t status = FALSE;
    struct dhcp *dhcp = netif_dhcp_data(&networkConfig.sNetIF);
	
    if (dhcp == NULL)
        status = FALSE;
    else
    {
	    if((dhcp->state == DHCP_STATE_BOUND) || dhcp->autoip_coop_state == DHCP_AUTOIP_COOP_STATE_ON)
        {
			status = TRUE;
        }
    }
    return status;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: HTTP_GetDNSServerName
//*
//* Return: uint32_t DNS Server address
//*			
//*
//* Description:
//*   Get DNS server Address on requested Index
//*
//****************************************************************************
//==============================================================================

uint32_t EthernetTask_GetDNSServerName(uint8_t Index)
{
    const ip_addr_t* DNSServer;
    DNSServer = dns_getserver(Index);
    return DNSServer->addr;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: HTTP_SetHostName
//*
//* Return: None
//*			
//*
//* Description:
//*  Set Hostname to device
//*
//****************************************************************************
//==============================================================================

void EthernetTask_SetHostName(char* HostName)
{
//    char* Buffer;
//	uint8_t cnt =0;
//	char End_OF_Str = '\0';
//	uint8_t TempHold[15];
//
//	do  //while(*(Buffer+cnt) != End_OF_Str)
//	{
//		TempHold[cnt] = *(Buffer+cnt);
//		cnt++;
//		if((cnt >= 15) && (*(Buffer+cnt) != End_OF_Str))
//			return;
//	}while(cnt<15);
//	MEMCPY((uint8_t*)&(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName),
//			(uint8_t*)&(TempHold), sizeof(TempHold));
//

        memset((char *)Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName,'\0',sizeof(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName));
        MEMCPY(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName, HostName, 20);
        LocatorAppTitleSet((char *)Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName);

}
//==============================================================================

//=============================SUBNET CALCULATOR================================

//****************************************************************************
//*
//* Function name: EthernetTask_CheckIPClass
//*
//* Return: uint16_t - (1-9)-IP address Class
//*						(0) - out of standard range
//*			
//*
//* Description:
//*  Check standard IP address class and return to Subnet Calculator
//*  for further operation.
//*
//****************************************************************************
//==============================================================================

static uint16_t EthernetTask_CheckIPClass(REG_uint32_t NetIP)
{
	uint16_t IPClass = 0;
	if (NetIP.byte[0] == 10) 
	{
		IPClass = 1;	// Class A Private address blocks //
	}
	else if ((NetIP.byte[0] == 172) && (NetIP.byte[1] >= 16) && (NetIP.byte[1] <= 31))
	{
		IPClass = 2;	// Class B Private address blocks //
	}
	else if ((NetIP.byte[0] == 192) && (NetIP.byte[1] == 168))
	{
		IPClass = 3;	// Class C Private address blocks //
	}
	else if (NetIP.byte[0] == 127) 
	{
		IPClass =  4;	// Loopback Address Reserved address blocks //
	}
	else if (NetIP.byte[0] < 127) 
	{
		IPClass = 5;
	}
	else if ((NetIP.byte[0] > 127) && (NetIP.byte[0] < 192)) 
	{
		IPClass = 6;
	}
	else if ((NetIP.byte[0] > 191) && (NetIP.byte[0] < 224)) 
	{
		IPClass = 7;
	}
	else if ((NetIP.byte[0] > 223) && (NetIP.byte[0] < 240)) 
	{
		IPClass = 8;
	}
	else if ((NetIP.byte[0] > 239) && (NetIP.byte[0] <= 255)) 
	{
		IPClass = 9;
	}
	else
	{
		IPClass = 0;	// Out of Range //
	}
	return IPClass;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_CheckSubnet
//*
//* Return: TRUE: Valid Mask
//*			FALSE: Invalid Mask
//*			
//*
//* Description:
//*  Check standard Subnet Mask Range and return it is valided or not.
//*
//****************************************************************************
//==============================================================================

static BOOL EthernetTask_CheckSubnet(uint32_t SubnetMask)
{
	uint32_t mask = 0;
	
    for (mask = SubnetMask; mask != 0; mask <<=1)
	{
        if ((mask & ( (unsigned int)(1<<31))) == 0)
        {      
            return (FALSE); // Highest bit is now zero, but mask is non-zero.
        }
    }
    return (TRUE); // Mask was, or became 0.

}

//==============================================================================

//****************************************************************************
//*
//* Function name: EthernetTask_getIncrement
//*
//* Return: uint16_t - 	return Increament IP address by		
//*			
//*
//* Description:
//*  Using mask should be decided No of BITS used for IP address range selection.
//*
//****************************************************************************
//==============================================================================

static uint16_t EthernetTask_getIncrement(REG_uint32_t decimalMask)
{
	uint16_t increment = 0;
	uint8_t cnt = 0, size = sizeof(REG_uint32_t)/sizeof(REG_uint8_t);
	
	for (cnt = 0; cnt < size; cnt++)
	{
		if (decimalMask.byte[cnt] == 255)
		{
			increment = 1;
		}
		else if(decimalMask.byte[cnt] == 254)
		{
			increment = 2;
			break;
		}
		else if(decimalMask.byte[cnt] == 252)
		{
			increment = 4;
			break;
		}
		else if(decimalMask.byte[cnt] == 248)
		{
			increment = 8;
			break;
		}
		else if(decimalMask.byte[cnt] == 240)
		{
			increment = 16;
			break;
		}
		else if(decimalMask.byte[cnt] == 224)
		{
			increment = 32;
			break;
		}
		else if(decimalMask.byte[cnt] == 192)
		{
			increment = 64;
			break;
		}
		else if(decimalMask.byte[cnt] == 128)
		{
			increment = 128;
			break;
		}
	}
	return increment;
}
//==============================================================================
#if 0
//****************************************************************************
//*
//* Function name: EthernetTask_getSubnets
//*
//* Return: uint16_t - 	No of IP address can be used in range.
//*			
//*
//* Description:
//*   using subnet and its bitselection with gw can use to determine 
//*   no of IP addresses can be use in respective range.
//*
//****************************************************************************
//==============================================================================

static uint32_t EthernetTask_getSubnets(REG_uint32_t decimalMask, uint16_t ipClass, REG_uint32_t subClassMask)
{
	uint16_t netBits = 0;
	uint8_t cnt = 0, size = sizeof(REG_uint32_t)/sizeof(REG_uint8_t);
	uint32_t subnets = 1;
	if (ipClass==1)
	{
		subClassMask.byte[0] = 255;
		subClassMask.byte[1] = 0;
		subClassMask.byte[2] = 0;
		subClassMask.byte[3] = 0;
	}
	else if(ipClass==2)
	{
		subClassMask.byte[0] = 255;
		subClassMask.byte[1] = 255;
		subClassMask.byte[2] = 0;
		subClassMask.byte[3] = 0;
	}
	else if(ipClass==3)
	{
		subClassMask.byte[0] = 255;
		subClassMask.byte[1] = 255;
		subClassMask.byte[2] = 255;
		subClassMask.byte[3] = 0;
	}
	else if(ipClass==4 || ipClass==5)
	{
		subClassMask.byte[0] = decimalMask.byte[0];
		subClassMask.byte[1] = decimalMask.byte[1];
		subClassMask.byte[2] = decimalMask.byte[2];
		subClassMask.byte[3] = decimalMask.byte[3];
	}

	for (cnt=0; cnt < size ; cnt++)
	{
		if (decimalMask.byte[cnt] != subClassMask.byte[cnt])
		{
			if (decimalMask.byte[cnt] == 255)
			{
				netBits += 8;
				continue;
			}
			else if (decimalMask.byte[cnt] == 254)
			{
				netBits += 7;
				continue;
			}
			else if (decimalMask.byte[cnt] == 252)
			{
				netBits += 6;
				continue;
			}
			else if (decimalMask.byte[cnt] == 248)
			{
				netBits += 5;
				continue;
			}
			else if (decimalMask.byte[cnt] == 240)
			{
				netBits += 4;
				continue;
			}
			else if (decimalMask.byte[cnt] == 224)
			{
				netBits += 3;
				continue;
			}
			else if (decimalMask.byte[cnt] == 192)
			{
				netBits += 2;
				continue;
			}
			else if (decimalMask.byte[cnt] == 128)
			{
				netBits += 1;
				continue;
			}
			else if (decimalMask.byte[cnt] == 0)
			{
				netBits += 0;
				continue;
			}
			else
			{
				netBits += 0;
			}
		}
	}

	for (cnt=1; cnt <= netBits; cnt++)
		subnets *= 2;
	
	return subnets;
}
//==============================================================================
#endif
//****************************************************************************
//*
//* Function name: EthernetTask_getNetIDRange
//*
//* Return: uint32_t - 	last IP address from Range
//*			
//*
//* Description:
//*   Last IP address from Range can be use as commerical.
//*   
//****************************************************************************
//==============================================================================

static uint32_t EthernetTask_getNetIDRange(REG_uint32_t decimalNetID, uint16_t netInc, REG_uint32_t decimalMask) 
{
	uint8_t cnt = 0, size = sizeof(REG_uint32_t)/sizeof(REG_uint8_t);

	REG_uint32_t netIDEnd;
	for (cnt = 0; cnt < size; cnt++)
	{
		if ((decimalMask.byte[cnt] == 255))
		{
			netIDEnd.byte[cnt] = decimalNetID.byte[cnt];
		}
		else if ((decimalMask.byte[cnt] < 255) && (decimalMask.byte[cnt] > 0))
		{
			netIDEnd.byte[cnt] = ((decimalNetID.byte[cnt] + netInc) - 1 );
		}
		else
		{
			netIDEnd.byte[cnt] = 255;
		}
	}
	return netIDEnd.all;
}
//==============================================================================
#if 0
//****************************************************************************
//*
//* Function name: EthernetTask_getHostsPerSubnet
//*
//* Return: uint32_t - 	No of Host IPs from Decimal Mask
//*			
//*
//* Description:
//*   Get number of hosts per subnet from range.
//*   
//****************************************************************************
//==============================================================================

static uint32_t EthernetTask_getHostsPerSubnet(REG_uint32_t decimalMask)
{
	uint8_t hostBits = 0;
	uint32_t hostsPerSubnet = 1;
	uint8_t cnt = 0, size = sizeof(REG_uint32_t)/sizeof(REG_uint8_t);
	
	for (cnt=0; cnt<size; cnt++)
	{
		if (decimalMask.byte[cnt] == 255)
		{
			hostBits += 0;
			continue;
		}
		else if (decimalMask.byte[cnt] == 254)
		{
			hostBits += 1;
			continue;
		}
		else if (decimalMask.byte[cnt] == 252)
		{
			hostBits += 2;
			continue;
		}else if (decimalMask.byte[cnt] == 248)
		{
			hostBits += 3;
			continue;
		}
		else if (decimalMask.byte[cnt] == 240)
		{
			hostBits += 4;
			continue;
		}
		else if (decimalMask.byte[cnt] == 224)
		{
			hostBits += 5;
			continue;
		}
		else if (decimalMask.byte[cnt] == 192)
		{
			hostBits += 6;
			continue;
		}
		else if (decimalMask.byte[cnt] == 128)
		{
			hostBits += 7;
			continue;
		}
		else if (decimalMask.byte[cnt] == 0)
		{
			hostBits += 8;
			continue;
		}
		else
		{
			hostBits = 0;
			break;
		}
	}
	
	for (cnt=1; cnt <= hostBits; cnt++)
		hostsPerSubnet *= 2;

	return (hostsPerSubnet-2);
}

//==============================================================================
#endif
//****************************************************************************
//*
//* Function name: EthernetTask_SubNetCalculator
//*
//* Return:  TRUE- IP is valid 
//*		     FALSE- IP is invalid	
//*
//* Description:
//*   Function to valid IP address with respect to gateway and subnet mask 
//*	  if it is in range of network subnet or not, it can use any type of IP
//*   static or dynamic.
//*   
//****************************************************************************
//==============================================================================

BOOL EthernetTask_SubNetCalculator(REG_uint32_t GatewayIP, REG_uint32_t SubnetMask, REG_uint32_t IPAddress)
{
	BOOL Status = FALSE, CheckSUB = FALSE;	
	REG_uint32_t GetNetID, GetNetRange;//, SubClassMask;						//starting ID and last Id
	uint16_t netInc = 0, IPClass = 0, checkCnt = 0, startcheck=0;
	//uint32_t No_of_Subnets = 0, No_of_Host_perSub=0;


	//SubClassMask.all = 0;
	//
	//Check if Gateway is provided or not; if yes then use Gateway also in validation if not then use
	//IP address as validation processure
	//

	if(((IPAddress.all == 0) || (SubnetMask.all == 0)) ||
		((IPAddress.all == 0xFFFFFFFF) || (SubnetMask.all == 0xFFFFFFFF)))
	{
		return FALSE;
	}

	if(GatewayIP.all == 0)
	{
		GatewayIP.all = IPAddress.all;
	}

	IPClass = EthernetTask_CheckIPClass((REG_uint32_t)GatewayIP);

	//apply swap to subnet mask to check in function from higher to low;
	CheckSUB = EthernetTask_CheckSubnet((uint32_t)htonl(SubnetMask.all)); 

	if((IPClass != 0) && (CheckSUB == TRUE))
	{
		//GetNetID.all = EthernetTask_getNetID((REG_uint32_t) GatewayIP.all,  (REG_uint32_t)SubnetMask.all);
		GetNetID.all = (GatewayIP.all & SubnetMask.all);
		
		netInc = EthernetTask_getIncrement((REG_uint32_t)SubnetMask);

		//No_of_Subnets = getSubnets((REG_uint32_t)SubnetMask.all, IPClass, (REG_uint32_t)SubClassMask.all);
		
		//EthernetTask_getSubnets((REG_uint32_t)SubnetMask.all, IPClass, (REG_uint32_t)SubClassMask.all);//-->
		GetNetRange.all = EthernetTask_getNetIDRange((REG_uint32_t)GatewayIP,  netInc, (REG_uint32_t)SubnetMask);

		//No_of_Host_perSub = getHostsPerSubnet((REG_uint32_t)SubnetMask.all);
		//EthernetTask_getHostsPerSubnet((REG_uint32_t)SubnetMask.all);//-->

		
		
		if(IPClass == 1)
		{
			if((IPAddress.byte[0] == GetNetID.byte[0]))
			{
				startcheck = 1;
			}
			else 
			{
				startcheck = 4;
				Status = FALSE;
			}
		}	
		else if((IPClass == 2))
		{
			if((IPAddress.byte[0] == GetNetID.byte[0]) && 
				(IPAddress.byte[1] == GetNetID.byte[1]))
			{
				startcheck = 2;
			}
			else 
			{
				startcheck = 4;
				Status = FALSE;
			}
				
		}

		else if((IPClass == 3))
		{
			if((IPAddress.byte[0] == GetNetID.byte[0]) && 
				(IPAddress.byte[1] == GetNetID.byte[1]) &&
				(IPAddress.byte[2] == GetNetID.byte[2]))
			{
				startcheck = 3;
			}
		
			else 
			{	
				startcheck = 4;
				Status = FALSE;
			}

		}
		for(checkCnt = startcheck; (checkCnt < 4); checkCnt++)		
		{
			if((IPAddress.byte[checkCnt] > GetNetID.byte[checkCnt]) &&
				(IPAddress.byte[checkCnt] < GetNetRange.byte[checkCnt]))
			{
				Status = TRUE;
			}
			else 
				Status = FALSE;
		}
	}	

	return Status;
}

//==============================================================================

//==============================================================================
// END OF EthernetTask.c FIlE
//==============================================================================

