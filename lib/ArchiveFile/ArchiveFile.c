#include <stdio.h>
#include <string.h>
#include "ArchiveFile.h"

/*
 This function gets the files that are not converted with the specified puid or checksum.
 The allocated ArchiveFileData object and its `ArchiveFile *files` field are dynamically allocated
 and must be deallocated with the free function.
 Arguments:
    *sqlite3 *db. The database object.
    *char *checksum.
    *char *puid
    *size_t buffer_size
 
 Returns:
    ArchiveFileData *data.

*/
ArchiveFileData* get_not_converted_files(sqlite3 *db, char *checksum, char *puid, size_t buffer_size){
    ArchiveFile *files = malloc(sizeof(ArchiveFile)*buffer_size); 
    sqlite3_stmt *stmt = NULL;
    int rc = 0;
    size_t i = 0;

    char sql_query[200];
    if(checksum == NULL)
        snprintf(sql_query, 200, "SELECT id, uuid, relative_path FROM _NotConverted WHERE puid = \'%s\';", puid);
    else
        snprintf(sql_query, 200, "SELECT id, uuid, relative_path FROM Files WHERE checksum = \'%s\';", checksum);

    rc = sqlite3_prepare_v2(
        db, sql_query,
       -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare SQL: %s\n", sqlite3_errmsg(db));
        return NULL; 
    }

    do {
        ArchiveFile *file = &files[i++];
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            printf("No more rows ...\n");
            break;
        } else if (rc != SQLITE_ROW) {
            fprintf(stderr, "Problem: %s\n", sqlite3_errmsg(db)); 
            sqlite3_finalize(stmt);
            return NULL;
        }

        
        file->id = strtol(sqlite3_column_text(stmt, 0), NULL, 10);
        strcpy(file->uuid, sqlite3_column_text(stmt, 1));
        strcpy(file->relative_path, sqlite3_column_text(stmt, 2));
    }

    while (i < buffer_size);

    sqlite3_finalize(stmt);
    ArchiveFileData *data = malloc(sizeof(ArchiveFileData));
    data->files = files;
    data->files_length = i-1;
    return data;       
}