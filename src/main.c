#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "declaration.h"
#include "crud.h"
#include "update.h"

// Function to read a string command from the user
static void read_command(char *buf, size_t size) {
    if (!fgets(buf, size, stdin)) return;
    // Remove trailing newline character if it exists
    buf[strcspn(buf, "\n")] = 0;
}

// Show menu instructions
static void print_menu(void) {
    puts("\n--- CMS Menu ---");
    puts("Type a command from the list below:");
    puts("open  - Open the database");
    puts("show all  - Show all records");
    puts("show all sort by id asc - Show records sorted by ID (Ascending)");
    puts("show all sort by id desc - Show records sorted by ID (Descending)");
    puts("show all sort by mark asc - Show records sorted by Mark (Ascending)");
    puts("show all sort by mark desc - Show records sorted by Mark (Descending)");
	puts("show summary  - Show summary statistics");
    puts("query ID=<id>  - Query a record by ID");
    puts("insert - Open insert menu");
	puts("create - Create database menu");
    puts("delete ID=<id> - Delete records(s) by ID (with confirmation)");
    puts("update <find_column>=<find_value> <update_column>=<new_value> - Update record");
    puts("save - save changes to the database");
    puts("exit  - Exit the program");
    printf("Enter command: ");
}

// Action to open the database
static void action_open(char* command) {
    free_records(); // Free previously loaded records

    char filename[512];
    if (sscanf(command, "open=%255[^\n]", filename) == 1) {
        if (open_database(filename) == 0) {
            printf("OPEN\n");
            printf("The database file \"%s\" is successfully opened.\n", filename);
        }
        else {
            printf("Failed to open the database file \"%s\".\n", filename);
        }
    }
    else {
        printf("Invalid query format. Use: open=<filepath>\n");
    }

}

// Action to show all records
static void action_show_all(void) {
    printf("SHOW ALL\n");
    show_all_records();
}

// Action to query a record by ID
static void action_query(char *command) {
    query_record(command);  // Pass the full query command (e.g., query ID=1234 or query Name=Joshua)
}

//Action to delete record(s) by ID with a single confirmation
static void action_delete(char *command) {
    int id;
    if (sscanf(command, "delete ID=%d", &id) !=1) {
        printf("Invalid delete format. Use: delete ID=<id>\n");
        return;
    }

    int cnt = count_records_by_id(id);
    if (cnt == -1) {
        puts("CMS: No records loaded. Please open the database first.");
        return;
    }
    if (cnt == 0) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return;
    }

    printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel.\n", id);
    char answer[16];
    read_command(answer, sizeof(answer));

    if (strcmp(answer, "Y") == 0 || strcmp(answer, "y") == 0) {
        // Delete all records with the same ID by repeatedly deleting one
        int total_deleted = 0;
        while (1) {
            int rc = delete_record_by_id(id);
            if (rc == 1) {
                total_deleted++;
                continue;
            }
            break;
        }
        if (total_deleted > 0) {
            printf("CMS: The record with ID=%d is successfully deleted.\n", id);
        } else { 
            printf("CMS: The record with ID=%d does not exist.\n", id);
        }
    } else {
        puts("CMS: The deletion is cancelled.");
    }
}

//Action to save the changes to the database
static void action_save(void) {
    if (save_table() == 0) {
        printf("\nDatabase successfully updated\n");
    }
    else {
        printf("\nError updating database please try again.\n");
    }
}

// Action for insert function and page
static void action_insert(void) {
    while (1) {
        char command[255];

        puts("\n--- CMS insert ---");
        puts("To insert into the database please follow the format:");
        puts("INSERT ID=<ID of student> Name=<Name of student> Programme=<Programme the student is enrolled in> Mark=<Their grade>");
        puts("EXAMPLE: INSERT ID=2401234 Name=Michelle Lee Programme=Information Security Mark=73.2");
        puts("To go back, enter back");
        printf(">>>");

        read_command(command, sizeof(command));

        //tackles stack overflow
        size_t len = strlen(command);
        if (len == sizeof(command) - 1 && command[len - 1] != '\n') {
            printf("\nError: Input line too long (exceeds %d characters)\n", (int)sizeof(command) - 1);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            continue;  
        }


        if (strncmp(command, "back", 4) == 0) {
            break;
        }
        if (insert_record(command) == 0) {
            printf("New changes added to the table\n");

        }

    }
}

static void action_create(void) {
    while (1) {
        char command[255];

        puts("\n--- CMS create ---");
        puts("To create a new database file please follow the format:");
        puts("CREATE Database=<Name of database> Authors=<Name of Authors> Table=<Name of table>");
        puts("EXAMPLE: CREATE Database=P12-9 Authors=Jane To Table=<Name of table>");
        puts("To go back, enter back");
        printf(">>>");

        read_command(command, sizeof(command));

        //tackles stack overflow
        size_t len = strlen(command);
        if (len == sizeof(command) - 1 && command[len - 1] != '\n') {
            printf("\nError: Input line too long (exceeds %d characters)\n", (int)sizeof(command) - 1);

            int c;
            while ((c = getchar()) != '\n' && c != EOF);

            continue;
        }


        if (strncmp(command, "back", 4) == 0) {
            break;
        }
        if (create_database(command) == 0) {
            printf("New databsae created\n");

        }

    }
}


// Main function
int main(void) {
    show_declaration();  // Show declaration when the program starts
    Record *records;
    int record_count;

    while (1) {
        print_menu();
        char command[255];
        read_command(command, sizeof(command));

        if (strncmp(command, "open", 4) == 0) {
            action_open(command);
        }
        else if (strcmp(command, "show all sort by id asc") == 0) {
            show_sorted_records(compare_id_asc);
        }
        else if (strcmp(command, "show all sort by id desc") == 0) {
            show_sorted_records(compare_id_desc);
        }
        else if (strcmp(command, "show all sort by mark asc") == 0) {
            show_sorted_records(compare_mark_asc);
        }
        else if (strcmp(command, "show all sort by mark desc") == 0) {
            show_sorted_records(compare_mark_desc);
        }
        else if (strncmp(command, "show all", 8) == 0) {
            action_show_all();
        }
        else if (strncmp(command, "show summary", 12) == 0) {
            show_summary();
        }
        else if (strncmp(command, "query", 5) == 0) {
            action_query(command);
        }
        else if (strncmp(command, "insert", 6) == 0) {
            action_insert();
        }
        else if (strncmp(command, "create", 6) == 0) {
            action_create();
        }
        else if (strncmp(command, "delete", 6) == 0) {
            action_delete(command);
        }
        else if (strncmp(command, "update", 6) == 0) {
            get_record_refs(&records, &record_count);
            action_update(records, record_count, command);
        }
        else if (strncmp(command, "save", 4) == 0) {
            action_save();
        }
        else if (strncmp(command, "exit", 4) == 0) {
            puts("Exiting. Goodbye!");
            free_records();
            return 0;
        }
        else {
            puts("Invalid command. Please try again.");
        }
    }
}

