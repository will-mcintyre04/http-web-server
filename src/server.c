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

void print_client_info_and_read(int client_socket_fd, int server_socket_fd, struct sockaddr_in *client_address){
    char buffer[BUFFER_SIZE]; // Server reads into this buffer
    int n; // Return value for the read() and write() system calls (num of chars read)
    
    // Print the client's IP and port
    printf("Client IP address: %s\n", inet_ntoa(client_address->sin_addr));
    printf("Client port number: %d\n", ntohs(client_address->sin_port));

    bzero(buffer,BUFFER_SIZE);
    n = read(client_socket_fd,buffer,BUFFER_SIZE);// Reads from socket, will block until there is somethin for it to read in the socket
    if (n < 0) perror("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);

    char *f = buffer + 5;
    printf("FILE: %s", f);
    int opened_fd = open(f, O_RDONLY);
    if (opened_fd == -1) {
        perror("Error opening file");
        return;  // Early exit if file cannot be opened
    }
    sendfile(client_socket_fd, opened_fd, 0, BUFFER_SIZE);
    close(opened_fd);

    // Close the client socket after handling the request
    close(client_socket_fd);
}