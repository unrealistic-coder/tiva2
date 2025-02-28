src="DLPage.js"
src="EPage.js"
src="FPage.js"
src="Mpage.js"
src="PPage.js"
src="SPage.js"

REQ_STATE_EMPTY = 0
REQ_STATE_FILLING = 1
REQ_STATE_FILLED = 2
REQ_STATE_SENDING = 3
REQ_STATE_SENT = 4

WEBSOCKET_EMPTY = 0
WEBSOCKET_CLOSE = 1
WEBSOCKET_OPEN = 2
WEBSOCKET_ERROR = 3

MAX_READ_SUB_GROUPS = 0
MAX_WRITE_SUB_GROUPS = 0
MAX_RESET_SUB_GROUPS = 0
MAX_EMAIL_SUB_GROUPS = 0

//*********************************************// GLOBLE VARIABLE START //***********************/
var url = "ws://" + window.location.hostname + ":8765";
const enableConsoleLogs = true;
var TimerHandle =0;
var socket = 0;
var WebSocketState = WEBSOCKET_EMPTY;

//***********************************// GLOBLE VARIABLE END //********************************* */
const WEBSOCKET_FUNCTIONCODE = {
	WEBSOCKET_WRITE:			0x00,
	WEBSOCKET_READ:				0x01,
	WEBSOCKET_RESET:			0x02,
	WEBSOCKET_EMAIL_CONFIG:		0x03,
	WEBSOCKET_MAX_FNC_CODE: 	0x04
};

const WEBSOCKET_GROUP =
{
    WEBSOCKET_All_DATA_GROUP:		0x00,
    WEBSOCKET_ALL_SETTING: 			0x01,
    WEBSOCKET_DATALOG_GROUP: 		0x02,
    WEBSOCKET_NO_OF_DATALOGS: 		0x03,
	MAX_READ_GROUPS:				0x04,

    WEBSOCKET_ETHERNET_SETTING: 	0x00,
    WEBSOCKET_SNMP_SETTING: 		0x01,
    WEBSOCKET_MODBUS_TCP_SETTING: 	0x02,
    WEBSOCKET_MON_SETTING: 			0x03,
	WEBSOCKET_COM1_SETTING: 		0x04,
	WEBSOCKET_PROTOCOL_SETTING: 	0x05,
	WEBSOCKET_COM2_SETTING: 		0x06,
	WEBSOCKET_GSMSMS_SETTING: 		0x07,
    WEBSOCKET_FLAG_SETTING: 		0x08,
	WEBSOCKET_SCHEDULAR_SETTING: 	0x09,
	WEBSOCKET_SNMPTRAP_SETTING: 	0x0A,
	WEBSOCKET_DATETIME_SETTING: 	0x0B,
	WEBSOCKET_LOGINTERVAL_SETTING:	0x0C,
	WEBSOCKET_FLAG_SETTING_REFRESH:	0x0D,
	WEBSOCKET_RTC_COMPENSATE_SETTING:0x0E,
	WEBSOCKET_CLEARLOGS_SETTING:      0x0F,
	MAX_WRITE_GROUPS:				0x10,

    WEBSOCKET_COM1_DIAG_SETTING: 		0x00,
	WEBSOCKET_COM2_DIAG_SETTING: 		0x01,
	WEBSOCKET_MODBUSTCP_DIAG_SETTING: 	0x02,
    WEBSOCKET_SNMP_DIAG_SETTING:      	0x03,
    WEBSOCKET_ETHERNET_DIAG_SETTING:  	0x04,
	MAX_RESET_GROUPS:					0x05,

	WEBSOCKET_SAVE_EMAIL_CONFIG:		0x00,
	WEBSOCKET_TEST_EMAIL:				0x01,
	MAX_EMAIL_GROUPS:					0x02,
};
//************************************// STRUCTURES START//************************************* */

let DataRequestFrame = {
	ReqState: REQ_STATE_EMPTY,
	ReqTimeoutCounter:0,
	ReqInterval: 500,

    startByte_1 : 0x55,
    startByte_2 : 0x55,
    Request_id : 0,
    FunctionCode : 0,
    Module : 0,
    subModule : 0,
    Group : 0,
    subGroup : 0,
    datalength : 0,
	Data : [],
	CRC : 0,
};
//***********************************//HEADER STRUCTURES END//************************************* */

//***********************************//STRUCTURES END//************************************* */

let SystemGroupStruct = {
	SystemSrNo : [],
	ClientName : [],
	SystemRating : [],
	Location : [],
	Manufacturer : [],
	BatteryAH : 0,
	SW_VerControl : 0,
	SW_VerDisplay : 0,
	SystemID : 0,
	ModID : 0
};

let EthernetSettingStruct = {
	HostName:[],
	EnableDHCP:0,
	IPAddress:[],
	SubnetMask:[],
	Gateway:[],
	PrimaryDNS:[],
	SecondaryDNS:[],
	MACAddress:[]
};

let HardFaultSettingStruct = {
	TimeStamp_1:[],
	OSVer_1:[],
	BootVer_1:0,
	FaultCode_1:0,
	TimeStamp_2:[],
	OSVer_2:[],
	BootVer_2:0,
	FaultCode_2:0,
	TimeStamp_3:[],
	OSVer_3:[],
	BootVer_3:0,
	FaultCode_3:0
};
let ExceptionFaultSettingStruct = {
	TimeStamp_1:[],
	OSVer_1:[],
	BootVer_1:0,
	FaultCode_1:0,
	FaultAddress_1:0,
	TimeStamp_2:[],
	OSVer_2:[],
	BootVer_2:0,
	FaultCode_2:0,
	FaultAddress_2:0,
	TimeStamp_3:[],
	OSVer_3:[],
	BootVer_3:0,
	FaultCode_3:0,
	FaultAddress_3:0
};

let AlarmandStatusStruct = {
	Alarms_1:0,
	Alarms_2:0,
	Alarms_3:0,
	Alarms_4:0,
	Status_1:0,
	Status_2:0,
	Status_3:0,
	Status_4:0
};

let SNMPSettingStruct = {
	TrapIPAddress:[],
	Port:0
};

let MODBUSSettingStruct = {
	SocketTimeOut:0,
	Port:0,
	ClientConnected:0
};

let COM1settingStruct = {
	COM1_Protocol:0,
	COM1_BaudRate:0,
	COM1_DataBits:0,
	COM1_StopBit:0,
	COM1_ParityBit:0,
	COM1_Media:0,
};

let COM1DiagnosticStruct = {
	COM1_goodTransaction:0,
	COM1_badTransaction:0,
	COM1_ReqTimedout:0,
	COM1_framesRecv:0,
	COM1_frameSent:0,
	COM1_DumpChars:0,
	COM1_charSent:0,
	COM1_charRecv:0,
	COM1_cntrlCmdPassCntr:0,
	COM1_cntrlCmdFailCntr:0,
};

let COM2settingStruct = {
	COM2_Protocol:0,
	COM2_BaudRate:0,
	COM2_DataBits:0,
	COM2_StopBit:0,
	COM2_ParityBit:0,
	COM2_Media:0,

};

