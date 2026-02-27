#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

void send_headers() {
    printf("Content-Type: application/json\n");
    printf("Access-Control-Allow-Origin: *\n");
    printf("Access-Control-Allow-Methods: GET, POST, DELETE\n\n");
}

int get_corsi_callback(void *NotUsed, int argc, char **argv, char **azColName) {
    static int count = 0;
    if (count > 0) printf(",");
    printf("{\"id\":%s, \"titolo\":\"%s\", \"aula\":\"%s\", \"data\":\"%s\", \"descrizione\":\"%s\"}",
           argv[0], argv[1], argv[2], argv[3], argv[4]);
    count++;
    return 0;
}

void init_db(sqlite3 *db) {
    char *err_msg = 0;
    const char *sql = "CREATE TABLE IF NOT EXISTS corsi (id INTEGER PRIMARY KEY AUTOINCREMENT, titolo TEXT, aula TEXT, data_corso TEXT, descrizione TEXT);";
    sqlite3_exec(db, sql, 0, 0, &err_msg);
}

int main(void) {
    send_headers();
    sqlite3 *db;
    
    if (sqlite3_open("/var/www/data/spaq.db", &db) != SQLITE_OK) {
        printf("{\"error\": \"Impossibile aprire il DB\"}\n");
        return 1;
    }
    init_db(db);

    char *method = getenv("REQUEST_METHOD");
    
    // GESTIONE POST (Inserimento)
    if (method && strcmp(method, "POST") == 0) {
        char *len_str = getenv("CONTENT_LENGTH");
        if (len_str) {
            int len = atoi(len_str);
            char *buffer = malloc(len + 1);
            fread(buffer, 1, len, stdin);
            buffer[len] = '\0';
            
            char *titolo = strtok(buffer, "|");
            char *aula = strtok(NULL, "|");
            char *data = strtok(NULL, "|");
            char *desc = strtok(NULL, "|");
            
            if(titolo && aula && data && desc) {
                sqlite3_stmt *stmt;
                const char *sql = "INSERT INTO corsi (titolo, aula, data_corso, descrizione) VALUES (?, ?, ?, ?)";
                sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
                sqlite3_bind_text(stmt, 1, titolo, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 2, aula, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 4, desc, -1, SQLITE_STATIC);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
                printf("{\"status\": \"success\"}\n");
            }
            free(buffer);
        }
    } 
    // GESTIONE DELETE (Eliminazione)
    else if (method && strcmp(method, "DELETE") == 0) {
        char *query = getenv("QUERY_STRING"); // Si aspetta qualcosa tipo "id=5"
        if (query && strncmp(query, "id=", 3) == 0) {
            int id = atoi(query + 3);
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "DELETE FROM corsi WHERE id = ?", -1, &stmt, NULL);
            sqlite3_bind_int(stmt, 1, id);
            
            if(sqlite3_step(stmt) == SQLITE_DONE) {
                printf("{\"status\": \"success\"}\n");
            } else {
                printf("{\"error\": \"Errore durante l'eliminazione\"}\n");
            }
            sqlite3_finalize(stmt);
        } else {
            printf("{\"error\": \"ID mancante\"}\n");
        }
    } 
    // GESTIONE GET (Lettura)
    else {
        printf("[");
        char *err_msg = 0;
        sqlite3_exec(db, "SELECT id, titolo, aula, data_corso, descrizione FROM corsi ORDER BY data_corso ASC;", get_corsi_callback, 0, &err_msg);
        printf("]\n");
    }

    sqlite3_close(db);
    return 0;
}