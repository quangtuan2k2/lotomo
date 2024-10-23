#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>

void resolveHost(char *hostname)
{
    struct addrinfo *result;
    struct addrinfo *res;
    char ipStr[INET_ADDRSTRLEN];
    char mainIp[INET_ADDRSTRLEN] = {0};
    char lastIp[INET_ADDRSTRLEN] = {0};

    int error = getaddrinfo(hostname, NULL, NULL, &result);
    if (error != 0)
    {
        if (error == EAI_SYSTEM)
        {
            perror("getaddrinfo");
        }
        else
        {
            printf("No information found\n");
        }
        exit(EXIT_FAILURE);
    }
    int found = 0;
    for (res = result; res != NULL; res = res->ai_next)
    {
        if (res->ai_family == AF_INET)
        {
            struct sockaddr_in *address = (struct sockaddr_in *)res->ai_addr;
            inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
            if (found == 0)
            {
                printf("Main IP: %s\n", ipStr);
                strncpy(mainIp, ipStr, sizeof(mainIp));
            }
            else
            {
                if (strcmp(mainIp, ipStr) != 0 && strcmp(lastIp, ipStr) != 0)
                {
                    printf("Alternative IP: %s\n", ipStr);
                }
            }
            strncpy(lastIp, ipStr, sizeof(lastIp));
            found = 1;
        }
    }
    if (found == 0)
    {
        printf("No information found\n");
    }
    freeaddrinfo(result);
}

void resolveIp(char *ip)
{
    struct sockaddr_in addr;
    char hostname[NI_MAXHOST];
    char servInfo[NI_MAXSERV];
    struct addrinfo hints, *res, *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        printf("No information found\n");
        return;
    }

    ret = getnameinfo((struct sockaddr *) &addr, sizeof(struct sockaddr), hostname, NI_MAXHOST, servInfo, NI_MAXSERV, 0);
    if (ret != 0) {
        printf("No information found\n");
        return;
    } else {
        printf("Main name: %s\n", hostname);
    }

    ret = getaddrinfo(hostname, NULL, &hints, &res);
    if (ret != 0) {
        printf("No information found\n");
        return;
    }

    int found = 0;
    for (p = res; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            struct sockaddr_in *address = (struct sockaddr_in *)p->ai_addr;
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
            if (strcmp(ipStr, ip) != 0) {
                if (found == 0) {
                    printf("Alternate names: ");
                }
                printf("%s\n", ipStr);
                found = 1;
            }
        }
    }

    if (found == 0) {
        printf("No alternate names found\n");
    }

    freeaddrinfo(res);
}

int isValidIp(char *input) {
    int ndots = 0, ndigits = 0;
    for(int i = 0; input[i]; ++i) {
        if(isdigit(input[i])) {
            ndigits++;
        } else if(input[i] == '.') {
            ndots++;
        } else {
            return 0;
        }
    }
    if (ndots != 3 || ndigits < 4) {
        return 0;
    }
    return 1;
}

int isValidHostname(char *input) {
    int linput = strlen(input), flag = 1;
    if(linput == 0 || linput > 253) {
        return 0;
    }
    for(int i = 0; i < linput; i++) {
        if(input[i] == '.') {
            if(i == 0 || i == linput - 1 || flag != 0) {
                return 0;
            }
            flag = 1;
        } else if (isalnum(input[i]) || input[i] == '-') {
            flag = 0;
        } else {
            return 0;
        }
    }
    if(flag != 0) {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Invalid option\n");
        return 1;
    }

    int option = atoi(argv[1]);
    char *parameter = argv[2];

    if (option != 1 && option != 2)
    {
        fprintf(stderr, "Invalid option\n");
        return 1;
    }

    if(isValidIp(parameter) == 0 && isValidHostname(parameter) == 0) {
        fprintf(stderr, "Invalid Option");
    }

    if (option == 1)
    {
        if (isValidIp(parameter) == 0)
        {
            fprintf(stderr, "Invalid option\n");
            return 1;
        }
        resolveIp(parameter);
    }
    else
    {
        if (isValidIp(parameter) == 1 ||isValidHostname(parameter) == 0)
        {
            fprintf(stderr, "Invalid option\n");
            return 1;
        }
        resolveHost(parameter);
    }

    return 0;
}