//****************************************************************************
//*
//*
//* File: SPIDriver.c
//*
//* Synopsis: Serial Driver interface - For Link Layer Protocols
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <SerialDriver.h>
#include <OnChipEEPROM.h>
#include "UPSInterface.h"
#include "SYSTask.h"
#include "utils/uartstdio.h"



//============================================================================
//                              > LOCAL DEFINES <
//----------------------------------------------------------------------------
#if (_SPI_DEBUG == 1)

uint32_t gpioTimeStampHigh= 0,gpioTimeStampLow= 0,gpioTimeStampRunning;
uint32_t spiTimeStampLow= 0xFFFFFFFF,spiTimeStampHigh= 0,spiTimeStampRunning= 0;
uint32_t spiRateTimeStampTemp= 0,spiRateTimeStamp=0,spiRateTimeStampHigh=0;
#endif


//============================================================================
//                              > LOCAL MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL FUNCTION Declaration <
//----------------------------------------------------------------------------


//============================================================================
//                              > LOCAL STRUCTURES <
//----------------------------------------------------------------------------

//============================================================================
//                              > LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------
HTTP_STATUS_STRUCT pStatus;
static const uint8_t gLookupTableCRC8[256] = {
/* 0x00 */  0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
            0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
            0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
            0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
            0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
            0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
            0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
            0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
/* 0x40 */  0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
            0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
            0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
            0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
            0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
            0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
            0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
            0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
/* 0x80 */  0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
            0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
            0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
            0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
            0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
            0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
            0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
            0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
/* 0xC0 */  0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
            0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
            0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
            0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
            0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
            0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
            0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
            0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};

SERIAL_RAM_STRUCT UpsSerialStruct;

SERIAL_RAM_STRUCT *pUpsSerialStruct;
SERIALPORT_CONFIG upsPortConfig[SYSTEM_SERIAL_PORT_COUNT]=
{\
    {\
        UART0_BASE,\
        SERIALPORT_BAUD_115200,\
        SERIALPORT_PARITY_NONE,\
        SERIALPORT_DATA_8BIT,\
        SERIALPORT_STOPBIT_1,\
        SERIAL_STDIO\
    },\
    {\
        UART1_BASE,\
        SERIALPORT_BAUD_19200,\
        SERIALPORT_PARITY_NONE,\
        SERIALPORT_DATA_8BIT,\
        SERIALPORT_STOPBIT_1,\
        SERIAL_GSM_GPRS\
    },\
    {\
        UART2_BASE,\
        SERIALPORT_BAUD_19200,\
        SERIALPORT_PARITY_NONE,\
        SERIALPORT_DATA_8BIT,\
        SERIALPORT_STOPBIT_1,\
        SERIAL_MODBUS_RTU_MASTER\
    },\
    {\
        UART3_BASE,\
        SERIALPORT_BAUD_19200,\
        SERIALPORT_PARITY_NONE,\
        SERIALPORT_DATA_8BIT,\
        SERIALPORT_STOPBIT_1,\
        SERIAL_MODBUS_RTU_SLAVE\
    },\
    }   ;

static const SERIAL_PROTOCOL_FUNC upsProtocolFunc[SERIAL_PROTOCOL_COUNT] =
{
    // NOTE: Order must match SERIAL_PROTOCOL_TYPE
    // SERIAL_SHUTDOWN = 0,
    // SERIAL_SHUTDOWN. Shutdown type
    {
        &UPS_StdioRx,
        &UPS_StdioTx,
        &UPS_StdioRxErr,
        &UPS_StdioStartup,
        &UPS_StdioShutdown,
    },
    // SERIAL_GSM_GPRS,
//    {
//        &GsmGprsCommon_RxISR,
//        &GsmGprsCommon_TxISR,
//        &GsmGprsCommon_RxErrISR,
//        &GsmGprsCommon_StartUp,
//        &GsmGprsCommon_Shutdown,
//    },
    // SERIAL_MODBUS_RTU_MASTER,
//    {
//        &ModbusRtuMaster_RxISR,
//        &ModbusRtuMaster_TxISR,
//        &ModbusRtuMaster_RxErrISR,
//        &ModbusRtuMaster_StartUp,
//        &ModbusRtuMaster_Shutdown,
//    },

    // SERIAL_MODBUS_RTU_SLAVE,
//    {
//        &ModbusRtuSlave_RxISR,
//        &ModbusRtuSlave_TxISR,
//        &ModbusRtuSlave_RxErrISR,
//        &ModbusRtuSlave_StartUp,
//        &ModbusRtuSlave_Shutdown,
//    },
};
//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------


//****************************************************************************
//*
//* Function name: CalcCRC8
//*
//* Return:   None
//*
//* Description: Calculates CRC
//*
//****************************************************************************

uint8_t CalcCRC8(const uint8_t *inpBuffer,
            int inSize)
{
    uint8_t aCRC;
    int i;
    /* --------------------------------------------------------------------- */
    aCRC = 0xFF;
    for (i = 0 ; i < (int)inSize ; i++){
        aCRC = (aCRC ^ *(inpBuffer+i));
        aCRC = gLookupTableCRC8[aCRC];
    }
    aCRC = ~aCRC;
    return (uint8_t)aCRC;
}

//****************************************************************************
//*
//* Function name: UPS_SetProtocol
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
void UPS_SetProtocol ( UART_PORT port, SERIAL_PROTOCOL_TYPE type, uint8_t powerUpFlag)
{
    //uint8_t index = 0;//CIPserialPortObj_GetSerialPortID ( port );
    BOOL Status=FALSE;

    if(powerUpFlag == FALSE)
    {
        // Shutdown current driver
         (*(upsProtocolFunc[type].shutdown))(UPS_GetProtocolDrvStruct(port),port );
    }

    // Startup new driver
    Status=(*(upsProtocolFunc[type].startup)) ( UPS_GetProtocolDrvStruct(port), port );
    if(Status == FALSE)
    {
        while(1)
        {
             COMTIMER_printf("COM Initialisation Failed\r\n");
             vTaskDelay(100);
        }
    }
}
//****************************************************************************
//*
//* Function name: UPS_UartTxIntDisable
//*
//* Return:   None
//*
//* Description: Disables TX interrupt
//*
//****************************************************************************
void UPS_UartTxIntDisable ( UART_PORT port  )
{
    uint32_t UARTBase;

       switch (port )
        {

         case PORT_UART1:
                 UARTBase= UART0_BASE;
               break;
         case PORT_UART2:
                 UARTBase= UART2_BASE;
             break;

         default:
             return;
        }

    MAP_UARTIntDisable(UARTBase, UART_INT_TX);
}
//****************************************************************************
//*
//* Function name: UPS_UartTxIntEnable
//*
//* Return:   None
//*
//* Description: Enables Tx Interrupt
//*
//****************************************************************************

