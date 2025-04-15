#include <unistd.h>

#include <sqlite3.h>

typedef struct film_struct {
    char* id;
    char* title;
    char* genres;
    char* director;
    char* year;
} Film;

typedef struct film_catalog_struct {
    sqlite3* db;
} FilmCatalog;


/**
 * Takes a user-provided command, and serialize it to be sent over the network.
 */
void serializeCommandLine(char* input, char* out, void (*token_func)(char**, char*),  int* out_len);

/**
 * Take a serialized command string, and extract from it the available film information.
 */
Film deserializeCommand(char* command_str);


FilmCatalog* createFilmCatalog();
void clearFilmCatalog(FilmCatalog*);

/* Defined operations */

/* (1) Cadastrar um novo filme */
int addFilmToCatalog(FilmCatalog*, char* title, char* genres, char* director, char* year);

/* (4) Listar todos os títulos de filmes com seus identificadores */
char* listFilmIDs(FilmCatalog*);
