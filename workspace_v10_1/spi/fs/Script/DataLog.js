src="WebSocket.js"
var startlog = 0;
var itemsPerPage = 50;
var currentPage = 1;
var totalEntries = 0;
var LogFlag ="";
var WaitCounter = 0;

var FF1 = [ 	"Battery Switch Open",
					"IP Neutral Loss",
					"IP Site Fail",
					"Line Phase Error",
					"Bypass Phase Error",
					"Bypass Frequency Unstable",
					"Battery Over Charge" ,
					"Battery Low",
					"Over load Warning",
					"Fan Lock Warning",
					"EPO Active",
					"Turn On Abnormal",
					"Over Temperature",
					"Charger Fail",
					"Remote Shutdown",
					"L1 IP Fuse Fail"];


	var FF2 = [		"L2 IP Fuse Fail",
					"L3 IP Fuse Fail",
					"L1 PFC PError",
					"L1 PFC NError",
					"L2 PFC PError",
					"L2 PFC NError",
					"L3 PFC PError",
					"L3 PFC NError",
					"CAN Comm Error",
					"Synch Line Error",
					"Synch Pulse Error",
					"Host Line Error",
					"Male Conn Error",
					"Female Conn Error",
					"Parallel Line Conn Error",
					"BattConn Diff"];

	var FF3 = [		"LineConn Diff",
					"Bypass Conn Diff",
					"Mode Type Diff",
					"Parallel INV_Volt Diff",
					"Parallel OutputFreqD iff",
					"BattCell OverCharge",
					"Parallel OutputParallel Diff",
					"Parallel OutputPhase Diff",
					"Parallel BypassForbidden Diff",
					"Parallel ConverterEnable Diff",
					"Parallel BypassFreqHighLoss Diff",
					"Parallel BypassFreqLowLoss Diff",
					"Parallel BypassVoltHighLoss Diff",
					"Parallel BypassVoltLowLoss Diff",
					"Parallel LineFreqHighLoss Diff",
					"Parallel LineFreqLowLoss Diff"];

		var FF4 = [	"Parallel LineVoltHighLoss Diff",
					"Parallel Line Volt LowLoss Diff",
					"Locked In Bypass",
					"Three Phase Current Unbalance",
					"Battery Fuse Broken",
					"INV Current Unbalance",
					"P1 CutOff PreAlarm",
					"Battery Replace",
					"Input Phase Error",
					"Cover Of Maintain SW open",
					"Phase Auto AdaptFailed",
					"Utility Extremely Unbalanced",
					"Bypass Unstable",
					"EEPROM Error" ,
					"Parallel Protect Warning",
					"Discharger Overly"];
					
	var STState1 = ["Reserved1",
					"Reserved2",
					"Reserved3",
					"Reserved4",
					"Battery Test OK",
					"Battery Test Fail",
					"Battery Silence",
					"Shutdown Active",
					"Test in Progress",
					"EPO",
					"UPS Failed",
					"Bypass Active",
					"Battery Low",
					"Utility Fail",
					"Live Status 1",
					"Live Status 2"];
					
		var STState2 = ["Standby",
					"Line Interactive",
					"On Line"];				

let DataLogStruct =
{
 	From:{Date:0, Month: 0, Year: 0 },
	To: { Date: 0, Month: 0, Year: 0 },
	Current:{ Date: 0, Month: 0, Year: 0 },
	LogCounter : 0,
	AllDaylogs : 0,
	readlogcnt : 0,
	Dayslogcount:[],
	Totaldayret: 0,
	AddLog: 0,
	DayLogs:0,
	Index : 0,
	packetLogs: 0,
};
let NewDataLogintervalStruct =
{
	NewDataLoginterval: 0,
};

function GetNextDate(current, daysToAdd)
{
    const daysInMonth = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
    let retDate = { Year: 0, Month: 0, Date: 0 };

    if (daysToAdd <= 62 && current.Month >= 1 && current.Month <= 12 &&
        current.Date >= 1 && current.Date <= 31)
    {
        retDate.Year = current.Year;
        retDate.Month = current.Month;
        retDate.Date = current.Date;

        if (current.Date + daysToAdd <= daysInMonth[current.Month - 1])
        {
            retDate.Date = current.Date + daysToAdd;
        }
        else
        {
            if (current.Month < 12)
            {
                retDate.Month = current.Month + 1;
                retDate.Date = 1;
            }
            else
            {
                retDate.Year = current.Year + 1;
                retDate.Month = 1;
                retDate.Date = 1;
            }
        }
    }
    return retDate;
}

