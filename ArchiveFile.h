#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

typedef struct ArchiveFile{
    long id;
    char uuid[36];
    char relative_path[200];
} ArchiveFile;

#endif