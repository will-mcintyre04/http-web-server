#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <time.h>

#define ERROR 42
#define LOG 44
#define FORBIDDEN 403
#define NOTFOUND  404
#define METHOD_NOT_ALLOWED 405
#define BUFFER_SIZE 1024
#define CONNECTION_QUEUE 5

struct {
    char *ext;
    char *filetype;
} extensions[] = {
    {"html", "text/html"},
    {"htm", "text/html"},
    {"txt", "text/plain"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"png", "image/png"},
    {"gif", "image/gif"},
    {0, 0} // End marker
};

// Initialize the server with a socket and bind the socket to an address, then listen
void init_server(HTTP_Server * http_server, int port){
    http_server->port = port;

    /*
    the socket() system call creates a new socket, taking in the address domain (IPv4)
    the type of socket (stream vs datagram) and the protocol
    (it will automatically choose TCP for stream)
    */
    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0){
        perror("Error opening socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    // Set the server address structure fields
    serv_addr.sin_family = AF_INET;
    // Contains IP address of the host, IP address of the machine that the server is running,
    // can accept incoming connections on any network interface
    serv_addr.sin_addr.s_addr = INADDR_ANY; //Listening on all network interfaces
    serv_addr.sin_port = htons(port); // Supports little endian computers for network byte order

    /*
    Bind system call, binds a socket to an address
    */
    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("ERROR on binding");
        exit(1);
    }

    // System call that allows process to listen on the socket for connections
    // CONNECTION_QUEUE is size of backlog queue(# of connections that can be waiting)
    listen(server_socket_fd,CONNECTION_QUEUE);

    http_server->socket = server_socket_fd;
    http_server->serv_addr = serv_addr;
    printf("Server initialized and listening on port %d\n", http_server->port);
}

