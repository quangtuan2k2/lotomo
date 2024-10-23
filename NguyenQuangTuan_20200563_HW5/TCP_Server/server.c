/*TCP Echo Server*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

#define BACKLOG 2
#define BUFF_SIZE 16384
#define LOG_FILE "log_20200563.txt"

#define NOTIFY_ERROR_SOCK printf("Error: can not construct socket \n")
#define NOTIFY_ERROR_BIND printf("Error: can not bind address to socket\n")
#define NOTIFY_ERROR_CONNECTION printf("Error: can not connect to client\n")
#define NOTIFY_ERROR_SEND_MESSAGE printf("Error: can not send to client\n")
#define NOTIFY_ERROR_RECV_MESSAGE printf("Error: can not receive from client\n")
#define NOTIFY_ERROR_OPEN_FILE(FILENAME) printf("> error: can not open file %s\n", FILENAME)
#define NOTIFY_ERROR_READ_FILE(FILENAME) printf("> error: can not read file %s\n", FILENAME)

#define NOTIFY_CLOSE_CONNECTION printf("Connection: closed !\n")

const char MESSAGE_SEND_FILE[] = "Please send file";
const char MESSAGE_REQUEST_FILENAME[] = "Request file name";
const char MESSAGE_SUCCESSFUL_UPLOAD[] = "Successful upload";
const char MESSAGE_ULDP_ERROR[] = "ULDP error";

char storage_path[FILENAME_MAX+10] = "./";
char storage_name[FILENAME_MAX];

typedef struct _Log {
    char timeline[80];
    char request[BUFF_SIZE];
    char response[254];
} Log;


/**
 * @brief Create a log object, with timeline is current time
 * 
 * @param status 1 if success, 0 if not
 * @param request string request from client
 * @param response result message to client
 * @return Log 
 */
Log create_log(char* request, char* response);


/**
 * @brief Write log message to file
 * 
 * @param log struct store status information
 * @param client_addr client IP address
 * @param client_port client port
 * @param mode 1 if log has request message. 0 if not
 */
void export_log(Log , char* , int , int);


/**
 * @brief Split UPLD command to parameter
 * 
 * @param request message from client send
 * @param filename variable to store file name
 * @param filesize variable to store file size
 * @return int 1 if is valid command.
 *             0 if not
 */
int split_upld_cmd(char *, char *, int *);


/**
 * @brief Handle successful file upload commands from the client
 * 
 * @param conn_sock 
 * @param client 
 */
void handle_client(int ,struct sockaddr_in );

void launch(int );

/* Main */

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Error: Invalid amount of parameters\n");
        return 1;
    }

    int server_port = atoi(argv[1]);

    strcpy(storage_name, argv[2]);
    strcat(storage_path, storage_name);

    DIR* dtr = opendir(storage_path);
    if(dtr == NULL) {
        printf("Error: Directory not exists\n");
        return 1;
    }
    closedir(dtr);

    launch(server_port);
    return 0;
}

/* Function */

Log create_log(char* request, char* response) {

    Log tmp; 
    
    time_t cur;
    struct tm *timeInfo;
    char timeline[80];

    time(&cur);
    timeInfo = localtime(&cur);
    strftime(timeline, sizeof(timeline), "%d/%m/%Y %H:%M:%S", timeInfo);

    strcpy(tmp.timeline, timeline);
    strcpy(tmp.request, request);
    strcat(tmp.response, response);

    return tmp;
}


void export_log(Log log, char* client_addr, int client_port, int mode) {
    FILE *f = fopen(LOG_FILE, "a");
    if(mode == 1) {
        fprintf(f, "[%s]$%s:%d$%s$%s\n", log.timeline, client_addr, client_port, log.request, log.response);
    }
    if(mode == 0) {
        fprintf(f, "[%s]$%s:%d$%s\n", log.timeline, client_addr, client_port, log.response);

    }
    fclose(f);
}


int split_upld_cmd(char *request, char *filename, int *filesize)
{
    char format[] = "UPLD %s %d";
    if (sscanf(request, format, filename, filesize) == 2)
        return 1;
    else
        return 0;
}


