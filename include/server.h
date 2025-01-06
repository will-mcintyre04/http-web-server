#ifndef Server_h
#define Server_h

#include <netinet/in.h>

typedef struct HTTP_Server {
    int socket;
    int port;
    struct sockaddr_in serv_addr;
} HTTP_Server;

void init_server(HTTP_Server * http_server, int port);
void print_client_info_and_read(int client_socket_fd, int server_socket_fd, struct sockaddr_in* client_address);
void handle_request_and_send_response(int client_socket_fd, char *buffer, struct sockaddr_in *client_address);
#endif