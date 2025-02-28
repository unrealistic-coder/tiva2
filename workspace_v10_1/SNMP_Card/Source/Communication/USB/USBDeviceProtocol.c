//*****************************************************************************
//
// usb_dev_bulk.c - Main routines for the generic bulk device example.
//
// Copyright (c) 2013-2020 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.2.0.295 of the EK-TM4C1294XL Firmware Package.
//
//*****************************************************************************
#include <USBDeviceProtocol.h>
#include <USBDeviceStruct.h>
#include <UPSInterface.h>
#include "OnChipEEPROM.h"
#include <SerialDriver.h>
#include "ErrorHandler.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Generic Bulk Device (usb_dev_bulk)</h1>
//!
//! This example provides a generic USB device offering simple bulk data
//! transfer to and from the host.  The device uses a vendor-specific class ID
//! and supports a single bulk IN endpoint and a single bulk OUT endpoint.
//! Data received from the host is assumed to be ASCII text and it is
//! echoed back with the case of all alphabetic characters swapped.
//!
//! Assuming you installed TivaWare in the default directory, a driver
//! information (INF) file for use with Windows XP, Windows Vista, Windows 7,
//! and Windows 10 can be found in C:/TivaWare_C_Series-x.x/windows_drivers.
//! For Windows 2000, the required INF file is in
//! C:/TivaWare_C_Series-x.x/windows_drivers/win2K.
//!
//! A sample Windows command-line application, usb_bulk_example, illustrating
//! how to connect to and communicate with the bulk device is also provided.
//! The application binary is installed as part of the ``TivaWare for C Series
//! PC Companion Utilities'' package (SW-TM4C-USB-WIN) on the installation CD
//! or via download from http://www.ti.com/tivaware .  Project files are
//! included to allow the examples to be built using
//! Microsoft Visual Studio 2008.  Source code for this application can be
//! found in directory ti/TivaWare_C_Series-x.x/tools/usb_bulk_example.
//
//*****************************************************************************


//*****************************************************************************
//
// Variables tracking transmit and receive counts.
//
//*****************************************************************************

volatile uint32_t g_ui32Flags = 0;
bool ReciveFlag = false;
uint16_t dataCount = 0;

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
//static volatile bool g_bUSBConfigured = false;
USB_PROTOCOL_STRUCT USBDeviceProtocol = 
{
	COMMAND_REC_IDLE,
	&g_pui8USBTxBuffer[0],
	&g_pui8USBRxBuffer[0],
	0,
	FALSE,
	{0}
};


//tUSBRingBufObject sTxRing;
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************

//*****************************************************************************
//
// Receive new data and echo it back to the host.
//
// \param psDevice points to the instance data for the device whose data is to
// be processed.
// \param pi8Data points to the newly received data in the USB receive buffer.
// \param ui32NumBytes is the number of bytes of data available to be
// processed.
//
// This function is called whenever we receive a notification that data is
// available from the host. We read the data, byte-by-byte and swap the case
// of any alphabetical characters found then write it back out to be
// transmitted back to the host.
//
// \return Returns the number of bytes of data processed.
//
//*****************************************************************************