void UPS_UartTxIntEnable(UART_PORT port)
{
    uint32_t UARTBase;

   switch (port )
    {

     case PORT_UART1:
             UARTBase= UART0_BASE;
             MAP_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_0, GPIO_PIN_0);
          break;
     case PORT_UART2:
             UARTBase= UART2_BASE;
             MAP_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6 , GPIO_PIN_7);
         break;

     default:

         return;
    }
	MAP_UARTIntEnable(UARTBase, UART_INT_TX);
 }
//****************************************************************************
//*
//* Function name: UPS_UartRxIntDisable
//*
//* Return:   None
//*
//* Description: Disables Rx Interrupt
//*
//****************************************************************************

void UPS_UartRxIntDisable(UART_PORT port)
{
    uint32_t UARTBase;
    switch (port )
     {
      case PORT_UART1:
              UARTBase= UART0_BASE;
           break;
      case PORT_UART2:
              UARTBase= UART2_BASE;
           break;

      default:

          return;
     }
    MAP_UARTIntDisable(UARTBase, UART_INT_RX);
}

//****************************************************************************
//*
//* Function name: UPS_UartRxIntEnable
//*
//* Return:   None
//*
//* Description: Enables Rx Interrupt
//*
//****************************************************************************
void UPS_UartRxIntEnable (UART_PORT port)
{
    uint32_t UARTBase;
     switch (port )
      {
       case PORT_UART1:
               UARTBase= UART0_BASE;
            break;
       case PORT_UART2:
               UARTBase= UART2_BASE;
            break;

       default:

           return;
      }
      MAP_UARTIntEnable(UARTBase, UART_INT_RX);
}

//****************************************************************************
//*
//* Function name: UPS_UART0_ISR
//*
//* Return:   None
//*
//* Description: UART0 ISR
//*
//****************************************************************************

void UPS_UART0_ISR(void)
{
    SERIAL_PROTOCOL_FUNC    *protocolFunc = (SERIAL_PROTOCOL_FUNC *) &upsProtocolFunc[upsPortConfig[PORT_UART1].protocolType];
    void *protocolStruct = UPS_GetProtocolDrvStruct(PORT_UART1);
    uint32_t uartStatusReg;// = MAP_UARTIntStatus(UART0_BASE, true);
    uint32_t uartError;

    //MAP_IntMasterDisable();

    uartStatusReg = MAP_UARTIntStatus(UART0_BASE, true);

    MAP_UARTIntClear(UART0_BASE,uartStatusReg);
    // ==================================================
    // A. Check Receiver errors.
    // ==================================================
    uartError=MAP_UARTRxErrorGet(UART0_BASE);
    if(uartError)
    {
        // Error Checking
        // Check over-run error (OE)
        (*protocolFunc->rxErr)(protocolStruct,uartError);
        MAP_UARTRxErrorClear(UART0_BASE);
    }

    // ==================================================
    // B. Check Received characters. Call Receive function
    // ==================================================
    if (ROM_UARTCharsAvail(UART0_BASE) )
    {   // Character(s) received
        do
        {
            (*protocolFunc->rxChar)(protocolStruct, ROM_UARTCharGetNonBlocking(UART0_BASE));
        } while (ROM_UARTCharsAvail(UART0_BASE));    // Consume all received bytes

    }


    // ==================================================
    // C. Check Transmitted Interrupts
    // ==================================================
    // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
    if( uartStatusReg & UART_INT_TX )
    {
        // Call Transmit function
        (*protocolFunc->txComplete)(protocolStruct);

    }

    //MAP_IntMasterEnable();

    // Done ISR

}

//****************************************************************************
//*
//* Function name: UPS_UART1_ISR
//*
//* Return:   None
//*
//* Description: UART1 ISR
//*
//****************************************************************************

void UPS_UART1_ISR(void)
{
    SERIAL_PROTOCOL_FUNC    *protocolFunc = (SERIAL_PROTOCOL_FUNC *) &upsProtocolFunc[upsPortConfig[PORT_UART2].protocolType];
    void *protocolStruct = UPS_GetProtocolDrvStruct(PORT_UART2);
    uint32_t uartStatusReg;
    uint32_t uartError;

    //MAP_IntMasterDisable();

    uartStatusReg = MAP_UARTIntStatus(UART1_BASE, true);


    MAP_UARTIntClear(UART1_BASE,uartStatusReg);

    // ==================================================
    // A. Check Receiver errors.
    // ==================================================
    uartError=MAP_UARTRxErrorGet(UART1_BASE);
    if(uartError)

    {
        // Error Checking
        // Check over-run error (OE)
        (*protocolFunc->rxErr)(protocolStruct,uartError);
        MAP_UARTRxErrorClear(UART1_BASE);
    }

    // ==================================================
    // B. Check Received characters. Call Receive function
    // ==================================================
    if (ROM_UARTCharsAvail(UART1_BASE))
    {   // Character(s) received
        do
        {
            (*protocolFunc->rxChar)(protocolStruct, ROM_UARTCharGetNonBlocking(UART1_BASE));
        } while (ROM_UARTCharsAvail(UART1_BASE));    // Consume all received bytes

    }


    // ==================================================
    // C. Check Transmitted Interrupts
    // ==================================================
    // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
    if( uartStatusReg & UART_INT_TX )
    {
        // Call Transmit function
        (*protocolFunc->txComplete)(protocolStruct);

    }
    //MAP_IntMasterEnable();

    // Done ISR

}

//****************************************************************************
//*
//* Function name: UPS_UART2_ISR
//*
//* Return:   None
//*
//* Description: UART2 ISR
//*
//****************************************************************************

