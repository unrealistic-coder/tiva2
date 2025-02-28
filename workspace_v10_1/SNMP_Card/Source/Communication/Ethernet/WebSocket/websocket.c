#include "websocket.h"
#include "EthernetTask.h"
#include "sha1.h"
#include "base64.h"
#include <string.h>
#include "HibernateCalendar.h"
#include "Common.h"
#include <COMTimer.h>
#include "SPIFlashTask.h"
#include "EthernetTask.h"
#include <UPSInterface.h>
#include "FTPServer.h"
#include <SNMPAgent.h>
#include <SMTPManager.h>
#include <SPIExtRTC.h>
#include "ModbusTCPServer.h"
#include "SPIFLash_Datalogger.h"
#include "SerialDriver.h"
#include "UPSInterface.h"
#include "lwip/inet_chksum.h"
#include "HibernateCalendar.h"
#include "SYSTask.h"

WEBSOCKET_TCP_RAM_STRUCT WebSocketStruct;
WEB_SOCKET_STRUCT ws_server = {
  .connected_clients_cnt = 0
};
//==============================================================================
// Global Macro Functions
//==============================================================================

const char *head_ws = "HTTP/1.1 101 Switching Protocols\n\
Upgrade: websocket\n\
Connection: Upgrade\nSec-WebSocket-Accept: \0";

static uint8_t*   ws_set_size_to_frame   ( uint32_t size, uint8_t *out_frame );
static uint8_t*   ws_set_data_to_frame   ( uint8_t *data, uint16_t size, uint8_t *out_frame );
char  ws_key_accept[64] = {0};
uint32_t LenTxBuf=0;
#if defined WEBSERVER_USING_SPI_FLASH
HTTPCONNSOCKET_RAM_STRUCT HttpServerStruct;

HTTP_SERVER_CONN_PAGE httpconnGETcommndList[20]=
{

    {"GET /WEB/CSS/mchp.css",17},
    {"GET /WEB/CSS/mchp2.css",18},
    {"GET /WEB/CSS/new.css",16},
    {"GET /WEB/CSS/tab.css",16},

    {"GET /WEB/Script/WebSkt.js",21},
    {"GET /WEB/Script/PPage.js",20},
    {"GET /WEB/Script/SPage.js",20},
    {"GET /WEB/Script/EPage.js",20},
    {"GET /WEB/Script/DLPage.js",21},
    {"GET /WEB/Script/FPage.js",20},
    {"GET /WEB/Script/SPage.js",20},
    {"GET /WEB/Script/Mpage.js",20},


    {"GET /WEB/images/FUJI.png",20},
    {"GET /WEB/DPage.shtml",16},
    {"GET /WEB/EPage.shtml", 16},
    {"GET /WEB/SPage.shtml", 16},
    {"GET /WEB/FPage.shtml", 16},
    {"GET /WEB/MPage.shtml",16},
    {"GET /WEB/PageS.shtml",16},
    {"GET / HTTP/1.1",14},

};
//==============================================================================

 //****************************************************************************
 //*
 //* Function name: HttpSocketServer_CheckConnectionTimeout
 //*
 //* Return:  TRUE- For Timeout
 //*          FALSE- Timer is still running.
 //*
 //* Description:
 //*   Modbus TCP sever timer sends keep alive message after connection timer
 //*   Timeout call by ModbusTCPSever_SocketOperation.
 //*
 //****************************************************************************
 //==============================================================================

 static BOOL HttpSocketServer_CheckConnectionTimeout(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruc1ptr,uint8_t Client)
 {

     // Just check if CurrentTick is within Tolerance zone.

     uint32_t currentTick=0;
     uint32_t nextTick=0;
     uint64_t totalTick=0;
     BOOL status=FALSE;

     return status;

     totalTick = pHttpServerStruc1ptr->ClientConnection[Client].startTimeConnectionTimeout + pHttpServerStruc1ptr->ClientConnection[Client].connectionTimeoutTime;

     currentTick=COMTimer_Get10MsecTick();

     if(totalTick >= COM_TICK_MAX)
     {
         if ( currentTick <= pHttpServerStruc1ptr->ClientConnection[Client].startTimeConnectionTimeout)
         {

             // rollover detected
             nextTick = (uint32_t)(totalTick - COM_TICK_MAX);
             if (currentTick >= nextTick )
                 status= TRUE;

         }
     }
     else
     {
         // Check if trigger tick is within tolerance. Tolerance for timer latency (late ISR)
         // Upper limit: tick time + tolerance
         // Lower limit: tick time
         // Current time must be within Upper and Lower

         // Check rollover
         if ( currentTick >= pHttpServerStruc1ptr->ClientConnection[Client].startTimeConnectionTimeout)
         {
             // no rollover
             nextTick = (uint32_t) (pHttpServerStruc1ptr->ClientConnection[Client].startTimeConnectionTimeout +
                     pHttpServerStruc1ptr->ClientConnection[Client].connectionTimeoutTime);
             if (currentTick >= nextTick)
                 status= TRUE;

         }
         else  // if ( upper >= COM_TICK_MAX )
         {
             // rollover detected
             nextTick = (uint32_t) (pHttpServerStruc1ptr->ClientConnection[Client].connectionTimeoutTime -
                                         ( COM_TICK_MAX - pHttpServerStruc1ptr->ClientConnection[Client].startTimeConnectionTimeout));

             if (currentTick >= nextTick)
                 status= TRUE;

         }
     }

     return status;

 }

void HttpServerFileFromSPIFlash(char *path, HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr, uint8_t ClientConn)
{
    FRESULT rc;
    int num_bytes = 0;
    int res = -1;
    res = f_open(&pHttpServerStructptr->ClientConnection[ClientConn].fileHandle, (TCHAR*)path, FA_READ);

    if (res == FR_OK)
    {
    do
     {
         Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
         rc = f_read(&pHttpServerStructptr->ClientConnection[ClientConn].fileHandle, pHttpServerStructptr->ClientConnection[ClientConn].txBuffer,
                     4096, (unsigned *) &num_bytes);

         if ((rc == FR_OK) && (num_bytes > 0))
         {
             send(pHttpServerStructptr->ClientConnection[ClientConn].sockfd,
                  (const unsigned char*)pHttpServerStructptr->ClientConnection[ClientConn].txBuffer,  (size_t)num_bytes, NETCONN_NOCOPY);
         }

     }while ((rc == FR_OK) && (num_bytes > 0));
    }
    f_close(&pHttpServerStructptr->ClientConnection[ClientConn].fileHandle);
}


static uint8_t FindFilePageIndex(char *rxBuffer,HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr, uint8_t ClientConn)
{
    uint8_t  filenum=0, found=0;
     for(filenum=0;filenum<20;filenum++)
     {
        if (strncmp(rxBuffer+4, (char *)pHttpServerStructptr->ClientConnection[ClientConn].httpcmd[filenum].pages+8,  (size_t)pHttpServerStructptr->ClientConnection[ClientConn].httpcmd[filenum].pagecmdsize-5) == 0)
         {
           found=filenum;

           break;
         }
     }
    if(found<19)
    {
        memset(pHttpServerStructptr->ClientConnection[ClientConn].txBuffer, 0x00, HTTP_CONN_SEND_BUF_SIZE);
        HttpServerFileFromSPIFlash((char *)pHttpServerStructptr->ClientConnection[ClientConn].httpcmd[found].pages+4 ,pHttpServerStructptr, ClientConn);
    }
    else if (found==19)
    {
        HttpServerFileFromSPIFlash("/WEB/MPage.shtml",pHttpServerStructptr, ClientConn);
    }
    return found;
}

//****************************************************************************
//*
//* Function name: HttpSocketServer_ProcessRequest
//*
//* Return: None
//*
//* Description:
//*   Process all Modbus request and fill with data as response and send over
//*   socket, call by ModbusTCPSever_SocketOperation.
//*
//****************************************************************************
//==============================================================================

static void HttpSocketServer_ProcessRequest(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr, uint8_t ClientConn)
{

    if (strncmp((char *)pHttpServerStructptr->ClientConnection[ClientConn].rxBuffer, "GET / HTTP/1.1", 14) == 0)
    {
        HttpServerFileFromSPIFlash("/WEB/MPage.shtml",pHttpServerStructptr, ClientConn);
    }
    else
    {
        FindFilePageIndex((char *)pHttpServerStructptr->ClientConnection[ClientConn].rxBuffer,pHttpServerStructptr, ClientConn);
    }
 }
//****************************************************************************
//*
//* Function name: HttpSocketServer_StartConnectionTimer
//*
//* Return: None
//*
//*
//* Description:
//*   Start Modbus TCP server Timer after Connection established and request
//*   received.
//*
//****************************************************************************
//==============================================================================

static void HttpSocketServer_StartConnectionTimer(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr,uint8_t ClientConn)
{
    pHttpServerStructptr->ClientConnection[ClientConn].startTimeConnectionTimeout =  COMTimer_Get10MsecTick();
}


//****************************************************************************
//*
//* Function name: HtppServer_LookForNewRequest
//*
//* Return: None
//*
//* Description:
//*   Recieves new packets from different clients and processes it. Also it sends replies
//*
//****************************************************************************
//==============================================================================

