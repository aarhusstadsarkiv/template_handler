#include <stdio.h>
#include <string.h>
#include "template.h"

/*
 Get the relative path of the template file based on the template specifier.

 Arguments: char template_specifier[]. The template specifier.
 Returns: char* template path.

*/
static char *get_template_file_path(char template_specifier[]){
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

static size_t get_file_size(FILE *fp){
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}


void insert_template(byte template[], size_t template_size, char* destination_dir){
    FILE *target;
    char destination_file_path[300];
    snprintf(destination_file_path, 300, "%s/1.tif", destination_dir);
    target = fopen(destination_file_path, "wb");
    if(target != NULL){
        fwrite(template, 1,  template_size, target);
        fclose(target);
    }

    else{
        perror("Could not write to destination: ");
    }
}

/*
 Get the template data structure based on the template_specifier.
 The template data structure and its `data` field returned by this function
 is dynamically allocated and should be deallocated
 by using free on the TemplateData->data attribute and then
 the TemplateData pointer.
 Arguments: char template_specifier[]. The template specifier.
 Returns: TemplateData *template_data.

*/

TemplateData* get_template(char template_specifier[]){
    char *template_path = get_template_file_path(template_specifier);
    FILE *fp_template = fopen(template_path, "rb");
    
    if(fp_template == NULL){
        perror("Could not open file: ");
        return NULL;
    }
    else{
        // Allocate memory for template buffer:
        size_t template_size = get_file_size(fp_template);
        byte *template_buffer = malloc(sizeof(byte)*template_size);
        
        // Read the template into the buffer:    
        fread(template_buffer, template_size, 1, fp_template);
        fclose(fp_template);
        
        // Construct the TemplateData object and return it.
        TemplateData *template_data = malloc(sizeof(TemplateData));
        template_data->data = template_buffer;
        template_data->data_size = template_size;
        return template_data;
    }
}