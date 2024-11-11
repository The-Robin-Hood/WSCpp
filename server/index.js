const WebSocket = require('ws');

// Create a new WebSocket server
const wss = new WebSocket.Server({ noServer: true });

// Handle connection upgrade
const server = require('http').createServer((req, res) => {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('WebSocket server is running');
});

server.on('upgrade', (request, socket, head) => {
    // Print all headers
    console.log('Headers:', request.headers);

    // Check for the subprotocol
    const subprotocols = request.headers['sec-websocket-protocol'];
    console.log('Subprotocols:', subprotocols);
    if (subprotocols && subprotocols.includes('echo')) {
        // Accept the connection if "echo" is in the subprotocols
        wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit('connection', ws, request);
        });
    } else {
        // Reject the connection
        socket.write('HTTP/1.1 400 Bad Request\r\n\r\n');
        socket.destroy();
    }
});

// Handle WebSocket connections
wss.on('connection', (ws) => {
    console.log('New client connected!');

    ws.on('message', (message) => {
        if (message === 'close') {
            ws.close();
            return;
        }

        if (message instanceof Buffer) {
            console.log(`Received binary message: ${message.toString('hex')}`);
            const fs = require('fs');
            fs.writeFileSync('file.jpg', message);
            ws.send(message);
            return;
        }


        console.log(`Received: ${message}`);
        ws.send(`Echo: ${message}`);
    });
    ws.on('close', (code, reason) => {
        console.log(`Client disconnected: ${code} - ${reason}`);
    });


    ws.on('pong', (data) => {
        console.log('Pong received:', data.toString());
    });
});

// Start the server
const PORT = 8000;
server.listen(PORT, () => {
    console.log(`Server started at http://localhost:${PORT}`);
});