let COM2DiagnosticStruct = {
	COM2_CharReceive:0,
	COM2_CharSent:0,
	COM2_FramesReceive:0,
	COM2_FramesSent:0,
	COM2_SMSSent:0,
	COM2_SMSFail:0,
	COM2_ModemError:0,
	COM2_ModemConnect:0,
	COM2_ResponseTimeout:0,
	COM2_ResponseWaitTimeout:0,
};

let DataLogintervalStruct = {
	SetDataLogInterval:0,
	Updated:0
};

let ProtocolsettingStruct = {
	SlaveId:0,
	ResponseTimeout:0,
	InterframeDelay:0
};

let GSMandGroupSettingsStruct = {
	MobileNo_1:[],
	MobileNo_2:[],
	MobileNo_3:[],
	MobileNo_4:[],
	MobileNo_5:[],
	AllGroup:0,
};

let DiagnosticParameterStruct = {
	ClientConnected:0,
	Client1:[],
	Client2:[],
	Client3:[]
}
let RTCCompensateSettingStruct = 
{
	Updated:0,
	RTCCompenEnable :0,
    NTPServer:[],
	RTCCompensateInterval:0,
	RTCCompensateTime:0
}
let FlagSettingStruct = {
	Updated:0,
	AudibleAlarm:0,
	BattModeAudibleWarning:0,
	ColdStart:0,
	BattOpenStatusCheck:0,
	HighEfficiencyMode:0,
	bypassForbiding:0,
	EnergySaving:0,
	ShortRestart3Times:0,
	InverterShortClearFn:0,
	OutputSocket1InBattMode:0,
	OutputSocket2InBattMode:0,
	SiteFaultDetect:0,
	SetHotStandby:0,
	DeepHighEfficiencyMode:0,
	BypasWhenUPSTurnOff:0,
	BypassAudibleWarning:0,
	ConstantPhaseAngleFn:0,
	AutoRestart:0,
	BattDeepDischProtect:0,
	BattLowProtect:0,
	FreeRunFunction:0,
	ConverterMode:0,
	limitedRuntimeOnBatt:0,
	OutputParallelFn:0,
	PhaseAutoAdapt:0,
	PeriodSelfTest:0,
	WEBEmail_Enable:0,
	RefreshFlagSetting:0
};

let VersionAndDateTime_Struct = {
	OS_Ver:[],
	Boot_Ver:0,
	Control_Ver:[],
	SysDate:0,
	Month:0,
	Year:0,
	Hours:0,
	Minutes:0,
	Seconds:0
};
let SNMPTrapSettingStruct = {
	TRAP1_BATTERY_OPEN:0,
	TRAP2_IP_N_LOSS:0,
	TRAP3_IP_SITE_FAIL:0,
	TRAP4_LINEPHASEERROR:0,
	TRAP5_BYPASSPHASEERROR:0,
	TRAP6_BYPASSFREQUENCYUNSTABLE:0,
	TRAP7_BATT_OVERCHARGE:0,
	TRAP8_BATTLOW:0,
	TRAP9_OVER_LOADWARNING:0,
	TRAP10_FAN_LOCKWARNING:0,
	TRAP11_EPO_ACTIVE:0,
	TRAP12_TURN_ON_ABNORMAL:0,
	TRAP13_OVER_TEMPERATURE:0,
	TRAP14_CHARGER_FAIL:0,
	TRAP15_REMOTE_SHUTDOWN:0,
	TRAP16_L1_IP_FUSE_FAIL:0,
	TRAP17_L2_IP_FUSE_FAIL:0,
	TRAP18_L3_IP_FUSE_FAIL:0,
	TRAP19_L1_PFC_PERROR:0,
	TRAP20_L1_PFC_NERROR:0,
	TRAP21_L2_PFC_PERROR:0,
	TRAP22_L2_PFC_NERROR:0,
	TRAP23_L3_PFC_PERROR:0,
	TRAP24_L3_PFC_NERROR:0,
	TRAP25_CAN_COMM_ERROR:0,
	TRAP26_SYNCH_LINE_ERROR:0,
	TRAP27_SYNCH_PULSE_ERROR:0,
	TRAP28_HOST_LINE_ERROR:0,
	TRAP29_MALE_CONN_ERROR:0,
	TRAP30_FEMALE_CONN_ERROR:0,
	TRAP31_PARALLEL_LINE_CONNERROR:0,
	TRAP32_BATT_CONNDIFF:0,
	TRAP33_LINE_CONNDIFF:0,
	TRAP34_BYPASS_CONNDIFF:0,
	TRAP35_MODE_TYPEDIFF:0,
	TRAP36_PARALLEL_INV_VOLTDIFF:0,
	TRAP37_PARALLEL_OUTPUT_FREQDIFF:0,
	TRAP38_BATT_CELL_OVERCHARGE:0,
	TRAP39_PARALLEL_OUTPUT_PARALLELDIFF:0,
	TRAP40_PARALLEL_OUTPUT_PHASEDIFF:0,
	TRAP41_PARALLEL_BYPASS_FORBIDDENDIFF:0,
	TRAP42_PARALLEL_CONVERTER_ENABLEDIFF:0,
	TRAP43_PARALLEL_BYP_FREQ_HLD:0,
	TRAP44_PARALLEL_BYP_FREQ_LLD:0,
	TRAP45_PARALLEL_BYP_VOLT_HLD:0,
	TRAP46_PARALLEL_BYP_VOLT_LLD:0,
	TRAP47_PARALLEL_LINE_FREQ_HLD:0,
	TRAP48_PARALLEL_LINE_FREQ_LLD:0,
	TRAP49_PARALLEL_LINE_VOLT_HLD:0,
	TRAP50_PARALLEL_LINE_VOLT_LLD:0,
	TRAP51_LOCKED_IN_BYPASS:0,
	TRAP52_THREE_PHASE_CURRENT_UNBALANCE:0,
	TRAP53_BATTERY_FUSE_BROKEN:0,
	TRAP54_INV_CURRENT_UNBALANCE:0,
	TRAP55_P1_CUTOFF_PREALARM:0,
	TRAP56_BATTERY_REPLACE:0,
	TRAP57_INPUT_PHAS_EERROR:0,
	TRAP58_COVER_OF_MAINTAIN_SW_OPEN:0,
	TRAP59_PHASE_AUTO_ADAPTFAILED:0,
	TRAP60_UTILITY_EXTREMELY_UNBALANCED :0,
	TRAP61_BYPASS_UNSTABLE:0,
	TRAP62_EEPROM_ERROR:0,
	TRAP63_PARALLEL_PROTECT_WARNING :0,
	TRAP64_DISCHARGER_OVERLY:0,

};