static void HtppServer_LookForNewRequest(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr)
{
    int numbytes;
        fd_set readFD;
        struct timeval timeout;
      //  uint16_t usLength;
        uint8_t i=0;

            timeout.tv_sec  = 0;
        timeout.tv_usec = 5000;

        if(pHttpServerStructptr->httpconn_clientIndex > 0)
        {
            for(i=0;i<WS_MAX_CLIENTS;i++)
            {
                if(pHttpServerStructptr->ClientConnection[i].sockfd >= 0)//pModbusStruct->ConnFD[i].sockfd != -1
                {

                    FD_ZERO(&readFD);

                    FD_SET(pHttpServerStructptr->ClientConnection[i].sockfd, &readFD);

                    if (select((pHttpServerStructptr->ClientConnection[i].sockfd + 1), &readFD, 0, 0, &timeout) > 0)//pModbusStruct->sockfd[i]
                    {

                        if (FD_ISSET(pHttpServerStructptr->ClientConnection[i].sockfd, &readFD)) //pModbusStruct->sockfd[i]
                        {
                            FD_CLR(pHttpServerStructptr->ClientConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                            numbytes = recv(pHttpServerStructptr->ClientConnection[i].sockfd,
                                            &pHttpServerStructptr->ClientConnection[i].rxBuffer[pHttpServerStructptr->ClientConnection[i].rxBufferSize],
                                            (HTTP_CONN_RX_BUF_SIZE - pHttpServerStructptr->ClientConnection[i].rxBufferSize), 0);

                            if (numbytes <= 0)
                            {
                                // ErrnoIndex = errno;
                                 if (errno != EWOULDBLOCK)
                                 {
                                    close(pHttpServerStructptr->ClientConnection[i].sockfd);//pModbusStruct->sockfd[i]
                                    pHttpServerStructptr->ClientConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                                    pHttpServerStructptr->httpconn_clientIndex--;
                                 }
                            }
                            else
                            {
                                  pHttpServerStructptr->HttpConnectDiag.framesRecv = (uint32_t) ((pHttpServerStructptr->HttpConnectDiag.framesRecv + 1) & 0xFFFFFFFF);    // allow rollover
//                                StSPIFlashFSTaskHandle.SPIFlashFSTasks[SPIFSTaskNo].task=READ_TASK_ID;
//                                StSPIFlashFSTaskHandle.SPIFlashFSTasks[SPIFSTaskNo].cmd=STATE_HTTP_READ_REQUEST;
//                                StSPIFlashFSTaskHandle.SPIFlashFSTasks[SPIFSTaskNo].clientconn=i;
                                    HttpSocketServer_ProcessRequest(pHttpServerStructptr, i);
                                    HttpSocketServer_StartConnectionTimer(pHttpServerStructptr,i);
                                    close(pHttpServerStructptr->ClientConnection[i].sockfd); //pModbusStruct->sockfd[i]
                                    pHttpServerStructptr->ClientConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                                    pHttpServerStructptr->httpconn_clientIndex--;


                            }
                        }
                        else
                        {
                            FD_CLR(pHttpServerStructptr->ClientConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                        }
                    }
                    else if((HttpSocketServer_CheckConnectionTimeout(pHttpServerStructptr,i) == TRUE))
                    {
                        close(pHttpServerStructptr->ClientConnection[i].sockfd); //pModbusStruct->sockfd[i]
                        pHttpServerStructptr->ClientConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                        pHttpServerStructptr->httpconn_clientIndex--;

                    }

                }
            }
        }
}

//****************************************************************************
//*
//* Function name: HtppServer_StartConnectionTimer
//*
//* Return: None
//*
//*
//* Description:
//*   Start Modbus TCP server Timer after Connection established and request
//*   recived.
//*
//****************************************************************************

static void HtppServer_StartConnectionTimer(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruc, uint8_t Client)
{
    pHttpServerStruc->ClientConnection[Client].startTimeConnectionTimeout =  COMTimer_Get10MsecTick();
}
//****************************************************************************
//*
//* Function HtppServer_LookForNewConnection
//*
//* Return: None
//*
//* Description:
//*   Accepts New connections
//*
//****************************************************************************
//==============================================================================

static void HtppServer_LookForNewConnection(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStructptr)
{
    int tempSockfd=-1;
    struct sockaddr_in remote;
    uint32_t addr_len = sizeof(struct sockaddr);
    fd_set readFD;
    struct timeval timeout;
    uint8_t i=0;

    timeout.tv_sec  = 0;
    timeout.tv_usec = 1000;

    if(pHttpServerStructptr->httpconn_clientIndex < HTTPSCOKET_CONN_MAX_CLIENTS)
    {

        FD_ZERO(&readFD);
        FD_SET(pHttpServerStructptr->listenSockfd, &readFD);//maxfd

        if ((select(pHttpServerStructptr->listenSockfd + 1, &readFD, 0, 0, &timeout) > 0))
        {
            if (FD_ISSET(pHttpServerStructptr->listenSockfd, &readFD))
            {

                FD_CLR(pHttpServerStructptr->listenSockfd, &readFD);

                tempSockfd = accept(pHttpServerStructptr->listenSockfd, (struct sockaddr *) &remote,
                                (socklen_t *) & addr_len);
                if (tempSockfd < 0)
                {
                    //handle this aituation later
                }
                else
                {
                    for(i=0;i<HTTPSCOKET_CONN_MAX_CLIENTS; i++)
                    {

                        if(pHttpServerStructptr->ClientConnection[i].sockfd < 0)//pModbusStruct->sockfd[i]
                        {

                            //pModbusStruct->ConnFD[i].connFlag=TRUE;
                            pHttpServerStructptr->ClientConnection[i].sockfd = tempSockfd;//pModbusStruct->sockfd[i]
                            pHttpServerStructptr->ClientConnection[i].remote = remote;//pModbusStruct->remote[pModbusStruct->mb_clientIndex]
                            pHttpServerStructptr->ClientConnection[i].rxBufferSize = 0;//pModbusStruct->rxBufferSize = 0;
                            HtppServer_StartConnectionTimer(pHttpServerStructptr,i);
                            pHttpServerStructptr->httpconn_clientIndex++;
                            //pModbusStruct->modbusDiag.clientConnections = pModbusStruct->mb_clientIndex;
                            break;
                        }
                    }
                    if(pHttpServerStructptr->httpconn_clientIndex>= WS_MAX_CLIENTS)
                    {

                        //FD_CLR(pModbusStruct->listenSockfd, &readFD);
                        close(pHttpServerStructptr->listenSockfd);
                        pHttpServerStructptr->listenSockfd = -1;
                       // pModbusStruct->mb_clientIndex--;
                    }

                }
            }
            else
            {
                FD_CLR(pHttpServerStructptr->listenSockfd, &readFD);
            }
        }
    }
}
//****************************************************************************
//*
//* Function name: HtppServer_netconn_CreateSocket
//*
//* Return: None
//*
//* Description:
//*   Create  Listen Socket
//*
//****************************************************************************

static uint8_t HtppServer_netconn_CreateSocket(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStrucptr)
{
    int tempSockfd=-1;
    int option=1,rc=0;

    struct sockaddr_in local;
    uint32_t addr_len = sizeof(struct sockaddr);



    if((networkConfig.CurrentState != STATE_LWIP_WORKING) && (pHttpServerStrucptr->listenSockfd < 0))
    {
        return 0;
    }
   // sys_thread_new("HTTP", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);

    if((pHttpServerStrucptr->listenSockfd < 0))
    {
        local.sin_port = htons(HTTPSCOKET_CONN_PORT);
        local.sin_family = PF_INET;
        local.sin_addr.s_addr = INADDR_ANY;



        tempSockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (tempSockfd < 0)
        {
            return 0;
        }

        /*************************************************************/
        /* Allow socket descriptor to be reuseable                   */
        /*************************************************************/
        rc = setsockopt(tempSockfd, SOL_SOCKET, SO_REUSEADDR,
                        (char *)&option, sizeof(option));
        if (rc < 0)
        {
           close(tempSockfd);
           return 0;
        }

        /*************************************************************/
        /* Set socket to be nonblocking. All of the sockets for      */
        /* the incoming connections will also be nonblocking since   */
        /* they will inherit that state from the listening socket.   */
        /*************************************************************/
        rc = ioctl(tempSockfd, FIONBIO, (char *)&option);
        if (rc < 0)
        {
           close(tempSockfd);
           return 0;
        }


        /*             21      */
        if (bind(tempSockfd, (struct sockaddr *) &local, addr_len) < 0)
        {
            close(tempSockfd);
            return 0;
        }
        if (listen(tempSockfd, 1) < 0)//MBTCP_MAX_CONNECTION
        {
            close(tempSockfd);
            return 0;
        }

        pHttpServerStrucptr->listenSockfd = tempSockfd;
        //pModbusStruct->maxfd = tempSockfd;
    }

    return 1;
}

//==============================================================================

//****************************************************************************
//*
//* Function name: WebSocketServer_SocketOperation
//*
//* Return: None
//*
//* Description:
//*   Initializing Socket as server, check binding, accept the socket and send
//*   data over the socket, call by ModbusTCPSever_Task.
//*
//****************************************************************************
//==============================================================================

void HtppServer_netconn_thread_SocketOperation(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruc1ptr)
{

    if(HtppServer_netconn_CreateSocket(pHttpServerStruc1ptr))
    {
        HtppServer_LookForNewConnection(pHttpServerStruc1ptr);

        HtppServer_LookForNewRequest(pHttpServerStruc1ptr);

        pHttpServerStruc1ptr->HttpConnectDiag.clientConnections = pHttpServerStruc1ptr->httpconn_clientIndex;

    }
}

void HttpConnServer_Init(HTTPCONNSOCKET_RAM_STRUCT *pHttpServerStruct)
{
    uint8_t count = 0;

    for (count = 0; count <3; count++)
    {
        pHttpServerStruct->ClientConnection[count].sockfd = -1;
        pHttpServerStruct->ClientConnection[count].startTimeConnectionTimeout=0;
        pHttpServerStruct->ClientConnection[count].connectionTimeoutTime =1000;
          //      Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout;

        pHttpServerStruct->ClientConnection[count].txRemainingSize=0;
        pHttpServerStruct->ClientConnection[count].rxBufferSize=0;
        pHttpServerStruct->ClientConnection[count].httpcmd = httpconnGETcommndList;
        memset(&pHttpServerStruct->ClientConnection[count].remote,0,sizeof(struct sockaddr_in));
        memset(pHttpServerStruct->ClientConnection[count].txBuffer,0,HTTPS_CONN_MSG_BUF_SIZE);
        memset(pHttpServerStruct->ClientConnection[count].rxBuffer,0,HTTPS_CONN_MSG_BUF_SIZE);

       // pHttpServerStruct->ClientConnection[count].webspiflashFileSystemStruct.
    }

    pHttpServerStruct->listenSockfd = -1;


    pHttpServerStruct->httpconn_clientIndex = 0;

   //Check MOdbus TCP is enable or not
    //pHttpServerStruct->modbusTCPEnable =
     //       Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_Enable.bits.modBusTCPEnable;

    //clear All diagnostic Parameteres on initialization
    memset(&pHttpServerStruct->HttpConnectDiag, 0x00, sizeof(HTTPCONN_SOCKET_DIAG_COUNTER));
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////
//**************************************************************************
/*
  * Function name: WS_Getkey
 *
 * Return: char
 *
 * Description:
 */

//**************************************************************************
static char* WS_Getkey( char *buf, size_t *len )
{
    char *p = strstr(buf, "Sec-WebSocket-Key: ");
    if (p)
    {
        p = p + strlen("Sec-WebSocket-Key: ");
        *len = strchr(p, '\r') - p;
    }
    return p;
}

//==========================================================================
//**************************************************************************
/*
* Function name: create_ws_key_accept
*
* Return: char
*
* Description:
*/
//**************************************************************************
//==========================================================================
static void  WS_CreateKeyAccept( char *inbuf , char *hash_base64)
{
   static char concat_key[64] = {0};
   static char hash[22] = {0};//, hash_base64[64] = {0};
   size_t len = 0, baselen = 0;

   memset(concat_key, 0x00, 64);
   memset(hash, 0x00, 22);
   memset(hash_base64, 0x00, 64);

   char *key = WS_Getkey(inbuf, &len);
   strncpy(concat_key, key, len);
   strcat(concat_key, WS_GUID);
   mbedtls_sha1((uint8_t*)concat_key, 60, (uint8_t*)hash);
   mbedtls_base64_encode((uint8_t*)hash_base64, 64, &baselen, (uint8_t*)hash, 20);

 //  return hash_base64;
}

//==========================================================================
//**************************************************************************
/*
* Function name: ws_set_size_to_frame
*
* Return: uint8_t
*
* Description:
*/
//**************************************************************************
//==========================================================================
static uint8_t* ws_set_size_to_frame( uint32_t size, uint8_t *out_frame )
{
   uint8_t *out_frame_ptr = out_frame;
   if (size < 126)
   {
       *out_frame_ptr = size;
       out_frame_ptr++;
   }
   else
   {
       out_frame_ptr[0] = 126;
       out_frame_ptr[1] = ( ( ((uint16_t)size) >> 8) & 0xFF );
       out_frame_ptr[2] = ( ( (uint16_t)size) & 0xFF );
       out_frame_ptr += 3;
   }
   return out_frame_ptr;
}
//==========================================================================
//**************************************************************************
/*
* Function name: ws_set_data_to_frame
*
* Return: uint8_t
*
* Description:
*/
//**************************************************************************
//==========================================================================
static uint8_t* ws_set_data_to_frame( uint8_t *data, uint16_t size, uint8_t *out_frame )
{
   memcpy(out_frame, data, size);
   return (out_frame+size);
}

//==============================================================================
//****************************************************************************
//* Function name: differenceInDays
//*
//* Return:  void
//*
//* Description: datalogs
//*
//****************************************************************************
//==============================================================================
//  leap year
uint16_t isLeapYear(uint16_t year)
{
   return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

uint8_t WS_GetDays(Time from, Time to)
{
   uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   uint8_t retDays=0;


   if((from.Month == 0) || (from.Month > 12) ||
      (to.Month == 0) || (to.Month > 12))
   {
       return retDays;
   }

   if (isLeapYear(from.Year))
   {
       daysInMonth[1] = 29;
   }

   if((from.Year == to.Year))
   {
       if((from.Month == to.Month))
       {
           if(from.Date == to.Date)
           {
               retDays=1;
           }
           else if(from.Date < to.Date)
           {
               // Valid Case
               // Same Year Same Month Adv To Date
               retDays=to.Date-from.Date + 1;
           }

           else
           {
               // Invalid Case
               // Same Year Same Month small or equal To Date
           }
       }
       else if((from.Month + 1) == to.Month)
       {
           // Valid Case
           // Same Year Adv To Month
           retDays=(daysInMonth[from.Month -1] - from.Date) + to.Date + 1;
       }
       else
       {
           // Invalid Case
           // Same Year small To Month
       }
   }
   else if((from.Year < to.Year))
   {
       // Valid Case
       // Adv Year
       if((from.Month == 12) && (to.Month == 1))
       {
           // Valid Case
           // Adv Year small To Month
           retDays=(daysInMonth[from.Month -1] - from.Date) + to.Date + 1;
       }
       else
       {
           // Invalid Case
           // More than a month or Year
       }
   }
   else
   {
       // Invalid Case
       // Small To Year
   }

   // Calculate the difference in days
   return retDays;
}

//==============================================================================
//****************************************************************************
//* Function name: differenceInDays
//*
//* Return:  void
//*
//* Description: datalogs
//*
//****************************************************************************
//==============================================================================
//  leap year

Time WS_GetNextDate(Time current, uint8_t daysToAdd)
{
   uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
   Time retDate={0};

   if((daysToAdd <= 62) &&
       (current.Month >= 1 ) &&
       (current.Month <= 12 ) &&
       (current.Date >= 1 ) &&
       (current.Date <= 31 ))
   {
       retDate.Year=current.Year;
       retDate.Month = current.Month;
       retDate.Date = current.Date;

       if((current.Date +  daysToAdd) <= daysInMonth[current.Month+1])
       {
           retDate.Date=current.Date + daysToAdd;
       }
       else
       {
           if(current.Month < 12)
           {
               retDate.Month = current.Month + 1;
               retDate.Date = 1;
           }
           else
           {
               retDate.Year=current.Year + 1;
               retDate.Month = 1;
               retDate.Date = 1;
           }
       }
   }

   return retDate;
}

///////////////////////////////////////////////////////////////////////////

 static void WS_ReadAllDataHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     REG_uint32_t IPAddress;
     uint8_t i=0;
     IPAddress.all = 0;

    pWebsocket->txBuffer[pWebsocket->txIndex++]=(UPSDb.protocolID& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++]=(uint8_t)((UPSDb.protocolID >> 8) & 0xFF);

    for (i = 0;  i < 26; ++i)
    {
        pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.UPSManufacturer[i];
    }
    for (i = 0;  i < 16; ++i)
    {
        pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.UPSModel[i];
    }
    for (i = 0;  i < 15; ++i)
    {
        pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.UPSCPUFwVersion[i];
    }

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.RatedOutputVA & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOutputVA  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOutputVA  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOutputVA  >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.RatedOuputVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOuputVolt  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.RatedOuputCurr & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOuputCurr  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.RatedOuputFreq & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedOuputFreq  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.RatedBattVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.RatedBattVolt  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.supportedPhase.Bytes.InputPhase);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.supportedPhase.Bytes.OutputPhase);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.NominalInputVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.NominalInputVolt  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.NominalOutputVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.NominalOutputVolt  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BatteryTestType & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BatteryTestType  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattVolt  >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattPieceNumber & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattPieceNumber   >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattVoltPerUnit & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattVoltPerUnit   >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattgroupNumber & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattgroupNumber   >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattCapacity & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattCapacity  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BattRemainTime & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BattRemainTime    >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.PBusVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.PBusVolt  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.NBusVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.NBusVolt  >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.PBattVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.PBattVolt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.NBattVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.NBattVolt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BatNumberInParallel & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BatNumberInParallel   >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.BatNumberInSeries & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.BatNumberInSeries >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.UPSMode[0]);// & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;//((UPSDb.UPSMode[1]));//   >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.type;

    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.subType;
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.VAType;
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.H_LV_Type;
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.year[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.year[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.month[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.month[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.ManufacturerID;

    for (i = 0;  i < 5; ++i)
    {
        pWebsocket->txBuffer[pWebsocket->txIndex++]=UPSDb.upsID.serialNumber[i];
    }
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.InputVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.InputVolt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.InputFreq & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.InputFreq >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.OutputVolt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.OutputVolt >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.OutputFreq & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.OutputFreq >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.OutputCurr & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.OutputCurr >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.OutputPWRFactor & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.OutputPWRFactor >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.OutputLoadPercent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.OutputLoadPercent >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.bypassFreq & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.bypassFreq >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.MaxTemp & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.MaxTemp >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.bypassVoltHighLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.bypassVoltHighLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.bypassVoltLowLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.bypassVoltLowLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.voltHighLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.voltHighLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.voltLowLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.voltLowLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.freqHighLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.freqHighLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.freqLowLossPt & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.freqLowLossPt >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsStatus.statusAll & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsStatus.statusAll >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.warningFlags.warning16_bit[0] & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[0] >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[1]) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[1] >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.warningFlags.warning16_bit[2] & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[2] >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[3]) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.warningFlags.warning16_bit[3] >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.FaultType & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.FaultType >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.InputVoltBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.InputVoltBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.InputFreqBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.InputFreqBeforeFault >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.OutputVoltBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.OutputVoltBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.OutputFreqBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.OutputFreqBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.OutputLoadBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.OutputLoadBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.OutputCurrentBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.OutputCurrentBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.PBusVoltageBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.PBusVoltageBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.NBusVoltBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.NBusVoltBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.BattVoltBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.BattVoltBeforeFault >> 8) & 0xFF);


    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.TempBeforeFault & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.TempBeforeFault >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSDb.upsFaultStruct.UPSRunningStatus.runningStatusAll >> 8) & 0xFF);

