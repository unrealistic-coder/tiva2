var EmailState = false;

var NewEmailStruct = {
    toUser1:new Array(64).fill(0),
    toUser2:new Array(64).fill(0),
    toUser3:new Array(64).fill(0),
    SmtpUser:new Array(64).fill(0),
    Smtppassword:[],
    SmtpHost:[],
    SmtpPort:0,
    SmtpSSL:false,
    EmailEvent:0,
    EmailInterval:0,
}

function handleChange(checkbox)
{
	checkbox.value = checkbox.checked;
}

function ValueChange(input)
{
    if (input.value < 1) 
        input.value = 1;
    if (input.value >500) 
        input.value = 500;
}

function EmailTest_Response(EmailResponse)
{
    if (EmailResponse.getUint16(16,true) == 0)
    {   
        if(EmailState)
        {
            document.getElementById('EmailError').value = "Email Sent";
            document.getElementById('EmailError').style.color = "green";
        }
    }
    else
    {
        document.getElementById('EmailError').value = "Email failed " + `${EmailResponse.getInt16(16,true)}`;
        document.getElementById('EmailError').style.color = "red";
    }
    document.getElementById('TestEmail').disabled = false;
    document.getElementById('SaveConfig').disabled = false;
}

async function SendEmail_Request()
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
		EmailSetting_Request();
	}
}

function EmailSetting_Request()
{
	DataRequestFrame.ReqState=REQ_STATE_FILLING;
    let i=0;
	DataRequestFrame.ReqTimeoutCounter=0;
	DataRequestFrame.FunctionCode=WEBSOCKET_FUNCTIONCODE.WEBSOCKET_EMAIL_CONFIG;
	DataRequestFrame.Group=WEBSOCKET_GROUP.WEBSOCKET_TEST_EMAIL;
	DataRequestFrame.datalength=0x183;

    for(i=0;i<64;i++)
    {
        DataRequestFrame.Data[0+i]=NewEmailStruct.SmtpUser[i];
        DataRequestFrame.Data[64+i]=NewEmailStruct.toUser1[i];
        DataRequestFrame.Data[128+i]=NewEmailStruct.toUser2[i];
        DataRequestFrame.Data[192+i]=NewEmailStruct.toUser3[i];
        DataRequestFrame.Data[256+i]=NewEmailStruct.Smtppassword[i];;
        DataRequestFrame.Data[320+i]=NewEmailStruct.SmtpHost[i];
    }

    DataRequestFrame.Data[384]=((NewEmailStruct.SmtpPort) & 0xFF);
	DataRequestFrame.Data[385]=((NewEmailStruct.SmtpPort>>8) & 0xFF);

    DataRequestFrame.Data[386]=NewEmailStruct.SmtpSSL;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState=REQ_STATE_FILLED;
	handleWebSocket_Request();
}

function EmailSaveConfig_Response(SavedResponse)
{
	let i=0;
	for(i=0;i<64;i++)
	{
        EmailStruct.SmtpUser[i] = SavedResponse.getUint8(16+i);
        EmailStruct.toUser1[i] = SavedResponse.getUint8(80+i);
        EmailStruct.toUser2[i] = SavedResponse.getUint8(144+i);
        EmailStruct.toUser3[i] = SavedResponse.getUint8(208+i);
        EmailStruct.Smtppassword[i] = SavedResponse.getUint8(272+i);
        EmailStruct.SmtpHost[i] = SavedResponse.getUint8(336+i);
	}
    EmailStruct.SmtpPort = SavedResponse.getUint16(400,true);
    EmailStruct.SmtpSSL = SavedResponse.getUint8(402);

    EmailStruct.EmailEvent = SavedResponse.getUint16(403,true);
    EmailStruct.EmailInterval = SavedResponse.getUint32(405,true);

	EmailStruct.Updated = 1;
	EmailSettings();
	setTimeout(ReadAllSettingAfterTimeout,500);
}

async function SaveEmailSend_Request()
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
		SaveEmail_Request();
	}
}

function SaveEmail_Request()
{
	DataRequestFrame.ReqState=REQ_STATE_FILLING;
    let i=0;
	DataRequestFrame.ReqTimeoutCounter=0;
	DataRequestFrame.FunctionCode=WEBSOCKET_FUNCTIONCODE.WEBSOCKET_EMAIL_CONFIG;
	DataRequestFrame.Group=WEBSOCKET_GROUP.WEBSOCKET_SAVE_EMAIL_CONFIG;
	DataRequestFrame.datalength=0x189;

    for(i=0;i<64;i++)
    {
        DataRequestFrame.Data[0+i]=NewEmailStruct.SmtpUser[i];
        DataRequestFrame.Data[64+i]=NewEmailStruct.toUser1[i];
        DataRequestFrame.Data[128+i]=NewEmailStruct.toUser2[i];
        DataRequestFrame.Data[192+i]=NewEmailStruct.toUser3[i];
        DataRequestFrame.Data[256+i]=NewEmailStruct.Smtppassword[i];;
        DataRequestFrame.Data[320+i]=NewEmailStruct.SmtpHost[i];
    }

    DataRequestFrame.Data[384]=((NewEmailStruct.SmtpPort) & 0xFF);
	DataRequestFrame.Data[385]=((NewEmailStruct.SmtpPort>>8) & 0xFF);

    DataRequestFrame.Data[386]= NewEmailStruct.SmtpSSL;

    DataRequestFrame.Data[387]=((NewEmailStruct.EmailEvent) & 0xFF);
	DataRequestFrame.Data[388]=((NewEmailStruct.EmailEvent>>8) & 0xFF);

    DataRequestFrame.Data[389]=((NewEmailStruct.EmailInterval) & 0xFF);
	DataRequestFrame.Data[390]=((NewEmailStruct.EmailInterval>>8) & 0xFF);
    DataRequestFrame.Data[391]=((NewEmailStruct.EmailInterval>>16) & 0xFF);
    DataRequestFrame.Data[392]=((NewEmailStruct.EmailInterval>>24) & 0xFF);

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState=REQ_STATE_FILLED;
	handleWebSocket_Request();
}