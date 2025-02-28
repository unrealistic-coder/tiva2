src="WebSocket.js"

let NewFlagSettingStruct = {
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
	ParameterFlagSetting:0,
	ParameterFlagIndex:0,
}

function WriteGroupEnabled_Response(GroupENResponse)
{
	FlagSettingStruct.AudibleAlarm = GroupENResponse.getUint8(16);
	FlagSettingStruct.BattModeAudibleWarning = GroupENResponse.getUint8(17);
	FlagSettingStruct.ColdStart = GroupENResponse.getUint8(18);
	FlagSettingStruct.BattOpenStatusCheck = GroupENResponse.getUint8(19);
	FlagSettingStruct.HighEfficiencyMode = GroupENResponse.getUint8(20);
	FlagSettingStruct.bypassForbiding = GroupENResponse.getUint8(21);
	FlagSettingStruct.EnergySaving = GroupENResponse.getUint8(22);
	FlagSettingStruct.ShortRestart3Times = GroupENResponse.getUint8(23);
	FlagSettingStruct.InverterShortClearFn = GroupENResponse.getUint8(24);
	FlagSettingStruct.OutputSocket1InBattMode = GroupENResponse.getUint8(25);
	FlagSettingStruct.OutputSocket2InBattMode = GroupENResponse.getUint8(26);
	FlagSettingStruct.SiteFaultDetect = GroupENResponse.getUint8(27);
	FlagSettingStruct.SetHotStandby = GroupENResponse.getUint8(28);
	FlagSettingStruct.DeepHighEfficiencyMode = GroupENResponse.getUint8(29);
	FlagSettingStruct.BypasWhenUPSTurnOff = GroupENResponse.getUint8(30);
	FlagSettingStruct.BypassAudibleWarning = GroupENResponse.getUint8(31);
	FlagSettingStruct.ConstantPhaseAngleFn = GroupENResponse.getUint8(32);
	FlagSettingStruct.AutoRestart = GroupENResponse.getUint8(33);
	FlagSettingStruct.BattDeepDischProtect = GroupENResponse.getUint8(34);
	FlagSettingStruct.BattLowProtect = GroupENResponse.getUint8(35);
	FlagSettingStruct.FreeRunFunction = GroupENResponse.getUint8(36);
	FlagSettingStruct.ConverterMode = GroupENResponse.getUint8(37);
	FlagSettingStruct.limitedRuntimeOnBatt = GroupENResponse.getUint8(38);
	FlagSettingStruct.OutputParallelFn = GroupENResponse.getUint8(39);
	FlagSettingStruct.PhaseAutoAdapt = GroupENResponse.getUint8(40);
	FlagSettingStruct.PeriodSelfTest = GroupENResponse.getUint8(41);
	FlagSettingStruct.WEBEmail_Enable = GroupENResponse.getUint8(42);

	FlagSettingStruct.Updated = 1;
	FlagSettingStruct.RefreshFlagSetting=1;
	UpdateGroupParam();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function ClickandSaveFlagSetting_Request()
{
		while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
		FlagSetting_Request();
	}
}

function FlagSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_FLAG_SETTING;
	DataRequestFrame.datalength = 2;//0x1B;
	
	DataRequestFrame.Data[0] = NewFlagSettingStruct.ParameterFlagSetting;
	DataRequestFrame.Data[1] = NewFlagSettingStruct.ParameterFlagIndex;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function RefreshFlagSetting_Request()
{
		while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
		FlagSetting_Refresh_Request();
	}
}

function FlagSetting_Refresh_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_FLAG_SETTING_REFRESH;
	DataRequestFrame.datalength = 2;
	
	DataRequestFrame.Data[0] = 0;
	DataRequestFrame.Data[1] = 0;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

//****************************************************************************************** */
//**************************************// END FILE //************************************* */
