//*****************************************************************************
//
// usbhandler.c - General USB handling routines.
//
// Copyright (c) 2007-2020 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of revision 2.2.0.295 of the Tiva USB Library.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/usb.h"
#include "usblib/usblib.h"
#include "usblib/usblibpriv.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdevicepriv.h"
#include "usblib/usblibpriv.h"
#include "Common.h"

#if (_USBDEV_DEBUG == 1)
uint32_t usbTimeStampLow= 0xFFFFFFFF,usbTimeStampHigh= 0,usbTimeStampRunning= 0;
uint32_t usbRateTimeStampTemp= 0,usbRateTimeStamp=0,usbRateTimeStampHigh=0;
#endif
//*****************************************************************************
//
//! \addtogroup device_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! The USB device interrupt handler.
//!
//! This the main USB interrupt handler entry point for use in USB device
//! applications.  This top-level handler will branch the interrupt off to the
//! appropriate application or stack handlers depending on the current status
//! of the USB controller.
//!
//! Applications which operate purely as USB devices (rather than dual mode
//! applications which can operate in either device or host mode at different
//! times) must ensure that a pointer to this function is installed in the
//! interrupt vector table entry for the USB0 interrupt.  For dual mode
//! operation, the vector should be set to point to \e USB0DualModeIntHandler()
//! instead.
//!
//! \return None.
//
//*****************************************************************************
void
USB0DeviceIntHandler(void)
{
    uint32_t ui32Status;
#if (_USBDEV_DEBUG == 1)
	uint32_t TimeStamp= 0,TimeStamp1=0;

	TimeStamp = SYS_GetTickMicroSecond();

	if(TimeStamp >= usbRateTimeStampTemp)
	{
		usbRateTimeStamp=TimeStamp-usbRateTimeStampTemp;
	}

	if(usbRateTimeStampHigh < usbRateTimeStamp)
			usbRateTimeStampHigh = usbRateTimeStamp;

	usbRateTimeStampTemp=TimeStamp;
#endif
    //
    // Get the controller interrupt status.
    //
    ui32Status = MAP_USBIntStatusControl(USB0_BASE);

    //
    // Call the internal handler.
    //
    USBDeviceIntHandlerInternal(0, ui32Status);

#if (_USBDEV_DEBUG == 1)

	TimeStamp1 = SYS_GetTickMicroSecond();
	if(TimeStamp1 >= TimeStamp)
	{
		usbTimeStampRunning = TimeStamp1 - TimeStamp;
        if((usbTimeStampRunning) >= usbTimeStampHigh)
        {
            usbTimeStampHigh=usbTimeStampRunning;
        }

        if((usbTimeStampRunning) <= usbTimeStampLow)
        {
            usbTimeStampLow=usbTimeStampRunning;
        }
	}
#endif	
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
