src="WebSocket.js"

NewRTCSettingStruct = {
	 Date:0,
    Month:0,
    Year:0,
    Hours:0,
    Minute:0,
    Seconds:0,
    RTCCompenEnable :0,
    NTPServer:[],
    RTCCompensateInterval:0,
    RTCCompensateTime:0,
}

function populateDropdowns() {
    const daySelect = document.getElementById('day');
    const monthSelect = document.getElementById('month');
    const yearSelect = document.getElementById('year');
    const hourSelect = document.getElementById('hour');
    const minuteSelect = document.getElementById('minute');
    const secondSelect = document.getElementById('second');

	const now = new Date();
    const currentDay = now.getDate();
    const currentMonth = now.getMonth() + 1; // Months are zero-based
    const currentYear = now.getFullYear();
    const currentHour = now.getHours();
    const currentMinute = now.getMinutes();
    const currentSecond = now.getSeconds();

    // Populate days
    for (let i = 1; i <= 31; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = i;
        if (i === currentDay) {
            option.selected = true;
        }
        daySelect.appendChild(option);
    }

    // Populate months
    const monthNames = [
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    ];
    monthNames.forEach((month, index) => {
        const option = document.createElement('option');
        option.value = index + 1;
        option.textContent = month;
        if (index + 1 === currentMonth) {
            option.selected = true;
        }
        monthSelect.appendChild(option);
    });

    // Populate years
    const startYear = currentYear - 100; // For example, 100 years back
    const endYear = currentYear + 10; // Future years
    for (let i = startYear; i <= endYear; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = i;
        if (i === currentYear) {
            option.selected = true;
        }
        yearSelect.appendChild(option);
    }

    // Populate hours (0-23 for 24-hour format)
    for (let i = 0; i < 24; i++) {
        const option = document.createElement('option');
        option.value = i < 10 ? '0' + i : i;
        option.textContent = i < 10 ? '0' + i : i;
        if (i === currentHour) {
            option.selected = true;
        }
        hourSelect.appendChild(option);
    }

    // Populate minutes (00-59)
    for (let i = 0; i < 60; i++) {
        const option = document.createElement('option');
        option.value = i < 10 ? '0' + i : i;  // Add leading zero
        option.textContent = i < 10 ? '0' + i : i;
        if (i === currentMinute) {
            option.selected = true;
        }
        minuteSelect.appendChild(option);
    }

    // Populate seconds (00-59)
    for (let i = 0; i < 60; i++) {
        const option = document.createElement('option');
        option.value = i < 10 ? '0' + i : i;  // Add leading zero
        option.textContent = i < 10 ? '0' + i : i;
        if (i === currentSecond) {
            option.selected = true;
        }
        secondSelect.appendChild(option);
    }
}

async function SaveRTCCompensateSetting_Request()
{
	var WaitCounter=0;
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        RTCCompensateSetting_Request();
    }
}
function RTCCompensateSetting_Request()
{
	var i = 0;
	DataRequestFrame.ReqState = REQ_STATE_FILLING;
	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_RTC_COMPENSATE_SETTING;
    
	DataRequestFrame.datalength = 0x29;
	
	DataRequestFrame.Data[0] = NewRTCSettingStruct.Date;
    DataRequestFrame.Data[1] = NewRTCSettingStruct.Month;
    DataRequestFrame.Data[2] = (NewRTCSettingStruct.Year).toString().slice(-2);;
    DataRequestFrame.Data[3] = NewRTCSettingStruct.Hours;
    DataRequestFrame.Data[4] = NewRTCSettingStruct.Minute;
    DataRequestFrame.Data[5] = NewRTCSettingStruct.Seconds;
	DataRequestFrame.Data[6] = NewRTCSettingStruct.RTCCompensateInterval;
	DataRequestFrame.Data[7] = NewRTCSettingStruct.RTCCompensateTime;

	DataRequestFrame.Data[8] = NewRTCSettingStruct.RTCCompenEnable;
	for(i=0;i<32;i++)
	DataRequestFrame.Data[9+i] = NewRTCSettingStruct.NTPServer[i];
	
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}
function WriteRTCComepnsate_Response(RTCCompensateResponse)
{
	RTCCompensateSettingStruct.RTCCompensateInterval=RTCCompensateResponse.getUint8(22);
	RTCCompensateSettingStruct.RTCCompensateTime=RTCCompensateResponse.getUint8(23);
	
	RTCCompensateSettingStruct.RTCCompenEnable = RTCCompensateResponse.getUint8(24);
	for(i=0;i<32;i++)
	RTCCompensateSettingStruct.NTPServer[i] = RTCCompensateResponse.getUint8(25+i);
	RTCCompensateSettingStruct.Updated=1;
}