let AllParameterPageStruct =
{
	protocolID:0,
	UPSManufacturer:[],
	UPSModel:[],
	RatedOutputVA:0,
	RatedOuputVolt:0,
	RatedOuputCurr:0,
	RatedOuputFreq1:0,
	RatedBattVolt:0,
	InputPhase:0,
	OutputPhase:0,
	NominalInputVolt:0,
	NominalOutputVolt:0,
	BatteryTestType:0,
	BattVolt:0,
	BattPieceNumber:0,
	BattVoltPerUnit:0,
	BattgroupNumber:0,
	BattCapacity:0,
	BattRemainTime:0,
	PBusVolt:0,
	NBusVolt:0,
	PBattVolt:0,
	NBattVolt:0,
	BatNumberInParallel:0,
	BatNumberInSeries:0,
	UPSMode:0,
	UPSCPUFwVersion:[],
	type:0,
	subType:0,
	VAType:0,
	H_LV_Type:0,
	year:0,
	month:0,
	ManufacturerID:0,
	serialNumber:[],
	InputVolt:0,
	InputFreq:0,
	OutputVolt:0,
	OutputCurr:0,
	OutputFreq:0,
	OutputPWRFactor:0,
	OutputLoadPercent:0,
	bypassFreq:0,
	MaxTemp:0,
	bypassVoltHighLossPt:0,
	bypassVoltLowLossPt:0,
	voltHighLossPt:0,
	voltLowLossPt:0,
	freqHighLossPt:0,
	freqLowLossPt:0,
	statusAll:0,
	flagAll:0,
	warningAll:0,
	FaultType:0,
	InputVoltBeforeFault:0,
	InputFreqBeforeFault:0,
	OutputVoltBeforeFault:0,
	OutputFreqBeforeFault:0,
	OutputLoadBeforeFault:0,
	OutputCurrentBeforeFault:0,
	PBusVoltageBeforeFault:0,
	NBusVoltBeforeFault:0,
	BattVoltBeforeFault:0,
	TempBeforeFault:0,
	runningStatusAll:0,

	goodTransaction:0,
	badTransaction:0,
	ReqTimedout:0,
	framesRecv:0,
	frameSent:0,
	DumpChars:0,
	charSent:0,
	charRecv:0,
	cntrlCmdPassCntr:0,
	cntrlCmdFailCntr:0,

	SNMPgoodTransaction:0,
	SNMPbadTransaction:0,
	SNMPReqTimedout:0,
	SNMPframesRecv:0,
	SNMPframeSent:0,
	SNMPDumpChars:0,
	SNMPcharSent:0,
	SNMPcharRecv:0,
	SNMPcntrlCmdPassCntr:0,
	SNMPcntrlCmdFailCntr:0,
	framesRecv:0,
	frameSent:0,
	goodTransaction:0,
	goodException:0,
	clientConnections:0,
	otherError:0,
	linkxmit:0,
	linkrecv:0,
	tcpxmit:0,
	tcprecv	:0,
	udpxmit:0,
	udprecv	:0,
};

 var EmailStruct = {
	Updated:0,
    toUser1:[],
	toUser2:[],
	toUser3:[],
    SmtpUser:[],
    Smtppassword:[],
    SmtpHost:[],
    SmtpPort:0,
    SmtpSSL:false,
    EmailEvent:0,
	EmailSent:0,
	EmailFail:0,
	EmailState:0,
	EmailInterval:0,
};

var DataBuffer = [];
//*********************************************// STRUCTURES END //******************************/

//***********************************// FUNCTION START //*************************************** */
function sleep(milliseconds)
{
    return new Promise(resolve => setTimeout(resolve, milliseconds));
}

function ConsoleMessage(message) {
    if (enableConsoleLogs) {
        console.log(message);
    }
}

//******************************// GET Read All Data & Setting REQUEST //******************************* */
function ReadAllData_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;
	DataRequestFrame.ReqTimeoutCounter = 0;

	DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ;
	DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_All_DATA_GROUP;
	DataRequestFrame.datalength = 0x00;

    DataRequestFrame.ReqTimeoutCounter=0;
    DataRequestFrame.ReqState = REQ_STATE_FILLED;
}

function ReadSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_ALL_SETTING;
	DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ReadAllSettingAfterTimeout()
{
	let WaitCounter = 0;
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
		//Fill The Request
		ReadSetting_Request();
	}
	
}
//******************************// GET Read All Data & Setting Response //******************************* */
function ReadAllData_Response(ParaBuf)
{
	 let i=0;
     let indx=16;
	 AllParameterPageStruct.ProtocolID= ParaBuf.getUint16(indx, true);
	 indx=18;
	 for(i=0;i<26;i++)
	 AllParameterPageStruct.UPSManufacturer[i]=ParaBuf.getUint8(indx+i, true);
 	 indx=44;//18+26;
	 for(i=0;i<16;i++)
	 AllParameterPageStruct.UPSModel[i]=ParaBuf.getUint8(indx+i, true);

	 indx=60;//44+16;
	 for(i=0;i<15;i++)
	 AllParameterPageStruct.UPSCPUFwVersion[i]=ParaBuf.getUint8(indx+i, true);
	 indx=75;//60+15;
	AllParameterPageStruct.RatedOutputVA=ParaBuf.getUint32(75, true);
	AllParameterPageStruct.RatedOuputVolt=ParaBuf.getUint16(79, true);
	AllParameterPageStruct.RatedOuputCurr=ParaBuf.getUint16(81, true);
	AllParameterPageStruct.RatedOuputFreq=ParaBuf.getUint16(83, true);
	AllParameterPageStruct.RatedBattVolt=ParaBuf.getUint16(85, true);

	AllParameterPageStruct.InputPhase=ParaBuf.getUint8(87, true);
	AllParameterPageStruct.OutputPhase=ParaBuf.getUint8(88, true);

	AllParameterPageStruct.NominalInputVolt=ParaBuf.getUint16(89, true);

	AllParameterPageStruct.NominalOutputVolt=ParaBuf.getUint16(91, true);

	AllParameterPageStruct.BatteryTestType=ParaBuf.getUint16(93, true);

	AllParameterPageStruct.BattVolt=ParaBuf.getUint16(95, true);

	AllParameterPageStruct.BattPieceNumber=ParaBuf.getUint16(97, true);

	AllParameterPageStruct.BattVoltPerUnit=ParaBuf.getUint16(99, true);

	AllParameterPageStruct.BattgroupNumber=ParaBuf.getUint16(101, true);

	AllParameterPageStruct.BattCapacity=ParaBuf.getUint16(103, true);

	AllParameterPageStruct.BattRemainTime=ParaBuf.getUint16(105, true);

	AllParameterPageStruct.PBusVolt=ParaBuf.getUint16(107, true);
	AllParameterPageStruct.NBusVolt=ParaBuf.getUint16(109, true);
	AllParameterPageStruct.PBattVolt=ParaBuf.getUint16(111, true);
	AllParameterPageStruct.NBattVolt=ParaBuf.getUint16(113, true);

	AllParameterPageStruct.BatNumberInParallel=ParaBuf.getUint16(115, true);
	AllParameterPageStruct.BatNumberInSeries=ParaBuf.getUint16(117, true);
	AllParameterPageStruct.UPSMode=ParaBuf.getUint16(119, true);

	AllParameterPageStruct.type=ParaBuf.getUint8(121, true);
	AllParameterPageStruct.subType=ParaBuf.getUint8(122, true);
	AllParameterPageStruct.VAType=ParaBuf.getUint8(123, true);
	AllParameterPageStruct.H_LV_Type=ParaBuf.getUint8(124, true);
	AllParameterPageStruct.year=ParaBuf.getUint16(125, true);
	AllParameterPageStruct.month=ParaBuf.getUint16(127, true);

	AllParameterPageStruct.ManufacturerID=ParaBuf.getUint8(129, true);

	AllParameterPageStruct.serialNumber[0]=ParaBuf.getUint8(130, true);
	AllParameterPageStruct.serialNumber[1]=ParaBuf.getUint8(131, true);
	AllParameterPageStruct.serialNumber[2]=ParaBuf.getUint8(132, true);
	AllParameterPageStruct.serialNumber[3]=ParaBuf.getUint8(133, true);
	AllParameterPageStruct.serialNumber[4]=ParaBuf.getUint8(134, true);

	AllParameterPageStruct.InputVolt=ParaBuf.getUint16(135, true);

	AllParameterPageStruct.InputFreq=ParaBuf.getUint16(137, true);

	AllParameterPageStruct.OutputVolt=ParaBuf.getUint16(139, true);

	AllParameterPageStruct.OutputFreq=ParaBuf.getUint16(141, true);

	AllParameterPageStruct.OutputCurr=ParaBuf.getUint16(143, true);

	AllParameterPageStruct.OutputPWRFactor=ParaBuf.getUint16(145, true);

	AllParameterPageStruct.OutputLoadPercent=ParaBuf.getUint16(147, true);
	AllParameterPageStruct.bypassFreq=ParaBuf.getUint16(149, true);
	AllParameterPageStruct.MaxTemp=ParaBuf.getUint16(151, true);
	AllParameterPageStruct.bypassVoltHighLossPt=ParaBuf.getUint16(153, true);
	AllParameterPageStruct.bypassVoltLowLossPt=ParaBuf.getUint16(155, true);
	AllParameterPageStruct.voltHighLossPt=ParaBuf.getUint16(157, true);
	AllParameterPageStruct.voltLowLossPt=ParaBuf.getUint16(159, true);
	AllParameterPageStruct.freqHighLossPt=ParaBuf.getUint16(161, true);
	AllParameterPageStruct.freqLowLossPt=ParaBuf.getUint16(163, true);


	//AllParameterPageStruct.statusAll=ParaBuf.getUint16(165, true);
	AllParameterPageStruct.Status_1=ParaBuf.getUint16(165, true);


	AlarmandStatusStruct.Alarms_1 =ParaBuf.getUint16(167, true);// AllParameterPageStruct.warningAll;//ParaBuf.getUint32(200, true);
	AlarmandStatusStruct.Alarms_2 =ParaBuf.getUint16(169, true);
	AlarmandStatusStruct.Alarms_3 =ParaBuf.getUint16(171, true);// AllParameterPageStruct.warningAll;//ParaBuf.getUint32(200, true);
	AlarmandStatusStruct.Alarms_4 =ParaBuf.getUint16(173, true);
	
	AllParameterPageStruct.FaultType=ParaBuf.getUint16(175, true);
	AllParameterPageStruct.InputVoltBeforeFault=ParaBuf.getUint16(177, true);
	AllParameterPageStruct.InputFreqBeforeFault=ParaBuf.getUint16(179, true);
	AllParameterPageStruct.OutputVoltBeforeFault=ParaBuf.getUint16(181, true);
	AllParameterPageStruct.OutputFreqBeforeFault=ParaBuf.getUint16(183, true);
	AllParameterPageStruct.OutputLoadBeforeFault=ParaBuf.getUint16(185, true);
	AllParameterPageStruct.OutputCurrentBeforeFault=ParaBuf.getUint16(187, true),
	AllParameterPageStruct.PBusVoltageBeforeFault=ParaBuf.getUint16(189, true);
	AllParameterPageStruct.NBusVoltBeforeFault=ParaBuf.getUint16(191, true);
	AllParameterPageStruct.BattVoltBeforeFault=ParaBuf.getUint16(193, true);
	AllParameterPageStruct.TempBeforeFault=ParaBuf.getUint16(195, true);

	AllParameterPageStruct.runningStatusAll=ParaBuf.getUint16(197, true);

	AllParameterPageStruct.charRecv=ParaBuf.getUint32(199, true);
	AllParameterPageStruct.charSent=ParaBuf.getUint32(203, true);
	AllParameterPageStruct.framesRecv=ParaBuf.getUint32(207, true);
	AllParameterPageStruct.frameSent=ParaBuf.getUint32(211, true);
	AllParameterPageStruct.DumpChars=ParaBuf.getUint32(215, true);
	AllParameterPageStruct.goodTransaction=ParaBuf.getUint32(219, true);
	AllParameterPageStruct.badTransaction=ParaBuf.getUint32(223, true);
	AllParameterPageStruct.ReqTimedout=ParaBuf.getUint32(227, true);
	AllParameterPageStruct.cntrlCmdPassCntr=ParaBuf.getUint32(231, true);
	AllParameterPageStruct.cntrlCmdFailCntr=ParaBuf.getUint32(235, true);
////////////////////////////////////////////////////////////////////////////////////
	COM1DiagnosticStruct.COM1_charRecv = ParaBuf.getUint32(199,true);
	COM1DiagnosticStruct.COM1_charSent = ParaBuf.getUint32(203,true);
	COM1DiagnosticStruct.COM1_framesRecv = ParaBuf.getUint32(207,true);
	COM1DiagnosticStruct.COM1_frameSent = ParaBuf.getUint32(211,true);
	COM1DiagnosticStruct.COM1_DumpChars = ParaBuf.getUint32(215,true);
	COM1DiagnosticStruct.COM1_goodTransaction = ParaBuf.getUint32(219,true);
	COM1DiagnosticStruct.COM1_badTransaction = ParaBuf.getUint32(223,true);
	COM1DiagnosticStruct.COM1_ReqTimedout = ParaBuf.getUint32(227,true);
	COM1DiagnosticStruct.COM1_cntrlCmdPassCntr = ParaBuf.getUint32(231,true);
	COM1DiagnosticStruct.COM1_cntrlCmdFailCntr = ParaBuf.getUint32(235,true);

	COM2DiagnosticStruct.COM2_CharReceive=ParaBuf.getUint32(239,true);
	COM2DiagnosticStruct.COM2_CharSent=ParaBuf.getUint32(243,true);
	COM2DiagnosticStruct.COM2_FramesReceive=ParaBuf.getUint32(247,true);
	COM2DiagnosticStruct.COM2_FramesSent=ParaBuf.getUint32(251,true);
	COM2DiagnosticStruct.COM2_SMSSent=ParaBuf.getUint32(255,true);
	COM2DiagnosticStruct.COM2_SMSFail=ParaBuf.getUint32(259,true);
	COM2DiagnosticStruct.COM2_ModemError=ParaBuf.getUint32(263,true);
	COM2DiagnosticStruct.COM2_ModemConnect=ParaBuf.getUint32(267,true);
	COM2DiagnosticStruct.COM2_ResponseTimeout=ParaBuf.getUint32(271,true);
	COM2DiagnosticStruct.COM2_ResponseWaitTimeout=ParaBuf.getUint32(275,true);
//////////////////////////////////////////////////////////////////////////////////
	AllParameterPageStruct.framesRecv=ParaBuf.getUint32(279, true);
	AllParameterPageStruct.frameSent=ParaBuf.getUint32(283, true);
	AllParameterPageStruct.goodTransaction=ParaBuf.getUint32(287, true);
	AllParameterPageStruct.goodException=ParaBuf.getUint32(291, true);
	AllParameterPageStruct.clientConnections=ParaBuf.getUint16(295, true);
	AllParameterPageStruct.otherError=ParaBuf.getUint32(299, true);

	for(i=0;i<6;i++)
	{
		ExceptionFaultSettingStruct.TimeStamp_1[i]= ParaBuf.getUint8(303+i);
		ExceptionFaultSettingStruct.TimeStamp_2[i]= ParaBuf.getUint8(309+i);
		ExceptionFaultSettingStruct.TimeStamp_3[i]= ParaBuf.getUint8(315+i);
	}

	ExceptionFaultSettingStruct.OSVer_1[0]= ParaBuf.getUint8(321,true);
	ExceptionFaultSettingStruct.OSVer_1[1]= ParaBuf.getUint8(322,true);
	ExceptionFaultSettingStruct.BootVer_1= ParaBuf.getUint16(323,true);
	ExceptionFaultSettingStruct.FaultCode_1= ParaBuf.getUint16(325,true);
	ExceptionFaultSettingStruct.FaultAddress_1= ParaBuf.getUint32(327,true);

//	ExceptionFaultSettingStruct.OSVer_2= ParaBuf.getUint16(331,true);
	ExceptionFaultSettingStruct.OSVer_2[0]= ParaBuf.getUint8(331,true);
	ExceptionFaultSettingStruct.OSVer_2[1]= ParaBuf.getUint8(332,true);
	ExceptionFaultSettingStruct.BootVer_2= ParaBuf.getUint16(333,true);
	ExceptionFaultSettingStruct.FaultCode_2= ParaBuf.getUint16(335,true);
	ExceptionFaultSettingStruct.FaultAddress_1= ParaBuf.getUint32(337,true);

	//ExceptionFaultSettingStruct.OSVer_3= ParaBuf.getUint16(341,true);
	ExceptionFaultSettingStruct.OSVer_3[0]= ParaBuf.getUint8(341,true);
	ExceptionFaultSettingStruct.OSVer_3[1]= ParaBuf.getUint8(342,true);
	ExceptionFaultSettingStruct.BootVer_3= ParaBuf.getUint16(343,true);
	ExceptionFaultSettingStruct.FaultCode_3= ParaBuf.getUint16(345,true);
	ExceptionFaultSettingStruct.FaultAddress_1= ParaBuf.getUint32(347,true);

	for(i=0;i<6;i++)
	{
		HardFaultSettingStruct.TimeStamp_1[i]= ParaBuf.getUint8(351+i);
		HardFaultSettingStruct.TimeStamp_2[i]= ParaBuf.getUint8(357+i);
		HardFaultSettingStruct.TimeStamp_3[i]= ParaBuf.getUint8(363+i);
	}


	HardFaultSettingStruct.OSVer_1[0]= ParaBuf.getUint8(369,true);
	HardFaultSettingStruct.OSVer_1[1]= ParaBuf.getUint8(370,true) ;
	HardFaultSettingStruct.BootVer_1= ParaBuf.getUint16(371,true);
		
	HardFaultSettingStruct.FaultCode_1= ParaBuf.getUint16(373,true);

//	HardFaultSettingStruct.OSVer_2= ParaBuf.getUint16(375,true);
	HardFaultSettingStruct.OSVer_2[0]= ParaBuf.getUint8(375,true);
	HardFaultSettingStruct.OSVer_2[1]= ParaBuf.getUint8(376,true) ;
	HardFaultSettingStruct.BootVer_2= ParaBuf.getUint16(377,true);
	HardFaultSettingStruct.FaultCode_2= ParaBuf.getUint16(379,true);

	//HardFaultSettingStruct.OSVer_3= ParaBuf.getUint16(381,true);
	HardFaultSettingStruct.OSVer_3[0]= ParaBuf.getUint8(381,true);
	HardFaultSettingStruct.OSVer_3[1]= ParaBuf.getUint8(382,true) ;
	
	HardFaultSettingStruct.BootVer_3= ParaBuf.getUint16(383,true);
	HardFaultSettingStruct.FaultCode_3= ParaBuf.getUint16(385,true);

	DiagnosticParameterStruct.ClientConnected = ParaBuf.getUint8(387);

	for ( i = 0; i < 4; i++)
	{
		DiagnosticParameterStruct.Client1[i] = ParaBuf.getUint8(391-i);
		DiagnosticParameterStruct.Client2[i] = ParaBuf.getUint8(395-i);
		DiagnosticParameterStruct.Client3[i] = ParaBuf.getUint8(399-i);
	}
	for(i=6;i<15;i++)
	{
		VersionAndDateTime_Struct.Control_Ver[i-6]=AllParameterPageStruct.UPSCPUFwVersion[i];
	}
	
	VersionAndDateTime_Struct.OS_Ver[0]=ParaBuf.getUint8(401);
	VersionAndDateTime_Struct.OS_Ver[1]=ParaBuf.getUint8(400);
    VersionAndDateTime_Struct.Boot_Ver=ParaBuf.getUint8(402);

	EmailStruct.EmailSent = ParaBuf.getUint16(403,true);
	EmailStruct.EmailFail = ParaBuf.getUint16(405,true);

	MODBUSSettingStruct.ClientConnected = ParaBuf.getUint8(407, true);
	AlarmandStatusStruct.Status_1= ParaBuf.getUint16(408, true);
	AllParameterPageStruct.crc = ParaBuf.getUint8(408, true);
}

