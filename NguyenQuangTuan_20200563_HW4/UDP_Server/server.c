#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 2048

char NONE[] = "";

/**
 * @brief resolve hostname to ip address
 *
 * @param input string hostname
 * @param response string list of ip addresses
 * @return int status 1 if success
 *                    0 if fail or error
 */
int resolveHostname(char *input, char *response)
{
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(input)) == NULL)
    {
        return 0;
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    strcat(response, inet_ntoa(*addr_list[0]));
    for (int i = 1; addr_list[i] != NULL; i++)
    {
        strcat(response, " ");
        strcat(response, inet_ntoa(*addr_list[i]));
    }

    return 1;
}

/**
 * @brief resolve ip address to hostname
 *
 * @param input string ip address
 * @param response string list of hostnames
 * @return int status 1 if success
 *                    0 if fail or error
 */
int resolveIp(char *input, struct in_addr ip, char *response)
{
    struct hostent *he;

    if ((he = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL)
    {
        return 0;
    }

    strcat(response, he->h_name);
    for (int i = 0; he->h_aliases[i] != NULL; i++)
    {
        strcat(response, " ");
        strcat(response, he->h_aliases[i]);
    }

    return 1;
}

typedef struct Account
{
    char username[50];
    char password[50];
    int status;
    struct Account *next;
} Account;

Account *create(char *username, char *password, int status)
{
    Account *newAccount = (Account *)malloc(sizeof(Account));
    strcpy(newAccount->username, username);
    strcpy(newAccount->password, password);
    newAccount->status = status;
    newAccount->next = NULL;
    return newAccount;
}

void add(Account **head, Account *newAccount)
{
    if (*head == NULL)
    {
        *head = newAccount;
    }
    else
    {
        Account *temp = *head;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = newAccount;
    }
}

Account *readFile(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return NULL;
    }

    Account *head = NULL;
    Account *tail = NULL;
    char username[50], password[50];
    int status;

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        char *token = strtok(line, " ");
        if (token == NULL)
        {
            continue;
        }
        strcpy(username, token);

        token = strtok(NULL, " ");
        if (token == NULL)
        {
            continue;
        }
        strcpy(password, token);

        token = strtok(NULL, " ");
        if (token == NULL)
        {
            continue;
        }
        status = atoi(token);

        Account *newAccount = create(username, password, status);
        add(&head, newAccount);
    }

    fclose(file);
    return head;
}

void traverse(Account *head)
{
    Account *temp = head;
    while (temp != NULL)
    {
        printf("%s %s %d\n", temp->username, temp->password, temp->status);
        temp = temp->next;
    }
}

