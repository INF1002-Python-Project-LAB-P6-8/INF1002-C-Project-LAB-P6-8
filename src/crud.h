#ifndef CRUD_H
#define CRUD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define the Record struct
typedef struct {
    int   id;
    char  name[64];
    char  programme[64];
    float mark;
} Record;

// Function declarations
int open_database(const char *folder, const char *filename);
void show_all_records(void);
void query_record(int id);  // Query by student ID
int delete_record_by_id(int id); // Delete by student ID
int count_records_by_id(int id);
void free_records(void);

int insert_record(const char* input); // Insert function

//validations for insert 
int id_check(int input);
int name_check(const char* input);
int mark_check(const char* input);
int prog_check(const char* input);

//Text filtering
void normalise_spaces(char* input);
void remove_spaces(char* input);

int save_table(const char* filename); //Save function

#endif // CRUD_H