function WriteDataloginterval_Response(IntervallogData)
{
	DataLogintervalStruct.SetDataLogInterval = IntervallogData.getUint32(16,true);
	DataLogintervalStruct.Updated = 1;
	DataLogInterval();

	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function SendOnIntervalSend()
{
	WaitCounter = 0;
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        SetInterval_Request();
    }
}

function SetInterval_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_LOGINTERVAL_SETTING;
    DataRequestFrame.datalength = 0x04;

	DataRequestFrame.Data[0] = ( NewDataLogintervalStruct.NewDataLoginterval & 0xFF);
	DataRequestFrame.Data[1] = ((NewDataLogintervalStruct.NewDataLoginterval >> 8) & 0xFF);
	DataRequestFrame.Data[2] = ((NewDataLogintervalStruct.NewDataLoginterval >> 16) & 0xFF);
	DataRequestFrame.Data[3] = ((NewDataLogintervalStruct.NewDataLoginterval >> 24) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}
async function ClickOnClearData()
{
	WaitCounter = 0;
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        ClearData_Request();
    }
}

function ClearData_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_CLEARLOGS_SETTING;
    DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickOnReadData()
{
	WaitCounter=0;

	if(startlog == 0)
	{
		itemsPerPage = 50;
		currentPage = 1;
		totalEntries = 0;
		startlog = 1;
		//document.getElementById("AlarmREADData").disabled = true;
		document.getElementById("READData").value = "Stop";
		document.getElementById("completionMessage").innerHTML = "Waiting for Data Logs";
	}
	else
	{
		document.getElementById("READData").value = "Read";
		document.getElementById("completionMessage").innerHTML = "Waiting for Data Logs";
		startlog =0;
		return;
	}

	var tableBody = null;
	tableBody = document.getElementById("DataLogitemRows");

	while (tableBody.firstChild) {
	tableBody.removeChild(tableBody.firstChild);
	}
	tableBody = document.getElementById("DataLogitemRows");

	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        ReadLogCount_Request();
    }
}

function ReadLogCount_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;
	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ;	
   	DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_NO_OF_DATALOGS;
	DataLogStruct.From.Date 	= document.getElementById("DATAFRMDATE").selectedIndex;
    DataLogStruct.From.Month  	= document.getElementById("DATAFRMMONTH").selectedIndex;
    DataLogStruct.From.Year  	= document.getElementById("DATAFRMYEAR").selectedIndex;
    DataLogStruct.To.Date 		= document.getElementById("DATATODATE").selectedIndex;
    DataLogStruct.To.Month 	    = document.getElementById("DATATOMONTH").selectedIndex;
    DataLogStruct.To.Year 		= document.getElementById("DATATOYEAR").selectedIndex;

    DataLogStruct.Current.Date  = DataLogStruct.From.Date;
    DataLogStruct.Current.Month = DataLogStruct.From.Month;
    DataLogStruct.Current.Year  = DataLogStruct.From.Year;
	DataLogStruct.LogCounter	= 0;
    DataLogStruct.AllDaylogs    = 0;
    DataLogStruct.Index         = 0;
    DataLogStruct.packetLogs    = 0;
    DataLogStruct.readlogcnt    = 0;
    DataLogStruct.DayLogs       = 0;
    DataRequestFrame.datalength = 0x06;

    DataRequestFrame.Data[0]=DataLogStruct.From.Date;
    DataRequestFrame.Data[1]=DataLogStruct.From.Month;
    DataRequestFrame.Data[2]=DataLogStruct.From.Year;
    DataRequestFrame.Data[3]=DataLogStruct.To.Date;
    DataRequestFrame.Data[4]=DataLogStruct.To.Month;
    DataRequestFrame.Data[5]=DataLogStruct.To.Year;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

function GetDataLog_Request() 
{
	if(startlog == 1)
	{
		DataRequestFrame.ReqState = REQ_STATE_FILLING;

		DataRequestFrame.ReqTimeoutCounter = 0;
		DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_READ;	
		DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_DATALOG_GROUP;
		DataRequestFrame.datalength = 0x05;

		DataRequestFrame.Data[0]=DataLogStruct.Current.Date;
		DataRequestFrame.Data[1]=DataLogStruct.Current.Month;
		DataRequestFrame.Data[2]=DataLogStruct.Current.Year;
		DataRequestFrame.Data[3] = (DataLogStruct.Index & 0xFF);
		DataRequestFrame.Data[4] = ((DataLogStruct.Index >> 8) & 0xFF);

		DataRequestFrame.CRC=0;

		DataRequestFrame.ReqState = REQ_STATE_FILLED;
		ConsoleMessage(` total Log Read  ${DataLogStruct.readlogcnt}  & Index  ${DataLogStruct.Index}`);
		handleWebSocket_Request();
	}
}

