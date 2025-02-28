//****************************************************************************
//*
//*
//* File: UPSInterface.h
//*
//* Synopsis: Serial interface with UPS.
//*
//*
//****************************************************************************
#ifndef UPS_INTERFACE_H
#define UPS_INTERFACE_H


//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------

#include <Common.h>
#include <SerialDriver.h>
#include <COMTimer.h>



//============================================================================
//                              > DEFINES <
//----------------------------------------------------------------------------
#if (_SPI_DEBUG == 1)
#define	SPI_printf(...)     UARTprintf(__VA_ARGS__)
#else
#define	SPI_printf(...)     {};
#endif

#define GET_BIT(VAR, BIT)       		((VAR   &   (1 << BIT)) != 0)
#define SET_BIT(VAR, BIT)				(VAR |= (1 << BIT))
#define CLEAR_BIT(VAR, BIT)				(VAR &= ~(1 << BIT))
#define GET_BITS(VAR,BITS) (VAR & ((1 << BITS) - 1))
#define REMOVE_BITS(VAR,BITS) (VAR & (~((1 << BITS) - 1)))
#define MASKBITS(BITS) 			((1 << BITS) - 1)
	
#define MAX_UPS_INQUIRY_CMDS            (19)


#define MAX_RX_BUFFER_SIZE              (100)
#define MAX_TX_BUFFER_SIZE              (100)
#define MAX_UPS_CONTROL_CMDS            (100)
#define MAX_REQ_COMMAND_SIZE            (20)

#define START_OF_FRAME		            (0x28)
#define END_OF_FRAME		            (0x0D)
#define SPACE_CHAR			            (0x20)

#define WRITE_ERROR                     (1)
#define WRITE_SUCCESS                   (0)
#define SERIAL_INTERFACE_TIMEOUT        (10)//100 msec

#define NO_OF_PARAM_IN_QPI_CMD			(1)
#define QPI_CMD_SIZE 	(4)
#define QMD_CMD_SIZE 	(4)
#define QMF_CMD_SIZE 	(4) 
#define QMOD_CMD_SIZE	(5)  
#define QRI_CMD_SIZE   	(4)
#define QGS_CMD_SIZE   	(4)
#define QBYV_CMD_SIZE  	(5)
#define QBYF_CMD_SIZE  	(5)
#define QYF_CMD_SIZE  	(4)
#define QFLAG_CMD_SIZE 	(6)
#define QWS_CMD_SIZE   	(4)
#define QFS_CMD_SIZE   	(4)
#define QVFW_CMD_SIZE  	(5)
#define QID_CMD_SIZE   	(4)
#define QBV_CMD_SIZE   	(4)
#define QHE_CMD_SIZE  	(4) 
#define QSK1_CMD_SIZE  	(5)
#define QSK2_CMD_SIZE  	(5)
#define QSKT1_CMD_SIZE 	(6)
#define QSKT2_CMD_SIZE 	(6)
#define QSR_CMD_SIZE    (4)

//Size of Control Commands
#define T_CMD_SIZE        (2)/*10 sec Test*/
#define TL_CMD_SIZE       (3)/* Test till battery low*/
#define TN_CMD_SIZE       (6)/*Test for specified time*/
#define T_N_CMD_SIZE      (5)/*Test for specified time*/
#define SN_CMD_SIZE       (4)/*Shutdown*/
#define SNR_CMD_SIZE       (9)/*Shutdown and restore*/
#define CS_CMD_SIZE       (3)/*cancel shutdown*/
#define CT_CMD_SIZE       (3)/*cancel test*/
#define BZOFF_CMD_SIZE    (6)/*silence buzzer beep*/
#define BZON_CMD_SIZE     (5)/*buzzer beep open*/
#define SON_CMD_SIZE     (4)/*Remote turn ON UPS*/
#define SOFF_CMD_SIZE     (5)/*Remote turn OFF UPS*/
#define SKONN_CMD_SIZE   (6)/*Remote turn on UPS output socket*/
#define SKOFFN_CMD_SIZE   (7)/*Remote turn oFF UPS output socket*/
#define PLV_CMD_SIZE      (7)/*Set bypass voltage low loss point*/
#define PHV_CMD_SIZE      (7)/*Set bypass voltage high loss point*/
#define PSF_CMD_SIZE      (8)/*Set bypass frequency loss loss point*/
#define PGF_CMD_SIZE      (8)/*Set bypass frequency high loss point*/
#define PF_CMD_SIZE      (3)/*Setting control parameter to default value*/
#define BATGRPN_CMD_SIZE    (8)/*Setting battery group number*/
#define BATPPN_CMD_SIZE    (7)/*Setting battery peiece number*/
#define HEH_CMD_SIZE      (7)/* Set high efficiency mode voltage high loss point*/
#define HEL_CMD_SIZE      (7)/* Set high efficiency mode voltage low loss point*/
#define PSK_CMD_SIZE      (8)/*Set output socket release delay time in battery mode*/
#define PEPD_CMD_SIZE       (4)/*setting some status enable/disable */
#define CFGBYP_NOT_ALLOWED  (4)/*setting some status enable/disable */
#define CFG_SITE_FAULT      (4)/*setting some status enable/disable */
#define BDR_CMD_SIZE        (6)/* Setting SCI baud rate*/
#define OPVSET_CMD_SIZE      (5)/*Output Voltage Setting*/

