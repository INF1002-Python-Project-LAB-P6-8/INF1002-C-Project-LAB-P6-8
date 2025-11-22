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

char global_db_name[64];
char global_table_name[64];
char global_filepath[512];


// Open the database file and load records
int open_database(const char* filepath) {

    strncpy(global_filepath, filepath, sizeof(global_filepath) - 1);
    global_filepath[sizeof(global_filepath) - 1] = '\0';

    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    char line[256];
    record_count = 0;
    int capacity = 10;
    records = (Record*)malloc(capacity * sizeof(Record));
    if (records == NULL) {
        fclose(file);
        perror("Memory allocation failed");
        return -1;
    }

    while (fgets(global_db_name, sizeof(global_db_name), file)) {
        if (strstr(global_db_name, "Database Name:")) {
            global_db_name[strcspn(global_db_name, "\n")] = '\0';
            break;
        }
    }

    while (fgets(global_table_name, sizeof(global_table_name), file)) {
        if (strstr(global_table_name, "Table Name:")) {
            global_table_name[strcspn(global_table_name, "\n")] = '\0';
            break;
        }
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
            Record* temp = (Record*)realloc(records, capacity * sizeof(Record));
            if (temp == NULL) {
                free(records);
                fclose(file);
                perror("Memory reallocation failed");
                return -1;
            }
            records = temp;
        }

        Record* rec = &records[record_count];
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

// Helper: trim leading/trailing spaces 
static void trim_spaces(char *str) {
    int start = 0, end = strlen(str) - 1;

    while (isspace((unsigned char)str[start])) start++;
    while (end >= start && isspace((unsigned char)str[end])) end--;

    for (int i = start; i <= end; i++)
        str[i - start] = str[i];

    str[end - start + 1] = '\0';
}

// Helper: normalize multiple spaces to single 
static void normalize_spaces_query(char *str) {
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
static int contains_case_insensitive(const char *haystack, const char *needle) {
    char h[128], n[64];
    int i = 0;

    // lowercase haystack
    while (haystack[i] && i < 127) {
        h[i] = tolower((unsigned char)haystack[i]);
        i++;
    }
    h[i] = '\0';

    // lowercase needle
    i = 0;
    while (needle[i] && i < 63) {
        n[i] = tolower((unsigned char)needle[i]);
        i++;
    }
    n[i] = '\0';

    return strstr(h, n) != NULL;
}

// Query a record based on the db fields
void query_record(const char *command) {
    bool found = false;

    // Create lowercase copy of command for parsing field names
    char command_lower[256];
    strncpy(command_lower, command, sizeof(command_lower) - 1);
    command_lower[sizeof(command_lower) - 1] = '\0';
    
    for (int i = 0; command_lower[i]; i++) {
        command_lower[i] = tolower((unsigned char)command_lower[i]);
    }

    //  ID QUERY 
    if (strncmp(command_lower, "query id=", 9) == 0) {
        int id = atoi(command + 9);  // Extract from original command
        
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
    else if (strncmp(command_lower, "query name=", 11) == 0) {
        char name[64];
        const char* name_value = command + 11;  // Extract from original command
        
        strncpy(name, name_value, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
        
        trim_spaces(name);
        normalize_spaces_query(name);
        
        // Check if name is empty after trimming
        if (strlen(name) == 0) {
            printf("Error: Name value cannot be empty\n");
            return;
        }

        for (int i = 0; i < record_count; i++) {
            char recname[64];
            strcpy(recname, records[i].name);
            trim_spaces(recname);
            normalize_spaces_query(recname);

            if (contains_case_insensitive(recname, name)) {
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
    else if (strncmp(command_lower, "query programme=", 16) == 0) {
        char programme[64];
        const char* prog_value = command + 16;  // Extract from original command
        
        strncpy(programme, prog_value, sizeof(programme) - 1);
        programme[sizeof(programme) - 1] = '\0';
        
        trim_spaces(programme);
        normalize_spaces_query(programme);
        
        // Check if programme is empty after trimming
        if (strlen(programme) == 0) {
            printf("Error: Programme value cannot be empty\n");
            return;
        }

        for (int i = 0; i < record_count; i++) {
            char recprog[64];
            strcpy(recprog, records[i].programme);
            trim_spaces(recprog);
            normalize_spaces_query(recprog);

            if (contains_case_insensitive(recprog, programme)) {
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
    else if (strncmp(command_lower, "query mark=", 11) == 0) {
        char operator[3] = "==";
        float mark;
        const char *mark_str = command + 11;  // Extract from original command
        
        // Parse operator
        if (mark_str[0] == '>') {
            if (mark_str[1] == '=') {
                strcpy(operator, ">=");
                mark = atof(mark_str + 2);
            } else {
                strcpy(operator, ">");
                mark = atof(mark_str + 1);
            }
        } else if (mark_str[0] == '<') {
            if (mark_str[1] == '=') {
                strcpy(operator, "<=");
                mark = atof(mark_str + 2);
            } else {
                strcpy(operator, "<");
                mark = atof(mark_str + 1);
            }
        } else {
            mark = atof(mark_str);
        }

        // Search records
        for (int i = 0; i < record_count; i++) {
            bool match = false;
            
            if (strcmp(operator, "==") == 0) {
                match = (fabs(records[i].mark - mark) < 0.01);
            } else if (strcmp(operator, ">") == 0) {
                match = (records[i].mark > mark);
            } else if (strcmp(operator, ">=") == 0) {
                match = (records[i].mark >= mark);
            } else if (strcmp(operator, "<") == 0) {
                match = (records[i].mark < mark);
            } else if (strcmp(operator, "<=") == 0) {
                match = (records[i].mark <= mark);
            }
            
            if (match) {
                if (!found) {
                    if (strcmp(operator, "==") == 0) {
                        printf("Record(s) with Mark=%.1f found:\n", mark);
                    } else {
                        printf("Record(s) with Mark%s%.1f found:\n", operator, mark);
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
    else {
        printf("Error: Invalid query format. Use 'query <field>=<value>'\n");
        return;
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
    /*
    The main function for insert major operations
    - checks if the input command starts with INSERT
    - creates a new typedef struct array to store the variables with placeholder values
    - identify headers: ID=, Name=, Programme=, Mark= and extracts out the data between them and store them
    in the new typedef struct array
    - checks if all variables are not holding placeholder values else return error messaage
    - normalise text and remove unncessary white spaces in the variables
    - calls validation functions:
        - ID: only 7 digits, no letters and special char
        - Name: only letters are allowed, no special char or numbers, char limit of 64
        - Programme: Only letters and numbers, no special char, char limit of 64
        - Mark: only numbers ranging from 0.0 to 100.0, no special char or letters
    - if all validation passes, append variables to typedef struct array else reject input
     
    Parameters
    ----------
    input: string
        the input insert command typed in by the user

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful
    */
	
    // Check if it starts with INSERT
    if (strncmp(input, "INSERT ", 7) != 0) {
        printf("\nError: Command must start with INSERT\n");
        return 1;
    }

	// Creates new typedef struct variable with placeholder values
    Record new_record;
    new_record.id = 0;
    new_record.mark = -1.0f;
    strcpy(new_record.name, "");
    strcpy(new_record.programme, "");

    // Find ID=
    char* id_pos = strstr(input, "ID=");
    // Find Name=
    char* name_pos = strstr(input, "Name=");
    // Find Programme=
    char* prog_pos = strstr(input, "Programme=");
    // Find Mark=
    char* mark_pos = strstr(input, "Mark=");

    int id_found = 0;

    // Extract input ID from between ID= and Name=
    if (id_pos && name_pos) {
        id_pos += 3;
        int length = name_pos - id_pos;

        if (length > 8) {
            printf("\nError: Field Name Exceeded 7 character limit");
            return 1;
        }

     
        char id_value[64];
        strncpy(id_value, id_pos, length);
        id_value[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && id_value[i] == ' '; i--) {
            id_value[i] = '\0';
        }

        if (id_value[0] != '\0') {
            if (id_check(id_value) == 1) {// ID validation
                return 1;
            }
            new_record.id = atoi(id_value);
            id_found = 1;
        }
    }

    // Check for duplicate ID
    for (int i = 0; i < record_count; i++) {
        if (records[i].id == new_record.id) {
            printf("\nDuplicate entry found, try again\n");
            return 1;
        }
    }

    // Extract string between Name= and Programme=
    if (name_pos && prog_pos) {
        name_pos += 5;
        int length = prog_pos - name_pos;

        if (length >= 64) {
            printf("\nError: Field Name Exceeded 64 character limit");
            return 1;
        }

        strncpy(new_record.name, name_pos, length);
        new_record.name[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && new_record.name[i] == ' '; i--) {
            new_record.name[i] = '\0';
        }
    }

    // Extract values between Programme= and Mark=
    if (prog_pos && mark_pos) {
        prog_pos += 10;
        int length = mark_pos - prog_pos;

        if (length >= 64) {
            printf("\nError: Programme Name Exceeded 64 character limit");
            return 1;
        }

        strncpy(new_record.programme, prog_pos, length);
        new_record.programme[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && new_record.programme[i] == ' '; i--) {
            new_record.programme[i] = '\0';
        }
    }

    // Extract input Mark
    int mark_found = 0;
    if (mark_pos) {
        char* mark_value = mark_pos + 5;

        int mark_length = 0;
        while (mark_value[mark_length] != '\0' &&
            mark_value[mark_length] != '\n' &&
            mark_value[mark_length] != '\r') {
            mark_length++;
        }

        if (mark_length > 4) {
            printf("\nError: Mark Name Exceeded 4 character limit");
            return 1;
        }

        if (mark_value[0] != '\0' && mark_value[0] != '\n') {
            if (mark_check(mark_value) == 1) { //mark validation
                return 1;
            }
            new_record.mark = atof(mark_value);
            mark_found = 1;
        }
    }

    // Validate for any empty fields against placeholder values
    if (id_found == 0 || strlen(new_record.name) == 0 ||
        strlen(new_record.programme) == 0 || mark_found == 0) {
        printf("\nError: Missing required fields\n");
        return 1;
    }

    // Name input validation
    if (special_and_number_check(new_record.name, "Name") == 1) {
        return 1;
    }

    // Programme input validation
    if (special_check(new_record.programme, "Programme") == 1) {
        return 1;
    }

	//appends records to typedef struct array 
    records[record_count] = new_record;
    (record_count)++;

    printf("Successfully inserted: %s (ID: %d)\n", new_record.name, new_record.id);
    return 0;
}


// special char and number input validation
int special_and_number_check(char* input, char* field) {
    /*
    function to check for special char and numbers
    - normalises text by removing leading and trailing spaces
        - convert all types of white spaces into a singel space
        - keeps words separated by only 1 space
    - checks for special char and numbers
    - prints out error message if validation fails

    Parameters
    ----------
    input: string
        the variable for validation

    field: string
        the name of the field being passed in for validation

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful

    */

	// normalises input
    normalise_spaces(input);
	
	// loops through each character detect any non letters char
    while (*input) {
        if (!isalpha((unsigned char)*input) && *input != ' ') {
            printf("\nError: %s should not contain special characters and numbers", field);
            return 1;
        }
        input++;
    }
    return 0;
}

// special char validation
int special_check(char* input, char* field) {
    /*
    function to check for special char
    - normalises text by removing leading and trailing spaces
        - convert all types of white spaces into a singel space
        - keeps words separated by only 1 space
    - checks for special char
    - prints out error message if validation fails

    Parameters
    ----------
    input: string
        the variable for validation

    field: string
        the name of the field being passed in for validation

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful

    */

    // normalises input
    normalise_spaces(input);
	
    // loops through each character detect any special char
    while (*input) {
        if (!isalnum((unsigned char)*input) && *input != ' ') {
            printf("\nError: %s should not contain special characters", field);
            return 1;
        }
        input++;
    }
    return 0;
}

// ID field input validation
int id_check(char* input) {
    /*
    function to validate ID
    - removes all forms of whitespace char
    - checks if input is only 7 digits

    Parameters
    ----------
    input: string
        the variable for validation

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful

    */

    // removes whitespaces	
    remove_spaces(input);  

	// Checks if length is not 7
    size_t len = strlen(input);
    if (len != 7) {
        printf("\nError: ID must contain exactly 7 digits\n");
        return 1;
    }

	// Checks for only digits 
    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        if (!isdigit((unsigned char)c)) {
            printf("\nError: ID must contain only digits\n");
            return 1;
        }
    }
    return 0;
}

// Mark field input validation
int mark_check(char* input) {
    /*
    function to validate Mark
    - removes all forms of whitespace char
    - checks if input is only 0.0 to 100.0
    - no letter chars
    - makes sure mark only has 1 deciaml point

    Parameters
    ----------
    input: string
        the variable for validation

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful

    */

    // removes whitespaces
    remove_spaces(input);
    size_t len = strlen(input);
    int dot_pos = -1;

    // Find decimal point position and validate characters
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '.') {
            if (dot_pos != -1) { 
                printf("\nError: Mark must have exactly 1 decimal place\n");
                return 1;
            }
            dot_pos = i;
        }
        else if (!isdigit((unsigned char)input[i])) {
            printf("\nError: Mark should only contain digits and 1 decimal place\n");
            return 1;
        }
    }

    // Check if have exactly 1 decimal with 1 digit after it
    if (dot_pos == -1 || dot_pos == 0 || dot_pos != len - 2) {
        printf("\nError: Mark must be in format X.Y (i.e 73.2)\n");
        return 1;
    }

    // Check range 0.0 to 100.0
    double value = atof(input);
    if (value < 0.0 || value > 100.0) {
        printf("\nError: Mark must be between 0.0 and 100.0\n");
        return 1;
    }
    return 0;
}

// function for normalising text
void normalise_spaces(char* input) {
    /*
    - normalises text by removing leading and trailing spaces
        - convert all types of white spaces into a singel space
        - keeps words separated by only 1 space


    Parameters
    ----------
    input: string
        the variable for text normalisation

    Returns
    -------
    void
    */
	
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

// function for removing white char
void remove_spaces(char* input) {
    /*
    - removes all forms of whitespace char
	
    Parameters
    ----------
    input: string
        the variable for removing white char

    Returns
    -------
    void

*/
    char* src = input;
    char* dst = input;

    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;  
        }
        src++;
    }
    *dst = '\0'; 
}


// save table to database function
int save_table(void) {
    /*
    The main function for saving table to the database function
    - opens the file, reads it and copies the metadata into a variable
    - close the file and open it in write mode
    - populate the metadata onto the file
    - loops through the typedef struct array and populate it with the current array
        
    Parameters
    ----------
    void

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful
    */    
	
    FILE* file = fopen(global_filepath, "r");
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
    file = fopen(global_filepath, "w");
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


// Sorting by ID in ascending order
int compare_id_asc(const void *a, const void *b) {
    const Record *rec_a = (const Record *)a;
    const Record *rec_b = (const Record *)b;
    return (rec_a->id > rec_b->id) - (rec_a->id < rec_b->id);
}

// Sorting by ID in descending order
int compare_id_desc(const void *a, const void *b) {
    const Record *rec_a = (const Record *)a;
    const Record *rec_b = (const Record *)b;
    return (rec_b->id > rec_a->id) - (rec_b->id < rec_a->id);
}

// Sorting by Mark in ascending order
int compare_mark_asc(const void *a, const void *b) {
    const Record *rec_a = (const Record *)a;
    const Record *rec_b = (const Record *)b;
    if (rec_a->mark < rec_b->mark) return -1;
    if (rec_a->mark > rec_b->mark) return 1;
    return 0;
}

// Sorting by Mark in descending order
int compare_mark_desc(const void *a, const void *b) {
    const Record *rec_a = (const Record *)a;
    const Record *rec_b = (const Record *)b;
    if (rec_a->mark > rec_b->mark) return -1;
    if (rec_a->mark < rec_b->mark) return 1;
    return 0;
}

// Sorting function
void show_sorted_records(int (*comparator)(const void *, const void *)) {
    if (records == NULL || record_count == 0) {
        puts("CMS: No records to sort.");
        return;
    }

    // Create a temporary copy of the records
    Record *sorted_records = (Record *)malloc(record_count * sizeof(Record));
    if (sorted_records == NULL) {
        perror("Memory allocation failed for sorting");
        return;
    }
    memcpy(sorted_records, records, record_count * sizeof(Record));

    // Sort the temporary copy
    qsort(sorted_records, record_count, sizeof(Record), comparator);

    // Print the sorted records
    printf("CMS: Here are all the records found in the table \"StudentRecords\".\n");
    printf("%-10s %-20s %-25s %-10s\n", "ID", "Name", "Programme", "Mark"); // Column headers
    for (int i = 0; i < record_count; i++) {
        printf("%-10d %-20s %-25s %-10.1f\n",
               sorted_records[i].id, sorted_records[i].name, sorted_records[i].programme, sorted_records[i].mark);
    }

    // Free the temporary copy
    free(sorted_records);
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

    printf("--- Summary of Student Records ---\n");
    printf("%-20s: %d\n", "Total Records", record_count);
    printf("%-20s: %.1f\n", "Average Mark", average_mark);
    printf("%-20s: %.1f (by %s)\n", "Highest Mark", highest_mark, highest_mark_student);
    printf("%-20s: %.1f (by %s)\n", "Lowest Mark", lowest_mark, lowest_mark_student);
    printf("----------------------------------\n");
}

// function to create a new database
int create_database(char* input) {
    /*
    The main function for creating a new database enhancement operations
    - checks if the input command starts with CREATE
    - creates variables with placeholder values
    - identify headers: Database=, Authors=, Table= and extracts out the data between them and store them in
      the variables
    - checks if all variables are not holding placeholder values else return error messaage
    - normalise text and remove unncessary white spaces in the variables
    - calls validation functions:
        - Database: char limit of 64
        - Authors: only letters are allowed, no special char or numbers, char limit of 64
        - Table: Only letters and numbers, no special char, char limit of 64
    - if all validation passes, a new txt file is created and metadata along with ID, Name, Programme, Mark headers
      are written into a new file

    Parameters
    ----------
    input: string
        the input insert command typed in by the user

    Returns
    -------
    int: 1 or 0
        returns 1 if an error is detected
        returns 0 if the operation is successful

    */

    // Check if it starts with CREATE
    if (strncmp(input, "CREATE ", 7) != 0) {
        printf("\nError: Command must start with CREATE\n");
        return 1;
    }

    // variables with placeholder values
    char database_name[64] = "";
    char authors[64] = "";
    char table_name[64] = "";

    // Find Database=
    char* db_pos = strstr(input, "Database=");
    // Find Authors=
    char* auth_pos = strstr(input, "Authors=");
    // Find Table=
    char* table_pos = strstr(input, "Table=");

    // Extract Database name
    if (db_pos && auth_pos) {
        db_pos += 9;
        int length = auth_pos - db_pos;

        if (length >= 64) {
            printf("\nError: Database name exceeds 64 character limit\n");
            return 1;
        }

        strncpy(database_name, db_pos, length);
        database_name[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && database_name[i] == ' '; i--) {
            database_name[i] = '\0';
        }
    }

    // Extract Authors name
    if (auth_pos && table_pos) {
        auth_pos += 8;
        int length = table_pos - auth_pos;

        if (length >= 64) {
            printf("\nError: Authors name exceeds 64 character limit\n");
            return 1;
        }

        strncpy(authors, auth_pos, length);
        authors[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && authors[i] == ' '; i--) {
            authors[i] = '\0';
        }
    }

    // Extract Table name
    if (table_pos) {
        table_pos += 6;

        int length = 0;
        while (table_pos[length] != '\0' &&
            table_pos[length] != '\n' &&
            table_pos[length] != '\r') {
            length++;
        }

        if (length >= 64) {
            printf("\nError: Table name exceeds 64 character limit\n");
            return 1;
        }

        strncpy(table_name, table_pos, length);
        table_name[length] = '\0';

        // Trim spaces
        for (int i = length - 1; i >= 0 && table_name[i] == ' '; i--) {
            table_name[i] = '\0';
        }
    }

    // Validate for any empty fields
    if (strlen(database_name) == 0 || strlen(authors) == 0 || strlen(table_name) == 0) {
        printf("\nError: Missing required fields (Database, Authors, or Table)\n");
        return 1;
    }

    // Name input validation
    if (special_check(table_name, "Table") == 1) {
        return 1;
    }

    // Programme input validation
    if (special_and_number_check(authors, "Authors") == 1) {
        return 1;
    }

    // Create filename from database name
    char filename[128];
    snprintf(filename, sizeof(filename), "%s.txt", database_name);

    // Check if file already exists
    FILE* check = fopen(filename, "r");
    if (check != NULL) {
        fclose(check);
        printf("\nError: Database '%s' already exists\n", database_name);
        return 1;
    }

    // Create the file and write header
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error creating database file");
        return 1;
    }

    // Populate file with metadata and headers
    fprintf(file, "Database Name: %s\n", database_name);
    fprintf(file, "Authors: %s\n", authors);
    fprintf(file, "\nTable Name: %s\n", table_name);
    fprintf(file, "ID\tName\t\tProgramme\t\tMark\n");

    fclose(file);

    printf("Successfully created database: %s.txt\n", filename);
    return 0;
}

