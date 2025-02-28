src="WebSocket.js"

let NewCOM1settingStruct = {
	COM1_Protocol:0,
	COM1_BaudRate:0,
	COM1_DataBits:0,
	COM1_StopBit:0,
	COM1_ParityBit:0,
	COM1_Media:0,
}

let NewCOM1DiagnosticStruct =
{
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
}
let NewCOM2settingStruct = {
	COM2_Protocol:0,
	COM2_BaudRate:0,
	COM2_DataBits:0,
	COM2_StopBit:0,
	COM2_ParityBit:0,
	COM2_Media:0,
}

let NewCOM2DiagnosticStruct = {
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
}

let NewProtocolsettingStruct = {
	SlaveId:0,
	ResponseTimeout:0,
	InterframeDelay:0
}

let NewGSMandGroupSettingsStruct = {
	MobileNo_1:0,
	MobileNo_2:0,
	MobileNo_3:0,
	MobileNo_4:0,
	MobileNo_5:0,
	AllGroup:0,

}

function MouseOver(e,divid,st){
	var left  = e.clientX + 10  + "px";
    var top  = e.clientY  + -120 + "px";

    var div = document.getElementById(divid);
	document.getElementById("divtoshow").style.display = "block";
    div.style.left = left;
    div.style.top = top;

	let group1=["Battery Switch Open","IP Neutral Loss","IP Site Fail", "Line Phase Error", "Bypass Phase Error",
				"Bypass Frequency Unstable","Battery Over Charge ","Battery Low","Over load Warning","Fan Lock Warning",
				"EPO Active","Turn On Abnormal","Over Temperature","Charger Fail","Remote Shutdown","L1 IP Fuse Fail"];

	let group2=["L2 IP Fuse Fail","L3 IP Fuse Fail","L1 PFC PError","L1 PFC NError","L2 PFC PError","L2 PFC NError",
				"L3 PFC PError","L3 PFC NError","CAN Comm Error","Synch Line Error","Synch Pulse Error", "Host Line Error",
				"Male Conn Error","Female Conn Error", "Parallel Line Conn Error","BattConn Diff"];

	let group3=["Parallel LineVoltHighLoss Diff","Bypass Conn Diff","Mode Type Diff","Parallel INV_Volt Diff",
				"Parallel OutputFreqD iff","BattCell OverCharge","Parallel OutputParallel Diff","Parallel OutputPhase Diff",
				"Parallel BypassForbidden Diff","Parallel ConverterEnable Diff","Parallel BypassFreqHighLoss Diff",
				"Parallel BypassFreqLowLoss Diff","Parallel BypassVoltHighLoss Diff", "Parallel BypassVoltLowLoss Diff"];

	let group4=["Parallel LineVoltHighLoss Diff","Parallel Line Volt", "LowLoss Diff","Locked In Bypass",
				"Three Phase Current Unbalance","Battery Fuse Broken","INV Current Unbalance","P1 CutOff PreAlarm",
				"Battery Replace","Input Phase Error","Cover Of Maintain SW open","Phase Auto AdaptFailed",
				"Utility Extremely Unbalanced","Bypass Unstable","EEPROM Error ","Parallel Protect Warning","Discharger Overly"];

	let group5=["Reserved1", "Reserved2",  "Reserved3",  "Reserved4",  "Battery Test OK",    "Battery Test Fail",  "Battery Silence",   "Shutdown Active",    "Test in Progress",   "EPO",    "UPS Failed", "Bypass Active",  "Battery Low",    "Utility Fail",   "Live Status",    "Reserved bit5"];

	let allGroup = [group1 , group2  , group3  , group4  , group5 ];

	let Alarms=document.getElementById("alarms");

	while (Alarms.hasChildNodes()) {
    		Alarms.removeChild(Alarms.firstChild);
		}

	if((st == 3)||(st ==4))
	{
		div.style.height = "60%";
	}else{
		div.style.height = "50%";
	}

	for(var i = 0; i < allGroup[st-1].length;i++)
	{
		var listitem=document.createElement("LI");
		listitem.appendChild(document.createTextNode(allGroup[st -1][i]));
		Alarms.appendChild(listitem);
	}

}

function Mouseout()
{
	document.getElementById("divtoshow").style.display = "none";
}

function getBitPos(varValue, bit) {
	return (varValue & (1 << bit)) !== 0;
}

