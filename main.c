#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "ArchiveFile.h"
#include "lib/paths/paths.h"

typedef unsigned char byte;
typedef struct TemplateData{
    byte* data;
    size_t data_size;
} TemplateData;


typedef struct ArchiveFileData{
    ArchiveFile *files;
    size_t files_length;
} ArchiveFileData;

void print_help();
void insert_template(byte template[], size_t template_size, char* destination_dir);
TemplateData *get_template(char template_specifier[]);
ArchiveFileData* get_not_converted_files(sqlite3 *db, char *checksum, char *puid);

// Helper functions for get_template.
char* get_template_file_path(char template_specifier[]);
size_t get_file_size(FILE *fp);



void correct_path(char *path, size_t path_length){
    for (size_t i = 0; i < path_length; i++)
    {
        if(path[i] == '\\')
            path[i] = '/';
    }
    
}

int main(int argc, char *argv[])
{
    if(strcmp(argv[1], "--help") == 0){
        print_help();
        return 0;
    }

    
    // Open database:
    sqlite3 *db;
    int error_flag = sqlite3_open(argv[1], &db);
    if(error_flag) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
    }


    // Get files from database.
    printf("%s\n", argv[3]);

    ArchiveFileData* filesData;
    /* If argv[3] is less than 10 characters, it is a puid.
        Else, it must be a checksum.
    */
    if(strlen(argv[3]) < 10)
        filesData = get_not_converted_files(db, NULL, argv[3]);
    else
        filesData = get_not_converted_files(db, argv[3], NULL);

    TemplateData *template_data = get_template(argv[4]);
    
    char absolute_path_buffer[300];
    char destination_dir_buffer[300];
    // Iterate over all files
    /*
        For file from database:
        1. Combine destination root with file.
        2. Make Destination folder.
        3. Invoke insert_template.
    */
    for (size_t i = 0; i < filesData->files_length; i++)
    {
            // Clear buffers.
            memset(absolute_path_buffer, 0, 300);
            memset(destination_dir_buffer, 0, 300);
            
            correct_path(filesData->files[i].relative_path, strlen(filesData->files[i].relative_path));
            // Get absolute path of destination file.
            insert_combined_path(absolute_path_buffer, argv[2], filesData->files[i].relative_path);
            // printf("Absolute path: %s\n", absolute_path_buffer);
            // make destination dir.
            get_parent_path(destination_dir_buffer, absolute_path_buffer, strlen(absolute_path_buffer));
            make_output_dir(destination_dir_buffer);
            // printf("\tDestination directory: %s\n", destination_dir_buffer);
            insert_template(template_data->data, template_data->data_size, destination_dir_buffer);
    }

    free(template_data->data);
    free(template_data);
    free(filesData->files);
    free(filesData);
    
}

void print_help(){
    printf("Usage: templateHandler {db_file_path} {destination_root} {puid or checksum} {template_name}\n");
    printf("Valid template names: file_damaged, file_empty, file_not_convertable, file_not_preservable, password_protected\n");
}

void insert_template(byte template[], size_t template_size, char* destination_dir){
    FILE *target;
    char destination_file_path[300];
    snprintf(destination_file_path, 300, "%s/1.tif", destination_dir);
    target = fopen(destination_file_path, "wb");
    fwrite(template, 1,  template_size, target);
    fclose(target);
}

TemplateData* get_template(char template_specifier[]){
    char *template_path = get_template_file_path(template_specifier);
    FILE *fp_template = fopen(template_path, "rb");
    
    if(fp_template == NULL){
        perror("Could not open file: ");
        return NULL;
    }
    else{
        size_t template_size = get_file_size(fp_template);
        byte *template_buffer = malloc(sizeof(byte)*template_size);    
        fread(template_buffer, template_size, 1, fp_template);
        fclose(fp_template);
        TemplateData *template_data = malloc(sizeof(TemplateData));
        template_data->data = template_buffer;
        template_data->data_size = template_size;
        return template_data;
    }
}

char *get_template_file_path(char template_specifier[]){
    if(strcmp(template_specifier, "file_damaged") == 0){
        return "templates/file_damaged.tif";   
    }
    
    else if(strcmp(template_specifier, "file_empty") == 0){
        return "templates/file_empty.tif";
    }
       

    else if(strcmp(template_specifier, "file_not_convertable") == 0){
       return "templates/file_not_convertable.tif";
    }

    else if(strcmp(template_specifier, "file_not_preservable") == 0){
        return "templates/file_not_preservable.tif";
    }

    else if(strcmp(template_specifier, "password_protected") == 0){
        return "templates/password_protected.tif";
    }

    else{
        printf("Not a valid template specifier: %s\n", template_specifier);
        return NULL;
    }
}

size_t get_file_size(FILE *fp){
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}

ArchiveFileData* get_not_converted_files(sqlite3 *db, char *checksum, char *puid){
    ArchiveFile *files = malloc(sizeof(ArchiveFile)*11464); 
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

    while (i < 11464);

    sqlite3_finalize(stmt);
    ArchiveFileData *data = malloc(sizeof(ArchiveFileData));
    data->files = files;
    data->files_length = i-1;
    return data;       
}