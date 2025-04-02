#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common/configuration.h"


/**
 * Run a REPL-like interactive comamnd-line interface.
 * This interactively sends commands to the server.
 */
void run_repl(int socket_fd)
{
    printf("Commands available:\n");
    printf("  exit: Exit this REPL and close the server connection (same as Ctrl+D).\n");
    printf("  raw <string>: Send a raw string directly to the server (for debug purposes).\n");
    printf("\n");

    int num_read;
    size_t in_buffer_size = 1024;
    char* in_buffer = NULL;
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

        free(in_buffer);
        in_buffer = NULL;
    }
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
