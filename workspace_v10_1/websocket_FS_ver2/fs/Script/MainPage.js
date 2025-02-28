var isRed = true;
const svg = document.querySelector("svg");

	var FF1 = [ "Battery Switch Open",
				"IP Neutral Loss",
				"IP Site Fail",
				"Line Phase Error",
				"Bypass Phase Error",
				"Bypass Frequency Unstable",
				"Battery Over Charge",
				"Battery Low",
				"Over load Warning",
				"Fan Lock Warning",
				"EPO Active",
				"Turn On Abnormal",
				"Over Temperature",
				"Charger Fail",
				"Remote Shutdown",
				"L1 IP Fuse Fail"];
	
	var FF2 = [	"L2 IP Fuse Fail",
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
	
	var FF3 = [	"LineConn Diff",
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
				
var SNMPState1 = [      "RES1",
						"RES2",
						"RES3",
						"RES4",
						"Batt Test OK",
						"Batt Test Fail",
						"Batt Silence",
						"Shutdown Active",
						"Test in Progress",
						"EPO",
						"UPS Failed",
						"Bypass Active",
						"Batt Low",
						"Utility Failed",
						"Live Status 1",
						"Live Status 2"];	
						
								
		var SNMPState2 = ["Standby",
					"Line Interactive",
					"On Line"];	
var hoverGroup = 0;
var hoverEVT =0;
	function showTooltip(evt,group) 
	{
		let tooltip = document.getElementById("tooltip");
		var data = document.getElementById(group);
		tooltip.innerHTML = data.textContent;
		tooltip.style.display = "block";

		if(group =="INPUTGROUP")
		{
			tooltip.style.height = data.height;
			tooltip.style.width = "300px"; 
			tooltip.style.left = evt.pageX + 5 + 'px';
			tooltip.style.top = evt.pageY + -40 + 'px';
		}
		if(group =="BYPASSGROUP")
		{
			tooltip.style.height = data.height;
			tooltip.style.width = "300px"; 
			tooltip.style.left = evt.pageX + 5 + 'px';
			tooltip.style.top = evt.pageY + -20 + 'px';
		}
		if( group =="BATTERYGROUP")
		{
			tooltip.style.height = data.height;
			tooltip.style.width = "300px"; 
			tooltip.style.left = evt.pageX + 5 + 'px';
			tooltip.style.top = evt.pageY + -100 + 'px';
		}
		if(group =="SYSTEMGROUP" || group =="SYSTEMGROUP1"){
			tooltip.style.width = "300px"; 
			tooltip.style.left = evt.pageX + 5 + 'px';
			tooltip.style.top = evt.pageY + -80 + 'px';
		}
		if(group == "OUTPUTGROUP"){
			tooltip.style.width = "300px"; 
			tooltip.style.left = evt.pageX + 5 + 'px';
			tooltip.style.top = evt.pageY + -100 + 'px';
		}
			
		hoverGroup = group;
		hoverEVT = evt;
	}

function hideTooltip() 
{
	var tooltip = document.getElementById("tooltip");
	tooltip.style.display = "none";
	hoverGroup = 0;
	hoverEVT = 0;
}



function UPdateFieldData() {
	INPUTGROUP.textContent 	=	"I/P VOLT (V)"+"&#8196;&#8196;:"+" "+ (AllParameterPageStruct.InputVolt/10).toFixed(2) + "<br>"+
								"I/P FREQ(Hz)"+"&#8196;&#8202;:"+" "+ (AllParameterPageStruct.InputFreq/10).toFixed(2)+"<br>";
								
	OUTPUTGROUP.textContent =	"O/P VOLT (V)"+"&#8196;&#8196;:"+" "+ (AllParameterPageStruct.OutputVolt/10).toFixed(2) +"<br>"+
								"O/P CURR (A)"+"&#8196;&#8202;:"+" "+ (AllParameterPageStruct.OutputCurr/10).toFixed(2) +"<br>"+
								"O/P POWR "+"&emsp;&nbsp;&nbsp;:"+" "+ (AllParameterPageStruct.OutputPWRFactor/10).toFixed(2) +"<br>"+
								"O/P LOAD (%)"+"&#8202;:"+" "+ (AllParameterPageStruct.OutputLoadPercent).toFixed(2) +"<br>"+
								"O/P FREQ(Hz)"+"&#8196;:"+" "+ (AllParameterPageStruct.OutputFreq/10).toFixed(2) +"<br>";
								
	BATTERYGROUP.textContent =	"BATT VOLT (V)"+"&#8196;:"+" "+ (AllParameterPageStruct.BattVolt).toFixed(2) + "<br>"+
								"BATT VPU"+"&emsp;&emsp;&#8196;&#8196;&#8202;&#8202;:"+" "+ (AllParameterPageStruct.BattVoltPerUnit/10).toFixed(2) +"<br>"+
								"BATT PLLEL"+"&emsp;&emsp;:"+" "+(AllParameterPageStruct.BatNumberInParallel/10).toFixed(2) +"<br>"+
								"BATT TIME"+"&emsp;&emsp;&#8196;:"+" "+ (AllParameterPageStruct.BattRemainTime/10).toFixed(2) +"<br>"+
								"BATT CAPA"+"&emsp;&emsp;&#8202;&#8202;:"+" "+ (AllParameterPageStruct.BattCapacity/10).toFixed(2) +"<br>";
								
	BYPASSGROUP.textContent  =	"BYP Freq(Hz)"+"&#8196;&#8196;:"+" "+ (AllParameterPageStruct.bypassFreq/10).toFixed(2) + "<br>";
								
	SYSTEMGROUP.textContent  =	"SYSTEM SR NO "+"&#8196;&#8202;&#8202;&#8202;&#8202;:"+" "+ AllParameterPageStruct.serialNumber.filter(value => value !== 0).map(value => String.fromCharCode(value)).join('') + "<br>"+
								"RATED OP VA "+"&emsp;&nbsp;&#8202;&#8202;:"+" "+ AllParameterPageStruct.RatedOutputVA + "<br>"+
								"BATTERY AH"+"&emsp;&#8196;&#8196;&#8196;&#8196;&#8202;:"+" "+ SystemGroupStruct.BatteryAH +"<br>"+
								"SYSTEM ID"+"&emsp;&emsp;&#8196;&#8196;&#8196;:"+" "+ AllParameterPageStruct.ManufacturerID +"<br>";
								
	SYSTEMGROUP1.textContent =	"SYSTEM SR NO "+"&nbsp;:"+" "+ AllParameterPageStruct.serialNumber.filter(value => value !== 0).map(value => String.fromCharCode(value)).join('') + "<br>"+
								"RATED OP VA"+"&emsp;&#8196;&#8196;:"+" "+ AllParameterPageStruct.RatedOutputVA  +"<br>"+
								"BATTERY AH"+"&emsp;&emsp;&#8202;&#8202;&#8202;:"+" "+ SystemGroupStruct.BatteryAH +"<br>"+
								"SYSTEM ID"+"&emsp;&emsp;&nbsp;&nbsp;&#8202;:"+" "+ AllParameterPageStruct.ManufacturerID +"<br>";
							
	if(hoverGroup != 0)
	{
		showTooltip(hoverEVT,hoverGroup);
	}
}

