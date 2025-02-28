src="WebSocket.js"

let NewEthernetSettingStruct = {
	HostName:[],
	EnableDHCP:0,
	IPAddress:[],
	SubnetMask:[],
	Gateway:[],
	PrimaryDNS:[],
	SecondaryDNS:[],
	MACAddress:[]
}

let NewSNMPSettingStruct = {
	TrapIPAddress:[],
	Port:0
}

let NewMODBUSSettingStruct = {
	SocketTimeOut:0,
	Port:0,
	ClientConnected:0
}

let NewSNMPTrapSettingStruct = {
	TRAP1_BATTERY_OPEN				:0,
	TRAP2_IP_N_LOSS					:0,
	TRAP3_IP_SITE_FAIL				:0,
	TRAP4_LINEPHASEERROR			:0,
	TRAP5_BYPASSPHASEERROR			:0,
	TRAP6_BYPASSFREQUENCYUNSTABLE	:0,
	TRAP7_BATT_OVERCHARGE			:0,
	TRAP8_BATTLOW 					:0,
	TRAP9_OVER_LOADWARNING			:0,
	TRAP10_FAN_LOCKWARNING			:0,
	TRAP11_EPO_ACTIVE				:0,
	TRAP12_TURN_ON_ABNORMAL			:0,
	TRAP13_OVER_TEMPERATURE			:0,
	TRAP14_CHARGER_FAIL      		:0,
	TRAP15_REMOTE_SHUTDOWN   		:0,
	TRAP16_L1_IP_FUSE_FAIL   		:0,
	TRAP17_L2_IP_FUSE_FAIL   		:0,
	TRAP18_L3_IP_FUSE_FAIL   		:0,
	TRAP19_L1_PFC_PERROR     		:0,
	TRAP20_L1_PFC_NERROR     		:0,
	TRAP21_L2_PFC_PERROR     		:0,
	TRAP22_L2_PFC_NERROR     		:0,
	TRAP23_L3_PFC_PERROR     		:0,
	TRAP24_L3_PFC_NERROR     		:0,
	TRAP25_CAN_COMM_ERROR    		:0,
	TRAP26_SYNCH_LINE_ERROR  		:0,
	TRAP27_SYNCH_PULSE_ERROR 		:0,
	TRAP28_HOST_LINE_ERROR   		:0,
	TRAP29_MALE_CONN_ERROR   		:0,
	TRAP30_FEMALE_CONN_ERROR        :0,
	TRAP31_PARALLEL_LINE_CONNERROR  :0,
	TRAP32_BATT_CONNDIFF            :0,
	TRAP33_LINE_CONNDIFF            :0,
	TRAP34_BYPASS_CONNDIFF              	    :0,
	TRAP35_MODE_TYPEDIFF              	        :0,
	TRAP36_PARALLEL_INV_VOLTDIFF                :0,
	TRAP37_PARALLEL_OUTPUT_FREQDIFF             :0,
	TRAP38_BATT_CELL_OVERCHARGE                 :0,
	TRAP38_1_BATT_CELL_OVERCHARGE                 :0,
	TRAP39_PARALLEL_OUTPUT_PD         :0,
	TRAP40_PARALLEL_OUTPUT_PHASEDIFF            :0,
	TRAP41_PARALLEL_BYPASS_FORBIDDENDIFF        :0,
	TRAP42_PARALLEL_CONVERTER_ENABLEDIFF        :0,
	TRAP43_PARALLEL_BYP_FHLD    :0,
	
	TRAP44_PARALLEL_BYP_FLLD     :0,
	TRAP45_PARALLEL_BYP_VHLD    :0,
	TRAP46_PARALLEL_BYP_VLLD    :0,
	TRAP47_PARALLEL_LINE_FHLD     :0,
	TRAP48_PARALLEL_LINE_FLLD      :0,
	TRAP49_PARALLEL_LINE_VHLD     :0,
	TRAP50_PARALLEL_LINE_VLLD      :0,
	TRAP51_LOCKED_IN_BYPASS              		:0,
	TRAP52_THREE_PHASE_CURRENT_UNBALANCE        :0,
	TRAP53_BATTERY_FUSE_BROKEN              	:0,
	TRAP54_INV_CURRENT_UNBALANCE              	:0,
	TRAP55_P1_CUTOFF_PREALARM              		:0,
	TRAP56_BATTERY_REPLACE              		:0,
	TRAP57_INPUT_PHAS_EERROR              		:0,
	TRAP58_COVER_OF_MAINTAIN_SW_OPEN            :0,
	TRAP59_PHASE_AUTO_ADAPTFAILED              	:0,
	TRAP60_UTILITY_EXTREMELY_UNBALANCED         :0,
	TRAP61_BYPASS_UNSTABLE              		:0,
	TRAP62_EEPROM_ERROR              			:0,
	TRAP63_PARALLEL_PROTECT_WARNING             :0,
	TRAP64_DISCHARGER_OVERLY              		:0,
}

