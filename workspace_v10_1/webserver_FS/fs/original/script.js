const cmds = ["zeus", "poseidon", "achilles", "thor"];

function check_cmd(message) {
    if (!cmds.includes(message)) {
        document.getElementById('inputBox').placeholder = "Invalid command!";
    } else {
        document.getElementById('inputBox').placeholder = "Enter Command";
    }
}

function checkEnter(event) {
    if (event.key === 'Enter') {
        sendMessage();
    }
}

function displayOutput(message) {
    var outputBox = document.getElementById('outputBox');
    outputBox.textContent = message;
}

let socket;

function initWebSockUpgrade() {
    const serverUrl = "ws://10.130.213.203:8765"; //TODO: PORT MODIFY HERE
    socket = new WebSocket(serverUrl);

    socket.onopen = function () {
        const outputBox = document.getElementById("outputBox");
        const timestamp = new Date().toLocaleTimeString();
        outputBox.innerHTML += `<div>${timestamp}: WebSocket connection established successfully!</div>`;
        outputBox.scrollTop = outputBox.scrollHeight;
    };

    socket.onmessage = function (event) {
        const outputBox = document.getElementById("outputBox");
        const timestamp = new Date().toLocaleTimeString();
        outputBox.innerHTML += `<div>${timestamp}: Server: ${event.data}</div>`;
        outputBox.scrollTop = outputBox.scrollHeight;
    };

    socket.onclose = function (event) {
        const outputBox = document.getElementById("outputBox");
        const timestamp = new Date().toLocaleTimeString();
        outputBox.innerHTML += `<div>${timestamp}: WebSocket connection closed.</div>`;
        outputBox.scrollTop = outputBox.scrollHeight;
    };

    socket.onerror = function (error) {
        const outputBox = document.getElementById("outputBox");
        const timestamp = new Date().toLocaleTimeString();
        outputBox.innerHTML += `<div>${timestamp}: WebSocket error: ${error.message}</div>`;
        outputBox.scrollTop = outputBox.scrollHeight;
    };
}

function sendMessage() {
    const inputBox = document.getElementById('inputBox');
    const outputBox = document.getElementById('outputBox');
    const message = inputBox.value;

    check_cmd(message);
    if (message) {
        const timestamp = new Date().toLocaleTimeString();
        if (socket && socket.readyState === WebSocket.OPEN) {
            socket.send(message);
            outputBox.innerHTML += `<div>${timestamp}: Sent: ${message}</div>`;
        } else {
            outputBox.innerHTML += `<div>${timestamp}: Error: WebSocket is not connected!</div>`;
        }
        outputBox.scrollTop = outputBox.scrollHeight;
        inputBox.value = '';
    }
}
