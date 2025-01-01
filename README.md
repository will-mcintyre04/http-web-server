# Multi-Process HTTP Server in C

## Overview

This project implements a simple, multi-process HTTP server in C. The server is designed to handle multiple client connections concurrently using process-based concurrency with `fork()`. It demonstrates foundational concepts in socket programming, process management, and server architecture, making it suitable for learning and extension.

## Features

- **Concurrent Client Handling**: Each client connection is handled in a separate process using `fork()`.
- **Socket Programming**: Uses low-level POSIX APIs for creating and managing TCP sockets.
- **Signal Handling**: Automatically reaps child processes with `SIGCHLD` to prevent zombie processes.
- **Modular Design**: Clearly separates server initialization, connection handling, and request processing for maintainability.

## How It Works

1. **Initialization**: The server creates a socket, binds it to a specified port, and listens for incoming connections.
2. **Connection Handling**: The `accept()` call blocks until a client connects. Upon connection, the server creates a child process with `fork()` to handle the client.
3. **Request Processing**: The child process handles the client’s request and logs the connection details. The parent process continues to listen for new connections.
4. **Signal Management**: The server uses `signal(SIGCHLD, SIG_IGN)` to ensure terminated child processes are cleaned up automatically.

## Building and Running

### Prerequisites

- GCC or another C compiler.
- A Unix-based operating system (Linux or macOS).

### Steps

1. **Clone the Repository**  
   Download the project files from your repository or source directory.

   ```bash
   git clone https://github.com/your-repo/multi-process-http-server.git
   cd multi-process-http-server

2. **Compile the Server**  
   Use the following command to compile the source files:

   ```bash
   gcc -o http_server main.c server.c