function onlyNumberKey(evt) {
	    var charCode = (evt.which) ? evt.which : evt.keyCode;
      if (charCode > 31
        && (charCode < 48 || charCode > 57 || charCode == 46 ))
         return false;

      return true;
}

function handleChange(checkbox)
{
	if(checkbox.checked==true)
		checkbox.value=1;
	else
		checkbox.value=0;
}

function WriteCOM1_Response(COM1Response)
{
	COM1settingStruct.COM1_BaudRate = COM1Response.getUint8(16);
	COM1settingStruct.COM1_DataBits = COM1Response.getUint8(17);
	COM1settingStruct.COM1_StopBit = COM1Response.getUint8(18);
	COM1settingStruct.COM1_ParityBit = COM1Response.getUint8(19);

	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

function WriteProtocol_Response(ProtocolResponse)
{
	ProtocolsettingStruct.SlaveId = ProtocolResponse.getUint16(16,true);
	ProtocolsettingStruct.ResponseTimeout = ProtocolResponse.getUint16(18,true);
	ProtocolsettingStruct.InterframeDelay = ProtocolResponse.getUint16(20,true);
	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

function WriteCOM1Diag_Response(COM1DiagResponse)
{
	COM1DiagnosticStruct.COM1_charRecv = COM1DiagResponse.getUint32(199,true);
	COM1DiagnosticStruct.COM1_charSent = COM1DiagResponse.getUint32(203,true);
	COM1DiagnosticStruct.COM1_framesRecv = COM1DiagResponse.getUint32(207,true);
	COM1DiagnosticStruct.COM1_frameSent = COM1DiagResponse.getUint32(211,true);
	COM1DiagnosticStruct.COM1_DumpChars = COM1DiagResponse.getUint32(215,true);
	COM1DiagnosticStruct.COM1_goodTransaction = COM1DiagResponse.getUint32(219,true);
	COM1DiagnosticStruct.COM1_badTransaction = COM1DiagResponse.getUint32(223,true);
	COM1DiagnosticStruct.COM1_ReqTimedout = COM1DiagResponse.getUint32(227,true);
	COM1DiagnosticStruct.COM1_cntrlCmdPassCntr = COM1DiagResponse.getUint32(231,true);
	COM1DiagnosticStruct.COM1_cntrlCmdFailCntr = COM1DiagResponse.getUint32(235,true);
	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);

}

function WriteCOM2_Response(COM2Response)
{
	COM2settingStruct.COM2_BaudRate = COM2Response.getUint8(16);
	COM2settingStruct.COM2_DataBits = COM2Response.getUint8(17);
	COM2settingStruct.COM2_StopBit = COM2Response.getUint8(18);
	COM2settingStruct.COM2_ParityBit = COM2Response.getUint8(19);
	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

function WriteGSMGroup_Response(GSMResponse)
{
	GSMandGroupSettingsStruct.MobileNo_1 = GSMResponse.getBigUint64(16,true);
	GSMandGroupSettingsStruct.MobileNo_2 = GSMResponse.getBigUint64(24 ,true);
	GSMandGroupSettingsStruct.MobileNo_3 = GSMResponse.getBigUint64(32 ,true);
	GSMandGroupSettingsStruct.MobileNo_4 = GSMResponse.getBigUint64(40 ,true);
	GSMandGroupSettingsStruct.MobileNo_5 = GSMResponse.getBigUint64(48 ,true);

	GSMandGroupSettingsStruct.AllGroup = GSMResponse.getUint16(56,true);

	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}
function ResetCOM2Diag_Response()
{
	COM1settingStruct.Updated=1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}
function WriteCOM2Diag_Response(COM2DiagResponse)
{
	COM2DiagnosticStruct.COM2_CharReceive = COM2DiagResponse.getUint32(16,true);
	COM2DiagnosticStruct.COM2_CharSent = COM2DiagResponse.getUint32(20,true);
	COM2DiagnosticStruct.COM2_FramesReceive = COM2DiagResponse.getUint32(24,true);
	COM2DiagnosticStruct.COM2_FramesSent = COM2DiagResponse.getUint32(28,true);
	COM2DiagnosticStruct.COM2_SMSSent = COM2DiagResponse.getUint32(32,true);
	COM2DiagnosticStruct.COM2_SMSFail = COM2DiagResponse.getUint32(36,true);
	COM2DiagnosticStruct.COM2_ModemError = COM2DiagResponse.getUint32(40,true);
	COM2DiagnosticStruct.COM2_ModemConnect = COM2DiagResponse.getUint32(44,true);
	COM2DiagnosticStruct.COM2_ResponseTimeout = COM2DiagResponse.getUint32(48,true);
	COM2DiagnosticStruct.COM2_ResponseWaitTimeout = COM2DiagResponse.getUint32(52,true);

	COM1settingStruct.Updated = 1;
	loaddata();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function ClickandSaveCOM1Setting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        COM1Setting_Request();
    }
}

function COM1Setting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_COM1_SETTING;
	DataRequestFrame.datalength = 0x04;

	DataRequestFrame.Data[0] = NewCOM1settingStruct.COM1_BaudRate;
	DataRequestFrame.Data[1] = NewCOM1settingStruct.COM1_DataBits;
	DataRequestFrame.Data[2] = NewCOM1settingStruct.COM1_StopBit;
	DataRequestFrame.Data[3] = NewCOM1settingStruct.COM1_ParityBit;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickandSaveProtocolSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        ProtocolSetting_Request();
    }
}
function ProtocolSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_PROTOCOL_SETTING;
	DataRequestFrame.datalength = 0x06;

	DataRequestFrame.Data[0] = ( NewProtocolsettingStruct.SlaveId & 0xFF);
	DataRequestFrame.Data[1] = ((NewProtocolsettingStruct.SlaveId >> 8) & 0xFF);

	DataRequestFrame.Data[2] = (NewProtocolsettingStruct.ResponseTimeout & 0xFF);
	DataRequestFrame.Data[3] = ((NewProtocolsettingStruct.ResponseTimeout>> 8) & 0xFF);

	DataRequestFrame.Data[4] = ( NewProtocolsettingStruct.InterframeDelay & 0xFF);
	DataRequestFrame.Data[5] = ((NewProtocolsettingStruct.InterframeDelay>> 8) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickCOM1ResetSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        COM1ResetSetting_Request();
    }
}