function NoOfDataLog_ResponseHandler(HEXValues)
{
	var index = 0,i=0,j=0;
	DataLogStruct.AllDaylogs=0;
	DataLogStruct.Totaldayret = HEXValues.getUint8(22);
	j= HEXValues.getUint8(22);;
	for (index = 0; index < j; index++)
	{
		i = 23 + (index * 2);
		DataLogStruct.Dayslogcount[index] = HEXValues.getUint16(i,true);
		DataLogStruct.AllDaylogs += HEXValues.getUint16(i,true);
	}

	ConsoleMessage(` total Log ret  ${DataLogStruct.AllDaylogs}`);
	ConsoleMessage(` total days ret  ${HEXValues.getUint8(22)}`);
	document.getElementById("LogCount").innerHTML =  DataLogStruct.AllDaylogs;
	if (DataLogStruct.AllDaylogs == 0) 
	{
		document.getElementById("LogCount").innerHTML = DataLogStruct.AllDaylogs;
		document.getElementById("completionMessage").innerHTML = "No Logs Found";
		document.getElementById("READData").value = "Read";
		startlog = 0;
	}
	else {
		GetDataLog_Request();
	}
}


function DataLog_ResponseHandler(DataLogResponse)
{
	DataLogStruct.packetLogs = DataLogResponse.getUint8(22);
	DataLogStruct.readlogcnt += DataLogStruct.packetLogs;
	DataLogStruct.Index = DataLogResponse.getUint16(20,true);
	DataLogStruct.DayLogs = DataLogResponse.getUint16(18,true);

	ConsoleMessage(` Packet Log   ${DataLogStruct.packetLogs}`);
	ConsoleMessage(` Index Log   ${DataLogStruct.Index}`);
	ConsoleMessage(` Day Log   ${DataLogStruct.DayLogs}`);
	ConsoleMessage(` total Log Read  ${DataLogStruct.readlogcnt}`);
	ConsoleMessage(` total Logs  ${DataLogStruct.AllDaylogs}`);

	if((DataBuffer.getUint8(8) == DataLogStruct.Current.Date) && 
	(DataBuffer.getUint8(9) == DataLogStruct.Current.Month) && 
	(DataBuffer.getUint8(10) == DataLogStruct.Current.Year))
	{
		if(DataLogStruct.Dayslogcount[(DataLogStruct.Totaldayret - 1)] != DataBuffer.getUint16(18,true))
		{
			DataLogStruct.AddLog =  DataBuffer.getUint16(18,true) - DataLogStruct.Dayslogcount[(DataLogStruct.Totaldayret - 1)]
			document.getElementById("LogCount").innerHTML = DataLogStruct.LogCounter +" of "+ (DataLogStruct.AllDaylogs + DataLogStruct.AddLog) ;
		}
	}
	
	if (DataLogStruct.AllDaylogs == 0) 
	{
		document.getElementById("LogCount").innerHTML = DataLogStruct.AllDaylogs;
		document.getElementById("completionMessage").innerHTML = "No Logs Found";
		document.getElementById("READData").value = "Read";
		startlog =0;
	}
	else if((DataLogStruct.readlogcnt < (DataLogStruct.AllDaylogs + DataLogStruct.AddLog)))
	{
		if(((DataLogStruct.Index + DataLogStruct.packetLogs) >= (DataLogStruct.DayLogs)) || (DataLogStruct.packetLogs == 0))
		{
			DataLogStruct.Index = 0;
			DataLogStruct.Current = GetNextDate(DataLogStruct.Current,1);
			
			ConsoleMessage(` Next Current Date ${DataLogStruct.Current.Date} ${DataLogStruct.Current.Month} ${DataLogStruct.Current.Year}`);
			if(startlog ==0)
			{
				document.getElementById("completionMessage").innerHTML = "Reading Logs Stopped";
			}
			else
			{
				document.getElementById("completionMessage").innerHTML = "Reading Logs...";
			}
			//document.getElementById("completionMessage").innerHTML = "Reading Logs...";
			document.getElementById("LogCount").innerHTML = DataLogStruct.LogCounter +" of "+ (DataLogStruct.AllDaylogs + DataLogStruct.AddLog);
			GetDataLog_Request();

		}
		else 
		{
			ConsoleMessage(`  Current Date ${DataLogStruct.Current.Date} ${DataLogStruct.Current.Month} ${DataLogStruct.Current.Year}`);
			if(startlog ==0)
			{
				document.getElementById("completionMessage").innerHTML = "Reading Logs Stopped";
			}
			else
			{
				document.getElementById("completionMessage").innerHTML = "Reading Logs...";
			}
			//document.getElementById("completionMessage").innerHTML = "Reading Logs...";
			document.getElementById("LogCount").innerHTML = DataLogStruct.LogCounter +" of "+  (DataLogStruct.AllDaylogs + DataLogStruct.AddLog);
			DataLogStruct.Index += DataLogStruct.packetLogs;
			GetDataLog_Request();
		}
	}
	else
	{
		ConsoleMessage(` All Logs Read`);
		document.getElementById("completionMessage").innerHTML = "Logs Read";
		document.getElementById("LogCount").innerHTML = (DataLogStruct.AllDaylogs + DataLogStruct.AddLog) +" of "+  (DataLogStruct.AllDaylogs + DataLogStruct.AddLog);
		document.getElementById("READData").value = "Read";
		startlog = 0;
	}
	if (DataLogStruct.packetLogs > 0)
	{
		DataLogpopoulateFunction(DataLogResponse);
		document.getElementById("LogCount").innerHTML = DataLogStruct.LogCounter +" of "+  DataLogStruct.AllDaylogs;
	}
	document.getElementById("downloadButton").disabled = false;
	document.getElementById("downloadButton").style.backgroundColor = "#6699ff";
	document.getElementById("downloadButton").style.border = 1;
}