typedef void UPS_CMD_RX_FUNC_PTR(void *ptr);

//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

typedef enum
{
	NO_TASK_ID=0, 
	MODBUS_TCP_TASK_ID, 
    WEBSERVER_TASK_ID,
    SNMP_TASK_ID,
    SYS_TASK_ID,
    UPS_SERIAL_TASK_ID,
    ETHERNET_TASK_ID,
    USB_APP_TASK_ID,
    SPI_FLASH_FS_TASK_ID,
    GSM_TASK_ID,
    MAX_TASK_COUNT
} ENUM_SPI_REQUESTOR_TASK_ID;


typedef enum
{
	STATE_UPS_REQUEST_IDLE=0, 
    STATE_UPS_REQUEST_XMITTING,
    STATE_UPS_REQUEST_XMITTED,
    STATE_UPS_REQUEST_RECEIVING,
    STATE_UPS_REQUEST_RECEIVED,
    STATE_UPS_REQUEST_PROCESSING,
    STATE_UPS_REQUEST_TIMEOUT,
    STATE_UPS_REQUEST_COUNT

} ENUM_UPS_CMD_STATE;
	

typedef enum
{
	STATE_UPS_WRITE_EMPTY=0,
	STATE_UPS_WRITE_WAIT,
	STATE_UPS_WRITE_CTRL_CMD_SENT,
    STATE_UPS_WRITE_SUCCESS,
    STATE_UPS_WRITE_ERROR,
    STATE_UPS_WRITE_CMDS_COUNT
} ENUM_UPS_WRITE_STATE;




//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------

typedef union __attribute__((packed))
{
	uint16_t InputOutputPhase;
    struct
    {
		uint8_t InputPhase;
        uint8_t OutputPhase;
    }Bytes;
}INPUT_OUTPUT_PHASE_STRUCT;

typedef union __attribute__((packed))
{
	uint16_t statusAll;
    uint8_t  status_8bit[2];
    struct
    {
    	uint16_t reserved:4;
		uint16_t batTestOK:1;
    	uint16_t battTestFail:1;
		uint16_t battSilence:1;
		uint16_t shutdownActive:1;
		uint16_t TestinProgress:1;
		uint16_t EPO:1;
		uint16_t UPSFailed:1;
		uint16_t BypassActive:1;
		uint16_t BattLow:1;
		uint16_t utilityFail:1;
		uint16_t LiveStatus:2;
    			
    }Bits;
}UPS_STATUS_STRUCT;


typedef union __attribute__((packed))
{
	uint32_t flagAll;
    struct
    {
		uint32_t	audibleAlarm:1;//a
		uint32_t	battModeAudibleWarning:1;//b
		uint32_t	codeStart:1;//c
		uint32_t	battOpenStatusCheck:1;//d
		uint32_t	highEfficiencyMode:1;//e
		uint32_t	bypassForbiding:1;//f
		uint32_t	energySaving:1;//g
		uint32_t	shortRestart3Times:1;//h
		uint32_t	inverterShortClearFn:1;//i
		uint32_t	OutputSocket1InBattMode:1;//j
		uint32_t	OutputSocket2InBattMode:1;//k
		uint32_t	SiteFaultDetect:1;//l
		uint32_t	SetHotStandby:1;//m
		uint32_t	deepHighEfficiencyMode:1;//n
		uint32_t	bypasWhenUPSTurnOff:1;//o
		uint32_t	bypassAudibleWarning:1;//p
		uint32_t	ConstantPhaseAngleFn:1;//q
		uint32_t	autoRestart:1;//r
		uint32_t	battDeepDischProtect:1;//s
		uint32_t	battLowProtect:1;//t
		uint32_t	FreeRunFunction:1;//u
		uint32_t	converterMode:1;//v
		uint32_t	limitedRuntimeOnBatt:1;//w
		uint32_t	outputParallelFn:1;//x
		uint32_t	phaseAutoAdapt:1;//y
		uint32_t	periodSelfTest:1;//z
    	uint32_t	reserved:6;
    }upsFlagsBits;
}UPS_FLAG_STRUCT;