void handle_client(int conn_sock, struct sockaddr_in client)
{
    char filename[128];
    int filesize;

    int request_bytes, received_bytes, sent_bytes;
    char request_data[BUFF_SIZE], received_data[BUFF_SIZE];

    char client_addr[INET_ADDRSTRLEN];
    int client_port;

    inet_ntop(AF_INET, &client.sin_addr, client_addr, sizeof(client_addr));
    client_port = ntohs(client.sin_port);

    // Notify when a client accesses
    printf("[%s:%d]$ Welcome to file server\n", client_addr, client_port);
    export_log(create_log("", "+OK Welcome to file server"), client_addr, client_port, 0);

    while (1)
    {
        // Wait for UPLD success command from client
        request_bytes = recv(conn_sock, request_data, BUFF_SIZE, 0);
        if (request_bytes < 0)
        {
            printf("[%s:%d]$ Error when receive\n", client_addr, client_port);
            break;
        }
        else if (request_bytes == 0)
        {
            // Notify and create log when client disconnect
            printf("[%s:%d]$ Connection closed\n", client_addr, client_port);
            export_log(create_log("", "+OK Connection closed"), client_addr, client_port, 0);
            break;
        }
        else
        {
            Log log;
            request_data[request_bytes-1] = '\0';
            printf("[%s:%d]$ %s\n", client_addr, client_port, request_data);
            log = create_log(request_data, "");

            // split command to variable
            int file_info = split_upld_cmd(request_data, filename, &filesize);
            
            // send "Please send file" to client and wait client send file data
            sent_bytes = send(conn_sock, MESSAGE_SEND_FILE, strlen(MESSAGE_SEND_FILE), 0);
            if (sent_bytes < 0)
            {
                NOTIFY_ERROR_SEND_MESSAGE;
                break;
            }

            // create path to file store in storage directory
            char storage_filename[FILENAME_MAX];
            strcpy(storage_filename, storage_path);
            strcat(storage_filename, "/");
            strcat(storage_filename, filename);

            FILE *recv_file;
            recv_file = fopen(storage_filename, "wb"); 

            int total_size = 0; // count number of bytes receive from client

            while (1)
            {
                // receive file data
                received_bytes = recv(conn_sock, received_data, BUFF_SIZE, 0);
                received_data[received_bytes] = '\0';

                // client send "Successful upload" message
                if (strcmp(received_data, MESSAGE_SUCCESSFUL_UPLOAD) == 0)
                {
                    // End receive data
                    break;
                }

                // client disconnect when send data
                if (received_bytes <= 0)
                {
                    break;
                }

                int write_bytes = fwrite(received_data, 1, received_bytes, recv_file);
                total_size += write_bytes;

                if (write_bytes < received_bytes)
                {
                    perror("Error writing to file");
                    fclose(recv_file);
                    break;
                }
                bzero(received_data, BUFF_SIZE);
            }
            fclose(recv_file);

            // Handle number bytes server receive and number bytes in UPLD parameter
            if (total_size < filesize)
            {
                printf("[%s:%d]$ Error upload file\n", client_addr, client_port);
                int rm = remove(storage_filename);
                strcpy(log.response, "-ERR Failure upload");
                export_log(log, client_addr, client_port, 1);
            }
            else
            {
                // Send "Successful upload" message to client
                sent_bytes = send(conn_sock, MESSAGE_SUCCESSFUL_UPLOAD, strlen(MESSAGE_SUCCESSFUL_UPLOAD), 0);
                if (sent_bytes < 0)
                {
                    NOTIFY_ERROR_SEND_MESSAGE;
                    break;
                }
                printf("[%s:%d]$ Upload successfully\n", client_addr, client_port);
                strcpy(log.response, "+OK Successful upload");
                export_log(log, client_addr, client_port, 1);
            }
        }
    }

    close(conn_sock);
    exit(0);
}

void launch(int server_port)
{
    printf("Listen on port %d...\n", server_port);
    int listen_sock, conn_sock;
    char client_addr[INET_ADDRSTRLEN];

    int sent_bytes, received_bytes, sin_size, client_port;

    struct sockaddr_in server;
    struct sockaddr_in client;

    // Construct a TCP socket to listen connection request
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("\nError: ");
        exit(EXIT_FAILURE);
    }

    // Bind address to socket
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("\nError: ");
        exit(EXIT_FAILURE); 
    }

    // Listen request from client
    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("\nError: ");
        exit(EXIT_FAILURE);
    }

    // Communicate with server
    while (1)
    {
        // Accept request
        sin_size = sizeof(struct sockaddr_in);
        if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
        {
            perror("\nError: ");
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();
        if (child_pid < 0)
        {
            perror("\nError: ");
            exit(EXIT_FAILURE);
        }
        else if (child_pid == 0)
        {
            close(listen_sock);
            handle_client(conn_sock, client);
        }
        else
        {
            close(conn_sock);
        }
    }

    close(listen_sock);
}