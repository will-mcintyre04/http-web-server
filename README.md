# Multi-Process HTTP Server in C

## Overview

This project implements a simple, lightweight, multi-process HTTP server in C. The server is designed to handle HTTP GET requests from multiple client connections concurrently using process-based concurrency. The server fetches static files and serves them with appropriate HTTP responses utilizing concepts in socket programming, TCP/IP networking communication, process management, and server architecture.

The server can be accessed locally using a web browser or using the client code provided.

All information and errors are logged in a `web-server.log` file.

## Features

- **Concurrent Client Handling**: Each client connection is handled in a separate process for concurrency.
- **Request Handling**: The server supports HTTP GET requests, serves static files, and logs connection details.
- **Socket Programming**: Uses low-level POSIX APIs for creating and managing TCP sockets.
- **Signal Handling**: Automatically reaps child processes with `SIGCHLD` to prevent zombie processes.
- **File Serving**: Supports multiple file types (HTML, HTM, TXT, GIF, JPG, JPEG, PNG) and returns appropriate HTTP headers and content to clients.
- **Modular Design**: Clearly separates server initialization, connection handling, and request processing for maintainability.

## How It Works

1. **Initialization**: The server creates a socket, binds it to a specified port, and listens for incoming connections.
2. **Connection Handling**: The `accept()` call blocks until a client connects. Upon connection, the server creates a child process with `fork()` to handle the client.
3. **Request Processing**: The child process handles the client’s request and logs the connection details. The parent process continues to listen for new connections.
4. **Response**: The server then sends a response back to the client with the requested file, or an error message detailing the request/server error.

## Building and Running

### Prerequisites

- GCC or another C compiler.
- A Unix-based operating system (Linux or macOS).

### Steps

1. **Clone the Repository**  
   Download the project files from your repository or source directory.

   ```bash
   git clone https://github.com/will-mcintyre04/http-web-server.git
   cd http-web-server

2. **Compile the Server**  
   The server can be run locally using the Makefile provided. The `make run` command creates the required directories, compiles the c code, assembles the object files and links them together to create and run an executable.

   ```bash
   make run ARGS="<port-number>"
   ```

   Note that you can use the `make` command to just compile and link the files without running the server.

3. **Accessing the Server**
   Open a browser or use curl to test the server:

   ```bash
   curl http://localhost:<port-number>/index.html
   ```

   This should return a 404 error indicating the requested resource is not found.

4. **Populating the Server**
   To populate the server with static files that can be accessed, create a file in the root directory. For example:

   ```bash
   cd http-web-server
   touch example.txt
   nano example.txt
   <FILL IN FILE WITH CONTENT>

   make run ASRGS="<port-number>"
   curl http://localhost:<port-number>/example.txt
   ```

   The response should contain the content of the example file created.