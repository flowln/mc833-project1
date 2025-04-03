// Needed for getline import
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common/configuration.h"


/**
 * Properly format output of 'add_film' to send to the server.
 */
char* format_add_film(char* input, int* out_len)
{
    char* out = malloc(4096 * sizeof(char));
    strcpy(out, "add_film ");

    char* in_cpy = malloc(strlen(input) * sizeof(char));
    strcpy(in_cpy, input);

    char* token = strtok(in_cpy, "-");
    // Skip first "add_film " token.
    token = strtok(NULL, "-");

    char* template = "";
    char field[256];
    while (token) {
        switch (token[0]) {
            case 't':
                template = "title='%s',";
                break;
            case 'g':
                template = "genres='%s',";
                break;
            case 'd':
                template = "director='%s',";
                break;
            case 'y':
                template = "year='%s',";
                break;
            default:
                printf("Invalid token: %s\n", token);
                break;
        }

        int token_len = strlen(token);
        if (token[token_len - 1] == ' ')
            token[token_len - 1] = '\0';

        sprintf(field, template, &token[2]);
        strcat(out, field);

        token = strtok(NULL, "-");
    }

    // Remove last comma.
    *out_len = strlen(out) - 1;
    out[*out_len] = '\0';

    free(in_cpy);

    return out;
}


/**
 * Run a REPL-like interactive comamnd-line interface.
 * This interactively sends commands to the server.
 */
void run_repl(int socket_fd)
{
    printf("Commands available:\n");
    printf("  exit: Exit this REPL and close the server connection (same as Ctrl+D).\n");
    printf("  raw <string>: Send a raw string directly to the server (for debug purposes).\n");
    printf("  add_film -t <title> -g <genre> [<genre> ...] -d <director> -y <year>: Add a new film to the catalog.\n");
    printf("  list_ids: List film IDs and titles available on the catalog.\n");
    printf("\n");

    int num_read;
    size_t in_buffer_size = 1024;
    char* in_buffer = NULL;

    size_t response_buf_size = 1024;
    char* response_buf = malloc(response_buf_size * sizeof(char));

    while (1) {
        printf("> ");
        num_read = getline(&in_buffer, &in_buffer_size, stdin);
        if (num_read <= 0) {
            free(in_buffer);
            break;
        }

        // Strip newline
        if (in_buffer[num_read - 1] == '\n') {
            in_buffer[num_read - 1] = '\0';
            num_read -= 1;
        }

        if (strncmp(in_buffer, "exit", 4) == 0) {
            free(in_buffer);
            break;
        }
        else if (strncmp(in_buffer, "raw ", 4) == 0) {
            write(socket_fd, &in_buffer[4], num_read - 4);
        }
        else if (strncmp(in_buffer, "add_film", 8) == 0) {
            int len_out_buffer;
            char* out_buffer = format_add_film(in_buffer, &len_out_buffer);
            write(socket_fd, out_buffer, len_out_buffer);
            free(out_buffer);

            int response_num = recv(socket_fd, response_buf, response_buf_size, 0);
            if (response_num > 0) {
                printf("Server response: %s\n", response_buf);
            }
        }
        else if (strncmp(in_buffer, "list_ids", 8) == 0) {
            const char* out_buffer = "list_ids";
            write(socket_fd, out_buffer, strlen(out_buffer));

            int response_num = recv(socket_fd, response_buf, response_buf_size, 0);
            if (response_num > 0) {
                printf("Server response: %s\n", response_buf);
            }
        }

        free(in_buffer);
        in_buffer = NULL;
    }

    free(response_buf);
}


int main(int argc, char* argv)
{
    int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == -1) {
        perror("Cannot create socket.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_LISTEN_PORT);
    int addr_fill_sts = inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

    if (addr_fill_sts == -1) {
        perror("Failed to save IP address into sockaddr buffer.");
        exit(EXIT_FAILURE);
    }

    int connect_sts = connect(socket_fd, (struct sockaddr*) &sa, sizeof(sa));
    if (connect_sts == -1) {
        perror("Failed to connect to the server.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    run_repl(socket_fd);

    close(socket_fd);
    return EXIT_SUCCESS;
}
