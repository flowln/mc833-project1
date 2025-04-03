#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configuration.h"
#include "data.h"

FilmCatalog* createFilmCatalog()
{
    int error;

    FilmCatalog* catalog = malloc(sizeof(FilmCatalog));
    error = sqlite3_open(SERVER_DB_LOCATION, &catalog->db);
    if (error != 0) {
        printf("Failed to open database file at %s\n", SERVER_DB_LOCATION);
        clearFilmCatalog(catalog);
        return NULL;
    }

    char* error_msg;
    const char* sql_command = "CREATE TABLE IF NOT EXISTS film_catalog (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, title TEXT, genres TEXT, director TEXT, year TEXT)";
    error = sqlite3_exec(catalog->db, sql_command, 0, 0, &error_msg);
    if (error != 0) {
        printf("Error creating the database table: %s\n", error_msg);
        clearFilmCatalog(catalog);
        return NULL;
    }

    return catalog;
}

void clearFilmCatalog(FilmCatalog* catalog)
{
    sqlite3_close(catalog->db);
    free(catalog);
}

int addFilmToCatalog(FilmCatalog* catalog, char* title, char* genres, char* director, char* year)
{
    char sql_command[4096];
    const char* sql_command_template = "INSERT INTO film_catalog (title,genres,director,year) VALUES ('%s','%s','%s','%s')";
    sprintf(sql_command, sql_command_template, title, genres, director, year);

    char* error_msg;
    int error = sqlite3_exec(catalog->db, sql_command, 0, 0, &error_msg);
    if (error != 0) {
        printf("Failed to add film to catalog: %s\n", error_msg);
        return -1;
    }

    return 0;
}

char* listFilmIDs(FilmCatalog* catalog)
{
    char* output = calloc(4096, sizeof(char));

    const char* sql_command = "SELECT id,title FROM film_catalog";

    sqlite3_stmt* statement;
    int err_code = sqlite3_prepare_v2(catalog->db, sql_command, -1, &statement, NULL);
    if (err_code != SQLITE_OK) {
        printf("Failed to compile SQL command with exit code %d.\n", err_code);
        free(output);
        return NULL;
    }

    char row_info[4096];

    int step_status = sqlite3_step(statement);
    while (step_status == SQLITE_ROW) {
        int id = sqlite3_column_int(statement, 0);
        const unsigned char* title = sqlite3_column_text(statement, 1);

        sprintf(row_info, "%d,%s;", id, title);
        strcat(output, row_info);

        step_status = sqlite3_step(statement);
    }

    if (step_status != SQLITE_DONE) {
        printf("Failed to iterate over rows: %d\n", step_status);
        free(output);
        return NULL;
    }

    sqlite3_finalize(statement);

    return output;
}
