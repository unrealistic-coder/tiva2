/*
 * USBDevice.h
 *
 *  Created on: 13-May-2022
 *      Author: rushikesh.sonone
 */

#ifndef SOURCE_COMMUNICATION_USB_USBDEVICEPROTOCOL_H_
#define SOURCE_COMMUNICATION_USB_USBDEVICEPROTOCOL_H_

#include "Common.h"
#include <COMTimer.h>

#if (_USBDEV_DEBUG == 1)
#define	USBDEV_printf(...)     UARTprintf(__VA_ARGS__)
#else
#define	USBDEV_printf(...)     {};
#endif


#define START_BYTE (0x63)

#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002
#define NO_OF_COMMANDS			 6

#define NO_OF_REQUEST_COMMANDS	 6
#define NO_OF_RESPONSE_COMMANDS  6

#define START_COMMAND_ID (1)
#define END_COMMAND_ID   (NO_OF_COMMANDS)
typedef enum
{
	COMMAND_REC_IDLE = 0,
	COMMAND_REC_RECEIVING = 1,
	COMMAND_REC_RECIVED = 2,
	COMMAND_REC_PROCESSING = 3,
	COMMAND_REC_TX_COMPLETED = 4
		
}COMMOND_REC_STATE;


typedef enum 
{
	NO_COMMAND = 0,
	USB_READ = 1,
	USB_WRITE = 2,
	
}COMMAND_ID_ENUM;

typedef enum 
{
	INVALID_SUB_CMD = 0,
	ETH_SET_READ_CMD = 1,
	PROT_SET_READ_CMD = 2,
	SYSTEM_INFO_READ_CMD = 3,
	FACTORY_SET_WRITE_CMD = 4,
	REBOOT_CMD = 5,
	ERROR_RESPONSE_CMD = 6

}COMMAND_SUBID_ENUM;
	

typedef union 
{
	uint8_t all;
	struct
	{
		uint8_t CommandType:1;
		uint8_t b1:1;
		uint8_t b2:1;
		uint8_t b3:1;
		uint8_t b4:1;
		uint8_t b5:1;
		uint8_t b6:1;
		uint8_t b7:1;
	}bits;
	
}PROTOCOL_HEADER_STRUCT;


typedef struct
{
	uint8_t StartByte;
	PROTOCOL_HEADER_STRUCT ProtocolHeader;
	COMMAND_ID_ENUM Command;
	uint8_t DataLength;
	uint8_t CRC8;
}USB_PROTOCOL_CMD_STRUCT;

typedef struct
{
	PROTOCOL_HEADER_STRUCT ProtocolHeader;
	COMMAND_ID_ENUM Command;
	uint8_t DataLength;
}USB_CMD_LENGTH_STRUCT;


typedef struct
{
	COMMOND_REC_STATE RxCmdState;
	uint8_t *TxBuffer;
	uint8_t *RxBuffer;
	uint8_t RxCount;
	bool USBConfigured;
	COM_TIMER_ID timeoutTimer;
	
}USB_PROTOCOL_STRUCT;

void USBDevice_init();
void USBDevice_Task();


#endif /* SOURCE_COMMUNICATION_USB_USBDEVICEPROTOCOL_H_ */
