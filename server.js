Bun.serve({
    fetch(req, server) {
        // upgrade the request to a WebSocket
        if (server.upgrade(req)) {
            return;
        }
        return new Response('Upgrade failed :(', { status: 500 });
    },
    websocket: {
        open(ws) {
            console.log("New client connected!");
            ws.send("You are connected to the echo server!");
        },

        message(ws, message) {
            console.log(`Received: ${message}`);
            ws.send(`Echo: ${message}`);
        },

        close(ws) {
            console.log("Client disconnected");
        },
    },
    port: 5000,
});

console.log("Server started at http://localhost:5000");