void UPS_UART2_ISR(void)
{
    SERIAL_PROTOCOL_FUNC    *protocolFunc = (SERIAL_PROTOCOL_FUNC *) &upsProtocolFunc[upsPortConfig[PORT_UART3].protocolType];
    void *protocolStruct = UPS_GetProtocolDrvStruct(PORT_UART3);
    uint32_t uartStatusReg;// = MAP_UARTIntStatus(UART2_BASE, true);
    uint32_t uartError;

    //MAP_IntMasterDisable();

    uartStatusReg = MAP_UARTIntStatus(UART2_BASE, true);


    MAP_UARTIntClear(UART2_BASE,uartStatusReg);

    // ==================================================
    // A. Check Receiver errors.
    // ==================================================
    uartError=MAP_UARTRxErrorGet(UART2_BASE);
    if(uartError)

    {
        // Error Checking
        // Check over-run error (OE)
        (*protocolFunc->rxErr)(protocolStruct,uartError);
        MAP_UARTRxErrorClear(UART2_BASE);
    }

    // ==================================================
    // B. Check Received characters. Call Receive function
    // ==================================================
    if (ROM_UARTCharsAvail(UART2_BASE) )
    {   // Character(s) received
        do
        {
            (*protocolFunc->rxChar)(protocolStruct, ROM_UARTCharGetNonBlocking(UART2_BASE));
        } while (ROM_UARTCharsAvail(UART2_BASE));    // Consume all received bytes

    }


    // ==================================================
    // C. Check Transmitted Interrupts
    // ==================================================
    // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
    if( uartStatusReg & UART_INT_TX )
    {
        // Call Transmit function
        (*protocolFunc->txComplete)(protocolStruct);

    }
    //MAP_IntMasterEnable();

    // Done ISR

}

//****************************************************************************
//*
//* Function name: UPS_UART3_ISR
//*
//* Return:   None
//*
//* Description: USRT3 ISR
//*
//****************************************************************************

void UPS_UART3_ISR(void)
{
    SERIAL_PROTOCOL_FUNC    *protocolFunc = (SERIAL_PROTOCOL_FUNC *) &upsProtocolFunc[upsPortConfig[PORT_UART4].protocolType];
    void *protocolStruct = UPS_GetProtocolDrvStruct(PORT_UART4);
    uint32_t uartStatusReg;// = MAP_UARTIntStatus(UART3_BASE, true);
    uint32_t uartError;// = MAP_UARTIntStatus(UART0_BASE, true);

    //MAP_IntMasterDisable();

    uartStatusReg = MAP_UARTIntStatus(UART3_BASE, true);

    MAP_UARTIntClear(UART3_BASE,uartStatusReg);

    // ==================================================
    // A. Check Receiver errors.
    // ==================================================
    uartError=MAP_UARTRxErrorGet(UART3_BASE);
    if(uartError)
    {
        // Error Checking
        // Check over-run error (OE)
        (*protocolFunc->rxErr)(protocolStruct,uartError);
        MAP_UARTRxErrorClear(UART3_BASE);
    }

    // ==================================================
    // B. Check Received characters. Call Receive function
    // ==================================================
    if (ROM_UARTCharsAvail(UART3_BASE) )
    {   // Character(s) received
        do
        {
            (*protocolFunc->rxChar)(protocolStruct, ROM_UARTCharGetNonBlocking(UART3_BASE));
        } while (ROM_UARTCharsAvail(UART3_BASE));    // Consume all received bytes

    }


    // ==================================================
    // C. Check Transmitted Interrupts
    // ==================================================
    // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
    if( uartStatusReg & UART_INT_TX )
    {
        // Call Transmit function
        (*protocolFunc->txComplete)(protocolStruct);

    }
    //MAP_IntMasterEnable();

    // Done ISR

}
void  GSM_Uart2IntHandler(void)
{
    uint32_t uartStatusReg;// = MAP_UARTIntStatus(UART0_BASE, true);
     uint32_t uartError;

       //MAP_IntMasterDisable();

       uartStatusReg = MAP_UARTIntStatus(UART2_BASE, true);

       MAP_UARTIntClear(UART2_BASE,uartStatusReg);
       // ==================================================
       // A. Check Receiver errors.
       // ==================================================
       uartError=MAP_UARTRxErrorGet(UART2_BASE);
       if(uartError)
       {
           // Error Checking
           // Check over-run error (OE)
           //(*protocolFunc->rxErr)(protocolStruct,uartError);
           MAP_UARTRxErrorClear(UART2_BASE);
       }

       // ==================================================
       // B. Check Received characters. Call Receive function
       // ==================================================
       if (ROM_UARTCharsAvail(UART2_BASE) )
       {   // Character(s) received
           do
           {
               Error_ResetTaskWatchdog(GSM_TASK_ID);
                GSMRxCallback((uint8_t)ROM_UARTCharGetNonBlocking(UART2_BASE));

            // GSM_RxISR((uint8_t)ROM_UARTCharGetNonBlocking(UART1_BASE));
           } while (ROM_UARTCharsAvail(UART2_BASE));    // Consume all received bytes

       }


       // ==================================================
       // C. Check Transmitted Interrupts
       // ==================================================
       // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
       if( uartStatusReg & UART_INT_TX )
       {
           // Call Transmit function
         //  GSM_TxISR();
           GsmCommon_TxISR(uartStatusReg);
       }

       //MAP_IntMasterEnable();

       // Done ISR
}
//****************************************************************************
void UPS_UartIntHandler(void)
{
    uint32_t uartStatusReg;// = MAP_UARTIntStatus(UART0_BASE, true);
    uint32_t uartError;

	//MAP_IntMasterDisable(); 

	uartStatusReg = MAP_UARTIntStatus(UART0_BASE, true);

	MAP_UARTIntClear(UART0_BASE,uartStatusReg);
    // ==================================================
    // A. Check Receiver errors.
    // ==================================================
    uartError=MAP_UARTRxErrorGet(UART0_BASE);
    if(uartError)
    {
        // Error Checking
        // Check over-run error (OE)
        //(*protocolFunc->rxErr)(protocolStruct,uartError);
		MAP_UARTRxErrorClear(UART0_BASE);
    }

    // ==================================================
    // B. Check Received characters. Call Receive function
    // ==================================================
    if (ROM_UARTCharsAvail(UART0_BASE) )
    {   // Character(s) received
        do
        {
           UPSInterface_RxISR((uint8_t)ROM_UARTCharGetNonBlocking(UART0_BASE));
        } while (ROM_UARTCharsAvail(UART0_BASE));    // Consume all received bytes

    }


    // ==================================================
    // C. Check Transmitted Interrupts
    // ==================================================
    // If (HW TX Ready is TRUE) and (TX Int mask is enabled)
    if( uartStatusReg & UART_INT_TX )
    {
        // Call Transmit function
        UPSInterface_TxISR();

    }

	//MAP_IntMasterEnable(); 

    // Done ISR
}
//****************************************************************************
//****************************************************************************
//*
//* Function name: UPS_Get_Baudrate
//*
//* Return:   Returns Baudrate Enum
//*
//* Description: Returns Baudrate Enum
//*
//****************************************************************************

