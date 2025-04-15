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

/* (2) Adicionar um novo gênero a um filme */
int addGenresToFilm(FilmCatalog*, char* film_id, char* new_genres);

/* (3) Remover um filme pelo identificador */
int deleteFilmFromCatalog(FilmCatalog*, char* film_id);

/* (4) Listar todos os títulos de filmes com seus identificadores */
char* listFilmIDs(FilmCatalog*);

/* (5) Listar informações de todos os filmes */
char* listAllFilms(FilmCatalog*);

/* (6) Listar informações de um filme específico */
char* listFilmById(FilmCatalog*, char* film_id);

/* (7) Listar todos os filmes de um determinado gênero */
char* listFilmByGenre(FilmCatalog*, char* genre);