typedef union __attribute__((packed))
{
	uint64_t warningAll;
	uint32_t warningAll_word[2];
	uint16_t warning16_bit[4];
    struct
    {
		uint64_t	BatteryOpen:1;
		uint64_t	IP_N_Loss:1;
		uint64_t	IP_Site_Fail:1;
		uint64_t	LinePhaseError:1;
		uint64_t	BypassPhaseError:1;
		uint64_t	BypassFrequencyUnstable:1;
		uint64_t	BattOverCharge:1;
		uint64_t	BattLow:1;
		uint64_t	OverloadWarning:1;
		uint64_t	FanLockWarning:1;
		uint64_t	EPOActive:1;
		uint64_t	TurnOnAbnormal:1;
		uint64_t	OverTemperature:1;
		uint64_t	ChargerFail:1;
		uint64_t	RemoteShutdown:1;
		uint64_t	L1_IP_Fuse_Fail:1;
		uint64_t	L2_IP_Fuse_Fail:1;
		uint64_t	L3_IP_Fuse_Fail:1;
		uint64_t	L1_PFC_PError:1;
		uint64_t	L1_PFC_NError:1;
		uint64_t	L2_PFC_PError:1;
		uint64_t	L2_PFC_NError:1;
		uint64_t	L3_PFC_PError:1;
		uint64_t	L3_PFC_NError:1;
		uint64_t	CAN_Comm_Error:1;
		uint64_t	Synch_Line_Error:1;
		uint64_t	Synch_Pulse_Error:1;
		uint64_t	Host_Line_Error:1;
		uint64_t	MaleConnError:1;
		uint64_t	FemaleConnError:1;
		uint64_t	ParallelLineConnError:1;
		uint64_t	BattConnDiff:1;
		uint64_t	LineConnDiff:1;
		uint64_t	BypassConnDiff:1;
		uint64_t	ModeTypeDiff:1;
		uint64_t	ParallelINV_VoltDiff:1;
		uint64_t	ParallelOutputFreqDiff:1;
		uint64_t	BattCellOverCharge:1;
		uint64_t	ParallelOutputParallelDiff:1;
		uint64_t	ParallelOutputPhaseDiff:1;
		uint64_t	ParallelBypassForbiddenDiff:1;
		uint64_t	ParallelConverterEnableDiff:1;
		uint64_t	ParallelBypassFreqHighLossDiff:1;
		uint64_t	ParallelBypassFreqLowLossDiff:1;
		uint64_t	ParallelBypassVoltHighLossDiff:1;
		uint64_t	ParallelBypassVoltLowLossDiff:1;
		uint64_t	ParallelLineFreqHighLossDiff:1;
		uint64_t	ParallelLineFreqLowLossDiff:1;
		uint64_t	ParallelLineVoltHighLossDiff:1;
		uint64_t	ParallelLineVoltLowLossDiff:1;
		uint64_t	LockedInBypass:1;
		uint64_t	ThreePhaseCurrentUnbalance:1;
		uint64_t	BatteryFuseBroken:1;
		uint64_t	INVCurrentUnbalance:1;
		uint64_t	P1CutOffPreAlarm:1;
		uint64_t	BatteryReplace:1;
		uint64_t	InputPhaseError:1;
		uint64_t	CoverOfMaintainSWopen:1;
		uint64_t	PhaseAutoAdaptFailed:1;
		uint64_t	UtilityExtremelyUnbalanced:1;
		uint64_t	BypassUnstable:1;
		uint64_t	EEPROMError:1;
		uint64_t	ParallelProtectWarning:1;
		uint64_t	DischargerOverly:1;
    }Bits;
}UPS_WARNING_STRUCT;