function DataLogpopoulateFunction(Values)
{
	var ihtml={};
	var j = 24;

	let ihtmlBuffer1 = "<td bgcolor='white' style='text-align:center; min-width:150px; max-width:150px; overflow: hidden; white-space: nowrap; '>";
	let ihtmlBuffer = "<td bgcolor='white' style='text-align:center; min-width:210px; max-width:210px; overflow: hidden; white-space: nowrap; '>";
	let ihtmlBuffer2 = "<td bgcolor='white' style='text-align:center; min-width:300px; max-width:300px; overflow: hidden; white-space: nowrap; '>";


	ConsoleMessage(` Total Logs to Populate ${Values.getUint8(22)}`);

	if(Values.getUint8(22) > 0)
	{
		for(var i = 0 ; i < Values.getUint8(22)  ; i++)
		{
			totalEntries++;
			DataLogStruct.LogCounter++;
			ihtml = ihtmlBuffer1 + Values.getUint8(j + 2).toString().padStart(2, '0') +"/"+ Values.getUint8(j + 1).toString().padStart(2, '0') +"/"+
                                    Values.getUint8(j + 0).toString().padStart(2, '0')+" "+ Values.getUint8(j + 3).toString().padStart(2, '0') +":"+
                                    Values.getUint8(j +4).toString().padStart(2, '0') +":"+ Values.getUint8(j+5).toString().padStart(2, '0')    +"</td>";
			//time
			ihtml += ihtmlBuffer+ (Values.getUint16(j+6, true)  / 10)+"</td>";//30//Input Volt
			ihtml += ihtmlBuffer+ (Values.getUint16(j+8, true) / 10)+"</td>";//Output V
			ihtml += ihtmlBuffer+ (Values.getUint16(j+10, true) / 10)+"</td>";//Output Freq
			ihtml += ihtmlBuffer+ (Values.getInt16(j+12, true)  / 1)+"</td>";//Output Load
			ihtml += ihtmlBuffer+ (Values.getUint16(j+14, true) / 10)+"</td>";//Battery Volt
			ihtml += ihtmlBuffer+ (Values.getUint16(j+16, true)/ 10)+"</td>" ;  //Max Temp
			ihtml += ihtmlBuffer+ (Values.getUint16(j+18, true) / 1)+"</td>";//EMD  Deg
			ihtml += ihtmlBuffer+ (Values.getInt16(j+20, true)  / 1)+"</td>";//Humidity
			
		
			var alarmvalue	= Values.getBigUint64(j+22, true);//BigInt(Alarmbigvalue);
			var statusvalue	= Values.getUint16(j+30, true);//BigInt(Alarmbigvalue);

			var dateStr = Values.getUint8(j + 4).toString().padStart(2, '0') + "/" + 
							Values.getUint8(j + 5).toString().padStart(2, '0') + "/" +
							Values.getUint8(j + 6).toString().padStart(2, '0');

			var timeStr = Values.getUint8(j + 2).toString().padStart(2, '0') + ":" +
							Values.getUint8(j + 1).toString().padStart(2, '0') + ":" +
							Values.getUint8(j).toString().padStart(2, '0');
		
			ihtml += ihtmlBuffer2 + GetAlarmStrings(alarmvalue, FF1,FF2,FF3,FF4)+"</td>";//Alarms
			ihtml += ihtmlBuffer2 + GetStatusStrings(statusvalue, STState1,STState2)+"</td>";	//Status
			
		//	if(totalEntries==1)
		//	j += 48;
		//	else if(totalEntries>1)
			j += 38;
			const newRow = document.createElement("tr");
			newRow.classList.add("item"); 
			newRow.innerHTML = ihtml;
			document.getElementById("DataLogitemRows").appendChild(newRow);
			updatePagination();

		}
	}
}

