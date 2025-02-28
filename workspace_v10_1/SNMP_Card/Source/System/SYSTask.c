//****************************************************************************
//*
//*
//* File: SYSTask.c
//*
//* Synopsis: System Task used as application on the top of 
//*			  TIVA C Contoller
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include "SYSTask.h"
#include "UPSInterface.h"
#include "SMTPManager.h"

static struct udp_pcb *ntp_pcb = NULL;   // UDP PCB for NTP
static int NTP_request_in_progress = 0;  // Flag to check if a request is pending
static uint32_t TimeSynchStartTime=0,TimeSynchIntervalTime=60*100;//1 minute //As Time is 10msec
//#define NTP_SERVER_IP "129.6.15.28"  //"pool.ntp.org"// Example NTP server (time.nist.gov)
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
//								> LOCAL VARIABLE DECLARATIONS <
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//****************************************************************************
//*
//* Function name: SYS_GetTickMicroSecond
//*
//* Return: uint32_t - Time in Microsecond
//*
//*
//* Description:
//*   Get Timer2 counts in microseconds use as mostly for Profiling.
//*
//****************************************************************************
//==============================================================================
uint32_t SYS_GetTickMicroSecond( void )
{
	return ((0xFFFFFFFF - TimerValueGet(TIMER2_BASE,TIMER_A))/SYS_MICROSECOND_TICK);
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SYSTask_TimerInit
//*
//* Return: None
//*
//*
//* Description:
//*  Initialze System timer, used for various purpose along the code.
//*
//****************************************************************************
//==============================================================================
void SYSTask_TimerInit( void )
{
     // Set up HAL
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

	// Wait for the UART module to be ready.
	while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
	{
	}
	MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	MAP_TimerLoadSet(TIMER2_BASE, TIMER_A, 0xFFFFFFFF);

 	MAP_TimerEnable(TIMER2_BASE, TIMER_A);

 	//GlobalEventInit();
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SYSTask_SaveConfigRequestInEEPROM
//*
//* Return: None
//*
//*
//* Description:
//*  Recive EEPROM Queue here, with respective module Index and Run respective
//*  operation for respective module. after saving module change save new 
//*  settings in EEPROM for that respective module. This function is called 
//*  from HTTP web server.
//*
//****************************************************************************
//==============================================================================
static void SYSTask_SaveConfigRequestInEEPROM(void)
{
	EEPROMReqQueue LatestRequest;
    BOOL Status = FALSE;
	if(xQueueReceive( Device_Configurations.EEPROMCurrentState.xEEPROMSaveHandler,
						&LatestRequest, 20))
	{	
		switch(LatestRequest.ModuleNo)
		{

            case (SNMPTrap_CONFIG_SAVE):
            {
                if(LatestRequest.ModuleState == 0x01)
                {
                    //update device config
                    Status = TRUE;//Device_updateID();
                }
                else if(LatestRequest.ModuleState == 0x02)
                {
                    Status = TRUE;//Device_updateName();
                }
                else
                {
                    //Device_updateID();
                    //Device_updateName();
                    Status=TRUE;
                }

            }break;

            case (MBTCP_CONFIG_SAVE):
			{
				if(LatestRequest.ModuleState == TRUE)
				{
					Status = TRUE;
				}

			}break;	
			case (ETH_SETTS_CONFIG_SAVE):
			{
				if(LatestRequest.ModuleState == TRUE)
				{
					EthernetTask_SaveNetworkChangeInConfig();
					Status = TRUE;
				}

			}break;	

			case (SYSTEM_INFO_SAVE):
			{
				if(LatestRequest.ModuleState == TRUE)
				{
					Status = TRUE;
				}

			}break;
			case (REBOOT_DEVICE):
			{
				Status = FALSE;
				swupdate_SoftwareUpdateBegin(FALSE);

			}break;					
			default: //break for while now
				return;

		}
		if(Status)
		{
			OnChipEEPROM_SaveConfiguration(LatestRequest.ModuleNo);
		}
	}
}
//==============================================================================
//****************************************************************************
//*
//* Function name: SYSTask_PeripheralEnable
//*
//* Return: None
//*
//*
//* Description:
//* Enables all GPIO peripherals
//*
//****************************************************************************
//==============================================================================
void SYSTask_PeripheralEnable(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
        vTaskDelay(10);
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
    {
        vTaskDelay(10);
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP))
    {
        vTaskDelay(10);
    }
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ))
    {
        vTaskDelay(10);
    }

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
	//
	// Set GPIO PJ0 and PJ1 as an output.  This drives an LED on the board that will
	// toggle when a watchdog interrupt is processed.
	//
