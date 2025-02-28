src="WebSocket.js"

let NewShutDownSettingStruct={

	ShutdownTime_HH:0,	
	ShutdownTime_MM:0,	
	ShutdownTime_SS:0,	
	RestartTime_HH:0,	
	RestartTime_MM:0,	
	RestartTime_SS:0,	
	ShutdownDelay_HH:0,	
	ShutdownDelay_MM:0,	
	ShutdownDelay_SS:0,
	Cbox_SS_EnDis:0,
	Cbox_SS_ByTime:0,
	Cbox_SS_ByMainsFail:0
}

function handleChange(checkbox)
{
	if(checkbox.checked==on)
	{
		checkbox.value=1;
		checkbox.checked=on;
	}
	else
	{
		checkbox.value=0;
		checkbox.checked=off;
	}
}

function WriteSchedule_Response(ScheduleResponse)
{
	ShutDownSettingStruct.ShutdownTime_HH=ScheduleResponse.getUint8(16);
	ShutDownSettingStruct.ShutdownTime_MM=ScheduleResponse.getUint8(17);
	ShutDownSettingStruct.ShutdownTime_SS=ScheduleResponse.getUint8(18);
	ShutDownSettingStruct.RestartTime_HH=ScheduleResponse.getUint8(19);
	ShutDownSettingStruct.RestartTime_MM=ScheduleResponse.getUint8(20);
	ShutDownSettingStruct.RestartTime_SS=ScheduleResponse.getUint8(21);
	ShutDownSettingStruct.ShutdownDelay_HH=ScheduleResponse.getUint8(22);
	ShutDownSettingStruct.ShutdownDelay_MM=ScheduleResponse.getUint8(23);
	ShutDownSettingStruct.ShutdownDelay_SS=ScheduleResponse.getUint8(24);

	ShutDownSettingStruct.Updated=1;
	UpdateScheduleParam();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function SaveScheduledSetting_Request() 
{
	let WaitCounter =0;
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
		ScheduledSetting_Request();
	}
}

function ScheduledSetting_Request()
{
	DataRequestFrame.ReqState=REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter=0;
	DataRequestFrame.FunctionCode=WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
	DataRequestFrame.Group=WEBSOCKET_GROUP.WEBSOCKET_SCHEDULAR_SETTING;
	DataRequestFrame.datalength=12;//0x09;

	DataRequestFrame.Data[0]=NewShutDownSettingStruct.ShutdownTime_HH;
	DataRequestFrame.Data[1]=NewShutDownSettingStruct.ShutdownTime_MM;
	DataRequestFrame.Data[2]=NewShutDownSettingStruct.ShutdownTime_SS;
	DataRequestFrame.Data[3]=NewShutDownSettingStruct.RestartTime_HH;
	DataRequestFrame.Data[4]=NewShutDownSettingStruct.RestartTime_MM;
	DataRequestFrame.Data[5]=NewShutDownSettingStruct.RestartTime_SS;
	DataRequestFrame.Data[6]=NewShutDownSettingStruct.ShutdownDelay_HH;
	DataRequestFrame.Data[7]=NewShutDownSettingStruct.ShutdownDelay_MM;
	DataRequestFrame.Data[8]=NewShutDownSettingStruct.ShutdownDelay_SS;
	DataRequestFrame.Data[9]=NewShutDownSettingStruct.Cbox_SS_EnDis;
	DataRequestFrame.Data[10]= NewShutDownSettingStruct.Cbox_SS_ByTime;
	DataRequestFrame.Data[11]=NewShutDownSettingStruct.Cbox_SS_ByMainsFail;
	
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState=REQ_STATE_FILLED;
	handleWebSocket_Request();
}