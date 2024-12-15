#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0){
            perror("ERROR on binding");
            exit(1);
            }

    // System call that allows process to listen on the socket for connections
    // 5 is size of backlog queue(# of connections that can be waiting)
    listen(server_socket_fd,5);

    http_server->socket = server_socket_fd;
    printf("Server initialized and listening on port %d\n", http_server->port);
}

// int main(int argc, char *argv[]){
//     // server_socket_fd and newserver_socket_fd are file descriptors
//     int server_socket_fd, newserver_socket_fd, portno;

//     // Stores the size of the address of the client
//     socklen_t clilen;
//     char buffer[256]; // Server reads into this buffer
//     struct sockaddr_in serv_addr, cli_addr; // Structure containing internet address (IP and port)
//     int n; // Return value for the read() and write() system calls (num of chars read or written)
    // /*
    // this checks if there is a port or not (name of program, then port number)
    // argv[0] -> program name
    // argv[1] -> argument (port number)
    // */
    // if (argc < 2){
    //     fprintf(stderr, "ERROR, no port provided\n");
    //     exit(1);
    // }

    // /*
    // the socket() system call creates a new socket, taking in the address domain (IPv4)
    // the type of socket (stream vs datagram) and the protocol
    // (it will automatically choose TCP for stream)
    // */
//     server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket_fd < 0)
//         error("ERROR opening socket");

//     // Sets all the values in a buffer to zero, takes a pointer to the buffer and size
//     bzero((char *) &serv_addr, sizeof(serv_addr));

//     // Fetches the port number from argument
//     portno = atoi(argv[1]);

//     // Set the server address structure fields
//     serv_addr.sin_family = AF_INET;
//     // Contains IP address of the host, IP address of the machine that the server is running,
//     // can accept incoming connections on any network interface
//     serv_addr.sin_addr.s_addr = INADDR_ANY; //Listening on all network interfaces
//     serv_addr.sin_port = htons(portno); // Supports little endian computers for network byte order
    
//     /*
//     Bind system call, binds a socket to an address
//     */
//     if (bind(server_socket_fd, (struct sockaddr *) &serv_addr,
//             sizeof(serv_addr)) < 0) 
//             error("ERROR on binding");

//     // System call that allows process to listen on the socket for connections
//     // 5 is size of backlog queue(# of connections that can be waiting)
//     listen(server_socket_fd,5);

//     clilen = sizeof(cli_addr);
    /*
    Accept system call causes the process to block until a client connects to the server
    */
//     newserver_socket_fd = accept(server_socket_fd, 
//                  (struct sockaddr *) &cli_addr, 
//                  &clilen);
//     if (newserver_socket_fd < 0) 
//         error("ERROR on accept");

//     // Use getpeername() to get the client info
//     struct sockaddr_in peer_addr;
//     socklen_t peer_addr_len = sizeof(peer_addr);
//     if (getpeername(newserver_socket_fd, (struct sockaddr *)&peer_addr, &peer_addr_len) < 0) {
//         error("ERROR getting client info");
//     }

//     // Print the client's IP and port
//     printf("Client IP address: %s\n", inet_ntoa(peer_addr.sin_addr));
//     printf("Client port number: %d\n", ntohs(peer_addr.sin_port));

//     // WE ONLY GET TO THIS POINT ONCE A CLIENT HAS CONNECTED TO OUR SERVER
//     bzero(buffer,256);
//     n = read(newserver_socket_fd,buffer,255);// Reads from socket, will block until there is somethin for it to read in the socket
//     if (n < 0) error("ERROR reading from socket");
//     printf("Here is the message: %s\n",buffer);
    
    
//     n = write(newserver_socket_fd,"I got your message",18);
//     if (n < 0) error("ERROR writing to socket");
//     close(newserver_socket_fd);
//     close(server_socket_fd);
//     return 0; 
// }