//UPS Interface Serial Diagnosis
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.charRecv & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charRecv  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charRecv  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charRecv >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.charSent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charSent  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charSent  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.charSent >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.framesRecv & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.framesRecv  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.framesRecv  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.framesRecv >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.frameSent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.frameSent  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.frameSent  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.frameSent >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.DumpChars & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.DumpChars  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.DumpChars  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.DumpChars >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.goodTransaction & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.goodTransaction  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.goodTransaction  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.goodTransaction >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.badTransaction & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.badTransaction  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.badTransaction  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.badTransaction >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.ReqTimedout & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.ReqTimedout  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.ReqTimedout  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.ReqTimedout >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.cntrlCmdPassCntr & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdPassCntr  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdPassCntr  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdPassCntr >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (UPSInterfaceStruct.uartDiag.cntrlCmdFailCntr & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdFailCntr  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdFailCntr  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((UPSInterfaceStruct.uartDiag.cntrlCmdFailCntr >> 24) & 0xFF);

   //GSM Diagnisis


   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.charRecv & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charRecv >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charRecv >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charRecv >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.charSent & 0xFF);            //243
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charSent >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charSent >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.charSent >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.framesRecv & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.framesRecv >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.framesRecv >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.framesRecv >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.frameSent & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.frameSent >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.frameSent >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.frameSent >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.NumberofSMSsent & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSsent >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSsent >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSsent >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.NumberofSMSNotsent & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSNotsent >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSNotsent >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.NumberofSMSNotsent >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.ModemState & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemState >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemState >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemState >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.ModemConnected & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemConnected >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemConnected >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.ModemConnected >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.TransctionmPassCnt & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransctionmPassCnt >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransctionmPassCnt >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransctionmPassCnt >> 24) & 0xFF);

   pWebsocket->txBuffer[pWebsocket->txIndex++] = (GsmStruct.GsmDiag.TransactionFailedCnt & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransactionFailedCnt >> 8) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransactionFailedCnt >> 16) & 0xFF);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = ((GsmStruct.GsmDiag.TransactionFailedCnt >> 24) & 0xFF);

   ///////////////Modbus TCP Diag

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.framesRecv & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.framesRecv  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.framesRecv  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.framesRecv >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.frameSent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.frameSent  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.frameSent  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.frameSent >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.goodTransaction & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodTransaction  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodTransaction  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodTransaction >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.goodException & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodException  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodException  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.goodException >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.clientConnections & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.clientConnections  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.clientConnections  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.clientConnections >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (modbusTCPStruct.modbusDiag.otherError & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.otherError  >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.otherError  >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((modbusTCPStruct.modbusDiag.otherError >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Year;//303
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Year;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Year;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultCode >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultAddress& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultAddress >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultAddress >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[0].faultAddress >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultCode >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultAddress& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultAddress >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultAddress >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[1].faultAddress >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultCode >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultAddress& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultAddress >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultAddress >> 16) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.exceptionStructArr[2].faultAddress >> 24) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Year;//303
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Year;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Date;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Month;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Year;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Hours;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Minutes;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].timeStamp.Seconds;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[0].faultCode >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[1].faultCode >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].OSVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].OSVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].BootVersion & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].BootVersion >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ( Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].faultCode& 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((Device_Configurations.EepromMapStruct.faultRecordStruct.hardFaultArr[2].faultCode >> 8) & 0xFF);


    //Client Number and Client IP and
    pWebsocket->txBuffer[pWebsocket->txIndex++] = WebSocketStruct.ws_clientIndex;

    IPAddress.all = ntohl(WebSocketStruct.ClientConnection[0].remote.sin_addr.s_addr);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[2];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[3];

    IPAddress.all = ntohl(WebSocketStruct.ClientConnection[1].remote.sin_addr.s_addr);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[2];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[3];

    IPAddress.all = ntohl(WebSocketStruct.ClientConnection[2].remote.sin_addr.s_addr);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[2];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddress.byte[3];

    //=================================//Boot_OS_Control_ver Group//============================
    pWebsocket->txBuffer[pWebsocket->txIndex++] =  (Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] =  ((Device_Configurations.EepromMapStruct.Save_SystemInfo.OS_Version >> 8) & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] =  Device_Configurations.EepromMapStruct.Save_SystemInfo.BOOTVersion;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.EmailSent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.EmailSent >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.EmailFailed & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.EmailFailed >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = modbusTCPStruct.mb_clientIndex;

    pWebsocket->txBuffer[pWebsocket->txIndex++]  = UPSDb.upsStatus.status_8bit[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++]  = UPSDb.upsStatus.status_8bit[1];

    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;//CRC

 }

 static void WS_ReadDataLogHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint16_t itr=0;
     Time logDate= {0};
     uint16_t index_byte=0,TotalLogs=0,LogCntIndex = 0;
     uint8_t noOfLogs = 0;
     LOG_HOLD_STRUCT LogData = {0};
     logDate.Date = pWebsocket->payload[10];
     logDate.Month = pWebsocket->payload[11];
     logDate.Year = pWebsocket->payload[12];
     TotalLogs = SPIFlash_GetRecordSize(1,&logDate);

     pWebsocket->txBuffer[pWebsocket->txIndex++]= 0x01;// Format  ---16
     pWebsocket->txBuffer[pWebsocket->txIndex++]= 0x00;// Res    --17
     pWebsocket->txBuffer[pWebsocket->txIndex++]= (uint8_t)(TotalLogs & 0xFF);// Total log Count Byte 1
     pWebsocket->txBuffer[pWebsocket->txIndex++]= (uint8_t)((TotalLogs >> 8) & 0xFF);// Total log Count Byte 2

     pWebsocket->txBuffer[pWebsocket->txIndex++]= pWebsocket->payload[13];// Index Byte 1
     pWebsocket->txBuffer[pWebsocket->txIndex++]= pWebsocket->payload[14];// Index Byte 2 --21
     LogCntIndex = pWebsocket->txIndex++;                            //--22
     pWebsocket->txBuffer[pWebsocket->txIndex++]= 0x00;// Res        //--23 day

     //msb << 8 | lsb & ff;
     index_byte = MAKE16BIT(pWebsocket->payload[13],pWebsocket->payload[14]);//SWAP_BYTES_IN_WORD(pWebsocket->payload[13]);

     noOfLogs = 0;
     for (itr = index_byte; itr < TotalLogs; itr++)
     {
         if((pWebsocket->txIndex + sizeof(LogData)) < WS_SEND_BUFFER_SIZE)
         {
             Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
             SPIFlash_ReadData(&logDate, itr , &LogData);
             memcpy(&pWebsocket->txBuffer[pWebsocket->txIndex],&LogData,sizeof(LogData)); //--24
             pWebsocket->txIndex += sizeof(LogData);
             noOfLogs++;
         }
     }
     pWebsocket->txBuffer[LogCntIndex] = noOfLogs;
 }

 static void WS_ReadNoOfDataLogHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint16_t itr1=0;
     uint8_t totalDaysOfLog = 0;
     Time logDate ={0},From ={0},To ={0};
     int16_t fullrecord = 0;

     From.Date = pWebsocket->rxBuffer[16];
     From.Month = pWebsocket->rxBuffer[17];
     From.Year = pWebsocket->rxBuffer[18];
     To.Date = pWebsocket->rxBuffer[19];
     To.Month = pWebsocket->rxBuffer[20];
     To.Year = pWebsocket->rxBuffer[21];
     totalDaysOfLog = WS_GetDays(From, To);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = From.Date;//16
     pWebsocket->txBuffer[pWebsocket->txIndex++] = From.Month;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = From.Year;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = To.Date;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = To.Month;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = To.Year;//21
     pWebsocket->txBuffer[pWebsocket->txIndex++] = totalDaysOfLog;//22

     if(totalDaysOfLog > 0)
     {
         for (itr1 = 0; itr1 < totalDaysOfLog; itr1++)
         {
             fullrecord=0;
             logDate = WS_GetNextDate(From,itr1);
             fullrecord =SPIFlash_GetRecordSize(1,&logDate);// fullrecord = SD_GetRecordSize(1,&logDate);
             pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(fullrecord & 0xFF);
             pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((fullrecord >> 8) & 0xFF);
         }
     }
 }

  static void WS_ReadAllSettingHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     REG_uint32_t IPAddr;
     REG_uint32_t GateAddr;
     REG_uint32_t SubAddr;
     REG_uint32_t DNSPRI;
     REG_uint32_t DNSSEC;
     uint8_t i,j, itr;
     REG_uint32_t datalogInterval,SMTPInterval;
     IPAddr.all = lwIPLocalIPAddrGet();
     GateAddr.all = lwIPLocalGWAddrGet();
     SubAddr.all = lwIPLocalNetMaskGet();
     DNSPRI.all = EthernetTask_GetDNSServerName(0);
     DNSSEC.all = EthernetTask_GetDNSServerName(1);

     for ( i = 0;  i < 16; ++i)
     {
         pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName[i];
     }

     if(EthernetTask_CheckDHCPEnable())
           pWebsocket->txBuffer[pWebsocket->txIndex++] = 1;
     else
          pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;

     pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddr.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddr.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddr.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = IPAddr.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = GateAddr.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = GateAddr.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = GateAddr.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = GateAddr.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = SubAddr.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SubAddr.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SubAddr.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SubAddr.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSPRI.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSPRI.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSPRI.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSPRI.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSSEC.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSSEC.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSSEC.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = DNSSEC.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[3];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[4];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = networkConfig.MAC_ADDRESS[5];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[3];

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo >> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout >> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port >> 8) & 0xFF);

     for(itr=0;itr<64;itr++)
     {
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[itr].trapsetting;
      }

     pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;//HTTPRequest_UPS_Get_BaudRate(0);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = 1;//HTTPRequest_UPS_Get_DataBits(0);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;//HTTPRequest_UPS_Get_StopBits(0);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;//HTTPRequest_UPS_Get_Parity(0);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;

    pWebsocket->txBuffer[pWebsocket->txIndex++] =0;
    pWebsocket->txBuffer[pWebsocket->txIndex++] =0;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;


     pWebsocket->txBuffer[pWebsocket->txIndex++] = HTTPRequest_UPS_Get_BaudRate(PORT_UART2);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = HTTPRequest_UPS_Get_DataBits(PORT_UART2);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = HTTPRequest_UPS_Get_StopBits(PORT_UART2);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = HTTPRequest_UPS_Get_Parity(PORT_UART2);


     for ( i = 0; i < 5; i++)
     {
         for ( j = 0; j < 8; j++)
         {
             pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[i].byte[j];
         }
     }
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(( Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS>> 8) & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.audibleAlarm;//a
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning;//b
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.codeStart;//c
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck;//d
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;//e
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassForbiding;//f
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.energySaving;//g
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.shortRestart3Times;//h
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn;//i
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode;//j
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode;//k
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect;//l
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SetHotStandby;//m
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.deepHighEfficiencyMode;//n
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff;//o
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning;//p
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.ConstantPhaseAngleFn;//q
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.autoRestart;//r
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect;//s
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battLowProtect;//t
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.FreeRunFunction;//u
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.converterMode;//v
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.limitedRuntimeOnBatt;//w
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.outputParallelFn;//x
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.phaseAutoAdapt;//y
     pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.periodSelfTest;//z

     datalogInterval.all = Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.all/100;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (datalogInterval.byte[3]);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (datalogInterval.byte[2]);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (datalogInterval.byte[1]);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (datalogInterval.byte[0]);

    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom, 64);
    pWebsocket->txIndex += 64;

    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add, 64);
    pWebsocket->txIndex += 64;
    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add, 64);
    pWebsocket->txIndex += 64;
    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add, 64);
    pWebsocket->txIndex += 64;

    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword, 64);
    pWebsocket->txIndex += 64;

    MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost, 64);
    pWebsocket->txIndex += 64;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.EmailConfig.SSLEnable;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent >> 8) & 0xFF);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled;
    SMTPInterval.all = Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[2];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[3];

    pWebsocket->txBuffer[pWebsocket->txIndex++] = 0;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroyafterhourSet;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroySecondsSet;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable;
    for(i=0;i<32;i++)
    {
        pWebsocket->txBuffer[pWebsocket->txIndex++] =Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer[i];
    }

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_HH;       //205
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_MM;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_SS;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_HH;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_MM;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_SS;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS; //213

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByBattLow;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByTime; //213
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByTime |Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByBattLow) ; //213

 }

 static void WS_WriteEthernetHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
         BOOL DHCP_Enable = FALSE;
         uint32_t IP=0, Gateway=0, Mask=0;
         REG_uint32_t bIP, bGateway, bMask, bDNS1, bDNS2;
         uint8_t i = 0;
         char* HostName[16] = {0};
         MEMCPY(HostName,&pWebsocket->rxBuffer[16],16);

         EthernetTask_SetHostName((char*)HostName);

         DHCP_Enable = pWebsocket->rxBuffer[32];

         bIP.all =0;
         bGateway.all =0;
         bMask.all =0;

         for (i = 0; i < 4; ++i)
         {
             bIP.byte[i] = pWebsocket->rxBuffer[33+i];
             bMask.byte[i] = pWebsocket->rxBuffer[37+i];
             bGateway.byte[i] = pWebsocket->rxBuffer[41+i];
         }

         IP = bIP.all;
         Gateway = bGateway.all;
         Mask = bMask.all;

         if(EthernetTask_ValidateNetworkSettings(IP, Mask, Gateway, DHCP_Enable, FALSE))
         {
             EthernetTask_SetNetworkSettings(IP, Mask, Gateway, DHCP_Enable);
         }

         for (i = 0;  i < 16; ++i)
         {
             pWebsocket->txBuffer[pWebsocket->txIndex++] =  Device_Configurations.EepromMapStruct.Save_EthernetSettings.CONFIG_HostName[i];
         }

         pWebsocket->txBuffer[pWebsocket->txIndex++] = DHCP_Enable;

         bIP.all = lwIPLocalIPAddrGet();
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bIP.byte[0];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bIP.byte[1];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bIP.byte[2];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bIP.byte[3];

         bGateway.all = lwIPLocalGWAddrGet();
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bGateway.byte[0];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bGateway.byte[1];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bGateway.byte[2];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bGateway.byte[3];

         bMask.all = lwIPLocalNetMaskGet();
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bMask.byte[0];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bMask.byte[1];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bMask.byte[2];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bMask.byte[3];

         bDNS1.all = EthernetTask_GetDNSServerName(0);
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS1.byte[0];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS1.byte[1];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS1.byte[2];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS1.byte[3];

         bDNS2.all = EthernetTask_GetDNSServerName(1);
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS2.byte[0];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS2.byte[1];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS2.byte[2];
         pWebsocket->txBuffer[pWebsocket->txIndex++] = bDNS2.byte[3];
 }

 static void  WS_WriteLOGIntervalSettingHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
      REG_uint32_t TempLogint;
     uint16_t CRC_Cal = 0;
     uint8_t *Buffer;
     Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SerialSettings.TransBuffer);
