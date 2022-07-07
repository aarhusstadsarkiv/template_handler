#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "lib/paths/paths.h"
#include "lib/templatelib/template.h"

void print_help();


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