function showPage(page)
{
    const items = document.querySelectorAll('#items .item');
    items.forEach(item =>
    {
        item.style.display = 'none';
    });

    const startIndex = (page - 1) * itemsPerPage;
    const endIndex = startIndex + itemsPerPage;

    for (let i = startIndex; i < endIndex && i < totalEntries; i++)
    {
        items[i].style.display = 'table-row';
    }
}

function renderPagination()
{
    const totalPages = Math.ceil(totalEntries / itemsPerPage);
    const pagination = document.getElementById('pagination');
    pagination.innerHTML = '';

    // Previous button
    const prevButton = document.createElement('li');
    prevButton.textContent = '<< Previous';
    prevButton.addEventListener('click', function()
    {
        if (currentPage > 1)
        {
            currentPage--;
            showPage(currentPage);
            updateActiveClass();
            updatePageSpan();
        }
    });

    pagination.appendChild(prevButton);

    // Next button
    const nextButton = document.createElement('li');
    nextButton.textContent = 'Next';
    nextButton.addEventListener('click', function()
    {
        if (currentPage < totalPages)
        {
            currentPage++;
            showPage(currentPage);
            updateActiveClass();
            updatePageSpan();
        }
    });

    pagination.appendChild(nextButton);

    // Current page span
    const pageSpan = document.createElement('span');
    pageSpan.textContent = 'Page ' + currentPage + ' of ' + totalPages;
    pageSpan.style.margin = "5px 1px 5px 1px";
    pagination.appendChild(pageSpan);
}

function updatePagination()
{
    renderPagination();
    showPage(currentPage);
}

function updateActiveClass()
{
    const paginationItems = document.querySelectorAll('#pagination li');
    paginationItems.forEach(item =>
    {
        item.classList.remove('active');
        if (parseInt(item.textContent) === currentPage)
        {
            item.classList.add('active');
        }
    });
}

function updatePageSpan()
{
    const pageSpan = document.querySelector('#pagination span');
    pageSpan.textContent = 'Page ' + currentPage + ' of ' + Math.ceil(totalEntries / itemsPerPage);
}