// Get the current timestamp in a readable format
void get_current_timestamp(char *buffer, size_t buffer_size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

void logger(int type, char *s1, char *s2, int socket_fd, struct sockaddr_in *client_address) {
    int fd;
    char logbuffer[BUFFER_SIZE * 2];
    char timestamp[20];  // Buffer to hold timestamp
    char client_ip[INET_ADDRSTRLEN]; // Buffer to hold client IP address

    // Get the current timestamp
    get_current_timestamp(timestamp, sizeof(timestamp));

    // Convert the client's IP address to string
    inet_ntop(AF_INET, &client_address->sin_addr, client_ip, sizeof(client_ip));

    // Choose log type and format the message
    switch (type) {
        case ERROR:
            sprintf(logbuffer, "[%s] [%s] ERROR: %s:%s", client_ip, timestamp, s1, s2);
            fprintf(stderr, "\033[0;31m%s\033[0m\n", logbuffer);  // Red color for errors in console
            break;
        case FORBIDDEN:
            (void)write(socket_fd,
                "HTTP/1.1 403 Forbidden\nConnection: close\nContent-Type: text/html\n\n"
                "<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1>The requested URL, "
                "file type or operation is not allowed on this simple static file webserver.</body></html>\n", 247);
            sprintf(logbuffer, "[%s] [%s] FORBIDDEN: %s: %s", client_ip, timestamp, s1, s2);
            break;
        case NOTFOUND:
            (void)write(socket_fd,
                "HTTP/1.1 404 Not Found\nConnection: close\nContent-Type: text/html\n\n"
                "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1>"
                "The requested URL was not found on this server.</body></html>\n", 200);
            sprintf(logbuffer, "[%s] [%s] NOT FOUND: %s: %s", client_ip, timestamp, s1, s2);
            break;
        case METHOD_NOT_ALLOWED:
            (void)write(socket_fd, 
                "HTTP/1.1 405 Method Not Allowed\nConnection: close\nContent-Type: text/html\n\n"
                "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1>"
                "<p>The requested HTTP method is not allowed. This server only accepts GET requests.</p></body></html>\n", 271);
            sprintf(logbuffer, "[%s] [%s] METHOD NOT ALLOWED: %s: %s", client_ip, timestamp, s1, s2);
            break;
        case LOG:
            sprintf(logbuffer, "[%s] [%s] INFO: %s: %s", client_ip, timestamp, s1, s2);
            break;
    }

    // Always log detailed information to the file
    if ((fd = open("web-server.log", O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
        write(fd, logbuffer, strlen(logbuffer));
        write(fd, "\n", 1);
        close(fd);
    }

    // If it's an error, exit after logging
    if (type == ERROR || type == NOTFOUND || type == FORBIDDEN) {
        exit(3);
    }
}

// Send an HTTP response header to the client socket
void send_response_header(int client_socket_fd, const char * status, const char * server, const char * content_type, long content_length){
    char header[BUFFER_SIZE];
    sprintf(header, "HTTP/1.0 %s\nServer: %s\nContent-Type: %s\nContent-Length: %ld\n\n", status, server, content_type, content_length);
    int n = write(client_socket_fd,header,strlen(header));
    if (n < 0){
        perror("ERROR writing to socket");
        exit(1);
    }
}
// Handle the client request, process the file and send the response
void handle_request_and_send_response(int client_socket_fd, char *buffer, struct sockaddr_in *client_address) {
    int file_fd;
    char *file;
    long len;
    char *file_type = NULL;

    // Check if the file extension is supported
    for (int i = 0; extensions[i].ext != 0; i++) {
        if (strstr(buffer, extensions[i].ext)) {
            file_type = extensions[i].filetype;
            break;
        }
    }

    if (!file_type) {
        // Log and send Forbidden error if the file extension is unsupported
        logger(FORBIDDEN, "File extension not supported", file_type, client_socket_fd, client_address);
        return;
    }

    // Determine the requested file path (skip "GET /")
    file = buffer + 5;

    // Remove the rest of the HTTP request from the buffer
    for (int i = 5; i < BUFFER_SIZE; i++){
        if (buffer[i] == ' '){
            buffer[i] = 0;
            break;
        }
    }
    
    // Check if file exists
    if (access(file, F_OK) == -1) {
        logger(NOTFOUND, "File not found", file, client_socket_fd, client_address);
        return;
    }

    // Open file for reading
    file_fd = open(file, O_RDONLY);
    if (file_fd == -1) {
        logger(ERROR, "Opening file", file, client_socket_fd, client_address);
        return;
    }

    // Get the file length
    len = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);

    // Send the HTTP response header for a successful request
    send_response_header(client_socket_fd, "200 OK", "WillsWebServer", file_type, len);

    // Send the file content using sendfile
    sendfile(client_socket_fd, file_fd, 0, len);

    close(file_fd);
    close(client_socket_fd);
}

// Print client info and read request
void process_client_request(int client_socket_fd, int server_socket_fd, struct sockaddr_in *client_address) {
    char buffer[BUFFER_SIZE];
    int n;

    // Read the request from the client
    bzero(buffer, BUFFER_SIZE);
    n = read(client_socket_fd, buffer, BUFFER_SIZE);
    if (n < 0) {
        logger(ERROR, "Reading from socket", strerror(errno), client_socket_fd, client_address);
    }

    // Validate the GET request (console message)
    if (strncmp(buffer, "GET ", 4) != 0) {
        logger(METHOD_NOT_ALLOWED, "Invalid request", "Only GET requests allowed", client_socket_fd, client_address);
        return;
    }

    // Remove headers after the request line
    char *line_end = strstr(buffer, "\r\n");
    
    if (line_end != NULL) {
        // Null-terminate the string at the end of the request line
        *line_end = '\0';
    }

    // Log the received request
    logger(LOG, "Received request", buffer, client_socket_fd, client_address);

    // Process the requested file
    handle_request_and_send_response(client_socket_fd, buffer, client_address);
}