static uint32_t UPS_Get_Baudrate (UART_PORT port)
{
    return upsPortConfig[port].baudrate;
}

//****************************************************************************
//*
//* Function name: UPS_Get_Stopbit
//*
//* Return:   Returns Stop bit Config
//*
//* Description:Returns Stop bit Config
//*
//****************************************************************************

static uint32_t UPS_Get_Stopbit (UART_PORT port)
{
    uint32_t stopbit=SERIALPORT_STOPBIT_1;

    switch(upsPortConfig[port].stopbit)
    {
        case SERIALPORT_STOPBIT_1:
            stopbit=UART_CONFIG_STOP_ONE;
            break;
        case SERIALPORT_STOPBIT_2:
            stopbit=UART_CONFIG_STOP_TWO;
            break;
    }

    return stopbit;
}

//****************************************************************************
//*
//* Function name: UPS_Get_Databit
//*
//* Return: Returns Databits Config
//*
//* Description: Returns Databits Config
//*
//****************************************************************************

static uint32_t UPS_Get_Databit (UART_PORT port)
{
    uint32_t databit=UART_CONFIG_WLEN_8;

    switch(upsPortConfig[port].databit)
    {
        case SERIALPORT_DATA_7BIT:
            databit=UART_CONFIG_WLEN_7;
            break;
        case SERIALPORT_DATA_8BIT:
            databit=UART_CONFIG_WLEN_8;
            break;
    }

    return databit;

}

//****************************************************************************
//*
//* Function name: UPS_Get_Parity
//*
//* Return: Returns Parity Config
//*
//* Description: Returns Parity Config
//*
//****************************************************************************

static uint32_t UPS_Get_Parity (UART_PORT port)
{
    uint32_t parity=SERIALPORT_PARITY_NONE;

    switch(upsPortConfig[port].parity)
    {
        case SERIALPORT_PARITY_NONE:
            parity=UART_CONFIG_PAR_NONE;
            break;
        case SERIALPORT_PARITY_ODD:
            parity=UART_CONFIG_PAR_ODD;
            break;
        case SERIALPORT_PARITY_EVEN:
            parity=UART_CONFIG_PAR_EVEN;
            break;
    }

    return parity;
}

//****************************************************************************
//*
//* Function name: UPS_UART_Config_FromEEPROM
//*
//* Return: None
//*
//* Description: Read Serial Port configuration from Global Struct which was taken from EEPROM
//*
//****************************************************************************
void UPS_UART_Config_FromEEPROM(UART_PORT port)
{
    upsPortConfig[port].baudrate =
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate;
    upsPortConfig[port].databit =
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].databit;
    upsPortConfig[port].parity =
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].parity;
    upsPortConfig[port].stopbit =
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].stopbit;
//  serialPortConfig[port].protocolType =
//      RCDConfigurations.Save_SerialSettings.CONFIG_SerialPort[port].protocolType;
}

//****************************************************************************
//*
//* Function name: UPS_UartInit
//*
//* Return:   None
//*
//* Description: UART0 ISR
//*
//****************************************************************************
/*
void UPS_UartInit(UART_PORT port)
{
    uint32_t UARTBase=UART0_BASE;
    uint32_t UART_Int=INT_UART0;
    uint32_t UARTpheripheral = SYSCTL_PERIPH_UART0;//peripheralClock; // BY default
    uint32_t cfg=(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    uint32_t baudrate = SERIALPORT_BAUD_19200;//uint32_t baudrate=19200;
    uint32_t baudrateValue = BAUD_19200;

    switch (port )
        {
            case PORT_UART1:
                UARTpheripheral = SYSCTL_PERIPH_UART0;
                UARTBase= UART0_BASE;
                UART_Int=INT_UART0;
                baudrate = UPS_Get_Baudrate(port);
                cfg = UPS_Get_Databit(port);
                cfg |= UPS_Get_Stopbit(port);
                cfg |= UPS_Get_Parity(port);
                MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
                MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
                MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
                break;
            case PORT_UART2:
                UARTpheripheral = SYSCTL_PERIPH_UART1;
                UARTBase= UART1_BASE;
                UART_Int=INT_UART1;
                baudrate = UPS_Get_Baudrate(port);
                cfg = UPS_Get_Databit(port);
                cfg |= UPS_Get_Stopbit(port);
                cfg |= UPS_Get_Parity(port);
                MAP_GPIOPinConfigure(GPIO_PB0_U1RX);
                MAP_GPIOPinConfigure(GPIO_PB1_U1TX);
                MAP_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
                break;
            case PORT_UART3:
                UARTpheripheral = SYSCTL_PERIPH_UART2;
                UARTBase= UART2_BASE;
                UART_Int=INT_UART2;
                baudrate = UPS_Get_Baudrate(port);
                cfg = UPS_Get_Databit(port);
                cfg |= UPS_Get_Stopbit(port);
                cfg |= UPS_Get_Parity(port);
                MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);
                MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
                MAP_GPIOPinConfigure(GPIO_PA6_U2RX);
                MAP_GPIOPinConfigure(GPIO_PA7_U2TX);
                MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);
                break;
            case PORT_UART4:
                UARTpheripheral = SYSCTL_PERIPH_UART3;
                UARTBase= UART3_BASE;
                UART_Int=INT_UART3;
                baudrate = UPS_Get_Baudrate(port);
                cfg = UPS_Get_Databit(port);
                cfg |= UPS_Get_Stopbit(port);
                cfg |= UPS_Get_Parity(port);
                MAP_GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_6);
                MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_6, 0);
                MAP_GPIOPinConfigure(GPIO_PA4_U3RX);
                MAP_GPIOPinConfigure(GPIO_PA5_U3TX);
                MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);
                break;
            default:
                return;
        }

        switch(baudrate)
        {
            case SERIALPORT_BAUD_NULL:
            case SERIALPORT_BAUD_19200:
                baudrateValue = BAUD_19200;
                break;
            case SERIALPORT_BAUD_9600:
                baudrateValue = BAUD_9600;
                break;
            case SERIALPORT_BAUD_38400:
                baudrateValue = BAUD_38400;
                break;
            case SERIALPORT_BAUD_57600:
                baudrateValue = BAUD_57600;
                break;
            case SERIALPORT_BAUD_115200:
                baudrateValue = BAUD_115200;
                break;
        }

        // Enable peripheral as per base address of UART
        MAP_SysCtlPeripheralEnable(UARTpheripheral);
        // Wait for the UART module to be ready.
        while(!ROM_SysCtlPeripheralReady(UARTpheripheral))
        {
        }

        MAP_UARTConfigSetExpClk(UARTBase, SYSTEM_CLOCK, baudrateValue, cfg);

        MAP_UARTFIFODisable(UARTBase);

        MAP_IntMasterEnable();

        MAP_IntEnable(UART_Int);
}
*/

