#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>

int main(int argc, char*argv[]){
    int client_socket_fd; // File descriptor for the client socket

    struct sockaddr_in cli_addr; // Structure containing IP and port
    socklen_t client_length = sizeof(cli_addr); // Stores size of the address of the client

    HTTP_Server http_server; // Server struct with socket/port

    int portno; // Port number for the server to run on

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
    init_server(&http_server, portno);

    // Ignore the SIGCHLD signal, meaning child processes will be reaped automatically
    signal(SIGCHLD, SIG_IGN);

    while(1){
        // Accept system call causes the process to block until a client connects to the server
        client_socket_fd = accept(http_server.socket, (struct sockaddr *) &cli_addr, &client_length);

        if (client_socket_fd < 0){
            perror("ERROR on accept");
            exit(1);
        }

        // Make child process
        pid_t pid = fork();

        if (pid < 0){
            perror("Error on fork");
            continue;
        }

        // Child process
        if (pid == 0){
            process_client_request(client_socket_fd, http_server.socket, &cli_addr);
            close(http_server.socket);
            exit(0);
        // Parent process
        } else {
            close(client_socket_fd);
        }
    }
    close(http_server.socket);
    return 0;
}