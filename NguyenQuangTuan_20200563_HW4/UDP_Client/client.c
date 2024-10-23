#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#define BUFF_SIZE 2048

void runClient(char* ip, int port) {

    int client_sock;
    char buff[BUFF_SIZE];
    int sent_bytes, receive_bytes;
    struct sockaddr_in server_addr;
    socklen_t sin_size;

    // Construct a UDP socket
    if((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("\nError");
        return;
    }

    // Define the address of server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Get input from stdin
    memset(buff, '\0', strlen(buff)+1);
    fgets(buff, BUFF_SIZE, stdin);

    // Check for normal hostname or ip input (less than 253 characters)
    if(strlen(buff) > 254) return ;

    // Communicate with server
    sin_size = sizeof(struct sockaddr);
    sent_bytes = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *) &server_addr, sin_size);
    if(sent_bytes < 0) {
        perror("\nError");
    }

    receive_bytes = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *) &server_addr, &sin_size);
    if (receive_bytes < 0) {
        perror("\nError");
    }

    buff[receive_bytes] = '\0';
    printf("%s\n", buff);

    close(client_sock);
}

int main(int argc, char** argv) {

    char SERV_IP[254];
    strcpy(SERV_IP, argv[1]);
    int SERV_PORT = atoi(argv[2]);

    printf("Connect to %s:%d\n", SERV_IP, SERV_PORT);

    while(1) {
        runClient(SERV_IP, SERV_PORT);
    }

    return 0;
}