typedef union __attribute__((packed))
{
	uint16_t runningStatusAll;

    struct
    {
 		uint16_t reserved0:1;
		uint16_t reserved1:1;
		uint16_t outputRelay:1;
		uint16_t inputRelay:1;
		uint16_t reserved4:1;
		uint16_t INV:1;
		uint16_t pfc:1;
		uint16_t dcTodc:1;
		uint16_t reserved:8;
    }upsRStatusBits;
}UPS_RUNNING_STATUS_STRUCT;


typedef  struct __attribute__ ((packed))
{
	uint16_t					FaultType;
	uint16_t					InputVoltBeforeFault;
	uint16_t					InputFreqBeforeFault;
	uint16_t					OutputVoltBeforeFault;
	uint16_t					OutputFreqBeforeFault;
	uint16_t					OutputLoadBeforeFault;
	uint16_t					OutputCurrentBeforeFault;
	uint16_t					PBusVoltageBeforeFault;
	uint16_t					NBusVoltBeforeFault;
	uint16_t					BattVoltBeforeFault;
	uint16_t					TempBeforeFault;
	UPS_RUNNING_STATUS_STRUCT	UPSRunningStatus;

}UPS_FAULT_STRUCT;

typedef  struct __attribute__ ((packed))
{
	uint8_t  	type;
	uint8_t		subType;
	uint8_t		VAType;
	uint8_t		H_LV_Type;
	uint8_t		year[2];
	uint8_t		month[2];
	uint8_t		ManufacturerID;
	int8_t		serialNumber[11];

}UPS_ID_STRUCT;

typedef struct __attribute__ ((packed))
{
	uint16_t protocolID;

	int8_t UPSManufacturer[26];
	int8_t UPSModel[16];
	uint32_t RatedOutputVA;
	uint16_t RatedOuputVolt;
	uint16_t RatedOuputCurr;
	uint16_t RatedOuputFreq;
	uint16_t RatedBattVolt;
	
	INPUT_OUTPUT_PHASE_STRUCT supportedPhase;
	uint16_t NominalInputVolt;
	uint16_t NominalOutputVolt;

	uint16_t BatteryTestType;
	uint16_t BattVolt;	
	uint16_t BattPieceNumber;
	uint16_t BattVoltPerUnit;
	uint16_t BattgroupNumber;
	uint16_t BattCapacity;
	uint16_t BattRemainTime;
	uint16_t PBusVolt;
	uint16_t NBusVolt;
	uint16_t PBattVolt;
	uint16_t NBattVolt;
	uint16_t BatNumberInParallel;
	uint16_t BatNumberInSeries;
	uint8_t UPSMode[2];
	int8_t UPSCPUFwVersion[15];
	UPS_ID_STRUCT upsID;

	uint16_t InputVolt;
	uint16_t InputFreq;
	uint16_t OutputVolt;
	uint16_t OutputFreq;
	uint16_t OutputCurr;
	uint16_t OutputPWRFactor;
	uint16_t OutputLoadPercent;
	uint16_t bypassFreq;// QYF command not supported now

	uint16_t MaxTemp;

	uint16_t bypassVoltHighLossPt;
	uint16_t bypassVoltLowLossPt;
	uint16_t voltHighLossPt;
	uint16_t voltLowLossPt;
	uint16_t freqHighLossPt;
	uint16_t freqLowLossPt;
	
	UPS_STATUS_STRUCT upsStatus;
	UPS_FLAG_STRUCT settingFlags;
	UPS_WARNING_STRUCT warningFlags;
	UPS_FAULT_STRUCT upsFaultStruct;

}UPS_DB_STRUCT;
extern UPS_DB_STRUCT UPSDb;
typedef struct __attribute__((packed))
{
	uint8_t command[MAX_REQ_COMMAND_SIZE];
	uint8_t cmdSize;
	UPS_CMD_RX_FUNC_PTR *cmdResponseFn;
}UPS_COMMAND_LIST_STRUCT;

typedef struct __attribute__((packed))
{
	ENUM_UPS_WRITE_STATE 		writeState;
	ENUM_SPI_REQUESTOR_TASK_ID 	taskID;
    uint8_t ctrlcommand[MAX_UPS_CONTROL_CMDS];
    uint8_t ctrlcmdSize;
}UPS_WRITE_REQUEST;

