
# HTTP Server

## Overview
This project implements a basic HTTP server in C using the Winsock2 library (Windows Sockets API).

## Features
- Listens for incoming HTTP requests on a configurable port (default: 8080)
- Accepts client connections and processes HTTP requests
- Sends HTTP responses back to clients
- Single-threaded request handling

## Building

### Prerequisites
- GCC compiler
- Windows development headers (included with most Windows setups)

### Compilation
```bash
gcc -o Server Server.c -lws2_32
```

The `-lws2_32` flag links the Winsock2 library required for socket operations on Windows.

## Usage

### Default Port (8080)
```bash
./Server
```

### Custom Port
```bash
./Server 3000
```

Replace `3000` with your desired port number.

### Connecting
Once running, connect via:
```bash
curl http://localhost:8080
```

Or open `http://localhost:8080` in your browser.

## Implementation Details
**Server.c** establishes a TCP socket, binds it to the specified port, and enters a loop accepting client connections. It reads incoming HTTP requests and sends the appropriate HTTP response.
