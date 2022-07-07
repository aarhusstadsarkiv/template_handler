#include <stdlib.h>
#include <sqlite3.h>

#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

typedef struct ArchiveFile{
    long id;
    char uuid[36];
    char relative_path[200];
} ArchiveFile;

typedef struct ArchiveFileData{
    ArchiveFile *files;
    size_t files_length;
} ArchiveFileData;

ArchiveFileData* get_not_converted_files(sqlite3 *db, char *checksum, char *puid, size_t buffer_size);

#endif