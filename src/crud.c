#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
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
void query_record(const char *command) {
    bool found = false;
    int id;
    char name[64];
    char programme[64];
    float mark;

    // Helper: trim leading/trailing spaces 
    void trim_spaces(char *str) {
        int start = 0, end = strlen(str) - 1;

        while (isspace((unsigned char)str[start])) start++;
        while (end >= start && isspace((unsigned char)str[end])) end--;

        for (int i = start; i <= end; i++)
            str[i - start] = str[i];

        str[end - start + 1] = '\0';
    }

    // Helper: normalize multiple spaces to single 
    void normalize_spaces(char *str) {
        int i = 0, j = 0;
        while (str[i]) {
            if (isspace((unsigned char)str[i])) {
                if (j > 0 && !isspace((unsigned char)str[j - 1]))
                    str[j++] = ' ';
            } else {
                str[j++] = str[i];
            }
            i++;
        }
        str[j] = '\0';
    }

    // Helper: case-insensitive substring search
    char* strcasestr_local(const char *haystack, const char *needle) {
        char h[128], n[64];
        int i = 0;

        // lowercase haystack
        while (haystack[i] && i < 127) {
            h[i] = tolower(haystack[i]);
            i++;
        }
        h[i] = '\0';

        // lowercase needle
        i = 0;
        while (needle[i] && i < 63) {
            n[i] = tolower(needle[i]);
            i++;
        }
        n[i] = '\0';

        return strstr(h, n);
    }

    //  ID QUERY 
    if (sscanf(command, "query ID=%d", &id) == 1) {

        for (int i = 0; i < record_count; i++) {
            if (records[i].id == id) {
                if (!found) {
                    printf("Record(s) with ID=%d found:\n", id);
                    printf("%-10s %-20s %-25s %-10s\n",
                           "ID", "Name", "Programme", "Mark");
                }
                printf("%-10d %-20s %-25s %-10.1f\n",
                       records[i].id, records[i].name,
                       records[i].programme, records[i].mark);
                found = true;
            }
        }
    }
    //  NAME QUERY 
    else if (sscanf(command, "query Name=%63[^\n]", name) == 1) {
        trim_spaces(name);
        normalize_spaces(name);

        for (int i = 0; i < record_count; i++) {
            char recname[64];
            strcpy(recname, records[i].name);
            trim_spaces(recname);
            normalize_spaces(recname);

            if (strcasestr_local(recname, name)) {   // PARTIAL MATCH
                if (!found) {
                    printf("Record(s) with Name containing \"%s\" found:\n", name);
                    printf("%-10s %-20s %-25s %-10s\n",
                           "ID", "Name", "Programme", "Mark");
                }
                printf("%-10d %-20s %-25s %-10.1f\n",
                       records[i].id, records[i].name,
                       records[i].programme, records[i].mark);
                found = true;
            }
        }
    }
    // PROGRAMME QUERY 
    else if (sscanf(command, "query Programme=%63[^\n]", programme) == 1) {
        trim_spaces(programme);
        normalize_spaces(programme);

        for (int i = 0; i < record_count; i++) {
            char recprog[64];
            strcpy(recprog, records[i].programme);
            trim_spaces(recprog);
            normalize_spaces(recprog);

            if (strcasestr_local(recprog, programme)) { // PARTIAL MATCH
                if (!found) {
                    printf("Record(s) with Programme containing \"%s\" found:\n", programme);
                    printf("%-10s %-20s %-25s %-10s\n",
                           "ID", "Name", "Programme", "Mark");
                }
                printf("%-10d %-20s %-25s %-10.1f\n",
                       records[i].id, records[i].name,
                       records[i].programme, records[i].mark);
                found = true;
            }
        }
    }

    // MARK QUERY 
    else if (strncmp(command, "query Mark=", 11) == 0) {
        char operator[3] = "==";  // Default to exact match
        float query_mark;
        const char *mark_str = command + 11;  // Skip "query Mark="
        
        // Check for comparison operators
        if (mark_str[0] == '>') {
            if (mark_str[1] == '=') {
                strcpy(operator, ">=");
                query_mark = atof(mark_str + 2);
            } else {
                strcpy(operator, ">");
                query_mark = atof(mark_str + 1);
            }
        } else if (mark_str[0] == '<') {
            if (mark_str[1] == '=') {
                strcpy(operator, "<=");
                query_mark = atof(mark_str + 2);
            } else {
                strcpy(operator, "<");
                query_mark = atof(mark_str + 1);
            }
        } else {
            // No operator, exact match
            query_mark = atof(mark_str);
        }

        for (int i = 0; i < record_count; i++) {
            bool match = false;
            
            if (strcmp(operator, "==") == 0) {
                // Use epsilon for floating point comparison
                match = (fabs(records[i].mark - query_mark) < 0.01);
            } else if (strcmp(operator, ">") == 0) {
                match = (records[i].mark > query_mark);
            } else if (strcmp(operator, ">=") == 0) {
                match = (records[i].mark >= query_mark);
            } else if (strcmp(operator, "<") == 0) {
                match = (records[i].mark < query_mark);
            } else if (strcmp(operator, "<=") == 0) {
                match = (records[i].mark <= query_mark);
            }
            
            if (match) {
                if (!found) {
                    if (strcmp(operator, "==") == 0) {
                        printf("Record(s) with Mark=%.1f found:\n", query_mark);
                    } else {
                        printf("Record(s) with Mark%s%.1f found:\n", operator, query_mark);
                    }
                    printf("%-10s %-20s %-25s %-10s\n",
                        "ID", "Name", "Programme", "Mark");
                }
                printf("%-10d %-20s %-25s %-10.1f\n",
                    records[i].id, records[i].name,
                    records[i].programme, records[i].mark);
                found = true;
            }
        }
    }

    if (!found) {
        printf("No matching record found.\n");
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
int insert_record(char* input) {
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
int name_check(char* input) {
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
int prog_check(char* input) {
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
int mark_check(char* input) {
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
