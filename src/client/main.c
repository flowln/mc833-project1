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

    // TODO: Send something useful to the server.
    sleep(2.0);

    char* message = "hello there!";
    write(socket_fd, message, strlen(message));

    sleep(2.0);

    close(socket_fd);
    return EXIT_SUCCESS;
}