//     Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.byte[0]=pWebsocket->rxBuffer[16];
//     Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.byte[1]=pWebsocket->rxBuffer[17];
//     Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.byte[2]=pWebsocket->rxBuffer[18];
//     Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.byte[3]=pWebsocket->rxBuffer[19];

     TempLogint.byte[0]=pWebsocket->rxBuffer[16];
     TempLogint.byte[1]=pWebsocket->rxBuffer[17];
     TempLogint.byte[2]=pWebsocket->rxBuffer[18];
     TempLogint.byte[3]=pWebsocket->rxBuffer[19];
     Device_Configurations.EepromMapStruct.Save_SerialSettings.DataLogInterval.all= TempLogint.all*100;
    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, SERIAL_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.Save_SerialSettings.CRC_Check_SerialSett = CRC_Cal;

   OnChipEEPROM_Write(SERIAL_CONFIG_SAVE, (uint8_t*)Buffer, SERIAL_CONFIG_SIZE);
   pWebsocket->txBuffer[pWebsocket->txIndex++] = TempLogint.byte[0];
   pWebsocket->txBuffer[pWebsocket->txIndex++] = TempLogint.byte[1];
   pWebsocket->txBuffer[pWebsocket->txIndex++] = TempLogint.byte[2];
   pWebsocket->txBuffer[pWebsocket->txIndex++] = TempLogint.byte[3];


 }

 static void WS_WriteDateTimeSettingHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint8_t itr =0;
     Time systime;
     systime.Date=pWebsocket->rxBuffer[16];
     systime.Month=pWebsocket->rxBuffer[17];
     systime.Year=pWebsocket->rxBuffer[18];
     systime.Hours=pWebsocket->rxBuffer[19];
     systime.Minutes=pWebsocket->rxBuffer[20];
     systime.Seconds=pWebsocket->rxBuffer[21];
     WriteNewDateTimeToRTC(systime);
     for (itr = 0; itr < 6; ++itr)
     {
        pWebsocket->txBuffer[pWebsocket->txIndex++] = pWebsocket->rxBuffer[itr+16];
     }
 }

 static void WS_EmailTestHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint16_t Ret=0;

     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom,&pWebsocket->rxBuffer[18],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add,&pWebsocket->rxBuffer[82],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add,&pWebsocket->rxBuffer[146],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add,&pWebsocket->rxBuffer[210],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword,&pWebsocket->rxBuffer[274],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost,&pWebsocket->rxBuffer[338],64);
     Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort = MAKE16BIT(pWebsocket->rxBuffer[402],pWebsocket->rxBuffer[403]);
     Device_Configurations.EepromMapStruct.EmailConfig.SSLEnable = pWebsocket->rxBuffer[404];

     sprintf((char *)&SMTPConfig.Subject[0], "Fuji Test Email (Sr No.%s)\n", UPSDb.upsID.serialNumber);

     sprintf((char *)SMTPConfig.EmailMSG, "Test Email\n%02d-%02d-20%02d %02d:%02d:%02d",
                           GlobalDateTime.Date, GlobalDateTime.Month,GlobalDateTime.Year,
                           GlobalDateTime.Hours, GlobalDateTime.Minutes, GlobalDateTime.Seconds);

     Ret = (uint16_t)SMTP_ConfigureAndSendEmail();

     memset((uint8_t *)SMTPConfig.Subject, 0x00, MAX_LEN);
     memset((uint8_t *)SMTPConfig.EmailMSG, 0x00, MAX_MSG_LEN);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.SMTP_result & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.SMTP_result >> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.SMTP_srv_err & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.SMTP_srv_err >> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.SMTP_err & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.SMTP_err >> 8) & 0xFF);
 }
 static void WriteEmailConfigDataInOnChipEpprom()
 {
      uint16_t  CRC_Cal=0;
      uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.EmailConfig.TransBuffer);
      CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, EMAIL_CONFIG_SIZE-2);
      Device_Configurations.EepromMapStruct.EmailConfig.CRC_EmailSett= CRC_Cal;
      OnChipEEPROM_Write(EMAIL_CONFIG_SAVE, (uint8_t*)Buffer, EMAIL_CONFIG_SIZE);
 }


 static void WS_EmailConfigSaveHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     REG_uint32_t SMTPInterval;
     SMTPConfig.EmailFailed = 0;
     SMTPConfig.EmailSent = 0;

     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom,&pWebsocket->rxBuffer[18],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add,&pWebsocket->rxBuffer[82],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add,&pWebsocket->rxBuffer[146],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add,&pWebsocket->rxBuffer[210],64);

     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword,&pWebsocket->rxBuffer[274],64);
     MEMCPY(Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost,&pWebsocket->rxBuffer[338],64);

     Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort = MAKE16BIT(pWebsocket->rxBuffer[402],pWebsocket->rxBuffer[403]);
     Device_Configurations.EepromMapStruct.EmailConfig.SSLEnable = pWebsocket->rxBuffer[404];

     Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent = MAKE16BIT(pWebsocket->rxBuffer[405],pWebsocket->rxBuffer[406]);

     Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval= (MAKE32BIT(pWebsocket->rxBuffer[407],pWebsocket->rxBuffer[408],pWebsocket->rxBuffer[409],pWebsocket->rxBuffer[410]));

     WriteEmailConfigDataInOnChipEpprom();

     SMTPConfig.IntervalTime = Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval*10;

     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailFrom, 64);
     pWebsocket->txIndex += 64;

     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[0].EmailTo_Add, 64);
     pWebsocket->txIndex += 64;
     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[1].EmailTo_Add, 64);
     pWebsocket->txIndex += 64;
     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailTo[2].EmailTo_Add, 64);
     pWebsocket->txIndex += 64;

     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.EmailPassword, 64);
     pWebsocket->txIndex += 64;

     MEMCPY(&pWebsocket->txBuffer[pWebsocket->txIndex], Device_Configurations.EepromMapStruct.EmailConfig.SMTPHost, 64);
     pWebsocket->txIndex += 64;

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.EmailConfig.SmtpPort >> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.EmailConfig.SSLEnable;

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.EmailConfig.EmailEvent >> 8) & 0xFF);

     SMTPInterval.all = Device_Configurations.EepromMapStruct.EmailConfig.SmtpMailInterval;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[0];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[1];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[2];
     pWebsocket->txBuffer[pWebsocket->txIndex++] = SMTPInterval.byte[3];
 }

 static void WS_WriteSNMPTrapsHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint8_t itr =0;
     uint16_t  CRC_Cal=0;
     uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TransBuffer);
     for (itr = 0; itr < 64; ++itr)
     {
          Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[itr].trapsetting = pWebsocket->rxBuffer[itr+16];
     }
     for (itr = 0; itr < 64; ++itr)
     {
          pWebsocket->txBuffer[pWebsocket->txIndex++] =Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.UpsSnmpTrapStruct[itr].trapsetting;
     }

   CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE-2);
   Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.CRC = CRC_Cal;
   OnChipEEPROM_Write(SNMPTrap_CONFIG_SAVE, (uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE);
 }

 static void WS_WriteMODbusHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
  {
     uint16_t  CRC_Cal=0;
     uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.TransBuffer);

     Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout = MAKE16BIT(pWebsocket->rxBuffer[16], pWebsocket->rxBuffer[17]);

     Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port = MAKE16BIT(pWebsocket->rxBuffer[18], pWebsocket->rxBuffer[19]);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(( Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout>> 8) & 0xFF);

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(( Device_Configurations.EepromMapStruct.Save_MBTCPSettings.Port>> 8) & 0xFF);

     CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, MBTCP_CONFIG_SIZE-2);
     Device_Configurations.EepromMapStruct.Save_MBTCPSettings.CRC = CRC_Cal;
     OnChipEEPROM_Write(MBTCP_CONFIG_SAVE, (uint8_t*)Buffer, MBTCP_CONFIG_SIZE);

     ModbusTCPSever_UpdateConnectionTimeout(Device_Configurations.EepromMapStruct.Save_MBTCPSettings.SocketTimeout);

  }
 static void WS_WriteSNMPHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
    uint8_t itr =0;
    uint16_t  CRC_Cal=0;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TransBuffer);

    for (itr = 0; itr < 4; ++itr)
    {
        Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[itr] = pWebsocket->rxBuffer[itr+16];
    }
    Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo = MAKE16BIT(pWebsocket->rxBuffer[20], pWebsocket->rxBuffer[21]);

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[0];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[1];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[2];
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TRAPIPAddress.byte[3];

    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)(Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo & 0xFF);
    pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.TrapPortNo >> 8) & 0xFF);

    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE-2);
    Device_Configurations.EepromMapStruct.Save_SNMPTrapSettings.CRC = CRC_Cal;
    OnChipEEPROM_Write(SNMPTrap_CONFIG_SAVE, (uint8_t*)Buffer, SNMPTrap_CONFIG_SIZE);


 }

 static void WS_WriteCOM1Handler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {

     uint8_t Comvalue = 0;

     Comvalue = pWebsocket->rxBuffer[16];
     if(Comvalue <= TYPES_BAUDRATE)
     {
         HTTPResponse_UPS_Put_BaudRate(PORT_UART1, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[17];
     if(Comvalue <= TYPES_DATABITS)
     {
         HTTPResponse_UPS_Put_DataBits(PORT_UART1, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[18];
     if(Comvalue <= TYPES_STOPBITS)
     {
         HTTPResponse_UPS_Put_StopBits(PORT_UART1, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[19];
     if(Comvalue <= TYPES_PARITY)
     {
         HTTPResponse_UPS_Put_Parity(PORT_UART1, Comvalue);
     }

     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_BaudRate(PORT_UART1);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_DataBits(PORT_UART1);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_StopBits(PORT_UART1);
     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_Parity(PORT_UART1);

 }

 static void WS_WriteCOM2Handler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint8_t Comvalue = 0;

     Comvalue = pWebsocket->rxBuffer[16];
     if(Comvalue <= TYPES_BAUDRATE)
     {
         HTTPResponse_UPS_Put_BaudRate(PORT_UART2, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[17];
     if(Comvalue <= TYPES_DATABITS)
     {
         HTTPResponse_UPS_Put_DataBits(PORT_UART2, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[18];
     if(Comvalue <= TYPES_STOPBITS)
     {
         HTTPResponse_UPS_Put_StopBits(PORT_UART2, Comvalue);
     }

     Comvalue =0;
     Comvalue = pWebsocket->rxBuffer[19];
     if(Comvalue <= TYPES_PARITY)
     {
         HTTPResponse_UPS_Put_Parity(PORT_UART2, Comvalue);
     }
    // UPS_ReInitProtocol(PORT_UART2);
//     pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_BaudRate(PORT_UART2);
  //   pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_StopBits(PORT_UART2);
  //   pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)HTTPRequest_UPS_Get_Parity(PORT_UART2);

 }


 static void HTTP_UpdataMobileNumbers(uint16_t MobileIndex, uint64_t MobileNumber)
 {
     if(MobileIndex < 5)
     {
        Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[MobileIndex].all= MobileNumber;
     }
 }
 static void WS_WriteGSMGroupHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint16_t CRC_Cal = 0;
     uint16_t MobRegNumber=0;
     REG_uint64_t MobileNoConfig;
     uint8_t startIndex=0,i = 0;
     uint8_t *Buffer = (uint8_t*) &(Device_Configurations.EepromMapStruct.Save_GSMSettings.TransBuffer);
     for (MobRegNumber = 0; MobRegNumber < 5; ++MobRegNumber)
     {
         if ((MobRegNumber >= 0) && (MobRegNumber <= NO_OF_MOBILE_REG))
         {
             MobileNoConfig.all = 0;

             startIndex = 16 + ((MobRegNumber) * 8);
             if (MobRegNumber >= 0 && MobRegNumber <= 4)
             {
                 for (i = 0; i < 8; ++i)
                 {
                     MobileNoConfig.byte[i] = pWebsocket->rxBuffer[startIndex + i];
                 }
             }

             if (MobileNoConfig.all != 0)
             {
                 if (((MobileNoConfig.byte[5] == 0) || (MobileNoConfig.byte[5] == 0x91))
                     && ((MobileNoConfig.byte[4] >= 0x01) && (MobileNoConfig.byte[4] <= 0xFF)))
                 {
                     // Save in Database
                     HTTP_UpdataMobileNumbers((MobRegNumber), MobileNoConfig.all);
                 }
             }
             else
             {
                 HTTP_UpdataMobileNumbers((MobRegNumber), 0x00);
             }
         }
     }

        Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS =  MAKE16BIT(pWebsocket->rxBuffer[56], pWebsocket->rxBuffer[57]);

        CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, GSM_CONFIG_SIZE-2);
        Device_Configurations.EepromMapStruct.Save_GSMSettings.CRC_CheckGSMSett = CRC_Cal;
        OnChipEEPROM_Write(GSM_CONFIG_SAVE, (uint8_t*)Buffer, GSM_CONFIG_SIZE);

        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[5] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[5] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[4] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[4] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[3] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[3] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[2] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[2] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[1] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[1] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[0] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[0].byte[0] & 0x0F;

        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[5] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[5] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[4] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[4] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[3] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[3] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[2] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[2] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[1] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[1] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[0] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[1].byte[0] & 0x0F;

        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[5] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[5] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[4] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[4] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[3] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[3] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[2] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[2] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[1] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[1] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[0] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[2].byte[0] & 0x0F;

        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[5] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[5] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[4] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[4] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[3] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[3] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[2] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[2] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[1] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[1] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[0] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[3].byte[0] & 0x0F;

        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[5] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[5] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[4] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[4] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[3] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[3] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[2] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[2] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[1] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[1] & 0x0F;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[0] >> 4;
        pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_MobilNumbers[4].byte[0] & 0x0F;

        pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)( Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS & 0xFF);
        pWebsocket->txBuffer[pWebsocket->txIndex++] = (uint8_t)((Device_Configurations.EepromMapStruct.Save_GSMSettings.CONFIG_AlarmGroupsForSMS >> 8) & 0xFF);

        pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.EmailSent & 0xFF);
        pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.EmailSent >> 8) & 0xFF);

        pWebsocket->txBuffer[pWebsocket->txIndex++] = (SMTPConfig.EmailFailed & 0xFF);
        pWebsocket->txBuffer[pWebsocket->txIndex++] = ((SMTPConfig.EmailFailed >> 8) & 0xFF);
 }
 static void ConrolCmd_EnableDisablFlagFn(uint8_t *dataBuff,uint8_t itr,uint8_t itr1)
 {

     switch(itr1)
     {
        case 0:
            dataBuff[itr]='A';
            break;
        case 1:
            dataBuff[itr]='B';
            break;
       case 2:
            dataBuff[itr]='C';
            break;
        case 3:
            dataBuff[itr]='D';
            break;
        case 4:
            dataBuff[itr]='E';
            break;
        case 5:
            dataBuff[itr]='F';
            break;
        case 6:
            dataBuff[itr]='G';
            break;
        case 7:
            dataBuff[itr]='H';
            break;
        case 8:
            dataBuff[itr]='I';
            break;
        case 9:
            dataBuff[itr]='J';
            break;
        case 10:
            dataBuff[itr]='K';
            break;
        case 11:
            dataBuff[itr]='L';
            break;
        case 12:
            dataBuff[itr]='M';
            break;
        case 13:
            dataBuff[itr]='N';
            break;
        case 14:
            dataBuff[itr]='O';
            break;
        case 15:
            dataBuff[itr]='P';
            break;
        case 16:
            dataBuff[itr]='Q';
            break;
        case 17:
            dataBuff[itr]='R';
            break;
        case 18:
            dataBuff[itr]='S';
            break;
        case 19:
            dataBuff[itr]='T';
            break;
        case 20:
            dataBuff[itr]='U';
            break;
        case 21:
            dataBuff[itr]='V';
            break;
        case 22:
            dataBuff[itr]='W';
            break;
        case 23:
            dataBuff[itr]='X';
            break;
        case 24:
            dataBuff[itr]='Y';
            break;
        case 25:
            dataBuff[itr]='Z';
            break;
     }
 }
 //==============================================================================
 //******************************************************************************
 //*
 //* Function name: HTTPApp_FlagSettingsConfCGIHandler
 //*
 //* Return:  char* - return page name to CGI handler over Socket.
 //*
 //*
 //* Description:
 //*   CGI Handler will allow to make change in Flag Settings from Web page.
 //*   but authentication is need.
 //*
 //*
 //****************************************************************************
 //==============================================================================
 static uint8_t ContrlCmds_FlagSettingsHandler(uint8_t FlagSetReset, uint8_t FlagNumber)
 {
        uint64_t currentcnt=0;
        uint8_t status=WRITE_ERROR;
        UPS_WRITE_REQUEST  pUpsWrtReqparamEn;
        UPS_WRITE_REQUEST  pUpsWrtReqparamDs;

        pUpsWrtReqparamEn.ctrlcommand[0]= 'P';
        pUpsWrtReqparamEn.ctrlcommand[1]= 'E';
        pUpsWrtReqparamEn.ctrlcommand[3]= '\r';
        pUpsWrtReqparamEn.ctrlcmdSize=4;

        pUpsWrtReqparamDs.ctrlcommand[0]= 'P';
        pUpsWrtReqparamDs.ctrlcommand[1]= 'D';
        pUpsWrtReqparamDs.ctrlcommand[3]= '\r';
        pUpsWrtReqparamDs.ctrlcmdSize=4;

        currentcnt= (uint64_t)(0x00000001)<<FlagNumber;

        if((UPSDb.settingFlags.flagAll & currentcnt)==0)
         {
             if(FlagSetReset==1)
             {
                    ConrolCmd_EnableDisablFlagFn(pUpsWrtReqparamEn.ctrlcommand,2,FlagNumber);

                    status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamEn);

                    if(status == WRITE_ERROR)
                    {
                       return status;
                    }
                    else
                    {
                       status = WRITE_SUCCESS;
                    }
              }
         }
         else
         {
            if(FlagSetReset==0)
           {
               ConrolCmd_EnableDisablFlagFn(pUpsWrtReqparamDs.ctrlcommand,2,FlagNumber);

               status=UPSInterface_InitiateWriteRequest(&pUpsWrtReqparamDs);

               if(status == WRITE_ERROR)
               {
                   return status;
               }
               else
               {
                   status = WRITE_SUCCESS;
               }
            }
        }

        return status;
 }

 static void WS_ClearDataLogHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
  {
     SPIFalsh_Fatfs_Init();  //Format and erase the SPI flash on clear command for now
  }
 static void WS_WriteRTCCompensateSettingHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
    uint16_t  CRC_Cal=0;
    uint8_t itr =0;
    Time systime;
    uint8_t* Buffer = (uint8_t*)&(Device_Configurations.EepromMapStruct.RTCCompenConfig.TransBuffer);

    Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroyafterhourSet= pWebsocket->rxBuffer[22];
    Device_Configurations.EepromMapStruct.RTCCompenConfig.RTCCompenstroySecondsSet = pWebsocket->rxBuffer[23];

    Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable = pWebsocket->rxBuffer[24];

    if( Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServerEnableDisable==0)
    {
        systime.Date=pWebsocket->rxBuffer[16];
        systime.Month=pWebsocket->rxBuffer[17];
        systime.Year=pWebsocket->rxBuffer[18];
        systime.Hours=pWebsocket->rxBuffer[19];
        systime.Minutes=pWebsocket->rxBuffer[20];
        systime.Seconds=pWebsocket->rxBuffer[21];
        WriteNewDateTimeToRTC(systime);
     }
    memset((uint8_t*)Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer,
              0, sizeof(Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer));
     for(itr=0;itr<32;itr++)
     {
         Device_Configurations.EepromMapStruct.RTCCompenConfig.NTPServer[itr]=pWebsocket->rxBuffer[25+itr];
     }
    CRC_Cal = COMCrcGenerator((uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE-2);
    Device_Configurations.EepromMapStruct.RTCCompenConfig.CRC_RTCCompensSett = CRC_Cal;
    OnChipEEPROM_Write(RTC_COMPENSATORY_SAVE, (uint8_t*)Buffer, RTC_COMPENSORTORY_SIZE);
 }

 static void WS_WriteFlagRefreshedHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.audibleAlarm;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.codeStart;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassForbiding;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.energySaving;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.shortRestart3Times;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SetHotStandby;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.deepHighEfficiencyMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.ConstantPhaseAngleFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.autoRestart;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battLowProtect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.FreeRunFunction;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.converterMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.limitedRuntimeOnBatt;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.outputParallelFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.phaseAutoAdapt;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.periodSelfTest;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled;
 }


 static void WS_WriteFlagEnabledHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
   uint8_t FlagSetReset=0, FlagNumber=0;
   FlagSetReset=pWebsocket->rxBuffer[16];
   FlagNumber=pWebsocket->rxBuffer[17];
   if(pWebsocket->rxBuffer[17]<26)
   {
      ContrlCmds_FlagSettingsHandler(FlagSetReset,FlagNumber);
   }
   else
   {
       Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled = FlagSetReset;
       WriteEmailConfigDataInOnChipEpprom();
   }
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.audibleAlarm;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battModeAudibleWarning;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.codeStart;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battOpenStatusCheck;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.highEfficiencyMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassForbiding;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.energySaving;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.shortRestart3Times;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.inverterShortClearFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket1InBattMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.OutputSocket2InBattMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SiteFaultDetect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.SetHotStandby;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.deepHighEfficiencyMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypasWhenUPSTurnOff;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.bypassAudibleWarning;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.ConstantPhaseAngleFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.autoRestart;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battDeepDischProtect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.battLowProtect;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.FreeRunFunction;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.converterMode;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.limitedRuntimeOnBatt;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.outputParallelFn;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.phaseAutoAdapt;
    pWebsocket->txBuffer[pWebsocket->txIndex++] = UPSDb.settingFlags.upsFlagsBits.periodSelfTest;

    pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.EmailConfig.WebEmail_Enabled;

 }

 static void WS_WriteScheduleHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint8_t ShutdownEnDis=0;
     Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_HH = pWebsocket->rxBuffer[16];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_MM = pWebsocket->rxBuffer[17];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_SS = pWebsocket->rxBuffer[18];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_HH = pWebsocket->rxBuffer[19];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_MM = pWebsocket->rxBuffer[20];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_SS = pWebsocket->rxBuffer[21];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH = pWebsocket->rxBuffer[22];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM = pWebsocket->rxBuffer[23];
     Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS = pWebsocket->rxBuffer[24];
     ShutdownEnDis    =  pWebsocket->rxBuffer[25];
     if(ShutdownEnDis==0)
     {
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByTime=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByBattLow = 0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_HH=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_MM=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_SS=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_HH=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_MM=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_SS=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM=0;
         Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS=0;

     }
     else
     {
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByTime =   pWebsocket->rxBuffer[26];
         Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnByBattLow = pWebsocket->rxBuffer[27];
     }
     uint16_t ShDnDelaySeconds = (Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH * 3600) +
                        (Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM * 60) + Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_HH;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_MM;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.ShDnTime_SS;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_HH;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_MM;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.RestartTime_SS;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_HH;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_MM;
     pWebsocket->txBuffer[pWebsocket->txIndex++] = Device_Configurations.EepromMapStruct.UPSShutDownSt.MainsFailShDnDly_SS;

 }
 //****************************************************************************
  //*
  //* Function name: WebSocket_Init
  //*
  //* Return: None
  //*
  //* Description:
  //*
  //*
  //****************************************************************************

 void WS_Init(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketStruct1)
{

  uint8_t count = 0;

  for (count = 0; count <WS_MAX_CLIENTS; count++)
  {
      pWebSocketStruct1->ClientConnection[count].sockfd = -1;
      pWebSocketStruct1->ClientConnection[count].startTimeConnectionTimeout=0;
      pWebSocketStruct1->ClientConnection[count].connectionTimeoutTime =modbusTCPStruct.modbusConnection[count].connectionTimeoutTime;
      pWebSocketStruct1->ClientConnection[count].txRemainingSize=0;
      pWebSocketStruct1->ClientConnection[count].rxBufferSize=0;
      memset(&pWebSocketStruct1->ClientConnection[count].remote,0,sizeof(struct sockaddr_in));
      memset(pWebSocketStruct1->ClientConnection[count].txBuffer,0,WS_MSG_BUFFER_SIZE);
      memset(pWebSocketStruct1->ClientConnection[count].rxBuffer,0,WS_MSG_BUFFER_SIZE);
  }

  pWebSocketStruct1->listenSockfd = -1;


  pWebSocketStruct1->ws_clientIndex= 0;

  memset(&pWebSocketStruct1->WebsocketDiag, 0x00, sizeof(WEBSOCKET_TCP_DIAG_COUNTER));
}


  //****************************************************************************
  //*
  //* Function name: WS_StartConnectionTimer
  //*
  //* Return: None
  //*
  //*
  //* Description:
  //*   Start Modbus TCP server Timer after Connection established and request
  //*   recived.
  //*
  //****************************************************************************


  static void WS_StartConnectionTimer(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr, uint8_t Client)
  {
      pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout =  COMTimer_Get10MsecTick();
  }
  //==============================================================================

  //****************************************************************************
  //*
  //* Function name: WS_CheckConnectionTimeout
  //*
  //* Return:  TRUE- For Timeout
  //*          FALSE- Timer is still running.
  //*
  //* Description:
  //*   Modbus TCP sever timer sends keep alive message after connection timer
  //*   Timeout call by ModbusTCPSever_SocketOperation.
  //*
  //****************************************************************************
  //==============================================================================

  static BOOL WS_CheckConnectionTimeout(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr,uint8_t Client)
  {

      // Just check if CurrentTick is within Tolerance zone.

      uint32_t currentTick=0;
      uint32_t nextTick=0;
      uint64_t totalTick=0;
      BOOL status=FALSE;

      //return status;
      totalTick = pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout +
                  pWebSocketPtr->ClientConnection[Client].connectionTimeoutTime;

      currentTick=COMTimer_Get10MsecTick();

      if(totalTick >= COM_TICK_MAX)
      {
          if ( currentTick <= pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout)
          {

              // rollover detected
              nextTick = (uint32_t)(totalTick - COM_TICK_MAX);
              if (currentTick >= nextTick )
                  status= TRUE;

          }
      }
      else
      {
          // Check if trigger tick is within tolerance. Tolerance for timer latency (late ISR)
          // Upper limit: tick time + tolerance
          // Lower limit: tick time
          // Current time must be within Upper and Lower

          // Check rollover
          if ( currentTick >= pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout)
          {
              // no rollover
              nextTick = (uint32_t) (pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout +
                                          pWebSocketPtr->ClientConnection[Client].connectionTimeoutTime);
              if (currentTick >= nextTick)
                  status= TRUE;

          }
          else  // if ( upper >= COM_TICK_MAX )
          {
              // rollover detected
              nextTick = (uint32_t) (pWebSocketPtr->ClientConnection[Client].connectionTimeoutTime -
                                          ( COM_TICK_MAX - pWebSocketPtr->ClientConnection[Client].startTimeConnectionTimeout));

              if (currentTick >= nextTick)
                  status= TRUE;

          }
      }

      return status;


  }

 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_SendMsgResponse
  *
  * Return: None
  *
  * Description:
  *  Send Msg on websocket response and send over
  * socket.
  */
 //**************************************************************************
 //==========================================================================
 void WS_SendMsgResponse(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket, ws_msg_t *msg )
 {
     uint8_t *outbuf_ptr = pWebsocket->send_buff;
     int packet_size = msg->prtcl_size + msg->msg_size;
     //WS_CLIENT_STRUCT *client;

     if (packet_size+7 > WS_SEND_BUFFER_SIZE)
         return;

     memset(outbuf_ptr, 0x00, WS_SEND_BUFFER_SIZE);
     outbuf_ptr[0] = (uint8_t)msg->msg_type;
     outbuf_ptr = ws_set_size_to_frame(packet_size, &outbuf_ptr[1]);

     if(msg->prtcl_size > 0)
     {
         outbuf_ptr = ws_set_data_to_frame(msg->protocol, msg->prtcl_size, outbuf_ptr);
     }

     if(msg->msg_size > 0)
     {
         outbuf_ptr = ws_set_data_to_frame(msg->message, msg->msg_size, outbuf_ptr);
     }

   // packet_size = outbuf_ptr - pWebsocket->txBuffer;
    packet_size = outbuf_ptr - pWebsocket->send_buff;

     if (pWebsocket->established == 1)
     {
         send( pWebsocket->sockfd,
                         pWebsocket->send_buff,//temp_buff,
                         packet_size,
                         NETCONN_NOCOPY);
     }
 }

 static void WS_ResetCOM1DiagHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     HTTPResponse_UART_ResetDiagnostic(0);
 }
 static void   WS_ResetMODTCPDiagHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
    ModbusTCPServer_DiagnosticParameters(0, TRUE);
 }
 static uint32_t SNMP_diagnosticParameter(uint16_t ParamIndex, BOOL Reset)
 {
     UPS_INTERFACE_RAM_STRUCT *pUPSInterfaceStruct = (UPS_INTERFACE_RAM_STRUCT *) &UPSInterfaceStruct;
     uint32_t paramValue = 0;
     if(Reset == TRUE)
     {
         //Reset all parameters
         memset(&pUPSInterfaceStruct->uartDiag, 0x00, sizeof(UART_DIAG_COUNTER));
     }
     else
     {
         switch(ParamIndex)
         {
             case 0:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.framesRecv;
                 break;
             }
             case 1:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.frameSent;
                 break;
             }
             case 2:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.goodTransaction;
                 break;
             }
             case 3:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.badTransaction;
                 break;
             }
             case 4:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.cntrlCmdFailCntr;
                 break;
             }
             case 5:
             {
                 paramValue = pUPSInterfaceStruct->uartDiag.cntrlCmdPassCntr;
                 break;
             }
             default:
                 break;
         }
     }
     return paramValue;
 }
 static void   WS_ResetSNMPDiagHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     SNMP_diagnosticParameter(0, TRUE);
 }

 static void WS_ResetETHERNETDiagHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     EthernetTask_DiagnosticParameters(0, TRUE);
 }

 static void WS_ResetCOM2DiagHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     HTTPResponse_UART_ResetDiagnostic(1);
 }

 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_GetMessageLen
  *
  * Return: uint32_t
  *
  * Description:
  */
 //**************************************************************************
 //==========================================================================
 static uint32_t WS_GetMessageLen( uint8_t *msg )
 {
     uint32_t len = 0;
     if ( (msg[1] & 0x7F) == 126 )
     {
         len = (msg[2] << 8) | msg[3];
     }
     else
     {
         len = msg[1] & 0x7F;
     }

     return len;
 }
 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_MaskedMsg
  *
  * Return: uint8_t
  *
  * Description:
  */
 //**************************************************************************
 //==========================================================================
 static uint8_t WS_MaskedMsg( uint8_t *msg )
 {
     if ( (msg[1] & 0x80) == 0x80 )
         return 1;
     return 0;
 }
 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_GetPayloadPtr
  *
  * Return: uint8_t
  *
  * Description:
  */
 //**************************************************************************
 //==========================================================================
 static uint8_t* WS_GetPayloadPtr( uint8_t *msg )
 {
     uint8_t *p = ( (msg[1] & 0x7F) == 126 ) ? (msg + 4) : (msg + 2);
     if (WS_MaskedMsg(msg))
         p += 4;
     return p;
 }
 //==========================================================================
 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_UnmaskMsgPayload
  *
  * Return: None
  *
  * Description:
  */
 //**************************************************************************
 //==========================================================================
 static void WS_UnmaskMsgPayload( uint8_t *pld, uint32_t len, uint8_t *mask )
 {
     int i = 0;
     for (i = 0; i < len; ++i)
     pld[i] = mask[i%4] ^ pld[i];
 }
 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_GetMask
  *
  * Return: uint8_t
  *
  * Description:
  */
 //**************************************************************************
 //==========================================================================
 static uint8_t* WS_GetMask( uint8_t *msg )
 {
     if ( (msg[1] & 0x7F) == 126 )
         return &msg[4];
     else
         return &msg[2];
 }
 //==========================================================================
 void (*WS_EmailHandlerFuncPtr[]) (WEBSOCKET_TCP_CONN_STRUCT *) =
 {
     WS_EmailConfigSaveHandler,  //0
     WS_EmailTestHandler         //1
 };

 void (*WS_WriteGroupHandlerFuncPtr[]) (WEBSOCKET_TCP_CONN_STRUCT *) =
  {
      WS_WriteEthernetHandler,          //0
      WS_WriteSNMPHandler,              //1
      WS_WriteMODbusHandler,     //2
      NULL,//WS_WriteMONHandler,        //3
      WS_WriteCOM1Handler,              //4
      NULL,//WS_WriteProtocolHandler,   //5
      WS_WriteCOM2Handler,              //6
      WS_WriteGSMGroupHandler,          //7
      WS_WriteFlagEnabledHandler,       //8
      WS_WriteScheduleHandler,   //9
      WS_WriteSNMPTrapsHandler,         //10
      WS_WriteDateTimeSettingHandler,   //11
      WS_WriteLOGIntervalSettingHandler, //12
      WS_WriteFlagRefreshedHandler,      //13
      WS_WriteRTCCompensateSettingHandler, //14
      WS_ClearDataLogHandler               //15
  };

  void (*WS_ReadGroupHandlerFuncPtr[]) (WEBSOCKET_TCP_CONN_STRUCT *) =
  {
      WS_ReadAllDataHandler,      //0
      WS_ReadAllSettingHandler,   //
      WS_ReadDataLogHandler,      //
      WS_ReadNoOfDataLogHandler,  //
  };

  void (*WS_ResetGroupHandlerFuncPtr[]) (WEBSOCKET_TCP_CONN_STRUCT *) =
  {
      WS_ResetCOM1DiagHandler,      //0
      WS_ResetCOM2DiagHandler,      //1
      WS_ResetMODTCPDiagHandler,    //2
      WS_ResetSNMPDiagHandler,      //3
      WS_ResetETHERNETDiagHandler   //4

  };

  void WS_SendErrorResponse(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket, uint8_t WS_Error)
  {
      uint16_t DataLengthIndex=0,DataLengthIndex1=0;
      ws_msg_t msg ={0};
      WS_HEADER_STRUCT header = {0};

      if((pWebsocket->rxBuffer[1] & 0x7F) == 126 )
      {
          header.startByte_1  = pWebsocket->rxBuffer[8];
          header.startByte_2  = pWebsocket->rxBuffer[9];
          header.Request_id   = pWebsocket->rxBuffer[10];
          header.FunctionCode = pWebsocket->rxBuffer[11];
          header.Module       = pWebsocket->rxBuffer[12];
          header.subModule    = pWebsocket->rxBuffer[13];
          header.Group        = pWebsocket->rxBuffer[14];
          header.subGroup     = pWebsocket->rxBuffer[15];
      }
      else
      {
          header.startByte_1  = pWebsocket->rxBuffer[6];
          header.startByte_2  = pWebsocket->rxBuffer[7];
          header.Request_id   = pWebsocket->rxBuffer[8];
          header.FunctionCode = pWebsocket->rxBuffer[9];
          header.Module       = pWebsocket->rxBuffer[10];
          header.subModule    = pWebsocket->rxBuffer[11];
          header.Group        = pWebsocket->rxBuffer[12];
          header.subGroup     = pWebsocket->rxBuffer[13];
      }
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.startByte_1;       //0  Start Byte1
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.startByte_2;       //1  Start Byte2
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Request_id;        //2  request ID
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.FunctionCode;      //5  FN Code
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Module;            //3  Module
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.subModule;         //4  Sub Module
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Group;             //6  Group
      pWebsocket->txBuffer[pWebsocket->txIndex++] = header.subGroup;          //7  Sub group
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Date;      //8
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Month;     //9
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Year;      //10
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Hours;     //11
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Minutes;   //12
      pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Seconds;   //13

      DataLengthIndex= pWebsocket->txIndex++;                                 //14
      DataLengthIndex1= pWebsocket->txIndex++;                                //15

      pWebsocket->txBuffer[pWebsocket->txIndex++] = WS_Error;

      pWebsocket->txBuffer[DataLengthIndex]  = (uint8_t)(pWebsocket->txIndex & 0xFF);
      pWebsocket->txBuffer[DataLengthIndex1] = (uint8_t)((pWebsocket->txIndex >> 8) & 0xFF);
      pWebsocket->txBuffer[pWebsocket->txIndex++]  = 0x00;//crc

      msg.msg_type = pWebsocket->msg_type;
      msg.message = pWebsocket->txBuffer;
      msg.msg_size = pWebsocket->txIndex;
      msg.prtcl_size = 0;

      WS_SendMsgResponse(pWebsocket,&msg);
  }
 //==========================================================================
 //**************************************************************************
 /*
  * Function name: WS_RequestHandler
  *
  * Return: None
  *
  * Description:
  *  handling websocket request and fill with data as request and send over
  * socket.
  */
 //**************************************************************************
 //==========================================================================
 void WS_RequestHandler(WEBSOCKET_TCP_CONN_STRUCT *pWebsocket)
 {
     uint16_t DataLengthIndex=0,DataLengthIndex1=0;
     ws_msg_t msg ={0};
     WS_HEADER_STRUCT header = {0};

     pWebsocket->txIndex=0;
     memset(pWebsocket->txBuffer, 0x00, WS_CLIENT_RECV_BUFFER_SIZE);
   //  WEB_SOCKET_TASK_NOT_PROCESSING=FALSE;

     if ( (pWebsocket->rxBuffer[1] & 0x7F) == 126 )
     {
         header.startByte_1  = pWebsocket->rxBuffer[8];
         header.startByte_2  = pWebsocket->rxBuffer[9];
         header.Request_id   = pWebsocket->rxBuffer[10];
         header.FunctionCode = pWebsocket->rxBuffer[11];
         header.Module       = pWebsocket->rxBuffer[12];
         header.subModule    = pWebsocket->rxBuffer[13];
         header.Group        = pWebsocket->rxBuffer[14];
         header.subGroup     = pWebsocket->rxBuffer[15];
     }
     else
     {
         header.startByte_1  = pWebsocket->rxBuffer[6];
         header.startByte_2  = pWebsocket->rxBuffer[7];
         header.Request_id   = pWebsocket->rxBuffer[8];
         header.FunctionCode = pWebsocket->rxBuffer[9];
         header.Module       = pWebsocket->rxBuffer[10];
         header.subModule    = pWebsocket->rxBuffer[11];
         header.Group        = pWebsocket->rxBuffer[12];
         header.subGroup     = pWebsocket->rxBuffer[13];
     }

     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.startByte_1;       //0  Start Byte1
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.startByte_2;       //1  Start Byte2
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Request_id;        //2  request ID
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.FunctionCode;      //3  FN Code
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Module;            //4  Module
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.subModule;         //5  Sub Module
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.Group;             //6  Group
     pWebsocket->txBuffer[pWebsocket->txIndex++] = header.subGroup;          //7  Sub group


    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Date;      //8
    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Month;     //9
    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Year;      //10
    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Hours;     //11
    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Minutes;   //12
    pWebsocket->txBuffer[pWebsocket->txIndex++] = GlobalDateTime.Seconds;   //13

     DataLengthIndex= pWebsocket->txIndex++;                                 //14
     DataLengthIndex1= pWebsocket->txIndex++;                                //15

     switch (header.FunctionCode)
             {
                 case WEBSOCKET_RESET:
                     WS_ResetGroupHandlerFuncPtr[header.Group](pWebsocket);
                     break;

                 case WEBSOCKET_READ:
                     WS_ReadGroupHandlerFuncPtr[header.Group](pWebsocket);
                     break;

                 case WEBSOCKET_WRITE:
                     WS_WriteGroupHandlerFuncPtr[header.Group](pWebsocket);
                     break;

                 case WEBSOCKET_EMAIL_CONFIG:
                      WS_EmailHandlerFuncPtr[header.Group](pWebsocket);
                      break;
             }

     pWebsocket->txBuffer[DataLengthIndex]  = (uint8_t)(pWebsocket->txIndex & 0xFF);
     pWebsocket->txBuffer[DataLengthIndex1] = (uint8_t)((pWebsocket->txIndex >> 8) & 0xFF);
     pWebsocket->txBuffer[pWebsocket->txIndex++]  = 0x00;//crc

     msg.msg_type = pWebsocket->msg_type;
     msg.message = pWebsocket->txBuffer;
     msg.msg_size = pWebsocket->txIndex;
     msg.prtcl_size = 0;//strlen(binaryData);

     WS_SendMsgResponse(pWebsocket,&msg);
//     WEB_SOCKET_TASK_NOT_PROCESSING=TRUE;
 }

 static uint8_t WS_ValidateRequest(WEBSOCKET_TCP_CONN_STRUCT *pWebSocketPtr)
 {
     uint8_t ret=1;
     WS_HEADER_STRUCT header = {0};
     if((pWebSocketPtr->rxBuffer[1] & 0x7F) == 126 )
     {
         header.startByte_1  = pWebSocketPtr->rxBuffer[8];
         header.startByte_2  = pWebSocketPtr->rxBuffer[9];
         header.Request_id   = pWebSocketPtr->rxBuffer[10];
         header.FunctionCode = pWebSocketPtr->rxBuffer[11];
         header.Module       = pWebSocketPtr->rxBuffer[12];
         header.subModule    = pWebSocketPtr->rxBuffer[13];
         header.Group        = pWebSocketPtr->rxBuffer[14];
         header.subGroup     = pWebSocketPtr->rxBuffer[15];
     }
     else
     {
         header.startByte_1  = pWebSocketPtr->rxBuffer[6];
         header.startByte_2  = pWebSocketPtr->rxBuffer[7];
         header.Request_id   = pWebSocketPtr->rxBuffer[8];
         header.FunctionCode = pWebSocketPtr->rxBuffer[9];
         header.Module       = pWebSocketPtr->rxBuffer[10];
         header.subModule    = pWebSocketPtr->rxBuffer[11];
         header.Group        = pWebSocketPtr->rxBuffer[12];
         header.subGroup     = pWebSocketPtr->rxBuffer[13];
     }

     if((header.startByte_1 == STARTBYTES) &&
         (header.startByte_2 == STARTBYTES) &&
         (header.Module <= MAX_MODULES) &&
         (header.subModule <= MAX_SUB_MODULES))
     {
         if(header.FunctionCode < WEBSOCKET_MAX_FNC_CODE)
         {
             switch (header.FunctionCode)
             {
                 case WEBSOCKET_READ:
                     if(header.Group <= MAX_READ_GROUPS)
                     {
                         if(header.subGroup <= MAX_READ_SUB_GROUPS)
                         {
                             ret=0;
                         }
                     }
                 case WEBSOCKET_WRITE:
                     if(header.Group <= MAX_WRITE_GROUPS)
                     {
                         if(header.subGroup <= MAX_WRITE_SUB_GROUPS)
                         {
                             ret=0;
                         }
                     }
                 case WEBSOCKET_RESET:
                     if(header.Group <= MAX_RESET_GROUPS)
                     {
                         if(header.subGroup <= MAX_RESET_SUB_GROUPS)
                         {
                             ret=0;
                         }
                     }
                 case WEBSOCKET_EMAIL_CONFIG:
                     if(header.Group <= MAX_EMAIL_GROUPS)
                     {
                         if(header.subGroup <= MAX_EMAIL_SUB_GROUPS)
                         {
                             ret=0;
                         }
                     }
                     break;
             }
         }
     }
     return ret;
 }
 //==============================================================================
 //****************************************************************************
 //*
 //* Function name: WS_ProcessRequest
 //*
 //* Return: None
 //*
 //* Description:
 //*   Process all Modbus request and fill with data as response and send over
 //*   socket, call by ModbusTCPSever_SocketOperation.
 //*
 //****************************************************************************
 //==============================================================================
 static void WS_ProcessRequest(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr, uint8_t ClientConn)
 {
     uint8_t ret=1;

     if((pWebSocketPtr->ClientConnection[ClientConn].rxBuffer[0] & 0x80) == 0x80)
     {
         uint32_t len = pWebSocketPtr->ClientConnection[ClientConn].rxBufferSize;
         pWebSocketPtr->ClientConnection[ClientConn].payload = WS_GetPayloadPtr(pWebSocketPtr->ClientConnection[ClientConn].rxBuffer);
         if (WS_MaskedMsg(pWebSocketPtr->ClientConnection[ClientConn].rxBuffer) == 1)
         {
             uint8_t *mask = WS_GetMask(pWebSocketPtr->ClientConnection[ClientConn].rxBuffer);
             WS_UnmaskMsgPayload(pWebSocketPtr->ClientConnection[ClientConn].payload, len, mask);
         }

         pWebSocketPtr->ClientConnection[ClientConn].txIndex=0;
         memset(pWebSocketPtr->ClientConnection[ClientConn].txBuffer, 0x00, WS_CLIENT_RECV_BUFFER_SIZE);

         ret=WS_ValidateRequest(&pWebSocketPtr->ClientConnection[ClientConn]);
         if(ret==0)
         {
             pWebSocketPtr->ClientConnection[ClientConn].msg_type = (ws_type_t)pWebSocketPtr->ClientConnection[ClientConn].rxBuffer[0];
             WS_RequestHandler(&pWebSocketPtr->ClientConnection[ClientConn]);
         }
         else
         {
             pWebSocketPtr->WebsocketDiag.otherError = (uint32_t) ((pWebSocketPtr->WebsocketDiag.otherError + 1) & 0xFFFFFFFF);
             pWebSocketPtr->ClientConnection[ClientConn].msg_type = (ws_type_t)pWebSocketPtr->ClientConnection[ClientConn].rxBuffer[0];
             WS_SendErrorResponse(&pWebSocketPtr->ClientConnection[ClientConn],ret);
         }
     }
 }
 //==============================================================================
 //****************************************************************************
 //*
 //* Function name: WS_CreateSocket
 //*
 //* Return: None
 //*
 //* Description:
 //*   Create Modbus TCP Listen Socket
 //*
 //****************************************************************************
 //==============================================================================
 static uint8_t WS_CreateSocket(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr)
 {
     int tempSockfd=-1;
     int option=1,rc=0;
     struct sockaddr_in local;
     uint32_t addr_len = sizeof(struct sockaddr);

     if((networkConfig.CurrentState != STATE_LWIP_WORKING) && (pWebSocketPtr->listenSockfd < 0))
     {
         return 0;
     }

     if((pWebSocketPtr->listenSockfd < 0) && (pWebSocketPtr->ws_clientIndex < WS_MAX_CLIENTS))
     {
         local.sin_port = htons(WS_PORT);
         local.sin_family = PF_INET;
         local.sin_addr.s_addr = INADDR_ANY;

         tempSockfd = socket(AF_INET, SOCK_STREAM, 0);
         if (tempSockfd < 0)
         {
             return 0;
         }

         /*************************************************************/
         /* Allow socket descriptor to be reuseable                   */
         /*************************************************************/
         rc = setsockopt(tempSockfd, SOL_SOCKET, SO_REUSEADDR,
                         (char *)&option, sizeof(option));
         if (rc < 0)
         {
            close(tempSockfd);
            return 0;
         }

         /*************************************************************/
         /* Set socket to be nonblocking. All of the sockets for      */
         /* the incoming connections will also be nonblocking since   */
         /* they will inherit that state from the listening socket.   */
         /*************************************************************/
         rc = ioctl(tempSockfd, FIONBIO, (char *)&option);
         if (rc < 0)
         {
            close(tempSockfd);
            return 0;
         }

         /*             21      */
         if (bind(tempSockfd, (struct sockaddr *) &local, addr_len) < 0)
         {
             close(tempSockfd);
             return 0;
         }

         if (listen(tempSockfd, 1) < 0)//MBTCP_MAX_CONNECTION
         {
             close(tempSockfd);
             return 0;
         }

         pWebSocketPtr->listenSockfd = tempSockfd;
         //pModbusStruct->maxfd = tempSockfd;
     }

     return 1;
 }
