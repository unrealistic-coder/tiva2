    const cmds = ["zeus","poseidon","achilles","thor"];

    function check_cmd(message) {
        // Check if the message is in the cars array
        if (!cmds.includes(message)) {
            // Change the input box placeholder text
            document.getElementById('inputBox').placeholder = "Invalid command!";
        } else {
            document.getElementById('inputBox').placeholder = "Enter Command";
        }
    }

    // Example function to add output dynamically
    function checkEnter(event) {
        if (event.key === 'Enter') {
            sendMessage(); // Call sendMessage() function when Enter key is pressed
        }
    }

    function displayOutput(message) {
        var outputBox = document.getElementById('outputBox');
        outputBox.textContent = message; // Update the output box with new message
    }

    function sendMessage() {
        const inputBox = document.getElementById('inputBox');
        const outputBox = document.getElementById('outputBox');

        const message = inputBox.value;
        check_cmd(message)
        if (message) {
            // Get current timestamp
            const timestamp = new Date().toLocaleTimeString();

            // Add message with timestamp to output box
            outputBox.innerHTML += `<div>${timestamp}:  ${message}</div>`;

            // Scroll to the bottom of the output box
            outputBox.scrollTop = outputBox.scrollHeight;

            // Clear the input box
            inputBox.value = '';
        }
    }

function create_websocet()
{
    const WebSocket = require('ws');  // Import the WebSocket library

    // Create a new WebSocket server
    const wss = new WebSocket.Server({ port: 8080 });

    // When a new client connects
    wss.on('connection', (ws) => {
      console.log('A new client connected!');

      // Send a welcome message to the client
      ws.send('Hello, server!');

      // Listen for incoming messages from the client
      ws.on('message', (message) => {
        console.log(`Received message from client: ${message}`);
        // Respond back to the client
        ws.send(`Server received: ${message}`);
      });

      // Handle WebSocket close event
      ws.on('close', () => {
        console.log('Client disconnected');
      });

      // Handle errors
      ws.on('error', (error) => {
        console.error('WebSocket error:', error);
      });
    });

    console.log('WebSocket server running on ws://localhost:8080');
}