function ReadAllSetting_Response(DataRes)
{
	let itr = 16;
	let i = 0;
	for ( i = 0; i < 16; i++)
	{
		EthernetSettingStruct.HostName[i] = DataRes.getUint8(16 + i);
	}

	EthernetSettingStruct.EnableDHCP = DataRes.getUint8(32);
	for ( i = 0; i < 4; i++)
	{
		EthernetSettingStruct.IPAddress[i] = DataRes.getUint8(33+i);
		EthernetSettingStruct.Gateway[i] = DataRes.getUint8(37+i);
		EthernetSettingStruct.SubnetMask[i] = DataRes.getUint8(41+i);
		EthernetSettingStruct.PrimaryDNS[i] = DataRes.getUint8(45+i);
		EthernetSettingStruct.SecondaryDNS[i] = DataRes.getUint8(49+i);
		SNMPSettingStruct.TrapIPAddress[i] = DataRes.getUint8(59+i);
	}

	for ( i = 0; i < 6; i++)
	{
		EthernetSettingStruct.MACAddress[i] = DataRes.getUint8(53+i);
	}

	SNMPSettingStruct.Port = DataRes.getUint16(63,true);
	MODBUSSettingStruct.SocketTimeOut = DataRes.getUint16(65,true);
	MODBUSSettingStruct.Port = DataRes.getUint16(67,true);

	itr = 69;
	SNMPTrapSettingStruct.TRAP1_BATTERY_OPEN              			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP2_IP_N_LOSS               			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP3_IP_SITE_FAIL              			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP4_LINEPHASEERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP5_BYPASSPHASEERROR            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP6_BYPASSFREQUENCYUNSTABLE     		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP7_BATT_OVERCHARGE             		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP8_BATTLOW              			    = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP9_OVER_LOADWARNING            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP10_FAN_LOCKWARNING            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP11_EPO_ACTIVE              			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP12_TURN_ON_ABNORMAL           		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP13_OVER_TEMPERATURE           		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP14_CHARGER_FAIL              			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP15_REMOTE_SHUTDOWN            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP16_L1_IP_FUSE_FAIL            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP17_L2_IP_FUSE_FAIL            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP18_L3_IP_FUSE_FAIL            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP19_L1_PFC_PERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP20_L1_PFC_NERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP21_L2_PFC_PERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP22_L2_PFC_NERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP23_L3_PFC_PERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP24_L3_PFC_NERROR              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP25_CAN_COMM_ERROR             		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP26_SYNCH_LINE_ERROR           		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP27_SYNCH_PULSE_ERROR          		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP28_HOST_LINE_ERROR            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP29_MALE_CONN_ERROR            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP30_FEMALE_CONN_ERROR          		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP31_PARALLEL_LINE_CONNERROR    		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP32_BATT_CONNDIFF              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP33_LINE_CONNDIFF              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP34_BYPASS_CONNDIFF            		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP35_MODE_TYPEDIFF              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP36_PARALLEL_INV_VOLTDIFF      		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP37_PARALLEL_OUTPUT_FREQDIFF   		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP38_BATT_CELL_OVERCHARGE       		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP39_PARALLEL_OUTPUT_PARALLELDIFF       = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP40_PARALLEL_OUTPUT_PHASEDIFF          = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP41_PARALLEL_BYPASS_FORBIDDENDIFF      = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP42_PARALLEL_CONVERTER_ENABLEDIFF      = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP43_PARALLEL_BYP_FREQ_HLD  = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP44_PARALLEL_BYP_FREQ_LLD   = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP45_PARALLEL_BYP_VOLT_HLD  = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP46_PARALLEL_BYP_VOLT_LLD  = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP47_PARALLEL_LINE_FREQ_HLD   = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP48_PARALLEL_LINE_FREQ_LLD 	= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP49_PARALLEL_LINE_VOLT_HLD   = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP50_PARALLEL_LINE_VOLT_LLD   = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP51_LOCKED_IN_BYPASS              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP52_THREE_PHASE_CURRENT_UNBALANCE      = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP53_BATTERY_FUSE_BROKEN              	= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP54_INV_CURRENT_UNBALANCE              = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP55_P1_CUTOFF_PREALARM              	= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP56_BATTERY_REPLACE              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP63_PARALLEL_PROTECT_WARNING			= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP64_DISCHARGER_OVERLY          		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP59_PHASE_AUTO_ADAPTFAILED             = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP60_UTILITY_EXTREMELY_UNBALANCED       = DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP61_BYPASS_UNSTABLE              		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP62_EEPROM_ERROR             	 		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP57_INPUT_PHAS_EERROR           		= DataRes.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP58_COVER_OF_MAINTAIN_SW_OPEN			= DataRes.getUint8(itr++);

	itr = 133;
	COM1settingStruct.COM1_BaudRate = DataRes.getUint8(itr++);
	COM1settingStruct.COM1_DataBits = DataRes.getUint8(itr++);
	COM1settingStruct.COM1_StopBit = DataRes.getUint8(itr++);
	COM1settingStruct.COM1_ParityBit = DataRes.getUint8(itr++);

	ProtocolsettingStruct.SlaveId = DataRes.getUint16(137,true),
	ProtocolsettingStruct.ResponseTimeout = DataRes.getUint16(139,true),
	ProtocolsettingStruct.InterframeDelay = DataRes.getUint16(141,true),

	itr = 143;
	COM2settingStruct.COM2_BaudRate = DataRes.getUint8(itr++);
	COM2settingStruct.COM2_DataBits = DataRes.getUint8(itr++);
	COM2settingStruct.COM2_StopBit = DataRes.getUint8(itr++);
	COM2settingStruct.COM2_ParityBit = DataRes.getUint8(itr++);
	//COM2settingStruct.COM2_Media = DataRes.getUint8(itr++);//(148);

	GSMandGroupSettingsStruct.MobileNo_1 = DataRes.getBigUint64(147,true);
	GSMandGroupSettingsStruct.MobileNo_2 = DataRes.getBigUint64(155,true);
	GSMandGroupSettingsStruct.MobileNo_3 = DataRes.getBigUint64(163,true);
	GSMandGroupSettingsStruct.MobileNo_4 = DataRes.getBigUint64(171,true);
	GSMandGroupSettingsStruct.MobileNo_5 = DataRes.getBigUint64(179,true);

	GSMandGroupSettingsStruct.AllGroup = DataRes.getUint16(187,true);

	itr=189;
	FlagSettingStruct.AudibleAlarm = DataRes.getUint8(itr++);
	FlagSettingStruct.BattModeAudibleWarning = DataRes.getUint8(itr++);
	FlagSettingStruct.ColdStart = DataRes.getUint8(itr++);
	FlagSettingStruct.BattOpenStatusCheck = DataRes.getUint8(itr++);
	FlagSettingStruct.HighEfficiencyMode = DataRes.getUint8(itr++);
	FlagSettingStruct.bypassForbiding = DataRes.getUint8(itr++);
	FlagSettingStruct.EnergySaving = DataRes.getUint8(itr++);
	FlagSettingStruct.ShortRestart3Times = DataRes.getUint8(itr++);
	FlagSettingStruct.InverterShortClearFn = DataRes.getUint8(itr++);
	FlagSettingStruct.OutputSocket1InBattMode = DataRes.getUint8(itr++);
	FlagSettingStruct.OutputSocket2InBattMode = DataRes.getUint8(itr++);
	FlagSettingStruct.SiteFaultDetect = DataRes.getUint8(itr++);
	FlagSettingStruct.SetHotStandby = DataRes.getUint8(itr++);
	FlagSettingStruct.DeepHighEfficiencyMode = DataRes.getUint8(itr++);
	FlagSettingStruct.BypasWhenUPSTurnOff = DataRes.getUint8(itr++);
	FlagSettingStruct.BypassAudibleWarning = DataRes.getUint8(itr++);
	FlagSettingStruct.ConstantPhaseAngleFn = DataRes.getUint8(itr++);
	FlagSettingStruct.AutoRestart = DataRes.getUint8(itr++);
	FlagSettingStruct.BattDeepDischProtect = DataRes.getUint8(itr++);
	FlagSettingStruct.BattLowProtect = DataRes.getUint8(itr++);
	FlagSettingStruct.FreeRunFunction = DataRes.getUint8(itr++);
	FlagSettingStruct.ConverterMode = DataRes.getUint8(itr++);
	FlagSettingStruct.limitedRuntimeOnBatt = DataRes.getUint8(itr++);
	FlagSettingStruct.OutputParallelFn = DataRes.getUint8(itr++);
	FlagSettingStruct.PhaseAutoAdapt = DataRes.getUint8(itr++);
	FlagSettingStruct.PeriodSelfTest = DataRes.getUint8(itr++);

    DataLogintervalStruct.LogInterval = DataRes.getUint32(215);

	/*for(i=0;i<64;i++)
	{
		EmailStruct.SmtpUser[i]= DataRes.getUint8(219+i);
		EmailStruct.toUser[i]= DataRes.getUint8(283+i);
		EmailStruct.Smtppassword[i]= DataRes.getUint8(347+i);
		EmailStruct.SmtpHost[i]= DataRes.getUint8(411+i);
	} 
	EmailStruct.SmtpPort = DataRes.getUint16(475,true);
	EmailStruct.SmtpSSL = DataRes.getUint8(477);

	EmailStruct.EmailEvent = DataRes.getUint16(478,true);

	FlagSettingStruct.WEBEmail_Enable =DataRes.getUint8(608);// DataRes.getUint8(480);
	EmailStruct.EmailInterval = DataRes.getUint32(481,true);

*/
	for(i=0;i<64;i++)
	{
		EmailStruct.SmtpUser[i]= DataRes.getUint8(219+i);
		EmailStruct.toUser1[i]= DataRes.getUint8(283+i);
		EmailStruct.toUser2[i]= DataRes.getUint8(347+i);
		EmailStruct.toUser3[i]= DataRes.getUint8(411+i);
		EmailStruct.Smtppassword[i]= DataRes.getUint8(475+i);
		EmailStruct.SmtpHost[i]= DataRes.getUint8(539+i);
	} 
	EmailStruct.SmtpPort = DataRes.getUint16(603,true);
	EmailStruct.SmtpSSL = DataRes.getUint8(605);

	EmailStruct.EmailEvent = DataRes.getUint16(606,true);

	FlagSettingStruct.WEBEmail_Enable = DataRes.getUint8(608);
	EmailStruct.EmailInterval = DataRes.getUint32(609,true);

	EmailStruct.Updated = 1;

	RTCCompensateSettingStruct.RTCCompensateInterval=DataRes.getUint8(614);
	RTCCompensateSettingStruct.RTCCompensateTime=DataRes.getUint8(615);
	
	RTCCompensateSettingStruct.RTCCompenEnable = DataRes.getUint8(616);
	for(i=0;i<32;i++)
	{
	
		RTCCompensateSettingStruct.NTPServer[i] = DataRes.getUint8(617+i);
	}
	RTCCompensateSettingStruct.Updated=1;
	EmailStruct.Updated = 1;
	DataLogintervalStruct.Updated =1;
	EthernetSettingStruct.Updated = 1;
	COM1settingStruct.Updated = 1;
	FlagSettingStruct.Updated = 1;
}