function downloadCSV()
{
	const items = document.querySelectorAll('#items .item');
	const csvRows = [];
	const headers = [];
	const delimiter = ',';

	// Extract headers
	items[0].querySelectorAll('td').forEach(cell =>
	{
		headers.push(cell.textContent.trim());
	});

	//var customHeadings = ['','Time', 'Output Voltage', 'Output Current', 'Output PF', 'Output Power(KW)', 'Output Power(KVA)',
	//					'Output Load(%)', 'Output IGBT(A)','Output Total power(KW)','Output Frequency(Hz)','Output Temp 1(deg)',
	//					'Output Peak I(A)', 'Battery voltage(V)', 'Battery Current(I)', 'Battery Peak I(A)','Input Voltage(V)',
	//					'Input Current(A)','Input PF','Input Power(KW)','Input Power(KVA)','Input Frequency(Hz)','Input Temp 1(Deg)\r']; // headers here
	//var customHeadings = ['','Time','Input volt(V)','Input Freq(Hz)','Output Volt(V)',	'Output Freq(Hz)','Output PF','Output Load(%)',	'Battery Volt(V)', 	'Max Temp (Deg)','Battery Volt Per Unit', 'Bypass Frequency(Hz)','N Battery Volt (V)','P Battery Volt(V)','P Bus Volt(V)','N Bus Volt(V)',
   //                    'Input Volt Before Fault(V)', 'Input Freq Before Fault(Hz)', 'Output Volt Before Fault(V)','Output Freq Before Fault(Hz)', 'Output Load Before Fault(%)', 'Output Current Before Fault(A)','P Bus Voltage Before Fault(V)', 'N Bus Voltage Before Fault(V)', 'Batt Volt Before Fault(V)','Temp Before Fault(Deg)', 'Alarms','Status\r'];

var customHeadings = ['','Time','Input voltage(V)',	'Output voltage(V)',	'Output frequency(Hz)',
	'Load(%)',	'Battery voltage(V)',	'Temp.(°C)',	'EMD Temp.(°C)',	'EMD humidity(%)','Warnings','Status\r'];

	headers.unshift(customHeadings); // Add custom headers at the beginning
	csvRows.push(headers.join(delimiter));

	// Extract data
	items.forEach(row =>
	{
		const rowData = [];
		row.querySelectorAll('td').forEach(cell =>
		{
			rowData.push(cell.textContent.trim());
		});
		csvRows.push(delimiter + rowData.join(delimiter));
	});

	const csvContent = csvRows.join('\n');
	const blob = new Blob([csvContent], { type: 'text/csv' });
	const link = document.createElement('a');
	link.href = window.URL.createObjectURL(blob);
	link.download = 'Datalog.csv';
	link.click();
}

function GetAlarmStrings(alarmValue, alarmArray1,alarmArray2,alarmArray3,alarmArray4) {
    let result = '';
    let m = BigInt(0n);
	let n = BigInt(0n);
    for (; BigInt(m) < BigInt(16); BigInt(m++)) {
        if ((BigInt(alarmValue) & BigInt((BigInt(1) << BigInt(m)))) !== BigInt(0)) {
            result += alarmArray1[m] + '; ';
        }
	}
	m = BigInt(0n);
	n = BigInt(16n);
	for (; BigInt(m) < BigInt(16); BigInt(n++),BigInt(m++)) {
        if ((BigInt(alarmValue) & BigInt((BigInt(1) << BigInt(n)))) !== BigInt(0)) {
            result += alarmArray2[m] + '; ';
        }
	}
		m = BigInt(0n);
		n = BigInt(32n);
	for (; BigInt(m) < BigInt(16); BigInt(n++),BigInt(m++)) {
        if ((BigInt(alarmValue) & BigInt((BigInt(1) << BigInt(n)))) !== BigInt(0)) {
            result += alarmArray3[m] + '; ';
        }
	}
		n = BigInt(48n);
		m = BigInt(0n);
	for (; BigInt(m) < BigInt(16); BigInt(n++),BigInt(m++)) {
        if ((BigInt(alarmValue) & BigInt((BigInt(1) << BigInt(n)))) !== BigInt(0)) {
            result += alarmArray4[m] + '; ';
        }
	}

    return result.slice(0, -2); // Remove the trailing comma and space
}
//****************************************************************************************** */

//******************************// GET STATUS STRING //************************************* */
function GetStatusStrings(statusValue, statusArray,statusArray2) {
    let result = '';
    let m = BigInt(0n);
    for (; BigInt(m) < BigInt(14); BigInt(m++)) {
        if ((BigInt(statusValue) & BigInt((BigInt(1) << BigInt(m)))) !== BigInt(0)) {
            result += statusArray[m] + '; ';
        }
    }
   
if((BigInt(statusValue) & BigInt((BigInt(1) << BigInt(14)))) == BigInt(0)
 && (BigInt(statusValue) & BigInt((BigInt(1) << BigInt(15)))) == BigInt(0))
 {
    result += statusArray2[0] + '; ';
 }
    
else if((BigInt(statusValue) & BigInt((BigInt(1) << BigInt(14)))) != BigInt(0)
 && (BigInt(statusValue) & BigInt((BigInt(1) << BigInt(15)))) == BigInt(0))
 {
    result += statusArray2[1] + '; ';
 }
    
else if((BigInt(statusValue) & BigInt((BigInt(1) << BigInt(14)))) == BigInt(0)
 && (BigInt(statusValue) & BigInt((BigInt(1) << BigInt(15)))) != BigInt(0))
{
    result += statusArray2[2] + '; ';
}
        
        
    return result.slice(0, -2); // Remove the trailing comma and space
}
//****************************************************************************************** */

//******************************// ALARM LOG DOWNLOAD //************************************ */
