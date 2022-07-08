#include <stdlib.h>

typedef unsigned char byte;
typedef struct TemplateData{
    byte* data;
    size_t data_size;
} TemplateData;


void insert_template(byte template[], size_t template_size, char* destination_dir);
TemplateData *get_template(char template_specifier[]);

// Helper functions for get_template.
static char* get_template_file_path(char template_specifier[]);
static size_t get_file_size(FILE *fp);