function COM1ResetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_COM1_DIAG_SETTING;
	DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickandSaveCOM2Setting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        COM2Setting_Request();
    }
}

function COM2Setting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_COM2_SETTING;
	DataRequestFrame.datalength = 0x04;

	DataRequestFrame.Data[0] = NewCOM2settingStruct.COM2_BaudRate;
	DataRequestFrame.Data[1] = NewCOM2settingStruct.COM2_DataBits;
	DataRequestFrame.Data[2] = NewCOM2settingStruct.COM2_StopBit;
	DataRequestFrame.Data[3] = NewCOM2settingStruct.COM2_ParityBit ;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickandSaveGSMSMSSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        GSMSMSSetting_Request();
    }
}

function GSMSMSSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_GSMSMS_SETTING;
	DataRequestFrame.datalength = 0x2A;


	var Mobile1 = parseInt(NewGSMandGroupSettingsStruct.MobileNo_1, 10);
	var Mobile2 = parseInt(NewGSMandGroupSettingsStruct.MobileNo_2, 10);
	var Mobile3 = parseInt(NewGSMandGroupSettingsStruct.MobileNo_3, 10);
	var Mobile4 = parseInt(NewGSMandGroupSettingsStruct.MobileNo_4, 10);
	var Mobile5 = parseInt(NewGSMandGroupSettingsStruct.MobileNo_5, 10);

	DataRequestFrame.Data[0] = Number((BigInt(Mobile1) & BigInt(0xFF)));
	DataRequestFrame.Data[1] = Number((BigInt(Mobile1) >> BigInt(8)) & BigInt(0xFF));
	DataRequestFrame.Data[2] = Number((BigInt(Mobile1) >> BigInt(16)) & BigInt(0xFF));
	DataRequestFrame.Data[3] = Number((BigInt(Mobile1) >> BigInt(24)) & BigInt(0xFF));
	DataRequestFrame.Data[4] = Number((BigInt(Mobile1) >> BigInt(32)) & BigInt(0xFF));
	DataRequestFrame.Data[5] = Number((BigInt(Mobile1) >> BigInt(40)) & BigInt(0xFF));
	DataRequestFrame.Data[6] = Number((BigInt(Mobile1) >> BigInt(48)) & BigInt(0xFF));
	DataRequestFrame.Data[7] = Number((BigInt(Mobile1) >> BigInt(56)) & BigInt(0xFF));

	DataRequestFrame.Data[8] = Number((BigInt(Mobile2) & BigInt(0xFF)));
	DataRequestFrame.Data[9] = Number((BigInt(Mobile2) >> BigInt(8)) & BigInt(0xFF));
	DataRequestFrame.Data[10] = Number((BigInt(Mobile2) >> BigInt(16)) & BigInt(0xFF));
	DataRequestFrame.Data[11] = Number((BigInt(Mobile2) >> BigInt(24)) & BigInt(0xFF));
	DataRequestFrame.Data[12] = Number((BigInt(Mobile2) >> BigInt(32)) & BigInt(0xFF));
	DataRequestFrame.Data[13] = Number((BigInt(Mobile2) >> BigInt(40)) & BigInt(0xFF));
	DataRequestFrame.Data[14] = Number((BigInt(Mobile2) >> BigInt(48)) & BigInt(0xFF));
	DataRequestFrame.Data[15] = Number((BigInt(Mobile2) >> BigInt(56)) & BigInt(0xFF));

	DataRequestFrame.Data[16] = Number((BigInt(Mobile3) & BigInt(0xFF)));
	DataRequestFrame.Data[17] = Number((BigInt(Mobile3) >> BigInt(8)) & BigInt(0xFF));
	DataRequestFrame.Data[18] = Number((BigInt(Mobile3) >> BigInt(16)) & BigInt(0xFF));
	DataRequestFrame.Data[19] = Number((BigInt(Mobile3) >> BigInt(24)) & BigInt(0xFF));
	DataRequestFrame.Data[20] = Number((BigInt(Mobile3) >> BigInt(32)) & BigInt(0xFF));
	DataRequestFrame.Data[21] = Number((BigInt(Mobile3) >> BigInt(40)) & BigInt(0xFF));
	DataRequestFrame.Data[22] = Number((BigInt(Mobile3) >> BigInt(48)) & BigInt(0xFF));
	DataRequestFrame.Data[23] = Number((BigInt(Mobile3) >> BigInt(56)) & BigInt(0xFF));

	DataRequestFrame.Data[24] = Number((BigInt(Mobile4) & BigInt(0xFF)));
	DataRequestFrame.Data[25] = Number((BigInt(Mobile4) >> BigInt(8)) & BigInt(0xFF));
	DataRequestFrame.Data[26] = Number((BigInt(Mobile4) >> BigInt(16)) & BigInt(0xFF));
	DataRequestFrame.Data[27] = Number((BigInt(Mobile4) >> BigInt(24)) & BigInt(0xFF));
	DataRequestFrame.Data[28] = Number((BigInt(Mobile4) >> BigInt(32)) & BigInt(0xFF));
	DataRequestFrame.Data[29] = Number((BigInt(Mobile4) >> BigInt(40)) & BigInt(0xFF));
	DataRequestFrame.Data[30] = Number((BigInt(Mobile4) >> BigInt(48)) & BigInt(0xFF));
	DataRequestFrame.Data[31] = Number((BigInt(Mobile4) >> BigInt(56)) & BigInt(0xFF));

	DataRequestFrame.Data[32] = Number((BigInt(Mobile5) & BigInt(0xFF)));
	DataRequestFrame.Data[33] = Number((BigInt(Mobile5) >> BigInt(8)) & BigInt(0xFF));
	DataRequestFrame.Data[34] = Number((BigInt(Mobile5) >> BigInt(16)) & BigInt(0xFF));
	DataRequestFrame.Data[35] = Number((BigInt(Mobile5) >> BigInt(24)) & BigInt(0xFF));
	DataRequestFrame.Data[36] = Number((BigInt(Mobile5) >> BigInt(32)) & BigInt(0xFF));
	DataRequestFrame.Data[37] = Number((BigInt(Mobile5) >> BigInt(40)) & BigInt(0xFF));
	DataRequestFrame.Data[38] = Number((BigInt(Mobile5) >> BigInt(48)) & BigInt(0xFF));
	DataRequestFrame.Data[39] = Number((BigInt(Mobile5) >> BigInt(56)) & BigInt(0xFF));

	DataRequestFrame.Data[40] = ( NewGSMandGroupSettingsStruct.AllGroup & 0xFF);
	DataRequestFrame.Data[41] =	((NewGSMandGroupSettingsStruct.AllGroup >> 8) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}
//************************************************************************************************
async function ClickCOM2ResetSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        COM2ResetSetting_Request();
    }
}

function COM2ResetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_COM2_DIAG_SETTING;

	DataRequestFrame.datalength = 0x00;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}
//************************************************************************************************
