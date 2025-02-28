//****************************************************************************
//*
//*
//* File: swupdate.c
//*
//* Synopsis: Software update application as the Ethernet bootloader software 
//*			  update functionality on Top of LWIP.
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include "utils/swupdate.h"
#include <EthernetTask.h>

//============================================================================
// 							 > LOCAL DEFINES <
//----------------------------------------------------------------------------
#define MPACKET_PORT            9


#define MPACKET_HEADER_LEN      6
#define MPACKET_MAC_REP         4
#define MPACKET_MAC_LEN         6
#define MPACKET_LEN             (MPACKET_HEADER_LEN +                         \
                                 (MPACKET_MAC_REP * MPACKET_MAC_LEN))

#define MPACKET_MARKER          0xAA

//============================================================================
// 							 > LOCAL MACROS <
//----------------------------------------------------------------------------

//============================================================================
// 							 > LOCAL ENUMERATED TYPES <
//----------------------------------------------------------------------------

//============================================================================
// 							 > LOCAL STRUCTURES <
//----------------------------------------------------------------------------

//============================================================================
// 							 > LOCAL FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

//============================================================================
// 							 > LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------
tSoftwareUpdateRequested g_pfnUpdateCallback = NULL;
static struct udp_pcb *g_psMagicPacketPCB = NULL;
//static uint8_t g_pui8MACAddr[6];

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//=============================================================================

//****************************************************************************
//*
//* Function name: SoftwareUpdateUDPReceive
//*
//* Return: None 			
//*
//*
//* Description:
//*   This function is called when the lwIP TCP/IP stack has an incoming
//*   UDP packet to be processed on the remote firmware update signal port.
//*
//****************************************************************************
//============================================================================

static void
//SoftwareUpdateUDPReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
SoftwareUpdateUDPReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    int8_t *pi8Data = p->payload;
    uint32_t ui32Loop, ui32MACLoop;

    //
    // Check that the packet length is what we expect.  If not, ignore the
    // packet.
    //
    if(p->len == MPACKET_LEN)
    {
        //
        // The length matches so now look for the 6 byte header
        //
        for(ui32Loop = 0; ui32Loop < MPACKET_HEADER_LEN; ui32Loop++)
        {
            //
            // Does this header byte match the expected marker?
            //
            if((*pi8Data & 0x000000FF)!= MPACKET_MARKER)
            {
                //
                // No - free the buffer and return - this is not a packet
                // we are interested in.
                //
                pbuf_free(p);
                return;
            }
            else
            {
                //
                // Byte matched so move on to the next one.
                //
                //UARTprintf("%d) ui32Loop %x \r\n",ui32Loop +1 , *pi8Data);
                pi8Data++;
				
            }
        }
    }
    else
    {
        //
        // No - free the buffer and return - this is not a packet
        // we are interested in.
        //
        pbuf_free(p);
        return;
    }

    //
    // If we get here, the packet length and header markers indicate
    // that this is a remote firmware update request.  Now check that it
    // is for us and that it contains the required number of copies of
    // the MAC address.
    //

    //
    // Loop through each of the expected MAC address copies.
    //
    for(ui32Loop = 0; ui32Loop < MPACKET_MAC_REP; ui32Loop++)
    {
        //
        // Loop through each byte of the MAC address in this
        // copy.
        //
        for(ui32MACLoop = 0; ui32MACLoop < MPACKET_MAC_LEN; ui32MACLoop++)
        {
            //
            // Does the payload MAC address byte match what we expect?
            //
            if((*pi8Data & 0x000000FF) != networkConfig.MAC_ADDRESS[ui32MACLoop])
            {
                //
                // No match - free the packet and return.
                //
                pbuf_free(p);
                return;
            }
            else
            {
                //
                // Byte matched so move on to the next one.
                //
                //UARTprintf("%d) ui32MACLoop  %x : %x\r\n",ui32MACLoop +1 , *pi8Data, g_pui8MACAddr[ui32MACLoop] );
                pi8Data++;
				
            }
        }
    }

    //
    // Free the pbuf since we are finished with it now.
    //
    pbuf_free(p);

    //
    // If we get this far, we've received a valid remote firmare update
    // request targetted at this board.  Signal this to the application
    // if we have a valid callback pointer.
    //
    if(g_pfnUpdateCallback)
    {
        g_pfnUpdateCallback();
    }
}
//=============================================================================

