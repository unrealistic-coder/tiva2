//****************************************************************************
//*
//*
//* File: FTPServer.h
//*
//* Synopsis: FTP Server define file - related Structures and interfaces
//*
//*
//****************************************************************************

#ifndef _FTPSERVER_H
#define _FTPSERVER_H

//#define WEBSERVER_USING_SPI_FLASH
//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
#include "SPIExtRTC.h"
//----------------------------------------------------------------------------

//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------

#define FTP_SERVER_CMD_PORT		21
#define FTP_SERVER_DATA_PORT	(FTP_SERVER_CMD_PORT - 1) 
#define FTP_SERVER_ROOT		"/"
#define FTP_SERVER_MAX_CONNECTION	1
#define FTP_SERVER_WELCOME_MSG		"220 Test FTP server for CC3200 ready.\r\n"
#define FTP_SERVER_BUFFER_SIZE		 (256*2)//(4096)//(128)//(4096)//
#define FTP_SERVER_CONNECTION_TIMEOUT (180000) // in millisecond --> 180* 1000ms = 3 mintute
//----------------------------------------------------------------------------
#define MAX_PAGESCMD        25

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------
typedef enum 
{
    CMD_USER,
    CMD_PASS,
    CMD_LIST,
    CMD_NLST,
    CMD_PWD,
    CMD_TYPE,
    CMD_PASV,
    CMD_RETR,
    CMD_STOR,
    CMD_SIZE,
    CMD_MDTM,
    CMD_SYST,
    CMD_CWD,
    CMD_CDUP,
    CMD_PORT,
    CMD_REST,
    CMD_MKD,
    CMD_DELE,
    CMD_RMD,
    CMD_FEAT,
    CMD_QUIT,
    CMD_UKNWN = -1,
} FTP_SERVER_CMD;
//----------------------------------------------------------------------------

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------
extern FIL httpfile;
extern struct http_state *hs1;
typedef struct __attribute__((packed))
{   FIL FTP_fileFS;
    int sockfd;
	int listenSockfd;
	int maxfd;
    struct sockaddr_in remote;
    char pasv_active;		/* pasv data */
    int pasv_sockfd;
    uint16_t pasv_port;
    size_t offset;
	uint32_t startTimeConnectionTimeout;
	uint32_t connectionTimeoutTime;	
    char FTPDataBuffer[FTP_SERVER_BUFFER_SIZE];	/* current directory */
    char spare_buf[256];
}FTP_SERVER_STRUCT;
//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------
void FTPServer_Task();
extern FTP_SERVER_STRUCT FTPConnHandle;
//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

void FTPServer_SocketInit();
void FTPServer_SocketOperation(FTP_SERVER_STRUCT *connPtr);
//----------------------------------------------------------------------------
#endif/*_FTPSERVER_H */
//==============================================================================
// END OF _FTPSERVER_H
//==============================================================================

