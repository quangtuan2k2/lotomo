#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 16384

#define ERROR_OPEN_FILE 20
#define ERROR_SIZE_FILE 21

#define NOTIFY_ERROR_CONNECTION printf("> error: can not connect to server\n")
#define NOTIFY_ERROR_SEND_MESSAGE printf("> error: can not send to server\n")
#define NOTIFY_ERROR_RECV_MESSAGE printf("> error: can not receive from server\n")
#define NOTIFY_ERROR_OPEN_FILE(FILENAME) printf("> error: can not open file %s\n", FILENAME)
#define NOTIFY_ERROR_READ_FILE(FILENAME) printf("> error: can not read file %s\n", FILENAME)
#define NOTIFY_ERROR_ULDP_COMMAND printf("> error: command not found\n")
#define NOTIFY_ERROR_SIZE_FILE(FILENAME) printf("> error: file %s size is not correct\n", FILENAME)

#define NOTIFY_PROGRESS_UPLOAD(PROGRESS) printf("> upload: %d %%\n", PROGRESS)
#define NOTIFY_SUCCESSFUL_UPLOAD printf("> upload: successfully !\n")
#define NOTIFY_CLOSE_CONNECTION printf("> connection: closed !\n")

#define CLIENT_COMMAND printf("\nEnter input to send: ");

const char MESSAGE_SEND_FILE[] = "Please send file";
const char MESSAGE_SUCCESSFUL_UPLOAD[] = "Successful upload";

/**
 * @brief Split UPLD command to parameter
 * 
 * @param request message from client send
 * @param filename variable to store file name
 * @param filesize variable to store file size
 * @return int 1 if is valid command.
 *             0 if not
 */
int split_upld_cmd(char * ,char * ,int * );

/**
 * @brief Check file exists or not, file size is valid or not, find error.
 * 
 * @param filename file name
 * @param filesize file size
 * @return int ERROR_OPEN_FILE if file not exists.
 *             ERROR_SIZE_FILE if size is not correct.
 *             0 is file is valid
 */
int ferror_file(char *, int );

/**
 * @brief send file from client to server in socket
 *
 * @param client_sock socket connect with server
 * @param filename file name to send
 */
void send_file(int ,char * ,int );

/**
 * @brief launch client connect to server
 *
 * @param server_addr
 * @param server_port
 */
void launch(char* ,int );


int main(int argc, char **argv)
{

    int server_port;
    char server_addr[253];

    if (argc != 3)
    {
        printf("Error: Invalid amount of parameters\n");
        return 1;
    }

    strcpy(server_addr, argv[1]);
    server_port = atoi(argv[2]);


    launch(server_addr, server_port);

    return 0;
}


int split_upld_cmd(char *request, char *filename, int *filesize)
{
    char format[] = "UPLD %s %d";
    if (sscanf(request, format, filename, filesize) == 2)
        return 1;
    else
        return 0;
}


int ferror_file(char* filename, int filesize) {
    FILE* fp;
    fp = fopen(filename, "rb");

    if (fp == NULL) 
    {
        NOTIFY_ERROR_OPEN_FILE(filename);
        return ERROR_OPEN_FILE;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    if(filesize != size)
    {
        NOTIFY_ERROR_SIZE_FILE(filename);
        return ERROR_SIZE_FILE;
    }

    fclose(fp);
    return 0;
}


void send_file(int client_sock, char *filename, int filesize)
{
    int buff_bytes, sent_bytes, received_bytes, total_bytes = 0;
    char buff[BUFF_SIZE];

    FILE *fp;
    fp = fopen(filename, "rb");

    while (1)
    {
        int read_bytes = fread(buff, 1, BUFF_SIZE, fp);
        total_bytes += read_bytes; // count bytes send to server

        if (read_bytes > 0) {
            if (send(client_sock, buff, read_bytes, 0) == -1)
                NOTIFY_ERROR_SEND_MESSAGE;
            else {
                // notify progress status send bytes to server
                int progress = total_bytes * 100 / filesize;
                NOTIFY_PROGRESS_UPLOAD(progress);
            }
        }

        if (read_bytes < BUFF_SIZE)
        {
            if (feof(fp))
                break;
            else
            {
                NOTIFY_ERROR_READ_FILE(filename);
                return;
            }
        }
        bzero(buff, BUFF_SIZE);
        usleep(500000);
    }

    fclose(fp);
    usleep(500000);

    // send "Successful upload" message to server
    sent_bytes = send(client_sock, MESSAGE_SUCCESSFUL_UPLOAD, strlen(MESSAGE_SUCCESSFUL_UPLOAD), 0);

    // wait for receiving "Successful upload" message from server
    received_bytes = recv(client_sock, buff, BUFF_SIZE, 0);
    buff[received_bytes] = '\0';

    if (strcmp(buff, MESSAGE_SUCCESSFUL_UPLOAD) == 0)
    {
        NOTIFY_SUCCESSFUL_UPLOAD;
    } else {
        printf("\n");
    }
}


void launch(char *server_addr, int server_port)
{
    printf("> start: connecting to %s:%d ...\n", server_addr, server_port);

    int client_sock;
    struct sockaddr_in server;

    // Construct socket
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        NOTIFY_ERROR_CONNECTION;
    }

    // Specify server address
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = inet_addr(server_addr);


    // Request to connect to server
    if (connect(client_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0)
    {
        NOTIFY_ERROR_CONNECTION;
        exit(EXIT_FAILURE);
    }

    // Communicate with server
    printf("Welcome to file server!\n");
    while (1)
    {
        char buff[BUFF_SIZE + 1], message[BUFF_SIZE], filename[FILENAME_MAX];
        int msg_len, sent_bytes, received_bytes, filesize;

        CLIENT_COMMAND;

        // Enter input to send
        memset(buff, '\0', (strlen(buff) + 1));
        fgets(buff, BUFF_SIZE, stdin);
        msg_len = strlen(buff);

        if(strcmp(buff, "\n") == 0) exit(EXIT_SUCCESS);

        int sp; // split UPLD command status
        if((sp = split_upld_cmd(buff, filename, &filesize)) == 1)
        {
            if(ferror_file(filename, filesize) != 0) {
                continue;
            }

            // send message to server
            sent_bytes = send(client_sock, buff, msg_len, 0);
            if (sent_bytes < 0)
                NOTIFY_ERROR_SEND_MESSAGE;

            // wait for receiving "Please send file" message, it message correct, send file
            received_bytes = recv(client_sock, message, BUFF_SIZE, 0);
            if (received_bytes < 0)
                NOTIFY_ERROR_RECV_MESSAGE;
            else if (received_bytes == 0)
                NOTIFY_CLOSE_CONNECTION;
            else
            {
                if(strcmp(message, MESSAGE_SEND_FILE) == 0) {
                    send_file(client_sock, filename, filesize);
                }
            }
        }
        else if(sp == 0)
            NOTIFY_ERROR_ULDP_COMMAND;
        else {
            break;
        }
    }

    close(client_sock);
}