#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "crud.h"

// Static variables for holding records
static Record *records = NULL;
static int record_count = 0;

// Open the database file and load records
int open_database(const char *folder, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", folder, filename);

    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    record_count = 0;
    int capacity = 10;
    records = (Record *)malloc(capacity * sizeof(Record));
    if (records == NULL) {
        fclose(file);
        perror("Memory allocation failed");
        return -1;
    }

    // Skip metadata lines
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "ID") &&
            strstr(line, "Name") &&
            strstr(line, "Programme") &&
            strstr(line, "Mark")) {
            break;
        }
    }

    // Read records
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (record_count >= capacity) {
            capacity *= 2;
            Record *temp = (Record *)realloc(records, capacity * sizeof(Record));
            if (temp == NULL) {
                free(records);
                fclose(file);
                perror("Memory reallocation failed");
                return -1;
            }
            records = temp;
        }

        Record *rec = &records[record_count];
        if (sscanf(line, "%d\t%63[^\t]\t%63[^\t]\t%f",
                   &rec->id, rec->name, rec->programme, &rec->mark) == 4) {
            record_count++;
        }
    }

    fclose(file);
    return 0;
}

// Show all records in the database
void show_all_records(void) {
    if (records == NULL || record_count == 0) {
        puts("CMS: No records loaded. Please open the database first.");
        return;
    }

    printf("CMS: Here are all the records found in the table \"StudentRecords\".\n");
    printf("%-10s %-20s %-25s %-10s\n", "ID", "Name", "Programme", "Mark"); // Column headers
    for (int i = 0; i < record_count; i++) {
        printf("%-10d %-20s %-25s %-10.1f\n",
               records[i].id, records[i].name, records[i].programme, records[i].mark);
    }
}

// Query a record based on the student ID
void query_record(int id) {
    bool found = false;

    // Print the column headers
    printf("The record with ID=%d ", id);
    
    // Search for the record
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id) {
            // Record found, print the "found" message and the record
            printf("is found in the data table.\n");
            printf("%-10s %-20s %-25s %-10s\n", "ID", "Name", "Programme", "Mark"); // Column headers
            printf("%-10d %-20s %-25s %-10.1f\n", records[i].id, records[i].name, records[i].programme, records[i].mark);
            found = true;
            break;
        }
    }

    // If no record was found
    if (!found) {
        printf("does not exist.\n");
    }
}

//delete record by student ID
int delete_record_by_id(int id) {
    if (records == NULL || record_count == 0) {
        return -1;
    }
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id) {
            for (int j = i + 1; j < record_count; j++) { 
                records[j - 1] = records[j];
            }
            record_count--;
            memset(&records[record_count], 0, sizeof(Record));
                return 1;
        }
    }
    return 0;   
}

int count_records_by_id(int id) {
    if (records == NULL || record_count == 0) {
        return -1;
    }
    int count = 0;
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == id) {
            count++;
        }
    }
    return count;
}

// Free the memory used for records
void free_records(void) {
    if (records != NULL) {
        free(records);
        records = NULL;
        record_count = 0;
    }
}

//Insert function
int insert_record(const char* input) {
    // Check if it starts with INSERT
    if (strncmp(input, "INSERT ", 7) != 0) {
        printf("\nError: Command must start with INSERT\n");
        return 1;
    }

    Record new_record;
    new_record.id = 0;
    new_record.mark = -1.0f;
    strcpy(new_record.name, "");
    strcpy(new_record.programme, "");

    // Find ID=
    char* id_pos = strstr(input, "ID=");
    if (id_pos) {
        new_record.id = atoi(id_pos + 3);

    }

    for (int i = 0; i < record_count; i++) {
        if (records[i].id == new_record.id) {
            printf("\nDuplicate entry found, try again\n");
            return 1;
        }
    }

    // Find Name=
    char* name_pos = strstr(input, "Name=");
    char* prog_pos = strstr(input, "Programme=");

    if (name_pos && prog_pos) {
        name_pos += 5;
        int length = prog_pos - name_pos;
        strncpy(new_record.name, name_pos, length);
        new_record.name[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && new_record.name[i] == ' '; i--) {
            new_record.name[i] = '\0';
        }
    }

    // Extract values between Programme= and Mark=
    char* mark_pos = strstr(input, "Mark=");
    if (prog_pos && mark_pos) {
        prog_pos += 10;
        int length = mark_pos - prog_pos;
        strncpy(new_record.programme, prog_pos, length);
        new_record.programme[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && new_record.programme[i] == ' '; i--) {
            new_record.programme[i] = '\0';
        }
    }

    int mark_found = 0;
    // Find Mark=
    if (mark_pos) {
        char* mark_value = mark_pos + 5; 

        if (mark_value[0] != '\0' && mark_value[0] != '\n') {
            if (mark_check(mark_value) == 1) { //mark input validation
                return 1;
            }

            new_record.mark = atof(mark_value);
            mark_found = 1;
        }
    }


    //Validate for any empty fields
    if (new_record.id == 0 || strlen(new_record.name) == 0 ||
        strlen(new_record.programme) == 0 || mark_found == 0) {
        printf("\nError: Missing required fields\n");
        return 1;
    }

    //id input validation
    if (id_check(new_record.id) == 1) {
        return 1;
    }
    //name input validation
    if (name_check(new_record.name) == 1) {
        return 1;
    }
    //programme input validation
    if (prog_check(new_record.programme) == 1) {
        return 1;
    }

    records[record_count] = new_record;
    (record_count)++;

    printf("Successfully inserted: %s (ID: %d)\n", new_record.name, new_record.id);
    return 0;

}



