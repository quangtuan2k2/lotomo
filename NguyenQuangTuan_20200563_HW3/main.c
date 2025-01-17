#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <arpa/inet.h>

#define ACCOUNT_FILE "nguoidung.txt"
#define HISTORY_FILE "history.txt"

#define MAX_USERNAME 20
#define MAX_PASSWORD 30
#define MAX_EMAIL 30
#define MAX_PHONE 10
#define PHONE_LENGTH 10
#define DEFAULT_STATUS 1

#define BUFFER_SIZE 255

typedef struct Account
{
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    int status;
    char homepage[BUFFER_SIZE];
    struct Account *next;
} Account;

Account *create_account(const char *username, const char *password, int status, const char *homepage)
{
    Account *new_account = (Account *)malloc(sizeof(Account));
    if (new_account == NULL)
    {
        printf("ERROR: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(new_account->username, username, sizeof(new_account->username) - 1);
    new_account->username[sizeof(new_account->username) - 1] = '\0';

    strncpy(new_account->password, password, sizeof(new_account->password) - 1);
    new_account->password[sizeof(new_account->password) - 1] = '\0';

    strncpy(new_account->homepage, homepage, sizeof(new_account->homepage) - 1);
    new_account->homepage[sizeof(new_account->homepage) - 1] = '\0';

    new_account->status = status;

    new_account->next = NULL;

    return new_account;
}

void add_account(Account **head, Account *new_account)
{
    if (*head == NULL)
    {
        *head = new_account;
        return;
    }
    Account *current = *head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_account;
}

Account *accounts_from_file(char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("ERROR: Failed to open file %s for reading.\n", filename);
        return NULL;
    }

    Account *head = NULL;
    Account *tail = NULL;
    char line[BUFFER_SIZE];

    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = '\0';
        char *token = strtok(line, " ");
        if (!token)
            continue;

        char username[BUFFER_SIZE];
        strncpy(username, token, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';

        token = strtok(NULL, " ");
        if (!token)
            continue;
        char password[BUFFER_SIZE];
        strncpy(password, token, sizeof(password) - 1);
        password[sizeof(password) - 1] = '\0';

        token = strtok(NULL, " ");
        if (!token)
            continue;
        int status = atoi(token);

        token = strtok(NULL, " ");
        if (!token)
            continue;
        char homepage[BUFFER_SIZE];
        strncpy(homepage, token, sizeof(homepage) - 1);
        homepage[sizeof(homepage) - 1] = '\0';

        Account *account = create_account(username, password, status, homepage);
        if (head == NULL)
        {
            head = account;
            tail = account;
        }
        else
        {
            tail->next = account;
            tail = account;
        }
    }
    fclose(f);
    return head;
}

void free_accounts(Account *head)
{
    Account *current = head;
    while (current != NULL)
    {
        Account *next = current->next;
        free(current);
        current = next;
    }
}

Account *find_account_by_username(Account *head, char *username)
{
    Account *current = head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void save_accounts(Account *head, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("ERROR: Failed to open file %s for writing.\n", filename);
        return;
    }

    Account *current = head;
    while (current != NULL)
    {
        fprintf(file, "%s %s %d %s\n", current->username, current->password, current->status, current->homepage);
        current = current->next;
    }

    fclose(file);
}

typedef struct History
{
    char username[MAX_USERNAME];
    char login_date[50];
    char login_time[50];
    struct History *next;
} History;

History *create_history(const char *username, const char *login_date, const char *login_time)
{
    History *new_history = (History *)malloc(sizeof(History));
    if (new_history == NULL)
    {
        printf("ERROR: Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(new_history->username, username, sizeof(new_history->username) - 1);
    new_history->username[sizeof(new_history->username) - 1] = '\0';

    strncpy(new_history->login_date, login_date, sizeof(new_history->login_date) - 1);
    new_history->login_date[sizeof(new_history->login_date) - 1] = '\0';

    strncpy(new_history->login_time, login_time, sizeof(new_history->login_time) - 1);
    new_history->login_time[sizeof(new_history->login_time) - 1] = '\0';

    new_history->next = NULL;

    return new_history;
}

void add_history(History **head, History *new_history)
{
    if (*head == NULL)
    {
        *head = new_history;
        return;
    }
    History *current = *head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_history;
}

History *histories_from_file(char *filename)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("ERROR: Failed to open file %s for reading.\n", filename);
        return NULL;
    }

    History *head = NULL;
    History *tail = NULL;
    char line[BUFFER_SIZE];

    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = '\0';
        char *token = strtok(line, " | ");
        if (!token)
            continue;

        char username[MAX_USERNAME];
        strncpy(username, token, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';

        token = strtok(NULL, " | ");
        if (!token)
            continue;
        char login_date[50];
        strncpy(login_date, token, sizeof(login_date) - 1);
        login_date[sizeof(login_date) - 1] = '\0';

        token = strtok(NULL, " | ");
        if (!token)
            continue;
        char login_time[50];
        strncpy(login_time, token, sizeof(login_time) - 1);
        login_time[sizeof(login_time) - 1] = '\0';

        History *history = create_history(username, login_date, login_time);
        if (head == NULL)
        {
            head = history;
            tail = history;
        }
        else
        {
            tail->next = history;
            tail = history;
        }
    }
    fclose(f);
    return head;
}

void save_histories(History *head, char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("ERROR: Failed to open file %s for writing.\n", filename);
        return;
    }

    History *current = head;
    while (current != NULL)
    {
        fprintf(file, "%s | %s | %s\n", current->username, current->login_date, current->login_time);
        current = current->next;
    }

    fclose(file);
}

