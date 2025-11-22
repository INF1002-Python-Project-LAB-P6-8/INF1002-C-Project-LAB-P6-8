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
int open_database(const char* filepath);
void show_all_records(void);
void query_record(const char *command);     // Accept full query command as a string
int delete_record_by_id(int id); // Delete by student ID
int count_records_by_id(int id);
void show_summary(void);
void free_records(void);

void show_sorted_records(int (*comparator)(const void *, const void *));
int compare_id_asc(const void *a, const void *b);
int compare_id_desc(const void *a, const void *b);
int compare_mark_asc(const void *a, const void *b);
int compare_mark_desc(const void *a, const void *b);

int insert_record(char* input); // Insert function

int id_check(char* input);
int special_and_number_check(char* input, char* field);
int mark_check(char* input);
int special_check(char* input, char* field);

void normalise_spaces(char* input);
void remove_spaces(char* input);


int create_database(char* input);

int save_table(void); // Save function

void get_record_refs(Record **records_out, int *record_count_out);
#endif // CRUD_H
