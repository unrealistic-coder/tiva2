//****************************************************************************
//*
//*
//* File: FTPServer.c
//*
//* Synopsis: FTP Server application over the top of LWIP library
//*
//*
//****************************************************************************

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <COMTimer.h>
#include <EthernetTask.h>
#include "FTPServer.h"
#include "ErrorHandler.h"
#include "UPSInterface.h"
#include <SYSTask.h>
#include "ff.h"
#include "SPIFlashTask.h"
#include "SPIFLash_Datalogger.h"
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
//                              > LOCAL VARIABLE DECLARATIONS <

FTP_SERVER_STRUCT FTPConnHandle={{0},-1,-1,{0},0,0,0,0,0,{0},{0}};// = NULL;

const char g_months[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "Ukn"
};
//----------------------------------------------------------------------------


//============================================================================
//                              > FUNCTION DEFINITIONS <
//----------------------------------------------------------------------------
//============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_StrBeginWith
//*
//* Return:  0 - if String match Found
//*			-1 - if String match not found
//*
//*
//* Description:
//*   Check match between two strings.
//*
//****************************************************************************
//==============================================================================

static int8_t FTPServer_StrBeginWith(const char *src, char *match)
{
    while (*match) 
	{
		if (*src == 0) 
	    {
	    	return -1;
		}

		if (*match != *src) 
		{
	    	return -1;

		}
		match++;
		src++;
    }
    return 0;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_SendReply
//*
//* Return:  None
//*			
//*
//* Description:
//*   Send single or multiple string replies over the socket.
//*
//****************************************************************************
//==============================================================================

static void FTPServer_SendReply(int sock, char *fmt, ...)
{
    static char str[256];
    va_list p_args;
    int r;

    va_start(p_args, fmt);
    r = vsnprintf(str, sizeof(str), fmt, p_args);
    va_end(p_args);

    if (r > 0) 
	{
		send(sock, str, r, 0);
    }
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_FullCmdList
//*
//* Return:  0 - Successfully Send
//*		   (-1)- Failed
//*
//*
//* Description:
//*   Send complete directory list as request in Command over the socket
//*
//****************************************************************************
//==============================================================================

static int8_t FTPServer_FullCmdList(char *name, int sockfd)
{
    DIR dir;
    FILINFO fno;
    FRESULT rc;
    uint16_t day, mon, year;		
    int8_t res = -1;


   uint32_t len = strlen(name);
    if (len > 1 && name[len - 1] == '/') 
	{
		name[len - 1] = 0;
    }

    do 
	{
		rc = f_opendir(&dir, (TCHAR *) name);
		if (rc) 
		{
	    	FTPServer_SendReply(sockfd, "500 Internal Server Error\r\n");
	    	break;
		} 
		else 
		{
	    	while(1)
			{
				rc = f_readdir(&dir, &fno);
				if (rc || !fno.fname[0]) 
				{
			    	break;	
				}
				day = GlobalDateTime.Date & 0x1f;//fno.fdate & 0x1f;
				mon = GlobalDateTime.Month -1; // ((fno.fdate >> 5) & 0x0f) - 1;
				year =  GlobalDateTime.Year;//((fno.fdate >> 9) & 0x3f) + 1980;
//
//				if (mon > 12)
//				{
//			    	mon = 0;
//			    	year = 2000;
//				}

				if (fno.fattrib & AM_DIR) 
				{
			    	FTPServer_SendReply(sockfd, "drw-r--r-- 1 root root %9d %3s %2u %4u %s\r\n", 0, g_months[mon], day, year, fno.fname);
				//    FTPServer_SendReply(sockfd, "drw-r--r-- 1 root root %s\r\n",fno.fname);
				}
				else 
				{
			  FTPServer_SendReply(sockfd, "-rw-r--r-- 1 root root %9d %3s %2u %4u %s\r\n", fno.fsize, g_months[mon], day, year, fno.fname);
				 // 	    FTPServer_SendReply(sockfd, "drw-r--r-- 1 root root %s\r\n",fno.fname);
				}
				res = 0;
	    	}	
		}
    } while (0);
   
    return res;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_SimpleCmdList
//*
//* Return:  0 - Successfully Send
//*		   (-1)- Failed
//*
//*
//* Description:
//*   list complete files from request directory and send over the socket.
//*
//****************************************************************************
//==============================================================================

static int8_t FTPServer_SimpleCmdList(char *name, int sockfd)
{
    FILINFO fno;
    FRESULT rc;
    DIR dir;

    
    rc = f_opendir(&dir, (TCHAR *) name);
    if (rc != FR_OK) 
	{
		FTPServer_SendReply(sockfd, "500 Internal Error\r\n");
		return -1;
    }
    
    while (1) 
	{
		rc = f_readdir(&dir, &fno);
		if (rc || !fno.fname[0]) 
		{
	    	break;
		}
		FTPServer_SendReply(sockfd, "%s\r\n", fno.fname);
    }

    return 0;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_GetFileSize
//*
//* Return:  int - requested file size.
//*		   (-1)  - requested file not found.
//*
//*
//* Description:
//*   Get requested file size and send to respected call.
//*
//****************************************************************************
//==============================================================================

static int FTPServer_GetFileSize(char *filename)
{
  //  FIL file;
    FTP_SERVER_STRUCT *conn;
    int res = -1;
    res = f_open(&conn->FTP_fileFS, (TCHAR*)filename, FA_READ);
	if (res == FR_OK)
   //if (f_open(&file, (TCHAR*)filename, FA_READ) == FR_OK) 
	{
		res = f_size(&conn->FTP_fileFS);
		//f_close(&file);
		f_close(&conn->FTP_fileFS);
    }
	f_close(&conn->FTP_fileFS);
    return res;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_ParseCommand
//*
//* Return:  FTP_SERVER_CMD - Validated Command Number.
//*		     CMD_UKNWN - for any invalidated command
//*
//*
//* Description:
//*   Validate recived FTP command and send Command Number as initialized in 
//*	   FTP_SERVER_CMD Enum.
//*
//****************************************************************************
//==============================================================================

static FTP_SERVER_CMD FTPServer_ParseCommand(char *buf)
{
	FTP_SERVER_CMD cmd = CMD_UKNWN;
    if (FTPServer_StrBeginWith(buf, "USER") == 0)
	{
		cmd = CMD_USER;
    } 
	else if (FTPServer_StrBeginWith(buf, "PASS") == 0) 
	{
		cmd = CMD_PASS;
    } 
	else if (FTPServer_StrBeginWith(buf, "LIST") == 0) 
	{
		cmd = CMD_LIST;
    } 
	else if (FTPServer_StrBeginWith(buf, "NLST") == 0) 
	{
		cmd = CMD_NLST;
    } 
	else if ((FTPServer_StrBeginWith(buf, "PWD") == 0) ||
			 (FTPServer_StrBeginWith(buf, "XPWD") == 0))
	{
		cmd = CMD_PWD;
    } 
	else if (FTPServer_StrBeginWith(buf, "TYPE") == 0) 
	{
		cmd = CMD_TYPE;
    } 
	else if (FTPServer_StrBeginWith(buf, "PASV") == 0) 
	{
		cmd = CMD_PASV;
    } 
	else if (FTPServer_StrBeginWith(buf, "RETR") == 0) 
	{
		cmd = CMD_RETR;
    } 
	else if (FTPServer_StrBeginWith(buf, "STOR") == 0) 
	{
		cmd = CMD_STOR;
    } 
	else if (FTPServer_StrBeginWith(buf, "SIZE") == 0) 
	{
		cmd = CMD_SIZE;
    } 
	else if (FTPServer_StrBeginWith(buf, "MDTM") == 0) 
	{
		cmd = CMD_MDTM;
    } 
	else if (FTPServer_StrBeginWith(buf, "SYST") == 0) 
	{
		cmd = CMD_SYST;
    } 
	else if (FTPServer_StrBeginWith(buf, "CWD") == 0) 
	{
		cmd = CMD_CWD;
    } 
	else if (FTPServer_StrBeginWith(buf, "CDUP") == 0) 
	{
		cmd = CMD_CDUP;
    } 
	else if (FTPServer_StrBeginWith(buf, "PORT") == 0) 
	{
		cmd = CMD_PORT;
    } 
	else if (FTPServer_StrBeginWith(buf, "REST") == 0) 
	{
		cmd = CMD_REST;
    } 
	else if (FTPServer_StrBeginWith(buf, "MKD") == 0) 
	{
		return CMD_MKD;
    } 
	else if (FTPServer_StrBeginWith(buf, "DELE") == 0) 
	{
		cmd = CMD_DELE;
    } 
	else if (FTPServer_StrBeginWith(buf, "RMD") == 0) 
	{
		cmd = CMD_RMD;
    } 
	else if (FTPServer_StrBeginWith(buf, "FEAT") == 0) 
	{
		cmd = CMD_FEAT;
    } 
	else if (FTPServer_StrBeginWith(buf, "QUIT") == 0) 
	{
		cmd = CMD_QUIT;
    } 
	/*else 
	{
		cmd = CMD_UKNWN;
    }*/
    return cmd;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_PathStepDown
//*
//* Return:  None
//*
//*
//* Description:
//*   Change Directory Path if not present or requested to change directory path
//*	  by FTP Client.
//*
//****************************************************************************
//==============================================================================

static void FTPServer_PathStepDown(char *path) // int
{
    uint16_t len=0, i=0;
    int8_t res = -1;

    len = strlen(path);


    for (i = len - 1; i > 0; i--) 
	{
		if (path[i] == '/') 
		{
	    	path[i] = 0;
	    	res = 0;
	    	break;
		}
    }

    if (res != 0) 
	{
		path[0] = '/';
		path[1] = 0;
    }

    //return 0;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_GetFullPath
//*
//* Return:  None
//*
//*
//* Description:
//*   Get full directory/Root path, or change path as suspious path request or
//*	  change in directory by FTP Client.
//*
//****************************************************************************
//==============================================================================
static void FTPServer_GetFullPath(FTP_SERVER_STRUCT *conn, char *path, char *new_path, size_t size)//int
{

    if (path[0] == '/')
	{
		strcpy(new_path, path);
    } 
	else if (strcmp("/", conn->FTPDataBuffer) == 0)
	{
		sprintf(new_path, "%s", path);
    } 
	else if (path[0] == '.' && path[1] == '.') 
	{
		//UARTprintf("Suspicius path\r\n");
		FTPServer_PathStepDown(conn->FTPDataBuffer);
		sprintf(new_path, "%s", conn->FTPDataBuffer);
    } 
	else 
	{
		sprintf(new_path, "%s/%s", conn->FTPDataBuffer, path);
    }

    //return 0;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_ProcessRequest
//*
//* Return:  int - Command Number.
//*		     0   - for any unknown command.
//*
//*
//* Description:
//*   After parsing FTP client request and validating it finally it will execuate
//*   request in process request and send information/data over data socket to
//*	  FTP Client.
//*
//****************************************************************************
//==============================================================================

static int FTPServer_ProcessRequest(FTP_SERVER_STRUCT *conn, char *buf)
{
    ssize_t SendError = ERR_OK;
    FRESULT rc;
    FTP_SERVER_CMD cmd;
    int num_bytes = 0;
    struct timeval tv;
    fd_set readfds;
    char *parameter_ptr, *ptr;
    struct sockaddr_in pasvremote, local;
    uint32_t addr_len = sizeof(struct sockaddr_in);
    int16_t ret = 0;	

    tv.tv_sec = 5;////3;
    tv.tv_usec = 0;

    ptr = buf;
    while (*ptr) 
	{
		if ((*ptr == '\r') || (*ptr == '\n'))
	    	*ptr = 0;
		ptr++;
    }


    parameter_ptr = strchr(buf, ' ');
    if (parameter_ptr != NULL)
		parameter_ptr++;


    cmd = (FTP_SERVER_CMD) FTPServer_ParseCommand(buf);
#if 0

    if (cmd > CMD_PASS && conn->status != LOGGED_STAT) 
	{
		FTPServer_SendReply(conn->sockfd, "550 Permission denied.\r\n");
		free(sbuf);
		return 0;
    }
#endif
    switch (cmd) 
	{


    	case CMD_USER:
#if 0
		UARTprintf("%s sent login \"%s\"\r\n", inet_ntoa(conn->remote.sin_addr), parameter_ptr);
		/* login correct */
		if (strcmp(parameter_ptr, FTP_USER) == 0) 
		{
	    	FTPServer_SendReply(conn->sockfd, "331 Password required for \"%s\"\r\n", parameter_ptr);
	    	conn->status = USER_STAT;
		}
		else 
		{
	    	/* incorrect login */
	    	FTPServer_SendReply(conn->sockfd, "530 Login incorrect. Bye.\r\n");
	    	ret = -1;
		}
#endif
		FTPServer_SendReply(conn->sockfd, "331 Password required for \"%s\"\r\n", parameter_ptr);
		break;


    	case CMD_PASS:
#if 0
		UARTprintf("%s sent password \"%s\"\r\n", inet_ntoa(conn->remote.sin_addr), parameter_ptr);

		/* password correct */
		if (strcmp(parameter_ptr, FTP_PASSWORD) == 0) 
		{
	    	FTPServer_SendReply(conn->sockfd, "230 User logged in\r\n");
	    	conn->status = LOGGED_STAT;
	    	UARTprintf("%s Password correct\r\n", inet_ntoa(conn->remote.sin_addr));
		} 
		else 
		{
	    	/* incorrect password */
	    	FTPServer_SendReply(conn->sockfd, "530 Login or Password incorrect. Bye!\r\n");
	    	conn->status = ANON_STAT;
	    	ret = -1;
		}
#endif
		FTPServer_SendReply(conn->sockfd, "230 Password OK. Current directory is %s\r\n", FTP_SERVER_ROOT);
		break;


	    case CMD_LIST:
		FTPServer_SendReply(conn->sockfd, "150 Opening Binary mode connection for file list.\r\n");
		FTPServer_FullCmdList(conn->FTPDataBuffer, conn->pasv_sockfd);
		close(conn->pasv_sockfd);
		conn->pasv_active = 0;
		FTPServer_SendReply(conn->sockfd, "226 Transfer complete.\r\n");
		break;


	    case CMD_NLST:
		FTPServer_SendReply(conn->sockfd, "150 Opening Binary mode connection for file list.\r\n");
		FTPServer_SimpleCmdList(conn->FTPDataBuffer, conn->pasv_sockfd);
		close(conn->pasv_sockfd);
		conn->pasv_active = 0;
		FTPServer_SendReply(conn->sockfd, "226 Transfer complete.\r\n");
		break;


	    case CMD_TYPE:
		if (strcmp(parameter_ptr, "I") == 0) 
		{
		    FTPServer_SendReply(conn->sockfd, "200 Type set to binary.\r\n");
		}
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "200 Type set to ascii.\r\n");
		}
		break;


	    case CMD_RETR:
		strcpy(conn->spare_buf, buf + 15);
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		num_bytes = FTPServer_GetFileSize(conn->spare_buf);

		if (num_bytes == -1) 
		{
		    FTPServer_SendReply(conn->sockfd, "550 \"%s\" : not a regular file\r\n", conn->spare_buf);
		    conn->offset = 0;
		    break;
		}

		rc = (FRESULT)f_open(&conn->FTP_fileFS, conn->spare_buf, FA_READ);
		if (rc != FR_OK) 
		{
		      break;
		}


		if (conn->offset > 0 && conn->offset < num_bytes) 
		{
		    //f_lseek(&file, conn->offset);
		    f_lseek(&conn->FTP_fileFS, conn->offset);
		    FTPServer_SendReply(conn->sockfd, "150 Opening binary mode data connection for partial \"%s\" (%d/%d bytes).\r\n",
		                        conn->spare_buf, num_bytes - conn->offset, num_bytes);
		} 
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "150 Opening binary mode data connection for \"%s\" (%d bytes).\r\n", conn->spare_buf, num_bytes);
		}


		strcpy((char*)&conn->spare_buf, conn->FTPDataBuffer);
	
		do 
		{
			Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
			//rc =  f_read(&file, sbuf, 124, (unsigned *) &num_bytes);
		    rc = f_read(&conn->FTP_fileFS, conn->FTPDataBuffer, FTP_SERVER_BUFFER_SIZE, (unsigned *) &num_bytes);
		    if ((rc == FR_OK) && (num_bytes > 0))
			{
				SendError = send(conn->pasv_sockfd, conn->FTPDataBuffer, num_bytes, 0);

				if(SendError == -1)
				{
					// error
				//    UARTprintf("Error in Send\r\n");
					break;
				}
		    }
		    vTaskDelay(1);
		}
		while ((rc == FR_OK) && (num_bytes > 0) && (SendError != -1));

		if((SendError == -1) || (rc != FR_OK))
		{
			FTPServer_SendReply(conn->sockfd, "401 Error\r\n");

		}
		else
		{
			FTPServer_SendReply(conn->sockfd, "226 Finished.\r\n");
		}

		f_close(&conn->FTP_fileFS);
		
		strcpy(conn->FTPDataBuffer, (char*)&conn->spare_buf);

		close(conn->pasv_sockfd);	
		break;

#if defined WEBSERVER_USING_SPI_FLASH
	    case CMD_STOR:

	           memset(conn->spare_buf,0,sizeof(conn->spare_buf));

	           FRESULT DirStatus  = FR_NO_PATH;
	           FRESULT FileStatus  = FR_OK;
	           char CurrentDir[30]  = {0};

	           char Filename[30];

	           FTPServer_GetFullPath(conn, parameter_ptr, CurrentDir, 256);

	           strcat( conn->spare_buf, CurrentDir);
    	       rc = f_open(&conn->FTP_fileFS, conn->spare_buf, (FA_OPEN_ALWAYS| FA_CREATE_ALWAYS | FA_WRITE));



	           if (rc != FR_OK)
	           {
	               FTPServer_SendReply(conn->sockfd, "550 Cannot open \"%s\" for writing.\r\n", conn->spare_buf);

	               break;
	           }
	           FTPServer_SendReply(conn->sockfd, "150 Opening binary mode data connection for \"%s\".\r\n", conn->spare_buf);

	           FD_ZERO(&readfds);
	           FD_SET(conn->pasv_sockfd, &readfds);

	           while (select(conn->pasv_sockfd + 1, &readfds, 0, 0, &tv) > 0)
	           {
	               Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
	               if ((num_bytes = recv(conn->pasv_sockfd,  conn->FTPDataBuffer, FTP_SERVER_BUFFER_SIZE, 0)) > 0)
	               {
	                   unsigned bw;

	                   rc = (FRESULT)f_write(&conn->FTP_fileFS, conn->FTPDataBuffer, num_bytes, &bw);

	               }
	               else if (num_bytes == 0)
	               {
	                   f_close(&conn->FTP_fileFS);
	                   FTPServer_SendReply(conn->sockfd, "226 Finished.\r\n");
	                   break;
	               }
	               else if (num_bytes == -1)
	               {
	                   f_close(&conn->FTP_fileFS);
	                   ret = -1;
	                   break;
	               }
	               Error_ResetTaskWatchdog(SPI_FLASH_FS_TASK_ID);
	           }
	           close(conn->pasv_sockfd);
	           f_close(&conn->FTP_fileFS);
	           break;
#endif
	    case CMD_SIZE:
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		num_bytes = FTPServer_GetFileSize(conn->spare_buf);
		if (num_bytes == -1) 
		{
		    FTPServer_SendReply(conn->sockfd, "550 \"%s\" : not a regular file\r\n", conn->spare_buf);
		} 
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "213 %d\r\n", num_bytes);
		}
		break;


	    case CMD_MDTM:
		FTPServer_SendReply(conn->sockfd, "550 \"/\" : not a regular file\r\n");
		break;


	    case CMD_SYST:
		FTPServer_SendReply(conn->sockfd, "215 UNIX Type: L8\r\n");
		break;


	    case CMD_PWD:
		FTPServer_SendReply(conn->sockfd, "257 \"%s\" is current directory.\r\n", conn->FTPDataBuffer);
		break;


	    case CMD_CWD:
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		FTPServer_SendReply(conn->sockfd, "250 Changed to directory \"%s\"\r\n", conn->spare_buf);
		strcpy(conn->FTPDataBuffer, conn->spare_buf);
		//UARTprintf("CWD: Changed to directory %s\r\n", spare_buf);
		break;


	    case CMD_CDUP:
		//sprintf(spare_buf, "%s/%s", conn->currentdir, "..");
		sprintf(conn->spare_buf, "%s", conn->FTPDataBuffer);
		FTPServer_PathStepDown(conn->spare_buf);
		strcpy(conn->FTPDataBuffer, conn->spare_buf);


		FTPServer_SendReply(conn->sockfd, "250 Changed to directory \"%s\"\r\n", conn->spare_buf);

		break;


	    case CMD_PORT:
		{
		    int portcom[6];
		    num_bytes = 0;
		    portcom[num_bytes++] = atoi(strtok(parameter_ptr, ".,;()"));
		    for (num_bytes = 0; num_bytes < 6; num_bytes++) 
			{
				portcom[num_bytes] = atoi(strtok(0, ".,;()"));
		    }
		    sprintf(conn->spare_buf, "%d.%d.%d.%d", portcom[0], portcom[1], portcom[2], portcom[3]);

		    FD_ZERO(&readfds);
		    if ((conn->pasv_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
				close(conn->pasv_sockfd);
				conn->pasv_active = 0;
				break;
		    }

		    local.sin_port = htons(FTP_SERVER_DATA_PORT);
		    local.sin_addr.s_addr = INADDR_ANY;
		    local.sin_family = PF_INET;
		    if (bind(conn->pasv_sockfd, (struct sockaddr *) &local, addr_len) < 0) 
			{

		    }
		    pasvremote.sin_addr.s_addr = ((uint32_t) portcom[3] << 24) | 
										 ((uint32_t) portcom[2] << 16) | 
										 ((uint32_t) portcom[1] << 8) | 
										 ((uint32_t) portcom[0]);
		    pasvremote.sin_port = htons(portcom[4] * 256 + portcom[5]);
		    pasvremote.sin_family = PF_INET;
		    if (connect(conn->pasv_sockfd, (struct sockaddr *) &pasvremote, addr_len) == -1) 
			{
				pasvremote.sin_addr = conn->remote.sin_addr;
				if (connect(conn->pasv_sockfd, (struct sockaddr *) &pasvremote, addr_len) == -1) 
				{
			    	FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
			    	close(conn->pasv_sockfd);
			    	break;
				}
		    }
		    conn->pasv_active = 1;
		    conn->pasv_port = portcom[4] * 256 + portcom[5];
		    FTPServer_SendReply(conn->sockfd, "200 Port Command Successful.\r\n");
		}
		break;


	    case CMD_REST:
		if (atoi(parameter_ptr) >= 0) 
		{
		    conn->offset = atoi(parameter_ptr);
		    FTPServer_SendReply(conn->sockfd, "350 Send RETR or STOR to start transfert.\r\n");
		}
		break;


	    case CMD_MKD:
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		if (f_mkdir(conn->spare_buf))
		{
		    FTPServer_SendReply(conn->sockfd, "550 File \"%s\" exists.\r\n", conn->spare_buf);
		} 
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "257 directory \"%s\" was successfully created.\r\n", conn->spare_buf);
		}
		break;


	    case CMD_DELE:
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		if (f_unlink(conn->spare_buf) == FR_OK)
		    FTPServer_SendReply(conn->sockfd, "250 file \"%s\" was successfully deleted.\r\n", conn->spare_buf);
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "550 Not such file or directory: %s.\r\n", conn->spare_buf);
		}
		break;


	    case CMD_RMD:
		FTPServer_GetFullPath(conn, parameter_ptr, conn->spare_buf, 256);
		if (f_unlink(conn->spare_buf))
		{
		    FTPServer_SendReply(conn->sockfd, "550 Directory \"%s\" doesn't exist.\r\n", conn->spare_buf);
		} 
		else 
		{
		    FTPServer_SendReply(conn->sockfd, "257 directory \"%s\" was successfully deleted.\r\n", conn->spare_buf);
		}
		break;

#if 1
	    case CMD_PASV:
		do 
		{
		    int dig1, dig2;
		    int sockfd;
		    ip_addr_t my_ipaddr;
			uint16_t port=4096;
			uint8_t bindRet=0;

			my_ipaddr.addr=lwIPLocalIPAddrGet();


		    FD_ZERO(&readfds);
		    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
			{
				FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
				ret = 1;
				break;
		    }


			while (bindRet==0) 
			{

				if(++port > 0x7fff)
					port = 4096;

			    conn->pasv_port = port;
			    conn->pasv_active = 1;
			    local.sin_port = htons(conn->pasv_port);
			    local.sin_addr.s_addr = INADDR_ANY;
			    local.sin_family = PF_INET;		
			
				dig1 = (int) (conn->pasv_port / 256);
				dig2 = conn->pasv_port % 256;

				if (bind(sockfd, (struct sockaddr *) &local, addr_len) == -1) 
				{
					if (4096 == port)
					{
						FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
						ret = 3;				
						bindRet=1;
					}	
				}
				else
				{
					bindRet=1;
				}

			}

		    if (listen(sockfd, 1) == -1) 
			{
				FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
				ret = 4;
				break;
		    }


		    FTPServer_SendReply(conn->sockfd, "227 Entering passive mode (%d,%d,%d,%d,%d,%d)\r\n",
				  (uint8_t)(my_ipaddr.addr >> 0), (uint8_t)(my_ipaddr.addr >> 8),(uint8_t)(my_ipaddr.addr >> 16),
	              (uint8_t)(my_ipaddr.addr >> 24), dig1, dig2);

		    FD_SET(sockfd, &readfds);
		    select(sockfd + 1, &readfds, 0, 0, &tv);
		    if (FD_ISSET(sockfd, &readfds)) 
			{
				conn->pasv_sockfd = accept(sockfd, (struct sockaddr *) &pasvremote, (socklen_t *) & addr_len);
				if (conn->pasv_sockfd < 0) 
				{

			    	FTPServer_SendReply(conn->sockfd, "425 Can't open data connection.\r\n");
			    	ret = 5;
			    	break;
				} 
				else 
				{

			    	conn->pasv_active = 1;
			    	close(sockfd);
				}
		    } 
			else 
			{
				ret = 6;
				break;
		    }
		} while (0);

		if (ret) 
		{

		    close(conn->pasv_sockfd);
		    conn->pasv_active = 0;
		    ret = 0;
		}
		break;
#endif


	    case CMD_FEAT:
		FTPServer_SendReply(conn->sockfd, "211 No-features\r\n");
		break;


	    case CMD_QUIT:
		FTPServer_SendReply(conn->sockfd, "221 Adios!\r\n");
		ret = -1;		
		break;


	    default:
			FTPServer_SendReply(conn->sockfd, "502 Not Implemented yet.\r\n");
		break;
    }
    return ret;
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_StartConnectionTimer
//*
//* Return:  None
//*
//*
//* Description:
//*   Start connection timer after connected with FTP Client, every time when
//*   timer's out it will start again as connection established with FTP Client.
//*	  
//*
//****************************************************************************
//==============================================================================