History *find_list_history_by_username(History *head, char *username)
{
    History *current = head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void print_list_history(History *head)
{
    printf("Date\t\tTime\n");
    History *current = head;
    while (current != NULL)
    {
        printf("%s\t%s\n", current->login_date, current->login_time);
        current = current->next;
    }
}

void free_histories(History *head)
{
    History *current = head;
    while (current != NULL)
    {
        History *next = current->next;
        free(current);
        current = next;
    }
}

void show_menu()
{
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("\t1. Register\n");
    printf("\t2. Sign in\n");
    printf("\t3. Change password\n");
    printf("\t4. Update account info\n");
    printf("\t5. Reset password\n");
    printf("\t6. View login history\n");
    printf("\t7. Homepage with domain name\n");
    printf("\t8. Homepage with IP address\n");
    printf("\t9. Log out\n\n");
}

void current_date(char *buf, int size)
{
    time_t now_time = time(NULL);
    struct tm *t = localtime(&now_time);
    strftime(buf, size, "%Y-%m-%d", t);
}

void current_time(char *buf, int size)
{
    time_t now_time = time(NULL);
    struct tm *t = localtime(&now_time);
    strftime(buf, size, "%H:%M:%S", t);
}

int is_valid_username(char *username)
{
    int is_valid = 1;
    if (strlen(username) == 0 || strlen(username) > MAX_USERNAME)
    {
        printf("ERROR: Username must not be empty and must not exceed %d characters.\n", MAX_USERNAME);
        is_valid = 0;
    }

    for (size_t i = 0; i < strlen(username); i++)
    {
        if (!((username[i] >= 'a' && username[i] <= 'z') || 
              (username[i] >= 'A' && username[i] <= 'Z') || 
              (username[i] >= '0' && username[i] <= '9')))
        {
            printf("ERROR: Username must only contain alphabet characters and numbers.\n");
            is_valid = 0;
            break;
        }
    }

    return is_valid;
}

int is_valid_password(char *password)
{
    int is_valid = 1;
    if (strlen(password) < 6 || strlen(password) > MAX_PASSWORD)
    {
        printf("ERROR: Password must be at least 6 characters long and must not exceed %d characters.\n", MAX_PASSWORD);
        is_valid = 0;
    }

    int has_alpha = 0, has_num = 0;
    for (size_t i = 0; i < strlen(password); i++)
    {
        if ((password[i] >= 'a' && password[i] <= 'z') || (password[i] >= 'A' && password[i] <= 'Z'))
        {
            has_alpha = 1;
        }
        else if (password[i] >= '0' && password[i] <= '9')
        {
            has_num = 1;
        }
    }

    if (!has_alpha || !has_num)
    {
        printf("ERROR: Password must contain at least one alphabet, one number.\n");
        is_valid = 0;
    }

    return is_valid;
}

int is_valid_otp(char *otp)
{
    int is_valid = 1;
    if (strlen(otp) != 6)
    {
        printf("ERROR: OTP must be 6 digits.\n");
        is_valid = 0;
    }

    for (size_t i = 0; i < strlen(otp); i++)
    {
        if (otp[i] < '0' || otp[i] > '9')
        {
            printf("ERROR: OTP must only contain digits.\n");
            is_valid = 0;
            break;
        }
    }

    return is_valid;
}


void resolve_host(char *hostname)
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
        return;
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

void resolve_ip(char *ip)
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

int is_valid_ip(char *input) {
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

int is_valid_hostname(char *input) {
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


int main()
{
    Account *accounts = accounts_from_file(ACCOUNT_FILE);
    History *histories = histories_from_file(HISTORY_FILE);
    int choice;
    char input[100];

    // current user session information
    Account *current_user = NULL;
    int login_status = 0;
    int just_started = 1;

    while (1)
    {
        if (just_started == 1)
        {
            show_menu();
            just_started = 0;
        }
        printf("Your choice (1-7, other to quit): ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input) - 1] = '\0';
        choice = atoi(input);

        if (choice < 1 || choice > 9)
        {
            printf("INFO: Exiting program.\n");
            break;
        }

        switch (choice)
        {

        /**
         * Register an account
         */
        case 1:
            if (login_status)
            {
                printf("ERROR: You must sign out to register a new account.\n");
                break;
            }
            else
            {
                char username[BUFFER_SIZE], password[BUFFER_SIZE], homepage[BUFFER_SIZE];

                printf("Username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = '\0';
                if (is_valid_username(username) == 0)
                {
                    break;
                }

                if (find_account_by_username(accounts, username) != NULL)
                {
                    printf("ERROR: Username already exists.\n");
                    break;
                }

                printf("Password: ");
                fgets(password, sizeof(password), stdin);
                password[strcspn(password, "\n")] = '\0';
                if (is_valid_password(password) == 0)
                {
                    break;
                }

                printf("Homepage: ");
                fgets(homepage, sizeof(homepage), stdin);
                homepage[strcspn(homepage, "\n")] = '\0';
                if (is_valid_ip(homepage) == 0 && is_valid_hostname(homepage) == 0)
                {
                    break;
                }

                Account *new_account = create_account(username, password, DEFAULT_STATUS, homepage);
                add_account(&accounts, new_account);
                save_accounts(accounts, ACCOUNT_FILE);
                printf("INFO: Register successfully.\n");
            }
            break;

        /**
         * Login
         */
        case 2:
            if (login_status)
            {
                printf("ERROR: You are already signed in.\n");
            }
            else
            {
                char username[BUFFER_SIZE], password[BUFFER_SIZE];

                printf("Username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = '\0';
                if (is_valid_username(username) == 0)
                {
                    break;
                }

                Account *account = find_account_by_username(accounts, username);
                if (account == NULL)
                {
                    printf("ERROR: Account not found.\n");
                    break;
                }

                if (account->status == 0)
                {
                    printf("ERROR: Account is blocked.\n");
                    break;
                }

                int tries = 0;
                while (tries < 3)
                {

                    printf("Password: ");
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = '\0';
                    if (is_valid_password(password) == 0)
                    {
                        tries++;
                        printf("ERROR: Password is invalid.\n");
                        continue;
                    }

                    if (strcmp(account->password, password) == 0)
                    {
                        printf("INFO: Welcome.\n");
                        char cur_date[50], cur_time[50];

                        current_date(cur_date, sizeof(cur_date));
                        current_time(cur_time, sizeof(cur_time));

                        History *history = create_history(username, cur_date, cur_time);
                        add_history(&histories, history);
                        save_histories(histories, HISTORY_FILE);

                        login_status = 1;
                        break;
                    }
                    else
                    {
                        tries++;
                        printf("ERROR: Password is incorrect.\n");
                    }
                }

                if (tries == 3)
                {
                    account->status = 0;
                    save_accounts(accounts, ACCOUNT_FILE);
                    printf("ERROR: Your account is blocked.\n");
                }

                current_user = account;
            }

            break;

        /**
         * Change password
         */
        case 3:
            if (!login_status)
            {
                printf("ERROR: You must sign in to change password.\n");
                break;
            }
            else
            {
                char old_password[BUFFER_SIZE], new_password[BUFFER_SIZE];

                printf("Old password: ");
                fgets(old_password, sizeof(old_password), stdin);
                old_password[strcspn(old_password, "\n")] = '\0';
                if (is_valid_password(old_password) == 0)
                {
                    break;
                }

                printf("New password: ");
                fgets(new_password, sizeof(new_password), stdin);
                new_password[strcspn(new_password, "\n")] = '\0';
                if (is_valid_password(new_password) == 0)
                {
                    break;
                }

                Account *account = find_account_by_username(accounts, current_user->username);
                if (strcmp(account->password, old_password) == 0)
                {
                    strncpy(account->password, new_password, sizeof(account->password) - 1);
                    account->password[sizeof(account->password) - 1] = '\0';
                    save_accounts(accounts, ACCOUNT_FILE);
                    printf("INFO: Password changed.\n");
                }
                else
                {
                    printf("ERROR: Old password is incorrect.\n");
                }
            }
            break;

        /**
         * Update email or phone information
         */
        case 4:
            if (!login_status)
            {
                printf("ERROR: You must sign in to update account info.\n");
                break;
            }
            else
            {
                char homepage[BUFFER_SIZE];

                printf("Username: %s\n", current_user->username);
                printf("Old homepage: %s\n", current_user->homepage);
                printf("New homepage: ");
                fgets(homepage, sizeof(homepage), stdin);
                homepage[strcspn(homepage, "\n")] = '\0';
                if (is_valid_ip(homepage) == 0 && is_valid_hostname(homepage) == 0)
                {
                    printf("ERROR: Homepage is invalid.\n");
                    break;
                }
                strncpy(current_user->homepage, homepage, sizeof(current_user->homepage) - 1);
                current_user->homepage[sizeof(current_user->homepage) - 1] = '\0';
                printf("INFO: Homepage updated.\n");
                save_accounts(accounts, ACCOUNT_FILE);
            }
            break;
        /**
         * Reset password
         */
        case 5:
            if (login_status)
            {
                printf("ERROR: You must sign out to reset password.\n");
            }
            else
            {
                char username[BUFFER_SIZE];

                printf("Enter username to reset password: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = '\0';

                if (is_valid_username(username) == 0)
                {
                    break;
                }

                Account *account = find_account_by_username(accounts, username);
                if (account == NULL)
                {
                    printf("ERROR: Account not found.\n");
                    break;
                }

                if (account->status == 0)
                {
                    printf("ERROR: Account is blocked.\n");
                    break;
                }

                // Generate OTP
                char otp[7];
                const char charset[] = "0123456789";
                for (int i = 0; i < 6; i++)
                {
                    otp[i] = charset[rand() % (sizeof(charset) - 1)];
                }
                otp[6] = '\0';

                // Save OTP to file
                FILE *otp_file = fopen("OTP.txt", "w");
                if (otp_file == NULL)
                {
                    printf("ERROR: Failed to create OTP file.\n");
                    break;
                }
                fprintf(otp_file, "%s\n", otp);
                fclose(otp_file);

                printf("INFO: OTP has been generated and saved to OTP.txt. Please check and enter the OTP.\n");

                // Verify OTP
                char entered_otp[7];
                printf("Enter OTP: ");
                fgets(entered_otp, sizeof(entered_otp), stdin);
                entered_otp[strcspn(entered_otp, "\n")] = '\0';

                if (is_valid_otp(entered_otp) == 0)
                {
                    printf("ERROR: OTP is invalid.\n");
                    break;
                }

                if (strcmp(otp, entered_otp) != 0)
                {
                    printf("ERROR: OTP is incorrect.\n");
                    break;
                }

                char new_password[BUFFER_SIZE];
                while (strlen(new_password) < 6)
                {
                    printf("New password: ");
                    fgets(new_password, sizeof(new_password), stdin);
                    new_password[strcspn(new_password, "\n")] = '\0';
                    if (strlen(new_password) < 6)
                        printf("INFO: Password must be at least 6 characters.\n");
                }

                strncpy(account->password, new_password, sizeof(new_password) - 1);
                account->password[sizeof(account->password) - 1] = '\0';
                save_accounts(accounts, ACCOUNT_FILE);
                printf("INFO: Password reset successfully.\n");
            }
            break;
        /**
         * View Login History
         */
        case 6:
            if (!login_status)
            {
                printf("ERROR: You must sign in to view login history.\n");
                break;
            }
            else
            {
                History *list_history = find_list_history_by_username(histories, current_user->username);
                if (list_history == NULL)
                {
                    printf("ERROR: No login history found.\n");
                    break;
                }

                print_list_history(list_history);
            }
            break;
        case 7:
            if (!login_status)
            {
                printf("ERROR: You must sign in to view homepage.\n");
                break;
            }
            else
            {
                if(is_valid_hostname(current_user->homepage) == 0)
                {
                    printf("ERROR: Homepage is not a host name.\n");
                    break;
                }
                resolve_host(current_user->homepage);
            }
            break;
        case 8:
            if (!login_status)
            {
                printf("ERROR: You must sign in to view homepage.\n");
                break;
            }
            else
            {
                if(is_valid_ip(current_user->homepage) == 0)
                {
                    printf("ERROR: Homepage is not an IP address.\n");
                    break;
                }
                resolve_ip(current_user->homepage);
            }
            break;
        /**
         * Logout
         */
        case 9:
            if (!login_status)
            {
                printf("ERROR: You must sign in to sign out.\n");
                break;
            }

            login_status = 0;
            current_user = NULL;
            printf("INFO: Signed out.\n");
            break;

        default:
            break;
        }
    }

    free_accounts(accounts);
}