void UPS_UartInit(void)
{

   // Enable peripheral as per base address of UART
   MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
   // Wait for the UART module to be ready.
   while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_UART0))
   {
   }

   MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
   MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
   MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);



   MAP_UARTConfigSetExpClk(UART0_BASE, SYSTEM_CLOCK, 2400,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
   
   MAP_UARTFIFODisable(UART0_BASE);
   
   MAP_IntMasterEnable();
   
   MAP_IntEnable(INT_UART0);
   
  UPS_UartRxIntEnable(PORT_UART1);
  UPS_UartTxIntDisable(PORT_UART1);

}

//**************************************************************************//
//****************************************************************************
//*
//* Function name: UPS_GetProtocolType
//*
//* Return:   returns SERIAL_PROTOCOL_TYPE enum
//*
//* Description: Returns Protocol type configured for Serial Port
//*
//****************************************************************************
SERIAL_PROTOCOL_TYPE UPS_GetProtocolType( UART_PORT port )
{
    switch (port )
    {
        case PORT_UART1:
        case PORT_UART2:
        case PORT_UART3:
        case PORT_UART4:
            return upsPortConfig[port].protocolType;
        default:
            return SERIAL_INVALID;
    }
}

//****************************************************************************
//*
//* Function name: UPS_GetPort
//*
//* Return:   returns UART_PORT enum
//*
//* Description: Returns Serial port number configured for protocol type
//*
//****************************************************************************

UART_PORT UPS_GetPort( SERIAL_PROTOCOL_TYPE protocolType )
{
    UART_PORT port=PORT_UART1;
    for(port=PORT_UART1;port<SYSTEM_SERIAL_PORT_COUNT;port++)
    {
        if(upsPortConfig[port].protocolType == protocolType)
        {
             return port;
        }
    }
    return PORT_INVALID;
}
//****************************************************************************
//*
//* Function name: UPS_GetProtocolDrvStruct
//*
//* Return:   void *
//*
//* Description: Returns protocol sttucture pointer configured for serial port
//*
//****************************************************************************
void* UPS_GetProtocolDrvStruct(UART_PORT port)
{
    SERIAL_PROTOCOL_TYPE pType = UPS_GetProtocolType(port);
    void* protocolStructPtr=NULL;
    switch (pType)
    {
        case SERIAL_STDIO:
            protocolStructPtr=(void*) &UPSInterfaceStruct;
            break;
        case SERIAL_GSM_GPRS:
                protocolStructPtr=(void*) &UpsSerialStruct;
            break;
        case SERIAL_MODBUS_RTU_MASTER:
                //protocolStructPtr=(void*) &modbusRTUMasterStruct;
            break;
        case SERIAL_MODBUS_RTU_SLAVE:
                //protocolStructPtr=(void*) &modbusRTUSlaveStruct;
            break;
        default:
            break;
    }
    return protocolStructPtr;
}

//****************************************************************************
//*
//* Function name: UPS_UartWriteChar
//*
//* Return:   None
//*
//* Description: Writes Databyte to serial port
//*
//****************************************************************************

void UPS_UartWriteChar ( uint8_t dataByte, UART_PORT port)
{
    uint32_t UARTBase;
    switch (port )
    {
        case PORT_UART1:
                UARTBase= UART0_BASE;
            break;
        case PORT_UART2:
                UARTBase= UART2_BASE;
            break;
        default:
            return;
    }
        // Enable Transmit
    UPS_UartTxIntEnable (port);
    UARTCharPutNonBlocking(UARTBase, dataByte);//  MAP_UARTCharPut(UARTBase, dataByte);
}

//****************************************************************************
//*
//* Function name: UPS_StdioRx
//*
//* Return:   None
//*
//* Description: STDIO is used for printf. This is a dummy function
//*
//****************************************************************************
void UPS_StdioRx(void *param, uint8_t character )
{
    // Do nothing
//    SERIAL_RAM_STRUCT *pSerialStruct = (SERIAL_RAM_STRUCT*)param;

    // Set Intercharacter timer
    // Wait for T3.5 character timeout instead
//    COMTimer_Start(pSerialStruct->rxInterFrameTimer,
//                   (COM_TIMER_TICK)ModbusRtu_GetRxInterFrameTime(pSerialStruct));
//    pSerialStruct->serialDiag.Hard_Fault = faultRecordStruct.hardfaultCounter;
}

//****************************************************************************
//*
//* Function name: UPS_StdioTx
//*
//* Return:   None
//*
//* Description:STDIO is used for printf. This is a dummy function
//*
//****************************************************************************
void UPS_StdioTx(void * param)
{
    // Do nothing
//    SERIAL_RAM_STRUCT *pSerialStruct = (SERIAL_RAM_STRUCT *)param;
//
//    if (pSerialStruct->txRemainingSize > 0)
//    {
//        UPS_WriteChar ( pSerialStruct->serialPort,
//                              pSerialStruct->txBuffer[pSerialStruct->txBufferIndex++]);
//
//    }
}

//****************************************************************************
//*
//* Function name: UPS_StdioStartup
//*
//* Return:   Returns Success (True) or Failure (False)
//*
//* Description: This function configures Serial port settings for STDIO
//*
//****************************************************************************

