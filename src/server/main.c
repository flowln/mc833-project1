#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common/configuration.h"


/**
 * Serve requests from the client application.
 * This function should only be called from a separate process.
 */
void serve_client(int connection_fd)
{
    printf("New process is serving the client!\n");

    int num_read;
    char receive_buffer[1024];

    while (1) {
        num_read = recv(connection_fd, receive_buffer, 1024, 0);
        if (num_read < 0) {
            perror("Error '%d' ocurred while reading from the connection socket.");
            close(connection_fd);
            exit(EXIT_FAILURE);
        }
        if (num_read == 0) {
            printf("Client has orderly closed the connection. Closing the connection on the server side.");
            close(connection_fd);
            exit(EXIT_SUCCESS);
        }

        printf("Received the following command from the client: %s\n", receive_buffer);
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
    memset(&sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(SERVER_LISTEN_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    int bind_sts = bind(socket_fd, (struct sockaddr*) &sa, sizeof(sa));
    if (bind_sts == -1) {
        perror("Failed to bind socket to the defined address.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    int listen_sts = listen(socket_fd, 32);
    if (listen_sts == -1) {
        perror("Failed to start listening to the socket for new connections.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    int exit_status = EXIT_SUCCESS;
    while(1) {
        // TODO: Use the two remaining arguments for some logging.
        int connection_fd = accept(socket_fd, NULL, NULL);
        if (connection_fd == -1) {
            perror("Failed to accept new connection.");
            exit_status = EXIT_FAILURE;
        }

        int pid = fork();
        if (pid == 0) {
            // Child process
            close(socket_fd);

            serve_client(connection_fd);
        }

        close(connection_fd);
    }

    close(socket_fd);
    return exit_status;
}
