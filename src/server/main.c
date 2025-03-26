#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common/configuration.h"

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

        // TODO: Fork / spawn new thread to use the new socket.
        printf("A new connection was made!\n");

        close(connection_fd);
    }

    close(socket_fd);
    return exit_status;
}