Account *findByUsername(Account *head, char *username)
{
    Account *temp = head;
    while (temp != NULL)
    {
        if (strcmp(temp->username, username) == 0)
        {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void setStatus(Account *head, char *username, int status)
{
    Account *account = findByUsername(head, username);
    if (account != NULL)
    {
        account->status = status;
    }
}

void writeFile(char *filename, Account *head)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        return;
    }

    Account *temp = head;
    while (temp != NULL)
    {
        fprintf(file, "%s %s %d\n", temp->username, temp->password, temp->status);
        temp = temp->next;
    }

    fclose(file);
}

typedef struct
{
    char username[50];
    int failed;
} LocalStorage;

LocalStorage store[100];

void initStore()
{
    for (int i = 0; i < 100; i++)
    {
        store[i].username[0] = '\0';
        store[i].failed = 0;
    }
}

void addFailed(char *username)
{
    int threshold = 3;
    for (int i = 0; i < 100; i++)
    {
        if (strcmp(store[i].username, NONE) == 0)
        {
            strcpy(store[i].username, username);
            store[i].failed = 1;
            return;
        }
        if (strcmp(store[i].username, username) == 0)
        {
            if (store[i].failed == threshold)
            {
                return;
            }
            store[i].failed++;
            return;
        }
    }
}

int checkFailed(char *username)
{
    int threshold = 3;
    for (int i = 0; i < 100; i++)
    {
        if (strcmp(store[i].username, NONE) == 0)
        {
            return 0;
        }
        if (strcmp(store[i].username, username) == 0)
        {
            if (store[i].failed == threshold)
            {
                return 1;
            }
        }
    }
    return 0;
}

typedef struct
{
    int isLogin;
    char currentUsername[50];
} Session;

// Execute server on port
void runServer(int port)
{
    Session session;
    session.isLogin = 0;
    session.currentUsername[0] = '\0';
    initStore();

    printf("Listen on port %d\n", port);

    Account *head = readFile("nguoidung.txt");
    traverse(head);

    int server_sock;

    int sent_bytes, receive_bytes;
    struct sockaddr_in server;
    struct sockaddr_in client;

    struct in_addr ip;

    socklen_t sin_size;

    // Construct a UDP socket
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("\nError");
        return;
    }

    // Bind address to socket
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == 1)
    {
        return;
    }
    int need_password = 1;
    char username[BUFF_SIZE], password[BUFF_SIZE];

    // Communicate with clients
    while (1)
    {
        char buff[BUFF_SIZE];

        sin_size = sizeof(struct sockaddr_in);
        int total_bytes = 0;
        receive_bytes = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
        buff[receive_bytes - 1] = '\0';
        printf("Received: %s\n", buff);

        if (receive_bytes < 0)
        {
            perror("\nError");
            continue;
        }

        if (session.isLogin == 0)
        {
            if (need_password == 1)
            {
                char message[] = "Insert password";
                strcpy(username, buff);
                sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                need_password = 0;
                continue;
            }
            else
            {
                strcpy(password, buff);
                Account *account = findByUsername(head, username);
                if (account == NULL || (account != NULL && account->status == 0))
                {
                    char message[] = "Account not ready";
                    sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                    need_password = 1;
                    continue;
                }
                else if (account != NULL && strcmp(account->password, password) != 0)
                {
                    addFailed(username);
                    if (checkFailed(username) == 1)
                    {
                        setStatus(head, username, 0);
                        writeFile("nguoidung.txt", head);
                        char message[] = "Account is blocked";
                        sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                        need_password = 1;
                        continue;
                    }
                    else
                    {
                        char message[] = "Not OK";
                        sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                        need_password = 1;
                        continue;
                    }
                }
                else if (account != NULL && strcmp(account->password, password) == 0)
                {
                    session.isLogin = 1;
                    strcpy(session.currentUsername, account->username);

                    char message[] = "OK";
                    sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                    continue;
                }
            }
        }
        else
        {
            char newPassword[50];
            buff[receive_bytes - 1] = '\0';
            strcpy(newPassword, buff);

            char LOGOUT_FLAG[] = "bye";
            char HOMEPAGE_FLAG[] = "homepage";
            if (strcmp(newPassword, LOGOUT_FLAG) == 0)
            {
                session.isLogin = 0;
                session.currentUsername[0] = '\0';
                char message[100];
                strcpy(message, "Goodbye ");
                strcat(message, session.currentUsername);
                sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                continue;
            }

            if (strcmp(newPassword, HOMEPAGE_FLAG) == 0)
            {
                char message[100];
                strcpy(message, "google.com");
                sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                continue;
            }

            // Check if newPassword contain character is not number or letter
            int isOk = 1;
            for (int i = 0; i < strlen(newPassword); i++)
            {
                if (!isalnum(newPassword[i]))
                {
                    isOk = 0;
                    break;
                }
            }
            if (isOk == 0)
            {
                char message[] = "Error";
                sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                continue;
            }
            else
            {
                char message[200];
                // Message is 2 string concatenated together example 1vasd143 to 1143\nvasd
                char number_part[50];
                char letter_part[50];
                int number_part_index = 0;
                int letter_part_index = 0;
                for (int i = 0; i < strlen(newPassword); i++)
                {
                    if (isdigit(newPassword[i]))
                    {
                        number_part[number_part_index++] = newPassword[i];
                    }
                    else
                    {
                        letter_part[letter_part_index++] = newPassword[i];
                    }
                }

                number_part[number_part_index] = '\0';
                letter_part[letter_part_index] = '\0';
                strcpy(message, number_part);
                strcat(message, "\n");
                strcat(message, letter_part);
                sent_bytes = sendto(server_sock, message, sizeof(message), 0, (struct sockaddr *)&client, sin_size);
                continue;
            }
        }
    }

    // Echo to client
    if (sent_bytes < 0)
    {
        perror("\nError");
    }

    close(server_sock);
}

int main(int argc, char **argv)
{
    int PORT = atoi(argv[1]);
    runServer(PORT);
    return 0;
}