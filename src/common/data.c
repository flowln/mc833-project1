// Needed for strtok_r
#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "configuration.h"
#include "data.h"

void serializeCommandLine(char* input, char* out, void (*token_func)(char**, char*),  int* out_len)
{
    char* in_cpy = malloc(strlen(input) * sizeof(char));
    strcpy(in_cpy, input);

    char* token = strtok(in_cpy, "-");
    // Skip first 'function name' token.
    token = strtok(NULL, "-");

    char* template = "";
    char field[256];
    while (token) {
        token_func(&template, token);

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
}

Film deserializeCommand(char* command_str)
{
    Film film;
    film.genres = "";  // Default value.

    char* outer_save_ptr;
    char* inner_save_ptr;

    char* argument = strtok_r(command_str, ",", &outer_save_ptr);
    while (argument) {
        char* arg_name = strtok_r(argument, "=", &inner_save_ptr);

        char* temp_arg_value = strtok_r(NULL, "=", &inner_save_ptr);
        int temp_arg_value_len = strlen(temp_arg_value);

        // Strip ' characters.
        temp_arg_value = &temp_arg_value[1];
        temp_arg_value[temp_arg_value_len - 2] = '\0'; // Consume "'," at the end.

        if (strcmp(arg_name, "id") == 0) {
            film.id = temp_arg_value;
        } else if (strcmp(arg_name, "title") == 0) {
            film.title = temp_arg_value;
        } else if (strcmp(arg_name, "genres") == 0) {
            film.genres = temp_arg_value;
        } else if (strcmp(arg_name, "director") == 0) {
            film.director = temp_arg_value;
        } else if (strcmp(arg_name, "year") == 0) {
            film.year = temp_arg_value;
        }

        argument = strtok_r(NULL, ",", &outer_save_ptr);
    }

    return film;
}

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


/***************************/
/*       SQL helpers       */
/***************************/

/**
 * Retrieve the current genres for a given film ID.
 */
char* _getGenresFromId(FilmCatalog* catalog, char* id)
{
    char* output = calloc(4096, sizeof(unsigned char));

    char sql_command[4096];
    const char* sql_command_template = "SELECT genres FROM film_catalog WHERE id=%s";
    sprintf(sql_command, sql_command_template, id);

    sqlite3_stmt* statement;
    int err_code = sqlite3_prepare_v2(catalog->db, sql_command, -1, &statement, NULL);
    if (err_code != SQLITE_OK) {
        printf("Failed to compile SQL command with exit code %d.\n", err_code);
        free(output);
        return NULL;
    }

    char row_info[4096];

    int step_status = sqlite3_step(statement);
    if (step_status == SQLITE_ROW) {
        const unsigned char* genres = sqlite3_column_text(statement, 0);

        strcpy(output, (const char*) genres);

        step_status = sqlite3_step(statement);
    } else {
        printf("Failed to retrieve current film genres. The specified film probably doesn't exist.\n");
        sqlite3_finalize(statement);
        free(output);
        return NULL;
    }

    if (step_status != SQLITE_DONE) {
        printf("Failed to iterate over rows: %d\n", step_status);
        sqlite3_finalize(statement);
        free(output);
        return NULL;
    }

    sqlite3_finalize(statement);

    return output;
}

/**
 * Check if a film with the given ID exists in the catalog
 */
int _checkForId(FilmCatalog* catalog, char* id)
{
    char sql_command[4096];
    const char* sql_command_template = "SELECT id FROM film_catalog WHERE id=%s";
    sprintf(sql_command, sql_command_template, id);

    sqlite3_stmt* statement;
    int err_code = sqlite3_prepare_v2(catalog->db, sql_command, -1, &statement, NULL);
    if (err_code != SQLITE_OK) {
        printf("Failed to compile SQL command with exit code %d.\n", err_code);
        return -1;
    }

    int step_status = sqlite3_step(statement);
    int id_exists = (step_status == SQLITE_ROW);

    sqlite3_finalize(statement);

    return id_exists;
}

/****************************/
/* Endpoint implementations */
/****************************/

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

int addGenresToFilm(FilmCatalog* catalog, char* film_id, char* new_genres)
{
    if (_checkForId(catalog, film_id) <= 0) {
        return -2;
    }

    char* genres = _getGenresFromId(catalog, film_id);

    strcat(genres, " ");
    strcat(genres, new_genres);

    char sql_command[4096];
    const char* sql_command_template = "UPDATE film_catalog SET genres = '%s' WHERE id = %s";
    sprintf(sql_command, sql_command_template, genres, film_id);

    char* error_msg;
    int error = sqlite3_exec(catalog->db, sql_command, 0, 0, &error_msg);
    if (error != 0) {
        printf("Failed to add genres to film: %s\n", error_msg);
        free(genres);
        return -1;
    }

    free(genres);
    return 0;
}

int deleteFilmFromCatalog(FilmCatalog* catalog, char* film_id)
{
    if (_checkForId(catalog, film_id) <= 0) {
        return -2;
    }

    char sql_command[4096];
    const char* sql_command_template = "DELETE FROM film_catalog WHERE id=%s";
    sprintf(sql_command, sql_command_template, film_id);

    char* error_msg;
    int error = sqlite3_exec(catalog->db, sql_command, 0, 0, &error_msg);
    if (error != 0) {
        printf("Failed to remove film from catalog: %s\n", error_msg);
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
        sqlite3_finalize(statement);
        free(output);
        return NULL;
    }

    sqlite3_finalize(statement);

    return output;
}
