src="WebSocket.js"

function DateAndTime(DateValues)
{
    var date = DateValues.Date;
    var month = DateValues.Month;
    var year = DateValues.Year;
    var hour = DateValues.Hours;
    var minute = DateValues.Minutes;
    var second = DateValues.Seconds;

	if((month >= 1 ) && (month <= 12 ) &&
		(date >= 1 ) && (date <= 31 )  &&
		(hour >= 0 ) && (hour <= 23 )  &&
		(minute >= 0) && (minute <= 59)&&
		(second >= 0 && second <= 59))
	{
		document.getElementById("dateTimeValue").value = date.toString().padStart(2, '0') + "-" + month.toString().padStart(2, '0') + "-" + year.toString().padStart(2, '0') +" "+
		hour.toString().padStart(2, '0') + ":" + minute.toString().padStart(2, '0') + ":" + second.toString().padStart(2, '0');
	}
	else
	{
		document.getElementById("dateTimeValue").value = "01-01-23 11:59:59";
	}
}

function Formated_Values(value)
{
	const formattedValue = value.toFixed(2);
	const parts = formattedValue.split('.');
	const integerPart = parts[0].padStart(2, '0');
	const decimalPart = parts[1];

	return (`${integerPart}.${decimalPart}`);
}

function TabButtons(evt, tabName, activeColor, defaultColor) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tabcontent");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tablinks");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
        tablinks[i].style.backgroundColor = defaultColor;
    }
    document.getElementById(tabName).style.display = "block";
    evt.currentTarget.className += " active";
    evt.currentTarget.style.backgroundColor = activeColor;
}

function toggleEmailSetting() {
    var emailSettingElement = document.getElementById('emailSetting');
    if (FlagSettingStruct.WEBEmail_Enable) {
        emailSettingElement.style.display = 'block';
    } else {
		emailSettingElement.style.display = 'none';
    }
}

function BootOsControlVersionPrint()
{
	document.getElementById('OSversion').value = VersionAndDateTime_Struct.OS_Ver[0] +"." + VersionAndDateTime_Struct.OS_Ver[1];
	document.getElementById('bootversion').value = (VersionAndDateTime_Struct.Boot_Ver);
	document.getElementById('Ctlversion').value = VersionAndDateTime_Struct.Control_Ver.filter(value => value !== 0).map(value => String.fromCharCode(value)).join('');	
}

document.addEventListener("DOMContentLoaded", function() 
{          
    const modal = document.getElementById("customModal");
    const openModalBtn = document.getElementById("openModalBtn");
    const submitPassword = document.getElementById("submitPassword");
    const passwordInput = document.getElementById("passwordInput");

    openModalBtn.addEventListener("click", function() 
    {
            modal.style.display = "flex";
            passwordInput.focus();
    });

    function handlePasswordSubmit() 
    {
        const password = passwordInput.value;
        if (password === "FujiElectric") 
        {
          window.location.href = "FlagSetting.shtml";
        } 
        else 
        {
          alert("Incorrect password. Access denied.");
        }
      }
      submitPassword.addEventListener("click", function(event) {
      event.preventDefault(); // Prevent default form submission
      handlePasswordSubmit();
      });
    
      modal.addEventListener("submit", function(event) {
        event.preventDefault(); // Prevent default form submission
        handlePasswordSubmit();
      });
    
    window.onclick = function(event) 
    {
        if (event.target == modal)
        {
            modal.style.display = "none";
        }
    };
});

// function promptForPassword()
// {
// 	let password=prompt("Password required to Access Features Setting");
// 	if(password!==null)
// 	{
// 		if(password=="FujiElectric")
// 		{
// 			window.location.href="FlagSetting.shtml";
// 		}
// 		else
// 		{
// 			alert("Incorrect password. Access denied.");
// 			window.location.href="MainPage.shtml";
// 		}
// 	}
// 	else
// 	{
// 		window.location.href="MainPage.shtml";
// 	}
// }

function confirmAction() {
	var result = confirm("Do you want to Update Date?");
	if (result) {
		// If user clicks 'OK', continue with further process
		SetDateOnSystem_Request();
	} else {
		// If user clicks 'Cancel', stop further process
	}
}

async function SetDateOnSystem_Request() 
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
		SetDateOnSystem();
	}
}

function SetDateOnSystem()
{
	var now = new Date();

	day = now.getDate();
	month = now.getMonth();
	year = now.getFullYear();

	hours = now.getHours();
	minutes = now.getMinutes();
	seconds = now.getSeconds();

	hours   = hours < 10 ? '0' + hours : hours;
    minutes = minutes < 10 ? '0' + minutes : minutes;
    seconds = seconds < 10 ? '0' + seconds : seconds;


	DataRequestFrame.ReqState=REQ_STATE_FILLING;

	DataRequestFrame.ReqTimeoutCounter=0;
	DataRequestFrame.FunctionCode=WEBSOCKET_FUNCTIONCODE.WEBSOCKET_WRITE;
	DataRequestFrame.Group=WEBSOCKET_GROUP.WEBSOCKET_DATETIME_SETTING;
	DataRequestFrame.datalength=0x06;

	DataRequestFrame.Data[0]= day;
	DataRequestFrame.Data[1]= month+1;
	DataRequestFrame.Data[2]= (year).toString().slice(-2);
	DataRequestFrame.Data[3]= hours;
	DataRequestFrame.Data[4]= minutes;
	DataRequestFrame.Data[5]= seconds;

	DataRequestFrame.CRC=0;

	DataRequestFrame.ReqState=REQ_STATE_FILLED;
	handleWebSocket_Request();
}