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

#define ERROR 42
#define LOG 44
#define FORBIDDEN 403
#define NOTFOUND  404
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

// Function to log errors and responses
void logger(int type, char *s1, char *s2, int socket_fd) {
    int fd;
    char logbuffer[BUFFER_SIZE * 2];

    switch (type) {
        case ERROR:
            sprintf(logbuffer, "ERROR: %s:%s Errno=%d exiting pid=%d", s1, s2, errno, getpid());
            break;
        case FORBIDDEN:
            // Write the response header for Forbidden error
            (void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head><title>403 Forbidden</title></head><body><h1>Forbidden</h1>The requested URL, file type or operation is not allowed on this simple static file webserver.</body></html>\n", 267);
            sprintf(logbuffer, "FORBIDDEN: %s:%s", s1, s2);
            break;
        case NOTFOUND:
            // Write the response header for Not Found error
            (void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1>The requested URL was not found on this server.</body></html>\n", 220);
            sprintf(logbuffer, "NOT FOUND: %s:%s", s1, s2);
            break;
        case LOG:
            sprintf(logbuffer, " INFO: %s:%s:%d", s1, s2, socket_fd);
            break;
    }

    // Log the information to the log file
    if ((fd = open("nweb.log", O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
        write(fd, logbuffer, strlen(logbuffer));
        write(fd, "\n", 1);
        close(fd);
    }

    // Exit after an error or if Forbidden/Not Found to prevent further processing
    if (type == ERROR || type == NOTFOUND || type == FORBIDDEN){
        exit(3);
        printf("exiting");
    }
}

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

void send_response_header(int client_socket_fd, const char * status, const char * server, const char * content_type, long content_length){
    char header[BUFFER_SIZE];
    sprintf(header, "HTTP/1.0 %s\nServer: %s\nContent-Type: %s\nContent-Length: %ld\n\n", status, server, content_type, content_length);
    int n = write(client_socket_fd,header,strlen(header));
    if (n < 0){
        perror("ERROR writing to socket");
        exit(1);
    }
}
// Function to handle the client request, process the file and send the response
void handle_request(int client_socket_fd, char *buffer) {
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
        logger(FORBIDDEN, "File extension not supported", buffer, client_socket_fd);
        return;
    }

    // Determine the requested file path (skip "GET /")
    file = buffer + 5;

    for (int i = 5; i < BUFFER_SIZE; i++){
        if (buffer[i] == ' '){
            buffer[i] = 0;
            break;
        }
    }

    printf("FILE: %s", file);

    if (access(file, F_OK) == -1) { // Check if file exists
        // Log and send Not Found error if file does not exist
        logger(NOTFOUND, "File not found", file, client_socket_fd);
        return;
    }

    // Open file for reading
    file_fd = open(file, O_RDONLY);
    if (file_fd == -1) {
        perror("Error opening file");
        return;
    }

    // Get the file length
    len = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);

    // Send the HTTP response header for a successful request
    send_response_header(client_socket_fd, "200 OK", "MySimpleServer", file_type, len);

    // Send the file content using sendfile
    sendfile(client_socket_fd, file_fd, 0, len);

    close(file_fd);
    close(client_socket_fd);
}

// Print client info and process the request
void print_client_info_and_read(int client_socket_fd, int server_socket_fd, struct sockaddr_in *client_address) {
    char buffer[BUFFER_SIZE];
    int n;

    // Print the client's IP and port
    printf("Client IP address: %s\n", inet_ntoa(client_address->sin_addr));
    printf("Client port number: %d\n", ntohs(client_address->sin_port));

    // Read the request from the client
    bzero(buffer, BUFFER_SIZE);
    n = read(client_socket_fd, buffer, BUFFER_SIZE);
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("Here is the message:\n%s", buffer);

    // Validate the GET request
    if (strncmp(buffer, "GET ", 4) != 0) {
        printf("The server only accepts simple GET requests");
        exit(1);
    }

    // Process the requested file
    handle_request(client_socket_fd, buffer);
}