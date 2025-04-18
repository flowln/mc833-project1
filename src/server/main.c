#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "common/configuration.h"
#include "common/data.h"


void _addFilm(int connection_fd, FilmCatalog* catalog, char* arguments)
{
    Film film = deserializeCommand(arguments);

    if (film.title == NULL) {
        const char* msg = "Required field 'title' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else if (film.director == NULL) {
        const char* msg = "Required field 'director' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else if (film.year == NULL) {
        const char* msg = "Required field 'year' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else {
        int res = addFilmToCatalog(catalog, film.title, film.genres, film.director, film.year);
        if (res != 0) {
            const char* msg = "Failed to add film to catalog.";
            write(connection_fd, msg, strlen(msg));
        } else {
            const char* msg = "Successfully added film to catalog.";
            write(connection_fd, msg, strlen(msg));
        }
    }
}

void _addGenres(int connection_fd, FilmCatalog* catalog, char* arguments)
{
    Film film = deserializeCommand(arguments);

    if (film.id == NULL) {
        const char* msg = "Required field 'id' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else if (film.genres[0] == '\0') {
        const char* msg = "Required field 'genres' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else {
        int res = addGenresToFilm(catalog, film.id, film.genres);

        if (res == -2) {
            const char* msg = "Failed to add genres to film, as the provided ID is not valid.";
            write(connection_fd, msg, strlen(msg));
        } else if (res != 0) {
            const char* msg = "Failed to add genres to film.";
            write(connection_fd, msg, strlen(msg));
        } else {
            const char* msg = "Successfully added genres to film.";
            write(connection_fd, msg, strlen(msg));
        }
    }
}

void _delFilm(int connection_fd, FilmCatalog* catalog, char* arguments)
{
    Film film = deserializeCommand(arguments);

    if (film.id == NULL) {
        const char* msg = "Required field 'id' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else {
        int res = deleteFilmFromCatalog(catalog, film.id);

        if (res == -2) {
            const char* msg = "Failed to remove film from the catalog, as the provided ID is not valid.";
            write(connection_fd, msg, strlen(msg));
        } else if (res != 0) {
            const char* msg = "Failed to remove film from the catalog.";
            write(connection_fd, msg, strlen(msg));
        } else {
            const char* msg = "Successfully removed film from the catalog.";
            write(connection_fd, msg, strlen(msg));
        }
    }
}

void _showFilm(int connection_fd, FilmCatalog* catalog, char* arguments)
{
    Film film = deserializeCommand(arguments);

    if (film.id == NULL) {
        const char* msg = "Required field 'id' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else {
        char* info = listFilmById(catalog, film.id);

        if (info == NULL) {
            const char* msg = "Failed to list information film about the film.";
            write(connection_fd, msg, strlen(msg));
        } else {
            write(connection_fd, info, strlen(info));
            free(info);
        }
    }
}

void _showFilmByGenre(int connection_fd, FilmCatalog* catalog, char* arguments)
{
    Film film = deserializeCommand(arguments);

    if (film.genres[0] == '\0') {
        const char* msg = "Required field 'genres' is missing.";
        write(connection_fd, msg, strlen(msg));
    } else {
        char* info = listFilmByGenre(catalog, film.genres);

        if (info == NULL) {
            const char* msg = "Failed to list films with the specified genre.";
            write(connection_fd, msg, strlen(msg));
        } else {
            write(connection_fd, info, strlen(info));
            free(info);
        }
    }
}


/**
 * Serve requests from the client application.
 * This function should only be called from a separate process.
 */
void serve_client(int connection_fd)
{
    printf("New process is serving the client!\n");

    int num_read;
    char receive_buffer[1024];

    FilmCatalog* catalog = createFilmCatalog();
    if (catalog == NULL) {
        perror("Failed to open/create the film catalog.");
        close(connection_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        num_read = recv(connection_fd, receive_buffer, 1024, 0);
        if (num_read < 0) {
            perror("Error ocurred while reading from the connection socket.");
            close(connection_fd);
            clearFilmCatalog(catalog);
            exit(EXIT_FAILURE);
        }
        if (num_read == 0) {
            printf("Client has orderly closed the connection. Closing the connection on the server side.");
            close(connection_fd);
            clearFilmCatalog(catalog);
            exit(EXIT_SUCCESS);
        }

        // Format as a string, since we aren't sending binary information.
        receive_buffer[num_read] = '\0';
        printf("Received the following command from the client: %s\n", receive_buffer);

        if (strncmp(receive_buffer, "add_film", 8) == 0) {
            char* arguments = malloc(strlen(&receive_buffer[9]) * sizeof(char));
            strcpy(arguments, &receive_buffer[9]);

            _addFilm(connection_fd, catalog, arguments);

            free(arguments);
        }
        else if (strncmp(receive_buffer, "add_genres", 10) == 0) {
            char* arguments = malloc(strlen(&receive_buffer[11]) * sizeof(char));
            strcpy(arguments, &receive_buffer[11]);

            _addGenres(connection_fd, catalog, arguments);

            free(arguments);
        }
        else if (strncmp(receive_buffer, "del_film", 8) == 0) {
            char* arguments = malloc(strlen(&receive_buffer[9]) * sizeof(char));
            strcpy(arguments, &receive_buffer[9]);

            _delFilm(connection_fd, catalog, arguments);

            free(arguments);
        }
        else if (strncmp(receive_buffer, "list_ids", 8) == 0) {
            char* info = listFilmIDs(catalog);
            if (info == NULL) {
                const char* msg = "Failed to list catalog information.";
                write(connection_fd, msg, strlen(msg));
                continue;
            }

            write(connection_fd, info, strlen(info));

            free(info);
        }
        else if (strncmp(receive_buffer, "list_all", 8) == 0) {
            char* info = listAllFilms(catalog);
            if (info == NULL) {
                const char* msg = "Failed to list catalog information.";
                write(connection_fd, msg, strlen(msg));
                continue;
            }

            write(connection_fd, info, strlen(info));

            free(info);
        }
        else if (strncmp(receive_buffer, "show", 4) == 0) {
            char* arguments = malloc(strlen(&receive_buffer[5]) * sizeof(char));
            strcpy(arguments, &receive_buffer[5]);

            _showFilm(connection_fd, catalog, arguments);

            free(arguments);
        }
        else if (strncmp(receive_buffer, "filter_by_genre", 15) == 0) {
            char* arguments = malloc(strlen(&receive_buffer[16]) * sizeof(char));
            strcpy(arguments, &receive_buffer[16]);

            _showFilmByGenre(connection_fd, catalog, arguments);

            free(arguments);
        }
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
