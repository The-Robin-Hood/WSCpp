import { createServer, IncomingMessage, Server } from "http";
import { WebSocket, WebSocketServer } from "ws";
import * as fs from "fs";

interface CustomWebSocket extends WebSocket {
  isAlive?: boolean;
}

class WebSocketHandler {
  private wss: WebSocketServer;
  private server: Server;
  private readonly port: number;

  constructor(port: number) {
    this.port = port;
    this.wss = new WebSocketServer({ noServer: true });
    this.server = this.createServer();
    this.initialize();
  }

  private createServer(): Server {
    return createServer((req, res) => {
      res.writeHead(200, { "Content-Type": "text/plain" });
      res.end("WebSocket server is running");
    });
  }

  private initialize(): void {
    this.setupUpgradeHandler();
    this.setupConnectionHandler();
    this.startServer();
  }

  private setupUpgradeHandler(): void {
    this.server.on("upgrade", (request: IncomingMessage, socket, head) => {
      console.log("Headers:", request.headers);

      const subprotocols = request.headers["sec-websocket-protocol"];
      console.log("Subprotocols:", subprotocols);

      if (subprotocols && subprotocols.includes("echo")) {
        this.wss.handleUpgrade(request, socket, head, (ws) => {
          this.wss.emit("connection", ws, request);
        });
      } else {
        socket.write("HTTP/1.1 400 Bad Request\r\n\r\n");
        socket.destroy();
      }
    });
  }

  private setupConnectionHandler(): void {
    this.wss.on("connection", (ws: CustomWebSocket) => {
      console.log("New client connected!");

      ws.on("message", (message: Buffer, isBinary: boolean) => {
        this.handleMessage(ws, message, isBinary);
      });

      ws.on("close", (code: number, reason: Buffer) => {
        console.log(`Client disconnected: ${code} - ${reason}`);
      });

      ws.on("pong", (data: Buffer) => {
        console.log("Pong received:", data.toString());
      });
    });
  }

  private handleMessage(
    ws: CustomWebSocket,
    message: Buffer,
    isBinary: boolean
  ): void {
    // Convert message to string if it's a text message
    const messageContent = isBinary ? message : message.toString();

    // Handle close message
    if (messageContent === "close") {
      ws.close();
      return;
    }

    if (isBinary) {
      console.log("Binary data received");
      // fs.writeFileSync("file.jpg", message);
      ws.send(message);
    } else {
      console.log("Text:", messageContent);
      ws.send(`Echo: ${messageContent}`);
    }
  }

  private startServer(): void {
    this.server.listen(this.port, () => {
      console.log(`Server started at http://localhost:${this.port}`);
    });
  }
}

// Initialize the WebSocket server
const wsHandler = new WebSocketHandler(9000);
