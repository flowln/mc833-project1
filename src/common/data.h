#include <sqlite3.h>

typedef struct film_catalog_struct {
    sqlite3* db;
} FilmCatalog;


FilmCatalog* createFilmCatalog();
void clearFilmCatalog(FilmCatalog*);

/* Defined operations */

/* (1) Cadastrar um novo filme */
int addFilmToCatalog(FilmCatalog*, char* title, char* genres, char* director, char* year);

/* (4) Listar todos os títulos de filmes com seus identificadores */
char* listFilmIDs(FilmCatalog*);
