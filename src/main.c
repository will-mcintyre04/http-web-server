#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char**argv){
    socklen_t client_length;
    HTTP_Server http_server;
    int client_socket_fd;
    struct sockaddr_in cli_addr;

    init_server(&http_server, 6969);

    client_length = sizeof(cli_addr);
    client_socket_fd = accept(http_server.socket, (struct sockaddr *) &cli_addr, &client_length);
    if (client_socket_fd < 0) 
        perror("ERROR on accept");

    // Use getpeername() to get the client info
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    if (getpeername(client_socket_fd, (struct sockaddr *)&peer_addr, &peer_addr_len) < 0) {
        perror("ERROR getting client info");
    }
    

    // Print the client's IP and port
    printf("Client IP address: %s\n", inet_ntoa(peer_addr.sin_addr));
    printf("Client port number: %d\n", ntohs(peer_addr.sin_port));

    // // WE ONLY GET TO THIS POINT ONCE A CLIENT HAS CONNECTED TO OUR SERVER
    // bzero(buffer,256);
    // n = read(newserver_socket_fd,buffer,255);// Reads from socket, will block until there is somethin for it to read in the socket
    // if (n < 0) error("ERROR reading from socket");
    // printf("Here is the message: %s\n",buffer);
    
    
    // n = write(newserver_socket_fd,"I got your message",18);
    // if (n < 0) error("ERROR writing to socket");
    // close(newserver_socket_fd);
    // close(server_socket_fd);
    return 0;
}