// ID field input validation
int id_check(int input) {
    if (input < 1000000 || input > 9999999) {
        printf("\nError: ID must contains 7 digits and no special characters\n");
        return 1;
    }
    return 0;
}



// Name field input validation
int name_check(const char* input) {
    normalise_spaces(input);

    size_t len = strlen(input);
    if (len == 0 || len >= 64) {
        printf("\nError: Exceeded 64 character limit");
        return 1;
    }
    while (*input) {
        if (!isalpha((unsigned char)*input) && *input != ' ') {
            printf("\nError: Name should not contain special characters and numbers ");
            return 1;
        }
        input++;
    }
    return 0;
}


// Programme field input validation
int prog_check(const char* input) {
    normalise_spaces(input);
    size_t len = strlen(input);
    if (len == 0 || len >= 64) {
        printf("\nError: Exceeded 64 character limit");
        return 1;
    }
    while (*input) {
        if (!isalnum((unsigned char)*input) && *input != ' ') {
            printf("\nError: Programme should not contain special characters");
            return 1;
        }
        input++;
    }

    return 0;
}


// Mark field input validation
int mark_check(const char* input) {
    remove_spaces(input);
    int dot_count = 0;
    int digits_after_dot = 0;
    int len = strlen(input);

    if (!isdigit((unsigned char)input[0])) {
        printf("\nError: Mark should only contain 0.0 to 100.0\n");
        return 1;
    }

    for (int i = 0; i < len; i++) {
        char c = input[i];

        if (c == '.') {
            dot_count++;
            if (dot_count > 1) {
                printf("\nError: Mark should only 1 decimal place\n");
                return 1; 
            }

            if (i == 0 || i == len - 1) {
                printf("\nError: Mark should not start or end with a decimal point\n");
                return 1;
            }
        }
        else if (!isdigit((unsigned char)c)) {
            printf("\nError: Mark should not have special characters\n");
            return 1;
        }
        else if (dot_count == 1) {
            digits_after_dot++;
            if (digits_after_dot > 1) {
                printf("\nError: Mark should not start or end with a decimal point\n");
                return 1;
            }
        }
    }
    double value = atof(input);
    if (value < 0.0 || value > 100.0) {
        printf("\nError: Mark should only contain 0.0 to 100.0\n");
        return 1;
    }
    return 0;
}



// TEXT FILTERING 
void normalise_spaces(char* input) {
    char* src = input;
    char* dst = input;

    while (isspace((unsigned char)*src)) {
        src++;
    }

    int space_seen = 0;

    while (*src) {
        if (isspace((unsigned char)*src)) {
            space_seen = 1;
        }
        else {
            if (space_seen && dst != input) {
                *dst++ = ' ';
            }
            *dst++ = *src;
            space_seen = 0;
        }
        src++;
    }
    if (dst > input && isspace((unsigned char)*(dst - 1))) {
        dst--;
    }
    *dst = '\0';
}


void remove_spaces(char* input) {
    char* src = input;
    char* dst = input;

    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;  // copy non-space character
        }
        src++;
    }

    *dst = '\0';  // null-terminate the result
}


// save table to database function
int save_table(const char* filename) {

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    char header_str[1024] = "";
    int header_done = 0;

    //Read file and extract header lines
    while (fgets(line, sizeof(line), file)) {
        strcat(header_str, line);

        // Check if this line contains column headers
        if (strstr(line, "ID") && strstr(line, "Name") &&
            strstr(line, "Programme") && strstr(line, "Mark")) {
            header_done = 1;
            break;
        }
    }

    fclose(file);

    if (!header_done) {
        printf("Column headers not found in file\n");
        return 1;
    }

    // Reopen file in write mode to overwrite
    file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return 1;
    }

    //Write header back
    fputs(header_str, file);

    //Append array
    for (int i = 0; i < record_count; i++) {
        fprintf(file, "%d\t%s\t%s\t%.1f\n",
            records[i].id,
            records[i].name,
            records[i].programme,
            records[i].mark);
    }

    fclose(file);
    return 0;
}

void get_record_refs(Record **records_out, int *record_count_out) {
  *records_out = records;
  *record_count_out = record_count;
}

void show_summary(void) {
    if (records == NULL || record_count == 0) {
        puts("CMS: No records loaded. Please open the database first.");
        return;
    }

    float total_marks = 0;
    float highest_mark = records[0].mark;
    float lowest_mark = records[0].mark;
	char highest_mark_student[64];
	char lowest_mark_student[64];
	strcpy(highest_mark_student, records[0].name);
	strcpy(lowest_mark_student, records[0].name);

    for (int i = 0; i < record_count; i++) {
        total_marks += records[i].mark;
        if (records[i].mark > highest_mark) {
            highest_mark = records[i].mark;
			strcpy(highest_mark_student, records[i].name);
        }
        if (records[i].mark < lowest_mark) {
            lowest_mark = records[i].mark;
			strcpy(lowest_mark_student, records[i].name);
        }
    }

    float average_mark = total_marks / record_count;

    printf("Summary of Student Records\n");
    printf("Total Records: %d\n", record_count);
    printf("Average Mark: %.1f\n", average_mark);
    printf("Highest Mark: %.1f by %s\n", highest_mark, highest_mark_student);
    printf("Lowest Mark: %.1f by %s\n", lowest_mark, lowest_mark_student);
}