//==============================================================================
//****************************************************************************
//*
//* Function name: WS_LookForNewRequest
//*
//* Return: None
//*
//* Description:
//*   Recieves new packets from different clients and processes it. Also it sends replies
//*
//****************************************************************************
//==============================================================================
// char KeyofWebSocket[256]={0};
static void WS_LookForNewRequest(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr)
{
    int numbytes;
    fd_set readFD;
    struct timeval timeout;
    uint8_t i=0;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 2000;
    uint32_t PayloadLen=0;
    uint8_t HeaderLen = 4;


    if(pWebSocketPtr->ws_clientIndex > 0)
    {
        for(i=0;i<WS_MAX_CLIENTS;i++)
        {
            if(pWebSocketPtr->ClientConnection[i].sockfd >= 0)//pModbusStruct->ConnFD[i].sockfd != -1
            {
                FD_ZERO(&readFD);

                FD_SET(pWebSocketPtr->ClientConnection[i].sockfd, &readFD);

                if (select((pWebSocketPtr->ClientConnection[i].sockfd + 1), &readFD, 0, 0, &timeout) > 0)//pModbusStruct->sockfd[i]
                {

                    if (FD_ISSET(pWebSocketPtr->ClientConnection[i].sockfd, &readFD)) //pModbusStruct->sockfd[i]
                    {
                        FD_CLR(pWebSocketPtr->ClientConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                        numbytes = recv(pWebSocketPtr->ClientConnection[i].sockfd,
                                        &pWebSocketPtr->ClientConnection[i].rxBuffer[pWebSocketPtr->ClientConnection[i].rxBufferSize],
                                        (WS_CLIENT_RECV_BUFFER_SIZE - pWebSocketPtr->ClientConnection[i].rxBufferSize), 0);

                        if (numbytes <= 0)
                        {
                            // ErrnoIndex = errno;
                             if (errno != EWOULDBLOCK)
                             {
                                close(pWebSocketPtr->ClientConnection[i].sockfd);//pModbusStruct->sockfd[i]
                                pWebSocketPtr->ClientConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                                pWebSocketPtr->ws_clientIndex--;
                                pWebSocketPtr->ClientConnection[i].established = 0;
                                memset(&pWebSocketPtr->ClientConnection[i].remote,0,sizeof(struct sockaddr_in));
                             }
                        }
                        else
                        {
                            pWebSocketPtr->ClientConnection[i].rxBufferSize += numbytes;
                            if (strncmp((char *)pWebSocketPtr->ClientConnection[i].rxBuffer, "GET /", 5) == 0)
                            {
                                WS_CreateKeyAccept((char *)pWebSocketPtr->ClientConnection[i].rxBuffer, (char *)ws_key_accept);

                                pWebSocketPtr->WebsocketDiag.HandshakeCounter = (uint32_t) ((pWebSocketPtr->WebsocketDiag.HandshakeCounter + 1) & 0xFFFFFFFF);    // allow rollover
                                WS_StartConnectionTimer(pWebSocketPtr,i);

                                memset(pWebSocketPtr->ClientConnection[i].txBuffer, 0x00, WS_SEND_BUFFER_SIZE);

                                sprintf((char *)pWebSocketPtr->ClientConnection[i].txBuffer, "%s%s%s", head_ws,
                                                                ws_key_accept,
                                                                "\r\n\r\n");
                                LenTxBuf= strlen((char *)pWebSocketPtr->ClientConnection[i].txBuffer);
                                send(pWebSocketPtr->ClientConnection[i].sockfd,
                                     pWebSocketPtr->ClientConnection[i].txBuffer,
                                     LenTxBuf,
                                        NETCONN_NOCOPY);

                                pWebSocketPtr->ClientConnection[i].rxBufferSize =0;
                                memset(pWebSocketPtr->ClientConnection[i].rxBuffer,0,WS_CLIENT_RECV_BUFFER_SIZE);

                            }
                            else if(pWebSocketPtr->ClientConnection[i].rxBufferSize >= 4)
                            {
                                HeaderLen =4;
                                if (WS_MaskedMsg(pWebSocketPtr->ClientConnection[i].rxBuffer) == 1)
                                {
                                    HeaderLen += 2;
                                }

                                if((pWebSocketPtr->ClientConnection[i].rxBuffer[1] & 0x7F) == 126 )
                                {
                                    HeaderLen += 2;
                                }

                                PayloadLen = WS_GetMessageLen(pWebSocketPtr->ClientConnection[i].rxBuffer);
                                if((PayloadLen + HeaderLen) == pWebSocketPtr->ClientConnection[i].rxBufferSize)
                                {
                                    pWebSocketPtr->WebsocketDiag.framesRecv = (uint32_t) ((pWebSocketPtr->WebsocketDiag.framesRecv + 1) & 0xFFFFFFFF);    // allow rollover
                                    WS_ProcessRequest(pWebSocketPtr, i);
                                    WS_StartConnectionTimer(pWebSocketPtr,i);
                                    pWebSocketPtr->ClientConnection[i].rxBufferSize =0;
                                    memset(pWebSocketPtr->ClientConnection[i].rxBuffer,0,WS_CLIENT_RECV_BUFFER_SIZE);
                                    pWebSocketPtr->ClientConnection[i].rxBufferSize =0;
                                }
                                else if(pWebSocketPtr->ClientConnection[i].rxBufferSize > (PayloadLen + HeaderLen) )
                                {
                                    pWebSocketPtr->WebsocketDiag.otherError = (uint32_t) ((pWebSocketPtr->WebsocketDiag.otherError + 1) & 0xFFFFFFFF);    // allow rollover
                                    WS_StartConnectionTimer(pWebSocketPtr,i);
                                    pWebSocketPtr->ClientConnection[i].rxBufferSize =0;
                                    memset(pWebSocketPtr->ClientConnection[i].rxBuffer,0,WS_CLIENT_RECV_BUFFER_SIZE);
                                }
                            }
                        }
                    }
                    else
                    {
                        FD_CLR(pWebSocketPtr->ClientConnection[i].sockfd, &readFD); //pModbusStruct->sockfd[i]
                    }
                }
                else if((WS_CheckConnectionTimeout(pWebSocketPtr,i) == TRUE))
                {
                    pWebSocketPtr->WebsocketDiag.WebSocketCloseCounter = (uint32_t) ((pWebSocketPtr->WebsocketDiag.WebSocketCloseCounter + 1) & 0xFFFFFFFF);    // allow rollover
                    close(pWebSocketPtr->ClientConnection[i].sockfd); //pModbusStruct->sockfd[i]
                    pWebSocketPtr->ClientConnection[i].sockfd = -1;//pModbusStruct->sockfd[i] = -1;
                    pWebSocketPtr->ClientConnection[i].established = 0;
                    memset(&pWebSocketPtr->ClientConnection[i].remote,0,sizeof(struct sockaddr_in));
                    pWebSocketPtr->ws_clientIndex--;
                    pWebSocketPtr->ClientConnection[i].rxBufferSize =0;
                }

            }
        }
    }

}
//==============================================================================
//****************************************************************************
//*
//* Function name:WS_LookForNewConnection
//*
//* Return: None
//*
//* Description:
//*   Accepts New Modbus TCP connections
//*
//****************************************************************************
//==============================================================================

static void WS_LookForNewConnection(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr)
{
    int tempSockfd=-1;
    struct sockaddr_in remote;
    uint32_t addr_len = sizeof(struct sockaddr);
    fd_set readFD;
    struct timeval timeout;
    uint8_t i=0;

    timeout.tv_sec  = 0;
    timeout.tv_usec = 2000;

    if(pWebSocketPtr->ws_clientIndex < WS_MAX_CLIENTS)
    {

        FD_ZERO(&readFD);
        FD_SET(pWebSocketPtr->listenSockfd, &readFD);//maxfd

        if ((select(pWebSocketPtr->listenSockfd + 1, &readFD, 0, 0, &timeout) > 0))
        {
            if (FD_ISSET(pWebSocketPtr->listenSockfd, &readFD))
            {

                FD_CLR(pWebSocketPtr->listenSockfd, &readFD);

                tempSockfd = accept(pWebSocketPtr->listenSockfd, (struct sockaddr *) &remote,
                                (socklen_t *) & addr_len);
                if (tempSockfd < 0)
                {
                    //handle this aituation later
                }
                else
                {
                    for(i=0;i<WS_MAX_CLIENTS; i++)
                    {

                        if(pWebSocketPtr->ClientConnection[i].sockfd < 0)//pModbusStruct->sockfd[i]
                        {

                            //pModbusStruct->ConnFD[i].connFlag=TRUE;
                            pWebSocketPtr->ClientConnection[i].sockfd = tempSockfd;//pModbusStruct->sockfd[i]
                            pWebSocketPtr->ClientConnection[i].remote = remote;//pModbusStruct->remote[pModbusStruct->mb_clientIndex]
                            pWebSocketPtr->ClientConnection[i].rxBufferSize = 0;//pModbusStruct->rxBufferSize = 0;
                            WS_StartConnectionTimer(pWebSocketPtr,i);
                            pWebSocketPtr->ClientConnection[i].established = 1;
                            pWebSocketPtr->ws_clientIndex++;
                            //pModbusStruct->modbusDiag.clientConnections = pModbusStruct->mb_clientIndex;
                            break;
                        }
                    }
                    if(pWebSocketPtr->ws_clientIndex >= WS_MAX_CLIENTS)
                    {

                        //FD_CLR(pModbusStruct->listenSockfd, &readFD);
                        close(pWebSocketPtr->listenSockfd);
                        pWebSocketPtr->listenSockfd = -1;
                       // pModbusStruct->mb_clientIndex--;
                    }

                }
            }
            else
            {
                FD_CLR(pWebSocketPtr->listenSockfd, &readFD);
            }
        }
    }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: WS_SocketOperation
//*
//* Return: None
//*
//* Description:
//*   Initializing Socket as server, check binding, accept the socket and send
//*   data over the socket, call by ModbusTCPSever_Task.
//*
//****************************************************************************
//==============================================================================
void WS_SocketOperation(WEBSOCKET_TCP_RAM_STRUCT *pWebSocketPtr)
{
    if(WS_CreateSocket(pWebSocketPtr))
    {
        WS_LookForNewConnection(pWebSocketPtr);

        WS_LookForNewRequest(pWebSocketPtr);

        pWebSocketPtr->WebsocketDiag.clientConnections = pWebSocketPtr->ws_clientIndex;

    }
}
//==============================================================================
// END OF websocket.c FIlE
//==============================================================================