static void FTPServer_StartConnectionTimer(FTP_SERVER_STRUCT *FTPConnPtr)
{
    FTPConnPtr->startTimeConnectionTimeout = COMTimer_Get10MsecTick();//COMTimer_GetTickMsec();
}
//==============================================================================

//****************************************************************************
//*
//* Function name: FTPServer_CheckConnectionTimeout
//*
//* Return:  TRUE- For Timeout
//*			 FALSE- Timer is still running.
//*
//* Description:
//*   Check timer every interval after FTP client connection established.
//*   Ideally FTP client should connected for 3 mins, after that Connection
//*	  should be terminated in Idle condition.
//*
//****************************************************************************
//==============================================================================

static BOOL FTPServer_CheckConnectionTimeout(FTP_SERVER_STRUCT *FTPConnPtr)
{
    // Just check if CurrentTick is within Tolerance zone.

    COM_TIMER_TICK currentTick=0;
    COM_TIMER_TICK nextTick=0;
    uint64_t totalTick=0;
    BOOL status=FALSE;

    totalTick = FTPConnPtr->startTimeConnectionTimeout + FTPConnPtr->connectionTimeoutTime;

    currentTick=COMTimer_Get10MsecTick();//COMTimer_GetTickMsec();

    if(totalTick >= COM_TICK_MAX)
    {
        if ( currentTick <= FTPConnPtr->startTimeConnectionTimeout)
        {

            // rollover detected
            nextTick = (COM_TIMER_TICK)(totalTick - COM_TICK_MAX);
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
        if ( currentTick >= FTPConnPtr->startTimeConnectionTimeout)
        {
            // no rollover
            nextTick = (COM_TIMER_TICK) (FTPConnPtr->startTimeConnectionTimeout +
                                        FTPConnPtr->connectionTimeoutTime);
            if (currentTick >= nextTick)
                status= TRUE;

        }
        else  // if ( upper >= COM_TICK_MAX )
        {
            // rollover detected
            nextTick = (COM_TIMER_TICK) (FTPConnPtr->connectionTimeoutTime -
                                        ( COM_TICK_MAX - FTPConnPtr->startTimeConnectionTimeout));

            if (currentTick >= nextTick)
                status= TRUE;

        }
    }

    return status;

}
//==============================================================================
//****************************************************************************
//*
//* Function name: FTPServer_SocketInit
//*
//* Return:  None
//*
//* Description:
//*   FTP Socket initialization for respective port.
//*   
//****************************************************************************
//==============================================================================

void FTPServer_SocketInit(void)
{
	FTP_SERVER_STRUCT *FTPconnPtr;
	
	FTPconnPtr = &FTPConnHandle;
	
	FTPconnPtr->listenSockfd = -1;
	FTPconnPtr->sockfd=-1;
	FTPconnPtr->maxfd=-1;
	
	memset(FTPconnPtr->FTPDataBuffer,0,FTP_SERVER_BUFFER_SIZE);
	memset(FTPconnPtr->spare_buf,0,256);
	memset(&FTPconnPtr->remote,0,sizeof(struct sockaddr_in));
	
	FTPconnPtr->startTimeConnectionTimeout=0;
	FTPconnPtr->connectionTimeoutTime =FTP_SERVER_CONNECTION_TIMEOUT;

}
//==============================================================================
//****************************************************************************
//*
//* Function name: FTPServer_LookForNewRequest
//*
//* Return: None
//*
//* Description:
//*
//*
//*
//****************************************************************************
//==============================================================================
static void FTPServer_LookForNewRequest(FTP_SERVER_STRUCT *FTPConnPtr)
{

      int option=1;

      struct sockaddr_in remote;
      uint32_t addr_len = sizeof(struct sockaddr);
      int numbytes;
      fd_set readFD;
      struct timeval timeout;
      int com_socket;
      char ftp_buf[50];

      /*************************************************************/
      /* Initialize the timeval struct to 3 minutes.  If no        */
      /* activity after 3 minutes this program will end.           */
      /*************************************************************/
      timeout.tv_sec  = 0;
      timeout.tv_usec = 10000;
      /* ���� ��������� ������ */

      FD_ZERO(&readFD);

      /* get maximum fd */
      FD_SET(FTPConnPtr->listenSockfd, &readFD);

      if(FTPConnPtr->sockfd >=0)
      {
          FD_SET(FTPConnPtr->sockfd, &readFD);
      }

      if (select(FTPConnPtr->maxfd + 1, &readFD, 0, 0, &timeout) > 0)
      {

          if ((FD_ISSET(FTPConnPtr->listenSockfd, &readFD)))
          {

              com_socket = accept(FTPConnPtr->listenSockfd, (struct sockaddr *) &remote, (socklen_t *) & addr_len);
              if (com_socket < 0)
              {

                  FD_CLR(FTPConnPtr->listenSockfd, &readFD);
                  close(FTPConnPtr->listenSockfd);
                  if(FTPConnPtr->sockfd >=0)
                  {
                      FD_CLR(FTPConnPtr->sockfd, &readFD);
                      close(FTPConnPtr->sockfd);
                  }
                  FTPServer_SocketInit();
              }
              else
              {
                  FTPConnPtr->sockfd = com_socket;
                  FTPConnPtr->remote = remote;
                  FTPConnPtr->offset =0;
                  if(FTPConnPtr->sockfd > FTPConnPtr->maxfd)
                  {
                      FTPConnPtr->maxfd = FTPConnPtr->sockfd;
                  }

                  FD_CLR(FTPConnPtr->sockfd, &readFD);

                  FTPServer_StartConnectionTimer(FTPConnPtr);
                  send(com_socket, FTP_SERVER_WELCOME_MSG, strlen(FTP_SERVER_WELCOME_MSG), 0);
                  /* new FTPConnPtr */
                  strcpy(FTPConnPtr->FTPDataBuffer, FTP_SERVER_ROOT);

              }
          }

          if (FD_ISSET(FTPConnPtr->sockfd, &readFD))
          {
              numbytes = recv(FTPConnPtr->sockfd, ftp_buf, 50, 0);
              if (numbytes <= 0)
              {
                   if (errno != EWOULDBLOCK)
                   {

                      FD_CLR(FTPConnPtr->sockfd, &readFD);
                      close(FTPConnPtr->sockfd);
                      FTPConnPtr->sockfd=-1;
                   }
              }
              else
              {

                  if (FTPServer_ProcessRequest(FTPConnPtr, ftp_buf) < 0)
                  {
                      FD_CLR(FTPConnPtr->sockfd, &readFD);
                      close(FTPConnPtr->sockfd);
                      FTPConnPtr->sockfd=-1;
                  }
                  FTPServer_StartConnectionTimer(FTPConnPtr);
              }
          }
          else
          {
              //UARTprintf(" Why No Activity\n");
          }
      }
      else if(FTPConnPtr->sockfd >= 0)
      {
          if(FTPServer_CheckConnectionTimeout(FTPConnPtr) == TRUE)
          {

              FD_CLR(FTPConnPtr->sockfd, &readFD);
              close(FTPConnPtr->sockfd);
              FTPConnPtr->sockfd=-1;
          }
      }
}
//==============================================================================
//****************************************************************************
//*
//* Function name: FTPServer_LookForNewConnection
//*
//* Return: None
//*
//* Description:
//*
//*
//*
//****************************************************************************
//==============================================================================
static void FTPServer_LookForNewConnection(FTP_SERVER_STRUCT *FTPConnPtr)
{
    int sockfd;
    struct sockaddr_in remote;
    uint32_t addr_len = sizeof(struct sockaddr);
    fd_set readFD;
    struct timeval timeout;
    int com_socket;
    /*************************************************************/
    /* Initialize the timeval struct to 3 minutes.  If no        */
    /* activity after 3 minutes this program will end.           */
    /*************************************************************/
    timeout.tv_sec  = 0;
    timeout.tv_usec = 10000;
    /* ���� ��������� ������ */

    FD_ZERO(&readFD);

    /* get maximum fd */
    FD_SET(FTPConnPtr->listenSockfd, &readFD);

    if(FTPConnPtr->sockfd >=0)
    {
        FD_SET(FTPConnPtr->sockfd, &readFD);
    }

    if (select(FTPConnPtr->maxfd + 1, &readFD, 0, 0, &timeout) > 0)
    {

        if ((FD_ISSET(FTPConnPtr->listenSockfd, &readFD)))
        {
            com_socket = accept(FTPConnPtr->listenSockfd, (struct sockaddr *) &remote, (socklen_t *) & addr_len);
            if (com_socket < 0)
            {

                FD_CLR(FTPConnPtr->listenSockfd, &readFD);
                close(FTPConnPtr->listenSockfd);
                if(FTPConnPtr->sockfd >=0)
                {
                    FD_CLR(FTPConnPtr->sockfd, &readFD);
                    close(FTPConnPtr->sockfd);
                }
                FTPServer_SocketInit();
            }
            else
            {
                FTPConnPtr->sockfd = com_socket;
                FTPConnPtr->remote = remote;
                FTPConnPtr->offset =0;
                if(FTPConnPtr->sockfd > FTPConnPtr->maxfd)
                {
                    FTPConnPtr->maxfd = FTPConnPtr->sockfd;
                }

                FD_CLR(FTPConnPtr->sockfd, &readFD);

                FTPServer_StartConnectionTimer(FTPConnPtr);
                send(com_socket, FTP_SERVER_WELCOME_MSG, strlen(FTP_SERVER_WELCOME_MSG), 0);
                /* new FTPConnPtr */
                strcpy(FTPConnPtr->FTPDataBuffer, FTP_SERVER_ROOT);

            }
        }
    }

}
//==============================================================================

//****************************************************************************
//*
//* Function name: ModbusTCPServer_SocketOperation
//*
//* Return: None
//*
//* Description:
//*   Initializing Socket as server, check binding, accept the socket and send
//*   data over the socket, call by ModbusTCPSever_Task.
//*
//****************************************************************************
//==============================================================================
static uint8_t FTPServer_CreateSocket(FTP_SERVER_STRUCT *FTPConnPtr)
{
    int sockfd;
   int option=1,rc=0;
   struct sockaddr_in local;

   uint32_t addr_len = sizeof(struct sockaddr);





      if((networkConfig.CurrentState != STATE_LWIP_WORKING) && (FTPConnPtr->listenSockfd < 0))
       {
          return 0;

       }
   if((FTPConnPtr->listenSockfd < 0))
   {

       /* ������� ����� TCP ��� ������ � ����� ���������� */
       local.sin_port = htons(FTP_SERVER_CMD_PORT);
       local.sin_family = PF_INET;
       local.sin_addr.s_addr = INADDR_ANY;

       sockfd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockfd < 0)
       {

           return 0;
       }

       /*************************************************************/
       /* Allow socket descriptor to be reuseable                   */
       /*************************************************************/
       rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                       (char *)&option, sizeof(option));
       if (rc < 0)
       {

          close(sockfd);
          return 0;
       }

       /*************************************************************/
       /* Set socket to be nonblocking. All of the sockets for      */
       /* the incoming connections will also be nonblocking since   */
       /* they will inherit that state from the listening socket.   */
       /*************************************************************/
       rc = ioctl(sockfd, FIONBIO, (char *)&option);
       if (rc < 0)
       {

          close(sockfd);
          return 0;
       }

       /* �������� �� 21 ���� */
       if (bind(sockfd, (struct sockaddr *) &local, addr_len) < 0)
       {
           close(sockfd);
           return 0;
       }


       if (listen(sockfd, FTP_SERVER_MAX_CONNECTION) < 0)
       {

           close(sockfd);
           return 0;
       }
       FTPConnPtr->listenSockfd = sockfd;
       FTPConnPtr->maxfd = sockfd;
   }

   return 1;
}
void FTPServer_SocketOperation(FTP_SERVER_STRUCT *FTPConnPtr)
{
    if(FTPServer_CreateSocket(FTPConnPtr))
       {
            FTPServer_LookForNewConnection(FTPConnPtr);

            FTPServer_LookForNewRequest(FTPConnPtr);

       }
}


//==============================================================================

//==============================================================================
// END OF FTPServer.c FIlE
//==============================================================================