function onlyNumberKey(evt)
{
    var charCode = (evt.which) ? evt.which : evt.keyCode;
	console.log(charCode);
      if (charCode > 31 && (charCode < 48 || charCode > 57 ) && charCode != 46)
         return false;

      return true;
}

function handleChange(checkbox)
{
	{
		if(checkbox.checked==true)
			checkbox.value=1;
		else
			checkbox.value=0;
	}
}

function WriteEthernet_Response(ETHResponse)
{
	let i = 0;
	for ( i = 0; i < 16; i++)
	{
		EthernetSettingStruct.HostName[i] = ETHResponse.getUint8(16+i);
	}
	EthernetSettingStruct.EnableDHCP = ETHResponse.getUint8(32);

	for ( i = 0; i < 4; i++)
	{
		EthernetSettingStruct.IPAddress[i] = ETHResponse.getUint8(33+i);
		EthernetSettingStruct.Gateway[i] = ETHResponse.getUint8(37+i);
		EthernetSettingStruct.SubnetMask[i] = ETHResponse.getUint8(41+i);
		EthernetSettingStruct.PrimaryDNS[i] = ETHResponse.getUint8(45+i);
		EthernetSettingStruct.SecondaryDNS[i] = ETHResponse.getUint8(49+i);
	}
	EthernetSettingStruct.Updated = 1;
	SettingShow();
	setTimeout(ReadAllSettingAfterTimeout,500);

}

async function SaveEthernetSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        EthernetSetting_Request();
    }
}

function EthernetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;
	var i = 0;
	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_ETHERNET_SETTING;
	DataRequestFrame.datalength = 0x25;

	for ( i = 0; i < 16; i++)
	{
		DataRequestFrame.Data[i] = NewEthernetSettingStruct.HostName[i];
	}
	DataRequestFrame.Data[16] = NewEthernetSettingStruct.EnableDHCP;

	for ( i = 0; i < 4; i++)
	{
    	DataRequestFrame.Data[17 + i] = parseInt(NewEthernetSettingStruct.IPAddress[i], 10);
		DataRequestFrame.Data[21 + i] = parseInt(NewEthernetSettingStruct.SubnetMask[i], 10);
		DataRequestFrame.Data[25 + i] = parseInt(NewEthernetSettingStruct.Gateway[i], 10);
		DataRequestFrame.Data[29 + i] = parseInt(NewEthernetSettingStruct.PrimaryDNS[i], 10);
		DataRequestFrame.Data[33 + i] = parseInt(NewEthernetSettingStruct.SecondaryDNS[i], 10);
	}

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

function WriteSNMP_Response(SNMPResponse)
{
	for ( i = 0; i < 4; i++)
	{
		SNMPSettingStruct.TrapIPAddress[i] =  SNMPResponse.getUint8(16+i);
	}
	SNMPSettingStruct.Port = SNMPResponse.getUint16(20,true);

	EthernetSettingStruct.Updated = 1;
	SettingShow();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function SaveSNMPSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        SNMPSetting_Request();
    }
}

function SNMPSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_SNMP_SETTING;
	DataRequestFrame.datalength = 0x06;

	for (var i = 0; i < 4; i++)
	{
    	DataRequestFrame.Data[i] = parseInt(NewSNMPSettingStruct.TrapIPAddress[i], 10);
	}
	DataRequestFrame.Data[4] = ((NewSNMPSettingStruct.Port) & 0xFF);
	DataRequestFrame.Data[5] = ((NewSNMPSettingStruct.Port >> 8) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

function WriteMODbus_Response(MODBUSResponse)
{
	MODBUSSettingStruct.SocketTimeOut = MODBUSResponse.getUint16(16,true);
	MODBUSSettingStruct.Port = MODBUSResponse.getUint16(18,true);

	EthernetSettingStruct.Updated = 1;
	SettingShow();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function SaveModbusTCPSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        ModbusTCPSetting_Request();
    }
}

function ModbusTCPSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_MODBUS_TCP_SETTING;
	DataRequestFrame.datalength = 0x04;

	DataRequestFrame.Data[0] = (NewMODBUSSettingStruct.SocketTimeOut & 0xFF);
	DataRequestFrame.Data[1] = ((NewMODBUSSettingStruct.SocketTimeOut >> 8) & 0xFF);

	DataRequestFrame.Data[2] = (NewMODBUSSettingStruct.Port & 0xFF);
	DataRequestFrame.Data[3] = ((NewMODBUSSettingStruct.Port >> 8) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickMODTCPResetSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        MODTCPResetSetting_Request();
    }
}
function MODTCPResetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_MODBUSTCP_DIAG_SETTING;
	DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

function WriteSNMPTrap_Response(SNMPTrapResponse)
{
	var itr = 16;
	SNMPTrapSettingStruct.TRAP1_BATTERY_OPEN              		    = SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP2_IP_N_LOSS               			= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP3_IP_SITE_FAIL              			= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP4_LINEPHASEERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP5_BYPASSPHASEERROR            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP6_BYPASSFREQUENCYUNSTABLE     		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP7_BATT_OVERCHARGE             		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP8_BATTLOW              			    = SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP9_OVER_LOADWARNING            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP10_FAN_LOCKWARNING            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP11_EPO_ACTIVE              			= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP12_TURN_ON_ABNORMAL           		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP13_OVER_TEMPERATURE           		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP14_CHARGER_FAIL              			= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP15_REMOTE_SHUTDOWN            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP16_L1_IP_FUSE_FAIL            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP17_L2_IP_FUSE_FAIL            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP18_L3_IP_FUSE_FAIL            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP19_L1_PFC_PERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP20_L1_PFC_NERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP21_L2_PFC_PERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP22_L2_PFC_NERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP23_L3_PFC_PERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP24_L3_PFC_NERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP25_CAN_COMM_ERROR             		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP26_SYNCH_LINE_ERROR           		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP27_SYNCH_PULSE_ERROR          		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP28_HOST_LINE_ERROR            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP29_MALE_CONN_ERROR            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP30_FEMALE_CONN_ERROR          		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP31_PARALLEL_LINE_CONNERROR    		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP32_BATT_CONNDIFF              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP33_LINE_CONNDIFF              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP34_BYPASS_CONNDIFF            		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP35_MODE_TYPEDIFF              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP36_PARALLEL_INV_VOLTDIFF      		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP37_PARALLEL_OUTPUT_FREQDIFF   		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP38_BATT_CELL_OVERCHARGE       		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP39_PARALLEL_OUTPUT_PARALLELDIFF         	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP40_PARALLEL_OUTPUT_PHASEDIFF            	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP41_PARALLEL_BYPASS_FORBIDDENDIFF        	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP42_PARALLEL_CONVERTER_ENABLEDIFF        	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP43_PARALLEL_BYP_FREQ_HLD    	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP44_PARALLEL_BYP_FREQ_LLD    	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP45_PARALLEL_BYP_VOLT_HLD    	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP46_PARALLEL_BYP_VOLT_LLD    	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP47_PARALLEL_LINE_FREQ_HLD    	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP48_PARALLEL_LINE_FREQ_LLD	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP49_PARALLEL_LINE_VOLT_HLD     	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP50_PARALLEL_LINE_VOLT_LLD      	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP51_LOCKED_IN_BYPASS              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP52_THREE_PHASE_CURRENT_UNBALANCE      	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP53_BATTERY_FUSE_BROKEN              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP54_INV_CURRENT_UNBALANCE              	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP55_P1_CUTOFF_PREALARM              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP56_BATTERY_REPLACE              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP57_INPUT_PHAS_EERROR              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP58_COVER_OF_MAINTAIN_SW_OPEN          	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP59_PHASE_AUTO_ADAPTFAILED             	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP60_UTILITY_EXTREMELY_UNBALANCED       	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP61_BYPASS_UNSTABLE              		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP62_EEPROM_ERROR             	 		= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP63_PARALLEL_PROTECT_WARNING           	= SNMPTrapResponse.getUint8(itr++);
	SNMPTrapSettingStruct.TRAP64_DISCHARGER_OVERLY              		= SNMPTrapResponse.getUint8(itr++);


	EthernetSettingStruct.Updated = 1;
	SettingShow();
	setTimeout(ReadAllSettingAfterTimeout,500);

}


