/*
 * CommonIncludes.h
 *
 *  Created on: 08-Dec-2021
 *      Author: rushikesh.sonone
 */

#ifndef SOURCE_INCLUDE_COMMON_H_
#define SOURCE_INCLUDE_COMMON_H_


//===============================TIVA C STANDARDS INCLUDES=====================//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
//----------------------------------------------------------------------------//
//================================FREE RTOS INCLUDES==========================//
#include "FreeRTOSConfig.h"
#include <FreeRTOS.h>
#include "portmacro.h"
#include <semphr.h>
#include <timers.h>
#include <event_groups.h>
#include "task.h"
#include "queue.h"
//----------------------------------------------------------------------------//
//=================================HARDWARE_INCLUDES==========================//
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_emac.h"
#include "inc/hw_i2c.h"
#include "inc/hw_uart.h"
#include "inc/hw_nvic.h"
#include "inc/hw_can.h"
#include "inc/hw_flash.h"
#include "inc/hw_ssi.h"

//----------------------------------------------------------------------------//
//=================================DRIVER LIBRARY INCLUDES====================//
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/udma.h"
#include "driverlib/can.h"
#include "driverlib/pwm.h"
#include "driverlib/hibernate.h"
#include "driverlib/i2c.h"
#include "driverlib/eeprom.h"
#include "driverlib/fpu.h"
#include "driverlib/watchdog.h"
#include "driverlib/flash.h"
#include "driverlib/ssi.h"

//----------------------------------------------------------------------------//
//=================================UTILS INCLUDES====================//

#include "utils/uartstdio.h"

//----------------------------------------------------------------------------//
//===================================LWIP INCLUDES============================//
#include "lwiplib.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/dns.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/api.h"
#include "lwip/altcp.h"
#include "lwip/tcp.h"
#include "lwip/opt.h"
#include "httpd.h"
#include "sntp.h"
#include "lwip/stats.h"
#include <lwip/snmp.h>

//----------------------------------------------------------------------------//
//====================================USB INCLUDES============================//
#include "usblib/usblib.h"
#include "usblib/device/usbdbulk.h"
#include "usblib/device/usbdevice.h"
#include "usblib/host/usbhost.h"
#include "usblib/host/usbhmsc.h"
//----------------------------------------------------------------------------//
//====================================FATFS INCLUDES==========================//
#include "ff.h"
#include "diskio.h"
#include "fatfs.h"
//----------------------------------------------------------------------------//
//====================================BACNET INCLUDES=========================//
#if 0
#include "bacdef.h"
#include "bacenum.h"
#include "wp.h"
#include "rd.h"
#include "rp.h"
#include "rpm.h"
#include "readrange.h"

#include "datalink.h"
#include "bacdef.h"
#include "bacdcode.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "apdu.h"
#include "wp.h" /* WriteProperty handling */
#include "rp.h" /* ReadProperty handling */
#include "dcc.h"        /* DeviceCommunicationControl handling */
#include "version.h"
#include "device.h"     /* me */
#include "handlers.h"
#include "address.h"
#include "timer.h"

#include "bacint.h"
#include "bip.h"
#include "bvlc.h"
#include "handlers.h"
#include "bacerror.h"
#endif

//----------------------------------------------------------------------------//
//====================================SNMP INCLUDES=========================//
#include "lwip/netif.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmpv3.h"
#include "lwip/apps/snmp_snmpv2_framework.h"
#include "lwip/apps/snmp_snmpv2_usm.h"
#include "snmpv3_dummy.h"
#include "private_mib.h"
#include "upsMIB2.h"
#include "upsPrivateMIB.h"

//==============================================================================
// GLOBAL TYPE DEFINITIONS
//==============================================================================
#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#define SYSTEM_CLOCK    	(120000000U)
#define SNMP_CARD_MAJOR_VERSION	(1)
#define SNMP_CARD_MINOR_VERSION	(0)

//==============================================================================
// DEBUG ENABLE AS MODULE VISE
//==============================================================================

#if defined(DEBUG)
#define _MBTCP_DEBUG  		0
#define _COMTIMER_DEBUG 	0
#define _HTTP_DEBUG 		0
#define _EEPROM_DEBUG 		0
#define _USBDEV_DEBUG 		0
#endif


typedef bool BOOL;


typedef struct __attribute__ ((packed))
{
    uint8_t Year;
    uint8_t Month;
    uint8_t Date;
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
}Time;	

extern Time GlobalDateTime;
typedef union __attribute__((packed))
{
    uint8_t all;

    struct {
        uint8_t b0 : 1;
        uint8_t b1 : 1;
        uint8_t b2 : 1;
        uint8_t b3 : 1;
        uint8_t b4 : 1;
        uint8_t b5 : 1;
        uint8_t b6 : 1;
        uint8_t b7 : 1;
    } bits;
} REG_uint8_t;

typedef union
{
    uint16_t all;
    uint8_t byte[sizeof(uint16_t)];

    struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
    } bits;
} REG_uint16_t;

typedef union
{
    uint32_t all;
    uint16_t word[sizeof(uint16_t)];
    uint8_t  byte[sizeof(uint32_t)];

    struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t b16:1;
         uint8_t b17:1;
         uint8_t b18:1;
         uint8_t b19:1;
         uint8_t b20:1;
         uint8_t b21:1;
         uint8_t b22:1;
         uint8_t b23:1;
         uint8_t b24:1;
         uint8_t b25:1;
         uint8_t b26:1;
         uint8_t b27:1;
         uint8_t b28:1;
         uint8_t b29:1;
         uint8_t b30:1;
         uint8_t b31:1;
    } bits;
} REG_uint32_t;

typedef union
{
    uint64_t all;
    uint32_t dword[sizeof(uint64_t)/sizeof(uint32_t)];
    uint16_t word[sizeof(uint64_t)/sizeof(uint16_t)];
    uint8_t byte[sizeof(uint64_t)];

    struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t b16:1;
         uint8_t b17:1;
         uint8_t b18:1;
         uint8_t b19:1;
         uint8_t b20:1;
         uint8_t b21:1;
         uint8_t b22:1;
         uint8_t b23:1;
         uint8_t b24:1;
         uint8_t b25:1;
         uint8_t b26:1;
         uint8_t b27:1;
         uint8_t b28:1;
         uint8_t b29:1;
         uint8_t b30:1;
         uint8_t b31:1;
         uint8_t b32:1;
         uint8_t b33:1;
         uint8_t b34:1;
         uint8_t b35:1;
         uint8_t b36:1;
         uint8_t b37:1;
         uint8_t b38:1;
         uint8_t b39:1;
         uint8_t b40:1;
         uint8_t b41:1;
         uint8_t b42:1;
         uint8_t b43:1;
         uint8_t b44:1;
         uint8_t b45:1;
         uint8_t b46:1;
         uint8_t b47:1;
         uint8_t b48:1;
         uint8_t b49:1;
         uint8_t b50:1;
         uint8_t b51:1;
         uint8_t b52:1;
         uint8_t b53:1;
         uint8_t b54:1;
         uint8_t b55:1;
         uint8_t b56:1;
         uint8_t b57:1;
         uint8_t b58:1;
         uint8_t b59:1;
         uint8_t b60:1;
         uint8_t b61:1;
         uint8_t b62:1;
         uint8_t b63:1;
    } bits;
} REG_uint64_t;


//==============================================================================
// GLOBAL VARIABLE DECLARATIONS
//==============================================================================
extern uint32_t output_clock_rate_hz;

//==============================END OF FILE===================================//
//============================================================================//


#endif /* SOURCE_INCLUDE_COMMON_H_ */
