/*
 * SMTPTask.h
 *  Created on: jan 2024
 *       by Mohit Dongre
 */
#include <Common.h>
#include "lwip/api.h"
#include "cmsis_os.h"
#include "OnChipEEPROM.h"
#include "smtp.h"

#define EMAIL_PROCESS_INTERVAL		1000      //10msec time base  i.e 5 sec
#define SMTP_CONNTIMEOUT			500        // 10 sec



typedef union __attribute__((packed))
{
	Time			ThisTime;
} SMTPBUFF;

typedef enum
{
    SMTP_DETECT_CHANGE      = 0x00,
    SMTP_PREPARE_EMAIL      = 0x01,
    SMTP_CHECK_INTERVAL     = 0x02,
    SMTP_SEND_EMAIL         = 0x03

} SMTP_OP_STATE;
typedef struct __attribute__((packed))
{
	uint32_t IntervalTime;
	uint32_t startIntervalTimeTimeout;
	uint16_t SMTP_srv_err;
    err_t SMTP_err;
    uint8_t SMTP_result;
	uint16_t EmailSent;
	uint16_t EmailFailed;
    uint8_t Subject[MAX_LEN];
    uint8_t EmailMSG[MAX_MSG_LEN];
    uint32_t feedbackTimeoutCntr;
    uint8_t To_Index;
    SMTP_OP_STATE SMTP_State;

} SMTPRUNBUFF;

extern SMTPRUNBUFF SMTPConfig;

err_t smtp_set_auth(const char* username, const char* pass);
err_t smtp_set_server_addr(const char* server);
void smtp_set_server_port(u16_t port);
err_t smtp_set_auth(const char* username, const char* pass);
err_t smtp_send_mail(const char* from, const char* to, const char* subject, const char* body,smtp_result_fn callback_fn, void* callback_arg);
err_t SMTP_ConfigureAndSendEmail();
void SMTP_ProcessPendingEmail();
BOOL SMTP_CheckConnectionTimeout();
void SMTP_StartConnectionTimer();
void SMTP_Init();

/***************************** END OF FILE ************************************/
