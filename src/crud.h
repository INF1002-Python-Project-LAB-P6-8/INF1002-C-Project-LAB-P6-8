#ifndef CRUD_H
#define CRUD_H

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

int insert_record(const char* input); // Insert function

int id_check(int input);
int name_check(const char* input);
int mark_check(const char* input);
int prog_check(const char* input);

void normalise_spaces(char* input);
void remove_spaces(char* input);

int save_table(const char *filename); // Save function

void get_record_refs(Record **records_out, int *record_count_out);
#endif // CRUD_H
