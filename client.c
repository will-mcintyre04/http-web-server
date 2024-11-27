#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr; // Address of server we want to connect to
    struct hostent *server; // Pointer to structure of type hostent

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    // does DNS look to convert human readable hostname into IP address
    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // Print the official hostname
    printf("Official hostname: %s\n", server->h_name);

    // Loop through the list of IP addresses and print them
    struct in_addr *address;
    for (int i = 0; server->h_addr_list[i] != NULL; i++) {
        address = (struct in_addr *)server->h_addr_list[i];
        printf("IP Address %d: %s\n", i + 1, inet_ntoa(*address)); 
    }

    // Zero buffer
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    // Because server->h_addr is a character string, we use bcopy to set the server address
    bcopy((char *)server->h_addr, // h_addr is an alias for the first address in the array of network addresses
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno); //converts to network byte order

    // Connect to server, client gets assigned a random port number here by the system
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}