function GetHeader_Response(arr)
{
	return {
		"StartByte_1":arr.getUint8(0),
		"StartByte_2":arr.getUint8(1),
		"Request_ID":arr.getUint8(2),
		"FunctionCode":arr.getUint8(3),
		"Module":arr.getUint8(4),
		"SubModule":arr.getUint8(5),
		"Group":arr.getUint8(6),
		"SubGroup":arr.getUint8(7),
		"Date":arr.getUint8(8),
		"Month":arr.getUint8(9),
		"Year":arr.getUint8(10),
		"Hours":arr.getUint8(11),
		"Minutes":arr.getUint8(12),
		"Seconds":arr.getUint8(13),
		"Datalength":arr.getUint16(14)
	}
}

function ResetGroupFunction(ResetGroups, ResponseData)
{
	switch (ResetGroups)
	{
		case WEBSOCKET_GROUP.WEBSOCKET_COM1_DIAG_SETTING:
			ResetCOM1Diag_Response();
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_COM2_DIAG_SETTING:
			ResetCOM2Diag_Response();
		break;
	}
}

function ReadGroupFunction(ReadGroups, ResponseData)
{
	switch (ReadGroups)
	{
		case WEBSOCKET_GROUP.WEBSOCKET_All_DATA_GROUP:
			ReadAllData_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_ALL_SETTING:
			ReadAllSetting_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_DATALOG_GROUP:
			DataLog_ResponseHandler(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_NO_OF_DATALOGS:
			NoOfDataLog_ResponseHandler(ResponseData);
		break;
		case WEBSOCKET_LOGINTERVAL_SETTING:
			WriteDataloginterval_Response(ResponseData);
		break;
	}
}
function WriteGroupFunction(WriteGroup, ResponseData)
{
	switch (WriteGroup)
	{
		case WEBSOCKET_GROUP.WEBSOCKET_ETHERNET_SETTING:
			WriteEthernet_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_SNMP_SETTING:
			WriteSNMP_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_MODBUS_TCP_SETTING:
			WriteMODbus_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_MON_SETTING:
			WriteMON_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_COM1_SETTING:
			WriteCOM1_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_PROTOCOL_SETTING:
			WriteProtocol_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_COM2_SETTING:
			WriteCOM2_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_GSMSMS_SETTING:
			WriteGSMGroup_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_FLAG_SETTING:
		case WEBSOCKET_GROUP.WEBSOCKET_FLAG_SETTING_REFRESH:
			WriteGroupEnabled_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_SCHEDULAR_SETTING:
			WriteSchedule_Response(ResponseData);
		break;

		case WEBSOCKET_GROUP.WEBSOCKET_SNMPTRAP_SETTING:
			WriteSNMPTrap_Response(ResponseData);
		break;
		case WEBSOCKET_GROUP.WEBSOCKET_RTC_COMPENSATE_SETTING:
			WriteRTCComepnsate_Response(ResponseData);
		break;
	}
}

function EmailConfigFunction(EmailGroup, ResponseData)
{
	switch (EmailGroup) 
	{
		case WEBSOCKET_GROUP.WEBSOCKET_SAVE_EMAIL_CONFIG:
			EmailSaveConfig_Response(ResponseData);
		break;
		
		case WEBSOCKET_GROUP.WEBSOCKET_TEST_EMAIL:
			EmailTest_Response(ResponseData);
		break;

	}
}

function SendWebsocket_Request()
{
	let arr = new ArrayBuffer(DataRequestFrame.datalength+11);
	let buffer = new Uint8Array(arr);
	let	itr=0;

	DataRequestFrame.ReqState = REQ_STATE_SENDING;
	DataRequestFrame.Request_id = Math.floor(Math.random() * 255);

	buffer[itr++] = DataRequestFrame.startByte_1;	//start byte 1
	buffer[itr++] = DataRequestFrame.startByte_2;	//start byte 2
	buffer[itr++] = DataRequestFrame.Request_id;
	buffer[itr++] = DataRequestFrame.FunctionCode;	//Function Code
	buffer[itr++] = DataRequestFrame.Module;		//Module
	buffer[itr++] = DataRequestFrame.subModule;		//Sub Module
	buffer[itr++] = DataRequestFrame.Group;			//Group
	buffer[itr++] = DataRequestFrame.subGroup;		//Subgroup //13

	buffer[itr++] = DataRequestFrame.datalength & 0xFF;          	// Low Byte
	buffer[itr++] = (DataRequestFrame.datalength >> 8) & 0xFF;		//High Byte  //15

	for (var i = 0; i < DataRequestFrame.datalength; i++)
	{
	buffer[itr+i] = DataRequestFrame.Data[i];
	}

	buffer[itr+i] = DataRequestFrame.CRC;
	if(socket.readyState == WebSocket.OPEN)
	{
		socket.send(arr);
	}
	DataRequestFrame.ReqState = REQ_STATE_SENT;
}

function ValidateResponse(header)
{
	let ret=1;
	if((header.StartByte_1 == DataRequestFrame.startByte_1) &&
		(header.StartByte_2 == DataRequestFrame.startByte_2) &&
		(header.Module <= DataRequestFrame.Module) &&
		(header.SubModule <= DataRequestFrame.subModule))
	{
		if(header.FunctionCode < WEBSOCKET_FUNCTIONCODE.WEBSOCKET_MAX_FNC_CODE)
		{
			switch (header.FunctionCode)
			{
				case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ:
					if(header.Group < WEBSOCKET_GROUP.MAX_READ_GROUPS)
					{
						if(header.SubGroup <= MAX_READ_SUB_GROUPS)
						{
							ret=0;
						}
					}
					break;
				case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE:
					if(header.Group < WEBSOCKET_GROUP.MAX_WRITE_GROUPS)
					{
						if(header.SubGroup <= MAX_WRITE_SUB_GROUPS)
						{
							ret=0;
						}
					}
					break;
				case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET:
					if(header.Group < WEBSOCKET_GROUP.MAX_RESET_GROUPS)
					{
						if(header.SubGroup <= MAX_RESET_SUB_GROUPS)
						{
							ret=0;
						}
					}
					break;
				case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_EMAIL_CONFIG:
					if(header.Group < WEBSOCKET_GROUP.MAX_EMAIL_GROUPS)
					{
						if(header.SubGroup <= MAX_EMAIL_SUB_GROUPS)
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

function handleWebSocket_Request()
{
	if (WebSocketState === WEBSOCKET_OPEN)
	{
		if(DataRequestFrame.ReqState == REQ_STATE_EMPTY)
		{
			ReadAllData_Request();
			SendWebsocket_Request();
		}
		else if(DataRequestFrame.ReqState == REQ_STATE_FILLED)
		{
			SendWebsocket_Request();
		}
		else if(DataRequestFrame.ReqState == REQ_STATE_SENT)
		{
			DataRequestFrame.ReqTimeoutCounter++;
			if((DataRequestFrame.ReqTimeoutCounter * DataRequestFrame.ReqInterval) >= 20000 )
			{
				DataRequestFrame.ReqState = REQ_STATE_EMPTY;
				DataRequestFrame.ReqTimeoutCounter = 0;

				document.getElementById("AlarmREADData").disabled = false;
				document.getElementById("AlarmREADData").style.backgroundColor = "#6699ff";

				document.getElementById("READData").disabled = false;
				document.getElementById("READData").style.backgroundColor = "#6699ff";
			}
		}
	}
	else if(WebSocketState == WEBSOCKET_CLOSE)
	{
		SocketCreation();
	}
}

//***********************************************************************************************
function CloseSocket()
{
	if (WebSocketState == WEBSOCKET_OPEN)
	{
		socket.close();
		WebSocketState = WEBSOCKET_CLOSE;
		ConsoleMessage("socket closed by user");
	} else {
		ConsoleMessage("WebSocket is not open.");
	}
}
//=======================================/ SOCKET CREATION /====================================== */
function WS_SocketError(error)
{
	WebSocketState = WEBSOCKET_ERROR;
	ConsoleMessage(`Socket Error ${error.message}`);

}

function WS_SocketClose()
{
	ConsoleMessage('WebSocket closed');
	WebSocketState = WEBSOCKET_CLOSE;
	socket = null;
}

function WS_SocketMessage(event)
{
	var Request_id = 0;
	var Error_OK = 1;
	if(DataRequestFrame.ReqState==REQ_STATE_SENT)
	{
		DataRequestFrame.ReqState=REQ_STATE_EMPTY;
		DataRequestFrame.ReqTimeoutCounter=0;
		Request_id = DataRequestFrame.Request_id ;

		var blob = event.data;

		if (typeof blob === 'string')
		{
			//string data
			ConsoleMessage('WebSocket string message');
		}
		else
		{
			const uint8Array = new Uint8Array(blob);
			const HEXValues = new DataView(uint8Array.buffer);
			DataBuffer = HEXValues;
			let ReqResponse = GetHeader_Response(HEXValues);
			if(Request_id == ReqResponse.Request_ID)
			{
				DateAndTime(ReqResponse);
				BootOsControlVersionPrint();

				Error_OK = ValidateResponse(ReqResponse);
				if(Error_OK==0)
				{
					switch (DataRequestFrame.FunctionCode)
					{
						case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET:
							ResetGroupFunction(ReqResponse.Group, HEXValues);
							break;

						case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ:
							ReadGroupFunction(ReqResponse.Group, HEXValues);
							break;

						case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE:
							WriteGroupFunction(ReqResponse.Group, HEXValues);
							break;

						case WEBSOCKET_FUNCTIONCODE.WEBSOCKET_EMAIL_CONFIG:
							EmailConfigFunction(ReqResponse.Group, HEXValues);	
							break;
					}
				}
				else
				{
					console.log("ERROR ")
					ConsoleMessage(`			Function Code: ${DataBuffer.getUint8(3)}
					Module: ${DataBuffer.getUint8(4)}
					Submodule: ${DataBuffer.getUint8(5)}
					Group: ${DataBuffer.getUint8(6)}
					SubGroup: ${DataBuffer.getUint8(7)}
					Datalength: ${DataBuffer.getUint16(14)}
					Error: ${DataBuffer.getUint8(16)}`);
				}
			}
		}
	}
}

function WS_SocketOpen()
{
	ConsoleMessage('WebSocket opened');
	WebSocketState = WEBSOCKET_OPEN;
	ReadSetting_Request();
}

function SocketCreation()
{
	if((WebSocketState != WEBSOCKET_OPEN))
	{
		socket = new WebSocket(url);
		socket.binaryType = "arraybuffer";
	}

	socket.onopen = WS_SocketOpen;

	socket.onmessage = WS_SocketMessage;

	socket.onclose = WS_SocketClose;

	socket.onerror = WS_SocketError;
}
//=======================================// SOCKET CREATION //============================== */

//****************************************************************************************** */

TimerHandle = setInterval(handleWebSocket_Request, DataRequestFrame.ReqInterval);

//****************************************************************************************** */
//**************************************// END FILE //************************************* */


















