#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "lib/paths/paths.h"
#include "lib/templatelib/template.h"
#include "lib/ArchiveFile/ArchiveFile.h"

#define PUID_LENGTH 20
#define PUID_LIST_SIZE 20

size_t get_puids(char fileBuffer[][PUID_LENGTH]){
  FILE *fp;
  fp = fopen("puid_ignore.txt", "r");
  size_t i = 0;
  while(fgets(fileBuffer[i], PUID_LENGTH, fp)){
    size_t line_length = strlen(fileBuffer[i]);
    if(fileBuffer[i][line_length - 1] == '\n'){
      fileBuffer[i][line_length - 1] = '\0';
    }
    i++;
  }
  return i;
}

int main(int argc, char *argv[])
{
    // Open database:
    sqlite3 *db;
    int error_flag = sqlite3_open(argv[1], &db);
    if(error_flag) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
    }

    char puids[PUID_LIST_SIZE][PUID_LENGTH];
    size_t puid_count = get_puids(puids);
    ArchiveFileData* filesDataList[puid_count];

    for (size_t i = 0; i < puid_count; i++)
    {
      printf("PUID: %s\n", puids[i]);
      filesDataList[i] = get_not_converted_files(db, NULL, puids[i], 1000);
    }
    
    // filesData = get_not_converted_files(db, NULL, argv[3]);
    TemplateData *template_data = get_template(argv[3]);


    char absolute_path_buffer[300];
    char destination_dir_buffer[300];
    // Iterate over all files
    /*
        For file from database:
        1. Combine destination root with file.
        2. Make Destination folder.
        3. Invoke insert_template.
    */
   
    printf("PUIDS count: %ld\n", puid_count);
    for (size_t i = 0; i < puid_count; i++)
    {
      //printf("Replacing files with puid: %s\n", puids[i]);
      //printf("Value of i: %ld\n", i);
      for (size_t j = 0; j < filesDataList[i]->files_length; j++)
      {
            //printf("Number of files: %ld\n", filesDataList[i]->files_length);
              // Clear buffers.
              memset(absolute_path_buffer, 0, 300);
              memset(destination_dir_buffer, 0, 300);
              correct_path(filesDataList[i]->files[j].relative_path, strlen(filesDataList[i]->files[j].relative_path));
              // Get absolute path of destination file.
              insert_combined_path(absolute_path_buffer, argv[2], filesDataList[i]->files[j].relative_path);

              // make destination dir.
              get_parent_path(destination_dir_buffer, absolute_path_buffer, strlen(absolute_path_buffer));
              make_output_dir(destination_dir_buffer);
              
              insert_template(template_data->data, template_data->data_size, destination_dir_buffer);
      }
    }
    free(template_data->data);
    free(template_data);
    
    for (size_t i = 0; i < puid_count; i++)
    {
        free(filesDataList[i]->files);
        free(filesDataList[i]);
    }

    printf("Finished placing templates for the puids from the puid_ignore.txt file.\n");
    return 0;
}