BOOL UPS_StdioStartup (void *protStruct, UART_PORT port )
{
    uint32_t ui32Baud;

    switch(upsPortConfig[port].baudrate)
    {
        case SERIALPORT_BAUD_NULL:
        case SERIALPORT_BAUD_19200:
            ui32Baud = BAUD_19200;
            break;
        case SERIALPORT_BAUD_9600:
            ui32Baud = BAUD_9600;
            break;
        case SERIALPORT_BAUD_38400:
            ui32Baud = BAUD_38400;
            break;
        case SERIALPORT_BAUD_57600:
            ui32Baud = BAUD_57600;
            break;
        case SERIALPORT_BAUD_115200:
            ui32Baud = BAUD_115200;
            break;
    }
    UARTStdioConfig((uint32_t)port, ui32Baud, output_clock_rate_hz);

    return TRUE;
}

//****************************************************************************
//*
//* Function name: UPS_StdioShutdown
//*
//* Return:   None
//*
//* Description: Not used now
//*
//****************************************************************************
void UPS_StdioShutdown (void *protStruct, UART_PORT port )
{

}

//****************************************************************************
//*
//* Function name: UPS_StdioRxErr
//*
//* Return:   None
//*
//* Description: Not used now
//*
//****************************************************************************
void UPS_StdioRxErr (void *protStruct, UART_PORT port )
{
    // Do nothing
}
//****************************************************************************
//*
//* Function name: UPS_InitProtocol
//*
//* Return:   None
//*
//* Description: This function initialises All serial ports with protocol settings at power up.
//*
//****************************************************************************
void UPS_InitProtocol(void)
{
    UART_PORT i=PORT_UART1;

    for ( i=PORT_UART1 ; i<SYSTEM_SERIAL_PORT_COUNT ; i++ )
    {
        if(IsInternalEEPROMActive())
            UPS_UART_Config_FromEEPROM(i);

        switch (upsPortConfig[i].protocolType )
        {
            case SERIAL_STDIO:
            case SERIAL_GSM_GPRS:
            case SERIAL_MODBUS_RTU_MASTER:
            case SERIAL_MODBUS_RTU_SLAVE:
//                UPS_UartInit(i);
                UPS_SetProtocol (i, upsPortConfig[i].protocolType, TRUE);
            break;
            default:
                break;
        }
    }
}


//****************************************************************************
//*
//* Function name: void HTTPResponse_UART_ResetDiagnostic(uint8_t Port)
//*
//* Return:   None
//*
//* Description:
//*
//****************************************************************************
//============================================================================

void HTTPResponse_UART_ResetDiagnostic(uint8_t Port)
{
    switch(Port)
    {
        case 0:
            memset(&UPSInterfaceStruct.uartDiag,0,sizeof(UART_DIAG_COUNTER));
            break;
        case 1:
            memset(&GsmStruct.GsmDiag,0,sizeof(GSM_DIAG_Counter));
            break;
        default:
            break;
    }
}


//****************************************************************************
//*
//* Function name: UPS_ReInitProtocol
//*
//* Return:   None
//*
//* Description:
//* This function reinitialises serial port with protocol settings when settings are
//* changed from web server.
//*
//****************************************************************************
void UPS_ReInitProtocol(UART_PORT Port)
{
    uint32_t ui32Baud;

    if(IsInternalEEPROMActive())
        UPS_UART_Config_FromEEPROM(Port);

    switch (upsPortConfig[Port].protocolType )
    {
        case SERIAL_STDIO:
            switch(upsPortConfig[Port].baudrate)
            {
                case SERIALPORT_BAUD_NULL:
                case SERIALPORT_BAUD_19200:
                    ui32Baud = BAUD_19200;
                    break;
                case SERIALPORT_BAUD_9600:
                    ui32Baud = BAUD_9600;
                    break;
                case SERIALPORT_BAUD_38400:
                    ui32Baud = BAUD_38400;
                    break;
                case SERIALPORT_BAUD_57600:
                    ui32Baud = BAUD_57600;
                    break;
                case SERIALPORT_BAUD_115200:
                    ui32Baud = BAUD_115200;
                    break;
            }
            UARTStdioConfig((uint32_t)Port, ui32Baud, output_clock_rate_hz);
            UPS_SetProtocol (Port,upsPortConfig[Port].protocolType,FALSE);
            break;
        case SERIAL_GSM_GPRS:
        case SERIAL_MODBUS_RTU_MASTER:
        case SERIAL_MODBUS_RTU_SLAVE:
//            UPS_UartInit(Port);
            UPS_SetProtocol (Port,upsPortConfig[Port].protocolType,FALSE);
        break;
        default:
            break;
    }

}
//*****************************HTTP Calls*********************************//
//****************************************************************************
//*
//* Function name: HTTPRequest_UPS_Get_Protocol
//*
//* Return:   16bit Protocol Type
//*
//* Description: This is being called from HTTP context to get serial protocol for serial port
//*
//****************************************************************************

uint16_t HTTPRequest_UPS_Get_Protocol (UART_PORT port)
{
    uint16_t protocol = SERIAL_INVALID;
    switch(upsPortConfig[port].protocolType)
    {
        case SERIAL_STDIO:
        case SERIAL_GSM_GPRS:
        case SERIAL_MODBUS_RTU_MASTER:
        case SERIAL_MODBUS_RTU_SLAVE:
            protocol = upsPortConfig[port].protocolType;
            break;
    }
    return protocol;
}

//****************************************************************************
//*
//* Function name: HTTPRequest_UPS_Get_BaudRate
//*
//* Return:   16bit Baudrate
//*
//* Description: This is being called from HTTP context to get serial Baudrate for serial port
//*
//****************************************************************************

uint16_t HTTPRequest_UPS_Get_BaudRate (UART_PORT port)
{
    uint16_t BaudRate = 0;

    switch(upsPortConfig[port].baudrate)
    {
        case SERIALPORT_BAUD_19200:
            BaudRate = 2;
            break;
        case SERIALPORT_BAUD_38400:
            BaudRate = 3;
            break;
        case SERIALPORT_BAUD_57600:
            BaudRate = 4;
            break;
        case SERIALPORT_BAUD_115200:
            BaudRate = 5;
            break;
    }
    return BaudRate;
}

