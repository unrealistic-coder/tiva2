//****************************************************************************
//*
//*
//* File: uartstdio.h
//*
//* Synopsis: Uart Stdio Console define file - related Structures and interfaces
//*
//*
//****************************************************************************

#ifndef __UARTSTDIO_H__
#define __UARTSTDIO_H__

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------

#ifdef UART_BUFFERED
#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE     128
#endif
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE     1024
#endif
#endif
//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

extern void UARTStdioConfig(uint32_t ui32Port, uint32_t ui32Baud,
                            uint32_t ui32SrcClock);
extern int UARTgets(char *pcBuf, uint32_t ui32Len);
extern unsigned char UARTgetc(void);
extern void UARTprintf(const char *pcString, ...);
extern void UARTvprintf(const char *pcString, va_list vaArgP);
extern int UARTwrite(const char *pcBuf, uint32_t ui32Len);
#ifdef UART_BUFFERED
extern int UARTPeek(unsigned char ucChar);
extern void UARTFlushTx(bool bDiscard);
extern void UARTFlushRx(void);
extern int UARTRxBytesAvail(void);
extern int UARTTxBytesFree(void);
extern void UARTEchoSet(bool bEnable);
#endif //UART_BUFFERED
//----------------------------------------------------------------------------

#endif // __UARTSTDIO_H__
//==============================================================================
// END OF __UARTSTDIO_H__
//==============================================================================

