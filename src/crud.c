#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
        if (strstr(line, "ID") && strstr(line, "Name") && strstr(line, "Programme") && strstr(line, "Mark")) {
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