//****************************************************************************
//*
//* Function name: HTTPRequest_UPS_Get_DataBits
//*
//* Return:   16bit Databits
//*
//* Description: This is being called from HTTP context to get serial Databits for serial port
//*
//****************************************************************************

uint16_t HTTPRequest_UPS_Get_DataBits (UART_PORT port)
{
    uint16_t DataBits = 1;//SERIALPORT_DATA_8BIT;
    switch(upsPortConfig[port].databit)
    {
        case SERIALPORT_DATA_8BIT:
            DataBits = 1;//SERIALPORT_DATA_8BIT;
            break;
        case SERIALPORT_DATA_7BIT:
            DataBits = 0;//SERIALPORT_DATA_7BIT;
            break;
    }
    return DataBits;
}
//****************************************************************************
//*
//* Function name: HTTPRequest_UPS_Get_StopBits
//*
//* Return:   16bit Stopbits
//*
//* Description: This is being called from HTTP context to get serial Stopbits for serial port
//*
//****************************************************************************

uint16_t HTTPRequest_UPS_Get_StopBits (UART_PORT port)
{
    uint16_t StopBits = 0;//SERIALPORT_STOPBIT_1;
    switch(upsPortConfig[port].stopbit)
    {
        case SERIALPORT_STOPBIT_1:
            StopBits = 0;//SERIALPORT_STOPBIT_1;
            break;
        case SERIALPORT_STOPBIT_2:
            StopBits = 1;//SERIALPORT_STOPBIT_2;
            break;
        default:
            StopBits = SERIALPORT_STOPBIT_INVALID;
    }
    return StopBits;
}

//****************************************************************************
//*
//* Function name: HTTPRequest_UPS_Get_Parity
//*
//* Return:   16bit Parity
//*
//* Description: This is being called from HTTP context to get serial parity for serial port
//*
//****************************************************************************

uint16_t HTTPRequest_UPS_Get_Parity (UART_PORT port)
{
    uint16_t Parity = SERIALPORT_PARITY_NONE;
    switch(upsPortConfig[port].parity)
    {
        case SERIALPORT_PARITY_ODD:
            Parity = SERIALPORT_PARITY_ODD;
            break;
        case SERIALPORT_PARITY_EVEN:
            Parity = SERIALPORT_PARITY_EVEN;
            break;
    }
    return Parity;
}

//****************************************************************************
//*
//* Function name: HTTPResponse_UPS_Put_BaudRate
//*
//* Return:   None
//*
//* Description: This is being called from HTTP context to set serial baudrate for serial port
//*
//****************************************************************************

void HTTPResponse_UPS_Put_BaudRate(UART_PORT port, uint32_t Index)
{
    switch(Index)
    {
    case 0:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_9600;
        break;
    case 1:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_14400;
        break;
    case 2:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_19200;
        break;
    case 3:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_38400;
        break;
    case 4:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_57600;
        break;
    case 5:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_115200;
        break;
    default:
        Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].baudrate = SERIALPORT_BAUD_19200;
    }

}

//****************************************************************************
//*
//* Function name: HTTPResponse_UPS_Put_DataBits
//*
//* Return:   None
//*
//* Description: This is being called from HTTP context to set serial databits for serial port
//*
//****************************************************************************

void HTTPResponse_UPS_Put_DataBits(UART_PORT port, uint32_t Index)
{
    switch(Index)
    {
        case 0:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].databit = SERIALPORT_DATA_7BIT;
            break;
        case 1:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].databit = SERIALPORT_DATA_8BIT;
            break;
        default:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].databit = SERIALPORT_DATA_INVALID;
     }

}
//****************************************************************************
//*
//* Function name: HTTPResponse_UPS_Put_StopBits
//*
//* Return:   None
//*
//* Description: This is being called from HTTP context to set serial stopbits for serial port
//*
//****************************************************************************

void HTTPResponse_UPS_Put_StopBits (UART_PORT port, uint32_t Index)
{
    switch(Index)
    {
        case 0:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].stopbit = SERIALPORT_STOPBIT_1;
            break;
        case 1:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].stopbit = SERIALPORT_STOPBIT_2;
            break;
        default :
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].stopbit = SERIALPORT_STOPBIT_INVALID;
    }

}
//****************************************************************************
//*
//* Function name: HTTPResponse_UPS_Put_Parity
//*
//* Return:   None
//*
//* Description: This is being called from HTTP context to set serial parity for serial port
//*
//****************************************************************************

void HTTPResponse_UPS_Put_Parity (UART_PORT port, uint32_t Index)
{
    switch(Index)
    {
        case 0:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].parity = SERIALPORT_PARITY_NONE;
            break;
        case 1:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].parity = SERIALPORT_PARITY_ODD;
            break;
        default:
            Device_Configurations.EepromMapStruct.Save_SerialSettings.CONFIG_SerialPort[port].parity = SERIALPORT_PARITY_EVEN;
     }

}

//**************************************************************************//
void HTTP_TrapSetParam(uint16_t Index, uint32_t Value)
{
    if(Index == 0)
    {
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[0] = Value;
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[1] = Value;
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[2] = Value;
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[3] = Value;
    }
    else if(Index == 1)
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo = Value;

}

//**************************************************************************

static void  HTTP_EnableDisablFlagFn(uint8_t *dataBuff,uint8_t itr,uint8_t itr1)
{

    switch(itr1)
        {
            case 0:
                dataBuff[itr]= 'A';
                break;
            case 1:
                dataBuff[itr]= 'B';
                break;
            case 2:
                dataBuff[itr]= 'C';
                break;
            case 3:
                dataBuff[itr]= 'D';
                break;
            case 4:
                dataBuff[itr]= 'E';
                break;
            case 5:
                dataBuff[itr]= 'F';
                break;
            case 6:
                dataBuff[itr]= 'G';
                break;
            case 7:
                dataBuff[itr]= 'H';
                break;
            case 8:
                dataBuff[itr]= 'I';
                break;
            case 9:
                dataBuff[itr]= 'J';
                break;
            case 10:
                dataBuff[itr]= 'K';
                break;
            case 11:
                dataBuff[itr]= 'L';
                break;
            case 12:
                dataBuff[itr]= 'M';
                break;
            case 13:
                dataBuff[itr]= 'N';
                break;
            case 14:
                dataBuff[itr]= 'O';
                break;
            case 15:
                dataBuff[itr]= 'P';
                break;
            case 16:
                dataBuff[itr]= 'Q';
                break;
            case 17:
                dataBuff[itr]= 'R';
                break;
            case 18:
                dataBuff[itr]= 'S';
                break;
            case 19:
                dataBuff[itr]= 'T';
                break;
            case 20:
                dataBuff[itr]= 'U';
                break;
            case 21:
                dataBuff[itr]= 'V';
                break;
            case 22:
                dataBuff[itr]= 'W';
                break;
            case 23:
                dataBuff[itr]= 'X';
                break;
            case 24:
                dataBuff[itr]= 'Y';
                break;
            case 25:
                dataBuff[itr]= 'Z';
                break;

         }

}

