#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char*argv[]){
    // File descriptor for the client socket
    int client_socket_fd;

    socklen_t client_length; // Stores size of the address of the client
    struct sockaddr_in cli_addr; // Structure containing IP and port

    HTTP_Server http_server; // Server struct with socket/port


    char buffer[1024]; // Server reads into this buffer
    int portno; // Port number for the server to run on
    int n; // Return value for the read() and write() system calls (num of chars read)

    /*
    This checks if there is a port or not (name of program, then port number)
    argv[0] -> program name
    argv[1] -> argument (port number)
    */
    if (argc < 2){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    portno = atoi(argv[1]);

    init_server(&http_server, 6969);

    client_length = sizeof(cli_addr);

    // Accept system call causes the process to block until a client connects to the server
    client_socket_fd = accept(http_server.socket, (struct sockaddr *) &cli_addr, &client_length);
    if (client_socket_fd < 0){
        perror("ERROR on accept");
        exit(1);
    }

    // Use getpeername() to get the client info
    if (getpeername(client_socket_fd, (struct sockaddr *)&cli_addr, &client_length) < 0) {
        perror("ERROR getting client info");
        exit(1);
    }


    // Print the client's IP and port
    printf("Client IP address: %s\n", inet_ntoa(cli_addr.sin_addr));
    printf("Client port number: %d\n", ntohs(cli_addr.sin_port));

    bzero(buffer,1024);
    n = read(client_socket_fd,buffer,1024);// Reads from socket, will block until there is somethin for it to read in the socket
    if (n < 0) perror("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
    
    
    // Close sockets
    close(client_socket_fd);
    close(http_server.socket);
    return 0;
}