typedef struct __attribute__((packed))
{
    //Diagnostic Counters
    uint32_t  	goodTransaction;
	uint32_t  	badTransaction;
   	uint32_t  	ReqTimedout;
    uint32_t  framesRecv;
    uint32_t  frameSent;	
	uint32_t  DumpChars;
	uint32_t  charSent;
	uint32_t  charRecv;
    uint32_t  cntrlCmdPassCntr;
	uint32_t  cntrlCmdFailCntr;
} UART_DIAG_COUNTER;


typedef struct __attribute__((packed))
{
	UPS_COMMAND_LIST_STRUCT *commandListPtr;
	uint8_t 			currCmdIndex;
	int8_t 				rxBuffer[MAX_RX_BUFFER_SIZE];
	int8_t              txBuffer[MAX_TX_BUFFER_SIZE];
	uint8_t				rxBufferSize;
	uint8_t				txRemainingSize;
	uint8_t				txBufferIndex;
	COM_TIMER_ID        reqTimeoutTimer;    // Request Timeout Timer
	COM_TIMER_TICK		reqTimeoutTime;
	COM_TIMER_ID        IntercharTimer;    // Request Timeout Timer
	COM_TIMER_ID        InterFrameTimer;    // Request Timeout Timer
	COM_TIMER_TICK      intercharTime;
	COM_TIMER_TICK      InterframeTime;
	ENUM_UPS_CMD_STATE  requestState;
	UART_DIAG_COUNTER 	uartDiag;
	UPS_WRITE_REQUEST 	UPSWriteRequestStruct;
	uint64_t Prev_WarningFlags;



} UPS_INTERFACE_RAM_STRUCT;


typedef struct __attribute__((packed))
{
    //Diagnostic Counters
    uint32_t    goodTransaction;
    uint32_t    badTransaction;
    uint32_t    NumberofSMSsent;
    uint32_t   framesRecv;
    uint32_t   frameSent;
    uint32_t   ModemConnected;
    uint32_t  charSent;
    uint32_t  charRecv;
    uint32_t  NumberofSMSNotsent;
    uint32_t  ModemState;
    uint32_t  TransctionmPassCnt;
    uint32_t   TransactionFailedCnt;

} GSM_DIAG_Counter;


extern GSM_DIAG_Counter GSMDiagSt;




//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------
void QPI_ProcessResponse(void *ptr);    
void QMD_ProcessResponse(void *ptr); 
void QMF_ProcessResponse(void *ptr);
void QMOD_ProcessResponse(void *ptr);   
void QRI_ProcessResponse(void *ptr);    
void QGS_ProcessResponse(void *ptr);    
void QBYV_ProcessResponse(void *ptr);   
void QBYF_ProcessResponse(void *ptr);
void QYF_ProcessResponse(void *ptr);
void QFLAG_ProcessResponse(void *ptr);  
void QWS_ProcessResponse(void *ptr);    
void QFS_ProcessResponse(void *ptr);    
void QVFW_ProcessResponse(void *ptr);   
void QID_ProcessResponse(void *ptr);    
void QBV_ProcessResponse(void *ptr);    
void QHE_ProcessResponse(void *ptr);    
void QSK1_ProcessResponse(void *ptr);    
void QSK2_ProcessResponse(void *ptr);    
void QSKT1_ProcessResponse(void *ptr);  
void QSKT2_ProcessResponse(void *ptr);  
void QSR_ProcessResponse(void *ptr);

void CtrlCmdProcessResponse(void *ptr);//As all responses are similar


//////////////////
BOOL UPSInterface_Init();
uint32_t UPSInterface_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset);
void UPSInterface_RxISR(uint8_t character);
void UPSInterface_TxISR();
uint8_t UPSInterface_InitiateWriteRequest (UPS_WRITE_REQUEST *pReqParam);
void UPSInterface_ValidateAndProcess(UPS_INTERFACE_RAM_STRUCT *pDIStruct);
void UPSInterface_XmitNextCmd(UPS_INTERFACE_RAM_STRUCT *pDIStruct);
uint8_t UPSInterface_GetWriteRequestStatus(UPS_INTERFACE_RAM_STRUCT *pDIStruct);
//==================================HTTP Calls============================================//

uint32_t HTTP_ReadSerialDiagnostic(uint16_t Port, uint16_t Index);


//============================================================================
//                              > EXTERN VARIABLES <
//----------------------------------------------------------------------------
extern UPS_INTERFACE_RAM_STRUCT UPSInterfaceStruct;
extern UPS_DB_STRUCT 			UPSDb;


//---------------------------------------------------------------------------------------//
#endif //UPS_INTERFACE_H