//****************************************************************************
//*
//* Function name: swupdate_SoftwareUpdateInit
//*
//* Return: None 			
//*
//*
//* Description:
//*   Callback function allowed to decide to jump in Software Updater.
//*   If the application wishes to allow the update to go ahead, 
//*   it must call SoftwareUpdateBegin() from non-interrupt context after the 
//*   callback is received. 
//*   Note that the callback will most likely be made in interrupt context 
//*   itself. so it is not safe to call SoftwareUpdateBegin() from within the  
//*   callback This function may be used on Ethernet-enabled parts to support
//*   remotely-signaled firmware updates over Ethernet.  The LM Flash Programmer
//*   (LMFlash.exe) application sends a magic packet to UDP port 9 whenever the
//*   user requests an Ethernet-based firmware update.  This packet consists of
//*   6 bytes of 0xAA followed by the target MAC address repeated 4 times.
//*   This function starts listening on UDP port 9 and, if a magic packet
//*   matching the MAC address of this board is received, makes a call to the
//*   provided callback function to indicate that an update has been requested.
//*   The callback function provided here will typically be called in the context
//*   of the lwIP Ethernet interrupt handler.  It is not safe to call
//*   SoftwareUpdateBegin() in this context so the application should use the
//*   callback to signal code running in a non-interrupt context to perform the
//*   update if it is to be allowed.
//*   UDP port 9 is chosen for this function since this is the well-known port
//*   associated with ``discard'' operation.  In other words, any other system
//*   receiving the magic packet will simply ignore it.  The actual magic packet
//*   used is modeled on Wake-On-LAN which uses a similar structure (6 bytes of
//*   0xFF followed by 16 repetitions of the target MAC address).  Some
//*   Wake-On-LAN implementations also use UDP port 9 for their signaling.
//*   note Applications using this function must initialize the lwIP stack prior
//*   to making this call and must ensure that the lwIPTimer() function is called
//*   periodically.  lwIP UDP must be enabled in lwipopts.h to ensure that the
//*   magic packets can be received.
//*   Software updater has no time window for execuation LMFasher can request
//*   any time in between the execuation.
//*
//****************************************************************************
//============================================================================

void swupdate_SoftwareUpdateInit(tSoftwareUpdateRequested pfnCallback)
{
    //uint32_t ui32User0, ui32User1;
	//uint8_t Status = 0;
    //
    // Remember the callback function pointer we have been given.
    //
    g_pfnUpdateCallback = pfnCallback;

    //
    // Get the MAC address from the user registers in NV ram.
    //
   // FlashUserGet(&ui32User0, &ui32User1);
	
    //
    // Convert the 24/24 split MAC address from NV ram into a MAC address
    // array.
    //
   // g_pui8MACAddr[0] = ui32User0 & 0xff;
 //   g_pui8MACAddr[1] = (ui32User0 >> 8) & 0xff;
  //  g_pui8MACAddr[2] = (ui32User0 >> 16) & 0xff;
 //   g_pui8MACAddr[3] = ui32User1 & 0xff;
  //  g_pui8MACAddr[4] = (ui32User1 >> 8) & 0xff;
 //   g_pui8MACAddr[5] = (ui32User1 >> 16) & 0xff;

    //
    // Set up a UDP PCB to allow us to receive the magic packets sent from
    // LMFlash.  These may be sent to port 9 from any port on the source
    // machine so we do not call udp_connect here (since this causes lwIP to
    // filter any packet that did not originate from port 9 too).
    //
    if(g_psMagicPacketPCB != NULL) 
	{
		udp_remove(g_psMagicPacketPCB);
	}
    g_psMagicPacketPCB = udp_new();
    udp_recv(g_psMagicPacketPCB, SoftwareUpdateUDPReceive, NULL);
    udp_bind(g_psMagicPacketPCB, IP_ADDR_ANY, MPACKET_PORT);
	/*UARTprintf("Status %d for MAC:- %x:%x:%x:%x:%x:%x \r\n",
				Status,g_pui8MACAddr[0],g_pui8MACAddr[1],g_pui8MACAddr[2],g_pui8MACAddr[3],
				g_pui8MACAddr[4],g_pui8MACAddr[5]);*/
	
}
//=============================================================================

//****************************************************************************
//*
//* Function name: swupdate_SoftwareUpdateBegin
//*
//* Return: None 			
//*
//*
//* Description:
//*   This function passes control to the bootloader and initiates an update of
//*   the main application firmware image via BOOTP across Ethernet.  This
//*   function may only be used on parts supporting Ethernet and in cases where
//*   the Ethernet boot loader is in use alongside the main application image.
//*   It must not be called in interrupt context.
//*   Applications wishing to make use of this function must be built to
//*   operate with the bootloader.  If this function is called on a system
//*   which does not include the bootloader, the results are unpredictable.
//*   note It is not safe to call this function from within the callback
//*   provided on the initial call to swupdate_SoftwareUpdateInit().  
//*   The application must use the callback to signal a pending update 
//*   (assuming the update is to be permitted) to some other code
//*   running in a non-interrupt context.
//*
//****************************************************************************
//============================================================================

void swupdate_SoftwareUpdateBegin(BOOL forceUpdate)
{

	vTaskDelay(100);// delay to make sure comms is finished

    HWREG(NVIC_DIS0) = 0xffffffff;
    HWREG(NVIC_DIS1) = 0xffffffff;
    HWREG(NVIC_DIS2) = 0xffffffff;
    HWREG(NVIC_DIS3) = 0xffffffff;
    HWREG(NVIC_DIS4) = 0xffffffff;

    //
    // Also disable the SysTick interrupt.
    //
  //  SysTickIntDisable();
  //  SysTickDisable();
	
	vTaskSuspendAll();
	vTaskEndScheduler();
	if(forceUpdate == TRUE)
	{
		HWREG(FORCE_UPDATE_ADDR) = FORCE_UPDATE_VALUE;
	}
	HWREG(NVIC_APINT) = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);

	while(1)
	{
	}
}
//============================================================================

//==============================================================================
// END OF swupdate.c FIlE
//==============================================================================