//**************************************************************************
uint32_t PrvWebServerSettingFlags=0;

static uint8_t HTTP_ControlCommands(uint32_t WebServerSettingFlags)
{
    uint8_t DBStatus=WRITE_ERROR;
    uint16_t itr=0;
    uint8_t status=WRITE_ERROR;
    uint8_t status1=WRITE_ERROR;
    uint32_t tempVar=0;

    UPS_WRITE_REQUEST  pUpsWrtReqparamEn;
    UPS_WRITE_REQUEST  pUpsWrtReqparamDs;
    uint8_t cnt=0;
    uint8_t cnt1=0;
   memset(pUpsWrtReqparamDs.ctrlcommand,0,30);
   memset(pUpsWrtReqparamEn.ctrlcommand,0,30);
    pUpsWrtReqparamEn.taskID = WEBSERVER_TASK_ID;
    pUpsWrtReqparamEn.ctrlcommand[cnt++]= 'P';
    pUpsWrtReqparamEn.ctrlcommand[cnt++]= 'E';

    pUpsWrtReqparamDs.taskID = WEBSERVER_TASK_ID;
    pUpsWrtReqparamDs.ctrlcommand[cnt1++]= 'P';
    pUpsWrtReqparamDs.ctrlcommand[cnt1++]= 'D';
 if(PrvWebServerSettingFlags!=WebServerSettingFlags)
 	{
	    for(itr=0;itr<26;itr++)
	    {
	        tempVar = (0x00000001 << itr);

			 if((WebServerSettingFlags & tempVar)==(PrvWebServerSettingFlags  & tempVar))
              {
                  status1 =  WRITE_SUCCESS;
                  status  =  WRITE_SUCCESS;
              }
			
			else if(((WebServerSettingFlags & tempVar)!=0)&&((PrvWebServerSettingFlags  & tempVar)!=1))
	        {
	            HTTP_EnableDisablFlagFn(pUpsWrtReqparamEn.ctrlcommand,2,itr);
	            pUpsWrtReqparamEn.ctrlcommand[3]= '\r';
	            pUpsWrtReqparamEn.ctrlcmdSize=4;
	            status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamEn);
	           if(status == WRITE_ERROR)
	           {
	               return WRITE_ERROR;
	           }
	        }
	       
			else if(((WebServerSettingFlags & tempVar)!=1)&&((PrvWebServerSettingFlags  & tempVar)!=0))
	        {
	            HTTP_EnableDisablFlagFn(pUpsWrtReqparamDs.ctrlcommand,2,itr);
	            pUpsWrtReqparamDs.ctrlcommand[3]= '\r';
	            pUpsWrtReqparamDs.ctrlcmdSize=4;
	            status1=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamDs);
	            if(status1 == WRITE_ERROR)
               {
	                return WRITE_ERROR;
               }
	        }

	    }
		PrvWebServerSettingFlags=WebServerSettingFlags;

	    if((status == WRITE_SUCCESS) && (status1 == WRITE_SUCCESS))
	    {
	       DBStatus= WRITE_SUCCESS;
	    }
	    else
	    {
	       DBStatus= WRITE_ERROR;
	    }
 	}
 else
 	{
 	DBStatus= WRITE_SUCCESS;
 	}
 	
     return DBStatus;

}
//**************************************************************************

void HTTP_FlagSetParam(uint16_t Index, uint32_t Value)
{
    if(Index == 0)
    {
        UPSDb.settingFlags.flagAll =0x00;
    }
    else if(Index == 1)
    {
        UPSDb.settingFlags.flagAll = (Value/2);
    }
    if((HTTP_ControlCommands(UPSDb.settingFlags.flagAll))==WRITE_SUCCESS)
    {
            //show Message
    }
    else
    {
        //Show message
    }
}


//************************************************************************************
void HTTP_StatusDisplay(uint16_t Index)
{
    pStatus.Bits.batTestOK      = UPSDb.upsStatus.Bits.batTestOK;
    pStatus.Bits.battTestFail   = UPSDb.upsStatus.Bits.battTestFail;
    pStatus.Bits.battSilence    = UPSDb.upsStatus.Bits.battSilence;
    pStatus.Bits.shutdownActive = UPSDb.upsStatus.Bits.shutdownActive;
    pStatus.Bits.TestinProgress = UPSDb.upsStatus.Bits.TestinProgress;
    pStatus.Bits.EPO            = UPSDb.upsStatus.Bits.EPO;
    pStatus.Bits.UPSFailed      = UPSDb.upsStatus.Bits.UPSFailed;
    pStatus.Bits.BypassActive   = UPSDb.upsStatus.Bits.BypassActive;
    pStatus.Bits.BattLow        = UPSDb.upsStatus.Bits.BattLow;
    pStatus.Bits.utilityFail    = UPSDb.upsStatus.Bits.utilityFail;
    pStatus.Bits.LiveStatus     = UPSDb.upsStatus.Bits.LiveStatus;
    pStatus.Bits.outputRelay    = UPSDb.upsFaultStruct.UPSRunningStatus.upsRStatusBits.outputRelay;
    pStatus.Bits.inputRelay     = UPSDb.upsFaultStruct.UPSRunningStatus.upsRStatusBits.inputRelay;
    pStatus.Bits.INV            = UPSDb.upsFaultStruct.UPSRunningStatus.upsRStatusBits.INV;
    pStatus.Bits.pfc            = UPSDb.upsFaultStruct.UPSRunningStatus.upsRStatusBits.pfc;
    pStatus.Bits.dcTodc         = UPSDb.upsFaultStruct.UPSRunningStatus.upsRStatusBits.dcTodc;
}