async function SaveSNMPTrapSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        SNMPTrapSetting_Request();
    }
}

function SNMPTrapSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_SNMPTRAP_SETTING;
	DataRequestFrame.datalength = 0x40;

	DataRequestFrame.Data[0] =  NewSNMPTrapSettingStruct.TRAP1_BATTERY_OPEN;
	DataRequestFrame.Data[1] =  NewSNMPTrapSettingStruct.TRAP2_IP_N_LOSS;
	DataRequestFrame.Data[2] =  NewSNMPTrapSettingStruct.TRAP3_IP_SITE_FAIL;
	DataRequestFrame.Data[3] =  NewSNMPTrapSettingStruct.TRAP4_LINEPHASEERROR;
	DataRequestFrame.Data[4] =  NewSNMPTrapSettingStruct.TRAP5_BYPASSPHASEERROR;
	DataRequestFrame.Data[5] =  NewSNMPTrapSettingStruct.TRAP6_BYPASSFREQUENCYUNSTABLE;
	DataRequestFrame.Data[6] =  NewSNMPTrapSettingStruct.TRAP7_BATT_OVERCHARGE;
	DataRequestFrame.Data[7] =  NewSNMPTrapSettingStruct.TRAP8_BATTLOW;
	DataRequestFrame.Data[8] =  NewSNMPTrapSettingStruct.TRAP9_OVER_LOADWARNING;
	DataRequestFrame.Data[9] =  NewSNMPTrapSettingStruct.TRAP10_FAN_LOCKWARNING	;
	DataRequestFrame.Data[10] = NewSNMPTrapSettingStruct.TRAP11_EPO_ACTIVE		;
	DataRequestFrame.Data[11] = NewSNMPTrapSettingStruct.TRAP12_TURN_ON_ABNORMAL	;
	DataRequestFrame.Data[12] = NewSNMPTrapSettingStruct.TRAP13_OVER_TEMPERATURE	;
	DataRequestFrame.Data[13] = NewSNMPTrapSettingStruct.TRAP14_CHARGER_FAIL      	;
	DataRequestFrame.Data[14] = NewSNMPTrapSettingStruct.TRAP15_REMOTE_SHUTDOWN   	;
	DataRequestFrame.Data[15] = NewSNMPTrapSettingStruct.TRAP16_L1_IP_FUSE_FAIL   	;
	DataRequestFrame.Data[16] = NewSNMPTrapSettingStruct.TRAP17_L2_IP_FUSE_FAIL   	;
	DataRequestFrame.Data[17] = NewSNMPTrapSettingStruct.TRAP18_L3_IP_FUSE_FAIL   	;
	DataRequestFrame.Data[18] = NewSNMPTrapSettingStruct.TRAP19_L1_PFC_PERROR     	;
	DataRequestFrame.Data[19] = NewSNMPTrapSettingStruct.TRAP20_L1_PFC_NERROR     	;
	DataRequestFrame.Data[20] = NewSNMPTrapSettingStruct.TRAP21_L2_PFC_PERROR     	;
	DataRequestFrame.Data[21] = NewSNMPTrapSettingStruct.TRAP22_L2_PFC_NERROR     	;
	DataRequestFrame.Data[22] = NewSNMPTrapSettingStruct.TRAP23_L3_PFC_PERROR     	;
	DataRequestFrame.Data[23] = NewSNMPTrapSettingStruct.TRAP24_L3_PFC_NERROR     	;
	DataRequestFrame.Data[24] = NewSNMPTrapSettingStruct.TRAP25_CAN_COMM_ERROR    	;
	DataRequestFrame.Data[25] = NewSNMPTrapSettingStruct.TRAP26_SYNCH_LINE_ERROR  	;
	DataRequestFrame.Data[26] = NewSNMPTrapSettingStruct.TRAP27_SYNCH_PULSE_ERROR 	;
	DataRequestFrame.Data[27] = NewSNMPTrapSettingStruct.TRAP28_HOST_LINE_ERROR   	;
	DataRequestFrame.Data[28] = NewSNMPTrapSettingStruct.TRAP29_MALE_CONN_ERROR   	;
	DataRequestFrame.Data[29] = NewSNMPTrapSettingStruct.TRAP30_FEMALE_CONN_ERROR   ;
	DataRequestFrame.Data[30] = NewSNMPTrapSettingStruct.TRAP31_PARALLEL_LINE_CONNERROR  		;
	DataRequestFrame.Data[31] = NewSNMPTrapSettingStruct.TRAP32_BATT_CONNDIFF            		;
	DataRequestFrame.Data[32] = NewSNMPTrapSettingStruct.TRAP33_LINE_CONNDIFF            		;
	DataRequestFrame.Data[33] = NewSNMPTrapSettingStruct.TRAP34_BYPASS_CONNDIFF              	;
	DataRequestFrame.Data[34] = NewSNMPTrapSettingStruct.TRAP35_MODE_TYPEDIFF              	;
	DataRequestFrame.Data[35] = NewSNMPTrapSettingStruct.TRAP36_PARALLEL_INV_VOLTDIFF              ;
	DataRequestFrame.Data[36] = NewSNMPTrapSettingStruct.TRAP37_PARALLEL_OUTPUT_FREQDIFF           ;
	DataRequestFrame.Data[37] = NewSNMPTrapSettingStruct.TRAP38_BATT_CELL_OVERCHARGE               ;
	DataRequestFrame.Data[38] = NewSNMPTrapSettingStruct.TRAP39_PARALLEL_OUTPUT_PARALLELDIFF       ;
	DataRequestFrame.Data[39] = NewSNMPTrapSettingStruct.TRAP40_PARALLEL_OUTPUT_PHASEDIFF          ;
	DataRequestFrame.Data[40] = NewSNMPTrapSettingStruct.TRAP41_PARALLEL_BYPASS_FORBIDDENDIFF      ;
	DataRequestFrame.Data[41] = NewSNMPTrapSettingStruct.TRAP42_PARALLEL_CONVERTER_ENABLEDIFF      ;
	DataRequestFrame.Data[42] = NewSNMPTrapSettingStruct.TRAP43_PARALLEL_BYP_FREQ_HLD  ;
	DataRequestFrame.Data[43] = NewSNMPTrapSettingStruct.TRAP44_PARALLEL_BYP_FREQ_LLD   ;
	DataRequestFrame.Data[44] = NewSNMPTrapSettingStruct.TRAP45_PARALLEL_BYP_VOLT_HLD  ;
	DataRequestFrame.Data[45] = NewSNMPTrapSettingStruct.TRAP46_PARALLEL_BYP_VOLT_LLD  ;
	DataRequestFrame.Data[46] = NewSNMPTrapSettingStruct.TRAP47_PARALLEL_LINE_FREQ_HLD   ;
	DataRequestFrame.Data[47] = NewSNMPTrapSettingStruct.TRAP48_PARALLEL_LINE_FREQ_LLD    ;
	DataRequestFrame.Data[48] = NewSNMPTrapSettingStruct.TRAP49_PARALLEL_LINE_VOLT_HLD   ;
	DataRequestFrame.Data[49] = NewSNMPTrapSettingStruct.TRAP50_PARALLEL_LINE_VOLT_LLD   ;
	DataRequestFrame.Data[50] = NewSNMPTrapSettingStruct.TRAP51_LOCKED_IN_BYPASS              	;
	DataRequestFrame.Data[51] = NewSNMPTrapSettingStruct.TRAP52_THREE_PHASE_CURRENT_UNBALANCE      ;
	DataRequestFrame.Data[52] = NewSNMPTrapSettingStruct.TRAP53_BATTERY_FUSE_BROKEN              	;
	DataRequestFrame.Data[53] = NewSNMPTrapSettingStruct.TRAP54_INV_CURRENT_UNBALANCE              ;
	DataRequestFrame.Data[54] = NewSNMPTrapSettingStruct.TRAP55_P1_CUTOFF_PREALARM              	;
	DataRequestFrame.Data[55] = NewSNMPTrapSettingStruct.TRAP56_BATTERY_REPLACE              	;
	DataRequestFrame.Data[56] = NewSNMPTrapSettingStruct.TRAP57_INPUT_PHAS_EERROR              	;
	DataRequestFrame.Data[57] = NewSNMPTrapSettingStruct.TRAP58_COVER_OF_MAINTAIN_SW_OPEN          ;
	DataRequestFrame.Data[58] = NewSNMPTrapSettingStruct.TRAP59_PHASE_AUTO_ADAPTFAILED             ;
	DataRequestFrame.Data[59] = NewSNMPTrapSettingStruct.TRAP60_UTILITY_EXTREMELY_UNBALANCED       ;
	DataRequestFrame.Data[60] = NewSNMPTrapSettingStruct.TRAP61_BYPASS_UNSTABLE              	;
	DataRequestFrame.Data[61] = NewSNMPTrapSettingStruct.TRAP62_EEPROM_ERROR              		;
	DataRequestFrame.Data[62] = NewSNMPTrapSettingStruct.TRAP63_PARALLEL_PROTECT_WARNING           ;
	DataRequestFrame.Data[63] = NewSNMPTrapSettingStruct.TRAP64_DISCHARGER_OVERLY              	;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

////////////////////////// Not used now //////////////////////////
async function ClickSNMPResetSetting_Request()
{
	while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        SNMPResetSetting_Request();
    }
}
function SNMPResetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_SNMP_DIAG_SETTING;
	DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

async function ClickETHERNERESETSetting_Request()
{
while((DataRequestFrame.ReqState != REQ_STATE_EMPTY) &&
			(WaitCounter < 100))
	{
		await sleep(10);
		WaitCounter++;
	}

	if((DataRequestFrame.ReqState == REQ_STATE_EMPTY))
	{
        ETHERNETResetSetting_Request();
    }
}
function ETHERNETResetSetting_Request()
{
	DataRequestFrame.ReqState = REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter = 0;
    DataRequestFrame.FunctionCode = WEBSOCKET_FUNCTIONCODE.WEBSOCKET_RESET;
    DataRequestFrame.Group = WEBSOCKET_GROUP.WEBSOCKET_ETHERNET_DIAG_SETTING;
	DataRequestFrame.datalength = 0x00;
	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState = REQ_STATE_FILLED;
    handleWebSocket_Request();
}

//****************************************************************************************** */
//**************************************// END FILE //************************************* */

