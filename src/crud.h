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
void show_summary(void);
void free_records(void);

#endif // CRUD_H