#if defined(PART_TM4C1292NCPDT)	    
		MAP_GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_2);
		MAP_GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_3);
		MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_2, 0);
		MAP_GPIOPinWrite(GPIO_PORTP_BASE, GPIO_PIN_3, 0);

		//Port configuration
		MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

		MAP_GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_4);
		MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

   	 	MAP_ADCSequenceEnable(ADC0_BASE, 3);
  		MAP_ADCIntClear(ADC0_BASE, 3);
		

#elif defined(PART_TM4C1294NCPDT)
		MAP_GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0);
		MAP_GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_1);
		MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_PIN_0);
		MAP_GPIOPinWrite(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_PIN_1);

#endif	

}
//==============================================================================
//****************************************************************************
//*
//* Function name: NTP_unix_to_rtc
//*
//* IST formate date time
//****************************************************************************
//==============================================================================
void NTP_unix_to_rtc(uint32_t unix_time, NTP_Time * rtc_time)
{
    uint32_t seconds_in_day = 86400;
    uint32_t days = unix_time / seconds_in_day;
    uint32_t remaining_seconds = unix_time % seconds_in_day;
    uint16_t days_in_year;
    // Days in each month (non-leap year)
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Calculate hours, minutes, and seconds
    rtc_time->Hours = remaining_seconds / 3600;
    rtc_time->Minutes = (remaining_seconds % 3600) / 60;
    rtc_time->Seconds = remaining_seconds % 60;

    // Unix epoch starts in 1970
    uint16_t year = 1970;

    while (1)
    {
        days_in_year = 365;

        if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))// Is Leap Year
        {
            days_in_year = 366;
        }

        if (days >= days_in_year)
        {
            days -= days_in_year;
            year++;
        }
        else
        {
            break;
        }
    }

    rtc_time->Year = year;

    // Determine the current month
    uint8_t month = 0;
    while (1) {
        uint8_t days_this_month = days_in_month[month];
        // Adjust for leap year in February
        if (month == 1 && isLeapYear(year)) {
            days_this_month = 29;
        }

        if (days >= days_this_month) {
            days -= days_this_month;
            month++;
        } else {
            break;
        }
    }

    rtc_time->Month = month + 1; // Months are 1-based in RTC
    rtc_time->Date = days + 1;    // Days are 1-based in RTC
}
//==============================================================================
//****************************************************************************
//*
//* Function name: dns_callback
//* DNS callback once the server name has been resolved
//****************************************************************************
//==============================================================================
void dns_callback(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr != NULL)
    {
        //printf("DNS resolved: %s -> %s\n", name, ipaddr_ntoa(ipaddr));
        // Send the NTP request now that we have the server IP
        NTP_send_request(ipaddr);
    } else
    {
     //printf("DNS resolution failed for: %s\n", name);
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//* Entry point for NTP client initialization with DNS lookup
//****************************************************************************
//==============================================================================
void NTP_start(void) {
    // Create a new UDP PCB
    ntp_pcb = udp_new();
    if (ntp_pcb == NULL) {
        printf("Failed to create UDP PCB.\n");
        return;
    }

    // Set up the receive callback for incoming packets
    udp_recv(ntp_pcb, NTP_receive_callback, NULL);

    // Start DNS resolution for the NTP server
    NTP_Init_Send_Funptr=NTP_start_dns;
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//* Function to start DNS resolution
//****************************************************************************
//==============================================================================
void NTP_start_dns(void)
{
    ip_addr_t resolved_ip;
    //printf("Starting DNS resolution for NTP server: %s\n",  Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer);
    //Perform DNS lookup with a non-blocking call

    err_t err = dns_gethostbyname((const char *)Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer, &resolved_ip, dns_callback, NULL);

    // If DNS cache has the entry, we already have the IP
    if (err == ERR_OK)
    {
      // printf("DNS lookup found in cache.\n");
        ip_addr_t ntp_server_ip;
        dns_gethostbyname((const char *)Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer, &ntp_server_ip, NULL, NULL);
        NTP_send_request(&ntp_server_ip);
    }
    else if (err == ERR_INPROGRESS)
    {
        //printf("DNS lookup in progress.\n");
        //Will wait for the callback to resolve the DNS
    }
    else
    {
     //   printf("DNS lookup failed with error: %d\n", err);
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//*Function to send an NTP request
//****************************************************************************
//==============================================================================
void NTP_send_request(const ip_addr_t *ntp_server_ip)
{
    if (NTP_request_in_progress)
    {
      //  printf("NTP request already in progress.\n");
        return;
    }
    // Prepare the NTP request packet (48 bytes)
    struct pbuf *ntp_packet = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_RAM);
    if (ntp_packet == NULL)
    {
       // printf("Failed to allocate pbuf for NTP packet.\n");
        return;
    }

    // Initialize the NTP packet (NTP request format)
    memset(ntp_packet->payload, 0, 48);
    uint8_t *ntp_payload = (uint8_t *)ntp_packet->payload;
    ntp_payload[0] = 0x1B;  // LI=0, VN=3, Mode=3 (Client)

    // Send the NTP request
    if (udp_sendto(ntp_pcb, ntp_packet, ntp_server_ip, NTP_PORT) == ERR_OK)
    {
        NTP_request_in_progress = 1;
        //printf("NTP request sent.\n");
        //Set a timeout for the response
        sys_timeout(NTP_TIMEOUT_MS, NTP_timeout, NULL);
    }
    else
    {
      //  printf("Failed to send NTP request.\n");
    }
    // Free the pbuf after sending
    pbuf_free(ntp_packet);
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//* Callback function for receiving NTP responses
//****************************************************************************
//==============================================================================
void NTP_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (p != NULL)
    {
        //printf("NTP response received.\n");
        //Process the NTP response data
        NTP_process_response((uint8_t *)p->payload);

        //Clear the NTP request flag
        NTP_request_in_progress = 0;

        //Free the pbuf
        pbuf_free(p);
    }
    else
    {
      //printf("Received empty NTP response.\n");
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//* Function to process the received NTP response and extract the time
//****************************************************************************
//==============================================================================
void NTP_process_response(uint8_t *ntp_data)
{
    Time SystemTimeSet;
    NTP_Time globaltime;
    // Extract the time from the NTP response (timestamp starts at byte 40)
    uint32_t ntp_time = (ntp_data[40] << 24) | (ntp_data[41] << 16) | (ntp_data[42] << 8) | ntp_data[43];
    // NTP timestamp starts on January 1, 1900; Unix time starts on January 1, 1970.
    // Subtract the difference in seconds between 1970 and 1900 (2208988800 seconds).
    uint32_t unix_time = ntp_time - 2208988800U;
    NTP_unix_to_rtc(unix_time,&globaltime);

     SystemTimeSet.Year=globaltime.Year%2000;
     SystemTimeSet.Month=globaltime.Month;
     SystemTimeSet.Date=globaltime.Date;
     SystemTimeSet.Hours= ((globaltime.Hours) + 5) % 24; // Add 5 hours
     SystemTimeSet.Minutes = ((globaltime.Minutes) + 30) % 60;  // Add 30 minutes
     if (SystemTimeSet.Minutes < 30)
     {
         SystemTimeSet.Hours += 1; // If adding 30 minutes rolls over an hour, increment hour
     }

     SystemTimeSet.Seconds=globaltime.Seconds;
     WriteNewDateTimeToRTC(SystemTimeSet);//Write Synched date in RTC
}
//==============================================================================
//****************************************************************************
//*
//* Function name: ntp_start
//* Timeout callback function if no NTP response is received in time
//****************************************************************************
//==============================================================================
void NTP_timeout(void *arg)
{
    if (NTP_request_in_progress)
    {
     //   printf("NTP request timed out.\n");
        NTP_request_in_progress = 0;
        // Optionally, retry the NTP request
        NTP_Init_Send_Funptr();  // Restart with DNS lookup again
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: NTP_ClockSynch
//*
//****************************************************************************
//==============================================================================
static void NTP_ClockSynch()
{
    if((networkConfig.CurrentState == STATE_LWIP_WORKING))
    {
      if(COMTimer_CheckTimerExpiry(TimeSynchStartTime,TimeSynchIntervalTime))
         {
             TimeSynchStartTime = COMTimer_Get10MsecTick();
             if(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable==1)
             {
                NTP_Init_Send_Funptr();
             }
         }

     }
}
//****************************************************************************
//*
//* Function name: SYS_Task
//*
//* Return: None
//*
//*
//* Description:
//*  System task run check for hardware update by UART and USB also check any 
//*  alarm change from RCD and DDC with GPIO also any request from HTTP web page
//*  change in settings of system.
//*
//****************************************************************************
//==============================================================================
void SYS_Task()
{
    SMTP_Init();
    NTP_Init_Send_Funptr=NTP_start;
    while(1)
    {
        Error_ResetTaskWatchdog(SYS_TASK_ID);

         SYSTask_SaveConfigRequestInEEPROM();

         SMTP_ProcessPendingEmail();

         ReadCurrentDateTime();

         NTP_ClockSynch();

         vTaskDelay(10);
    }
}
//==============================================================================
//==============================================================================
// END OF SYSTask.c FIlE
//==============================================================================