void USBDeviceProtocol_RequestTimeout(COM_TIMER_ID id, void* param)
{
	USB_PROTOCOL_STRUCT * pUSBDeviceProtocol=(USB_PROTOCOL_STRUCT *)param;
	pUSBDeviceProtocol->RxCmdState = COMMAND_REC_IDLE;
	memset(pUSBDeviceProtocol->RxBuffer, 0x00, BULK_BUFFER_SIZE);
	pUSBDeviceProtocol->RxCount = 0;
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDeviceProtocol_ValidateRequest
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static void USBDeviceProtocol_ValidateRequest(USB_PROTOCOL_STRUCT * pUSBDeviceProtocol)
{
	uint8_t ui8dataLen =4;
	uint8_t ui8CRC = 0;
	uint8_t crcValid = FALSE;

	ui8dataLen = pUSBDeviceProtocol->RxCount - 1;
	ui8CRC = CalcCRC8(&pUSBDeviceProtocol->RxBuffer[0], ui8dataLen);
	//ui8CRC = Crc8CCITT(0, &pUSBDeviceProtocol->RxBuffer[0], ui8dataLen);
	if(ui8CRC == pUSBDeviceProtocol->RxBuffer[ui8dataLen])
	{
		crcValid = TRUE;
	}

	
	if((pUSBDeviceProtocol->RxBuffer[0] == START_BYTE) && 
		(crcValid == TRUE) &&
		(pUSBDeviceProtocol->RxBuffer[1] == 0) || (pUSBDeviceProtocol->RxBuffer[1] == 1))
	{
		switch(pUSBDeviceProtocol->RxBuffer[2])
		{
	  		case USB_READ:
	  		case USB_WRITE:
				pUSBDeviceProtocol->RxCmdState = COMMAND_REC_RECIVED;
				break;
			default:
				USBDeviceProtocol.RxCmdState = COMMAND_REC_IDLE;
				//clear all data by memsets
				break;
		}

	}

}
//==============================================================================
//****************************************************************************
/*
* Function name: COMMOND_REC_STATE
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static COMMOND_REC_STATE USBDeviceProtocol_ExtractHostUSBCommand(tUSBDBulkDevice *psDevice, uint8_t *pi8Data,
                  uint32_t ui32NumBytes)
{
	uint8_t ui8count = 0;

	if (ui32NumBytes > 0) 
	{
		USBDeviceProtocol.RxCmdState = COMMAND_REC_RECEIVING;
		ui8count=USBDeviceProtocol.RxCount;
		USBDeviceProtocol.RxCount +=ui32NumBytes;
		memcpy(&USBDeviceProtocol.RxBuffer[ui8count],pi8Data,ui32NumBytes);
		ui8count=USBDeviceProtocol.RxCount;
		//start timeout timer
		COMTimer_Start(USBDeviceProtocol.timeoutTimer,(COM_TIMER_TICK)10);
				
		if( USBDeviceProtocol.RxCount >= 4 )
		{
			/* Is the frame already complete. */
			if(USBDeviceProtocol.RxCount < (USBDeviceProtocol.RxBuffer[3] + 5))
			{
			}
			else if(USBDeviceProtocol.RxCount == (USBDeviceProtocol.RxBuffer[3] + 5) &&
			        USBDeviceProtocol.RxBuffer[2] == USB_READ)
			{
				COMTimer_Stop(USBDeviceProtocol.timeoutTimer);
				USBDeviceProtocol_ValidateRequest(&USBDeviceProtocol);
			}
            else if(USBDeviceProtocol.RxCount == (USBDeviceProtocol.RxBuffer[3] + 6) &&
                    USBDeviceProtocol.RxBuffer[2] == USB_WRITE)
            {
                COMTimer_Stop(USBDeviceProtocol.timeoutTimer);
                USBDeviceProtocol_ValidateRequest(&USBDeviceProtocol);	
            }
			else
			{
				/* This should not happen. We can't deal with such a client and
				 * drop the connection for security reasons.
				 */
				USBDeviceProtocol.RxCmdState = COMMAND_REC_IDLE;
				USBDeviceProtocol.RxCount = 0;
				memset(&USBDeviceProtocol.RxBuffer[0], 0x00, BULK_BUFFER_SIZE);
			}
		}
	}	
	
    return(USBDeviceProtocol.RxCmdState);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the transmit channel (data to
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    if(ui32Event == USB_EVENT_TX_COMPLETE)
    {

    }
    return(0);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // Which event are we being sent?
    //

    COMMOND_REC_STATE RxState;
	//uint32_t ui32IntEnabled = 0;
	
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
        {
            USBDeviceProtocol.USBConfigured = true;
            g_ui32Flags |= COMMAND_STATUS_UPDATE;

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
             USBDeviceProtocol.USBConfigured = false;
            g_ui32Flags |= COMMAND_STATUS_UPDATE;
            break;
        }

        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            tUSBDBulkDevice *psDevice;

            //
            // Get a pointer to our instance data from the callback data
            // parameter.
            //
            psDevice = (tUSBDBulkDevice *)pvCBData;

            //
            // Read the new packet and echo it back to the host.
            //
            RxState = USBDeviceProtocol_ExtractHostUSBCommand(psDevice, pvMsgData, ui32MsgValue);
			
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

			USBBufferInit(&g_sTxBuffer);
			USBBufferInit(&g_sRxBuffer);

			return (uint16_t)RxState;
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        {
			//
			// Save if the interrupt was enabled or not.
			//
			/*ui32IntEnabled = IntIsEnabled(g_psDCDInst[0].ui32IntNum);
			
			//
			// Disable the USB interrupt if it was enabled.
			//
			if(ui32IntEnabled)
			{
				//OS_INT_DISABLE(g_psDCDInst[0].ui32IntNum);
			}*/

        }
		break;
        case USB_EVENT_RESUME:
            break;

        //
        // Ignore all other events and return 0.
        //
        default:
            break;
    }

    return(0);
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_init
*
* Return: None
*
* Description:
*   This is the main application entry function.
*
*
*/
//**************************************************************************
//==============================================================================
void USBDevice_init()
{
    uint32_t ui32PLLRate;

	 USBDeviceProtocol.USBConfigured = FALSE;
	
	COM_TIMER_PARAM   comTimerParam;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0xff;
	MAP_GPIOPinConfigure(GPIO_PD6_USB0EPEN);
	MAP_GPIOPinTypeUSBAnalog(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	MAP_GPIOPinTypeUSBDigital(GPIO_PORTD_BASE, GPIO_PIN_6);
	MAP_GPIOPinTypeUSBAnalog(GPIO_PORTL_BASE, GPIO_PIN_6 | GPIO_PIN_7);
	MAP_GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_4);

	USBDEV_printf("\033[2J\nUSB Device Communication\n");
    USBDEV_printf("---------------------------------\n\n");

    //
    // Tell the user what we are up to.
    //
    USBDEV_printf("Configuring USB... \n");

    //
    // Initialize the transmit and receive buffers.
    //
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    //
    // Tell the USB library the CPU clock and the PLL frequency.  This is a
    // new requirement for TM4C129 devices.
    //
    SysCtlVCOGet(SYSCTL_XTAL_25MHZ, &ui32PLLRate);
    USBDCDFeatureSet(0, USBLIB_FEATURE_CPUCLK, &output_clock_rate_hz);
    USBDCDFeatureSet(0, USBLIB_FEATURE_USBPLL, &ui32PLLRate);
    //
    // Initialize the USB stack for device mode.
    //
    USBStackModeSet(0, eUSBModeDevice, 0);

    //
    // Pass our device information to the USB library and place the device
    // on the bus.
    //
    USBDBulkInit(0, &g_sBulkDevice);

    //
    // Wait for initial configuration to complete.
    //
    USBDEV_printf("Waiting for host...\r");
	
    comTimerParam.param = (void*) &USBDeviceProtocol;
    comTimerParam.timeoutFunc = USBDeviceProtocol_RequestTimeout;
    comTimerParam.recurringFlag = 0;
    USBDeviceProtocol.timeoutTimer = COMTimer_Create(&comTimerParam);
    if ( COM_TIMER_ID_INVALID != USBDeviceProtocol.timeoutTimer)
    {	
    }
	USBDeviceProtocol.TxBuffer = &g_pui8USBTxBuffer[0];
	USBDeviceProtocol.RxBuffer = &g_pui8USBRxBuffer[0];
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_ReadETHSettings
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static uint8_t USBDevice_ReadETHSettings(USB_PROTOCOL_STRUCT *USBDeviceProtocol, uint8_t Count)
{
	uint8_t TxCount = Count;
	
	USBDeviceProtocol->TxBuffer[TxCount++] = networkConfig.protocolConfig.bits.dhcpEnable;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32IPAddr.addr >> 0) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32IPAddr.addr >> 8) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32IPAddr.addr >> 16) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32IPAddr.addr >> 24) & 0xFF;

	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32GWAddr.addr >> 0) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32GWAddr.addr >> 8) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32GWAddr.addr >> 16) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32GWAddr.addr >> 24) & 0xFF;

	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32NetMask.addr >> 0) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32NetMask.addr >> 8) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32NetMask.addr >> 16) & 0xFF;
	USBDeviceProtocol->TxBuffer[TxCount++] = (networkConfig.ui32NetMask.addr >> 24) & 0xFF;

	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[0]; 
	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[1]; 
	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[2]; 
	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[3]; 
	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[4];
	USBDeviceProtocol->TxBuffer[TxCount++] =  networkConfig.MAC_ADDRESS[5]; 

	return TxCount;
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_WriteETHSettings
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static BOOL USBDevice_WriteETHSettings(USB_PROTOCOL_STRUCT *USBDeviceProtocol, uint8_t TxCount)
{
	REG_uint32_t IPAddress, GatewayAddress, SubnetAddress;
	uint8_t count = TxCount;
	uint8_t DHCP= 0;
	BOOL status = TRUE;
	
	DHCP = GET_BIT(USBDeviceProtocol->RxBuffer[count++], 0);
	
	IPAddress.byte[0] = USBDeviceProtocol->RxBuffer[count++];
	IPAddress.byte[1] = USBDeviceProtocol->RxBuffer[count++];
	IPAddress.byte[2] = USBDeviceProtocol->RxBuffer[count++];
	IPAddress.byte[3] = USBDeviceProtocol->RxBuffer[count++];

	GatewayAddress.byte[0] = USBDeviceProtocol->RxBuffer[count++];
	GatewayAddress.byte[1] = USBDeviceProtocol->RxBuffer[count++];
	GatewayAddress.byte[2] = USBDeviceProtocol->RxBuffer[count++];
	GatewayAddress.byte[3] = USBDeviceProtocol->RxBuffer[count++];

	SubnetAddress.byte[0] = USBDeviceProtocol->RxBuffer[count++];
	SubnetAddress.byte[1] = USBDeviceProtocol->RxBuffer[count++];
	SubnetAddress.byte[2] = USBDeviceProtocol->RxBuffer[count++];
	SubnetAddress.byte[3] = USBDeviceProtocol->RxBuffer[count++];

	if(EthernetTask_ValidateNetworkSettings(IPAddress.all, SubnetAddress.all, GatewayAddress.all, DHCP, FALSE))
	{
		if((networkConfig.CurrentState == STATE_LINK_DETECT) || (networkConfig.CurrentState == STATE_LINK_BREAK))
		{
			status = FALSE;
		}
		else
		{
			EthernetTask_SetNetworkSettings(IPAddress.all, SubnetAddress.all, GatewayAddress.all, DHCP);
		}
	}
	return status;
	
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_ReadSystemInfo
*
* Return: None
*
* Description:
*
*
*
*/
//**************************************************************************
//==============================================================================
static uint8_t USBDevice_ReadSystemInfo(USB_PROTOCOL_STRUCT *USBDeviceProtocol, uint8_t Count)
{
	uint8_t TxCount = Count;
	
	USBDeviceProtocol->TxBuffer[TxCount++] = ((Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version >> 8) & 0x00FF);
	USBDeviceProtocol->TxBuffer[TxCount++] = (Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version & 0x00FF);
	USBDeviceProtocol->TxBuffer[TxCount++] = Device_Configurations.EepromMapStruct.Save_SystemInfo.BOOTVersion;
	USBDeviceProtocol->TxBuffer[TxCount++] = 0;
	
	return TxCount;
}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_ProcessPackets
*
* Return: None
*
* Description:
*   USBDevice  Process Packets
*
*
*/
//**************************************************************************
//==============================================================================
static void USBDevice_ProcessPackets()
{
	uint8_t ui8CRC = 0,ui8dataLen = 0, TxCount=0;
	uint8_t ValidCmdBuild = FALSE;
	
#if (_USBDEV_DEBUG == 1)
	uint8_t cnt=0;
#endif

	BOOL status = TRUE;
	
	if( USBDeviceProtocol.USBConfigured)
	{
		if(USBDeviceProtocol.RxCmdState == COMMAND_REC_RECIVED)
		{
			TxCount=0;
			USBDeviceProtocol.RxCmdState = COMMAND_REC_PROCESSING;
			USBDeviceProtocol.TxBuffer[TxCount++] = START_BYTE; // 0
			USBDeviceProtocol.TxBuffer[TxCount++] = 1; //Protocol HEAD 2
			USBDeviceProtocol.TxBuffer[TxCount++] = USBDeviceProtocol.RxBuffer[2]; //Cmd 2
			
			if(USBDeviceProtocol.RxBuffer[4] == 0) //check sub command is valid or not
			{
				USBDeviceProtocol.TxBuffer[TxCount++] = 0;//Length 3
				USBDeviceProtocol.TxBuffer[TxCount++] = ERROR_RESPONSE_CMD;//Sub command 4
				ValidCmdBuild = TRUE; 
				//its done calculate CRC and send to the USB host
			}
			else
			{
				switch(USBDeviceProtocol.RxBuffer[2])
				{
					case USB_READ:
					{
						if(USBDeviceProtocol.RxBuffer[4] == ETH_SET_READ_CMD)
						{
							
							USBDeviceProtocol.TxBuffer[TxCount++] = 20; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = ETH_SET_READ_CMD;						
							TxCount = USBDevice_ReadETHSettings(&USBDeviceProtocol, TxCount);
							
							ValidCmdBuild = TRUE;
						}
						else if(USBDeviceProtocol.RxBuffer[4] == PROT_SET_READ_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 2; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = PROT_SET_READ_CMD;
	
							USBDeviceProtocol.TxBuffer[TxCount++] = 
							(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.FTPServerEnable << 2) |
							(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable << 1) |
							(Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable << 0);
	
							
							ValidCmdBuild = TRUE;
						}
						else if(USBDeviceProtocol.RxBuffer[4] == SYSTEM_INFO_READ_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 5; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = SYSTEM_INFO_READ_CMD;
							TxCount = USBDevice_ReadSystemInfo(&USBDeviceProtocol, TxCount);
							ValidCmdBuild = TRUE;
						}
						else if(USBDeviceProtocol.RxBuffer[4] == FACTORY_SET_WRITE_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 2; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = FACTORY_SET_WRITE_CMD;
							USBDeviceProtocol.TxBuffer[TxCount++] = 
							        Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.FactorySettingEnable;
							
							ValidCmdBuild = TRUE;
						}						
						else
						{
							ValidCmdBuild = FALSE;
						}
							
					}
					break;
					case USB_WRITE:
					{
						if(USBDeviceProtocol.RxBuffer[4] == ETH_SET_READ_CMD)
	
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 1; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = ETH_SET_READ_CMD;
							status = USBDevice_WriteETHSettings(&USBDeviceProtocol, TxCount);
							if(status == FALSE)
							{
								USBDeviceProtocol.TxBuffer[4] = ERROR_RESPONSE_CMD;
							}
							ValidCmdBuild = TRUE;
						}
						else if(USBDeviceProtocol.RxBuffer[4] == PROT_SET_READ_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 1; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = PROT_SET_READ_CMD;
							Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.SNMPEnable =
								GET_BIT(USBDeviceProtocol.RxBuffer[TxCount], 0);
							Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable =
								GET_BIT(USBDeviceProtocol.RxBuffer[TxCount], 1);
							Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.FTPServerEnable =
								GET_BIT(USBDeviceProtocol.RxBuffer[TxCount], 2);
							//TxCount++;
							ValidCmdBuild = TRUE;
							OnChipEEPROM_SendEEPROMSaveEvent(ETH_SETTS_CONFIG_SAVE, TRUE);
						}
						else if(USBDeviceProtocol.RxBuffer[4] == FACTORY_SET_WRITE_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 1; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = FACTORY_SET_WRITE_CMD;
							Device_Configurations.EepromMapStruct.Save_SystemInfo.SystemInfoGroup.bits.FactorySettingEnable =
									GET_BIT(USBDeviceProtocol.RxBuffer[TxCount], 0);
							
							ValidCmdBuild = TRUE;
							OnChipEEPROM_SendEEPROMSaveEvent(SYSTEM_INFO_SAVE, TRUE);
						}
						else if(USBDeviceProtocol.RxBuffer[4] == REBOOT_CMD)
						{
							USBDeviceProtocol.TxBuffer[TxCount++] = 1; //length
							USBDeviceProtocol.TxBuffer[TxCount++] = REBOOT_CMD;
							
							ValidCmdBuild = TRUE;
							OnChipEEPROM_SendEEPROMSaveEvent(REBOOT_DEVICE, TRUE);
						}						
						else
						{
							ValidCmdBuild = FALSE;
						}
					}
					break;
					default:
						ValidCmdBuild = FALSE;
						
	
				}
			}
	
			if(ValidCmdBuild == TRUE)
			{
				ui8dataLen = TxCount;	
				ui8CRC = CalcCRC8(&USBDeviceProtocol.TxBuffer[0], ui8dataLen);
				//ui8CRC = Crc8CCITT(0, &USBDeviceProtocol.TxBuffer[0], ui8dataLen);
				USBDeviceProtocol.TxBuffer[ui8dataLen] = ui8CRC;
				
				USBBufferDataWritten(&g_sTxBuffer, TxCount+1);
				/*USBBufferWrite(&g_sTxBuffer, &USBDeviceProtocol.TxBuffer[ui8dataLen],
								TxCount+1);*/

#if (_USBDEV_DEBUG == 1)				
				USBDEV_printf("Recived Data from Host bytes received -->%d %d\r\n",TxCount, ui8dataLen);
				
				for(cnt=0; cnt<(TxCount+1); cnt++)
				{
					USBDEV_printf("0x%x\r\n",USBDeviceProtocol.TxBuffer[cnt]);
				}
#endif
			}
			//clear all flags and memory
			USBDeviceProtocol.RxCmdState = COMMAND_REC_IDLE;
			USBDeviceProtocol.RxCount = 0;
	
			memset(&USBDeviceProtocol.RxBuffer[0], 0x00, BULK_BUFFER_SIZE);
			memset(&USBDeviceProtocol.TxBuffer[0], 0x00, BULK_BUFFER_SIZE);
			USBBufferFlush(&g_sTxBuffer);
			
		}
	
	}
	else
	{
	  USBDeviceProtocol.RxCmdState = COMMAND_REC_IDLE;
	  
	}

}
//==============================================================================
//****************************************************************************
/*
* Function name: USBDevice_Task
*
* Return: None
*
* Description:
*   USBDevice Task process communication with Device configuration
*
*
*/
//**************************************************************************
//==============================================================================
void USBDevice_Task()
{

    while(1)
    {
    	Error_ResetTaskWatchdog(USB_APP_TASK_ID);
		USBDevice_ProcessPackets();
		vTaskDelay(10);
    }
}

//==============================================================================
// END OF FILE
//==============================================================================

