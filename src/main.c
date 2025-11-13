#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "declaration.h"
#include "crud.h"

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
	puts("show summary  - Show summary statistics");
    puts("query ID=<id>  - Query a record by ID");
    puts("delete ID=<id> - Delete records(s) by ID (with confirmation)");
    puts("exit  - Exit the program");
    printf("Enter command: ");
}

// Action to open the database
static void action_open(void) {
    const char *folder = "data";
    const char *filename = "P6-8-CMS.txt"; // Update filename to match expected output
    free_records(); // Free previously loaded records

    if (open_database(folder, filename) == 0) {
        printf("OPEN\n");
        printf("The database file \"%s\" is successfully opened.\n", filename);
    } else {
        printf("Failed to open the database file \"%s\".\n", filename);
    }
}

// Action to show all records
static void action_show_all(void) {
    printf("SHOW ALL\n");
    show_all_records();
}

// Action to query a record by ID
static void action_query(char *command) {
    int id;
    // Extract the ID from the query command (format: query ID=<id>)
    if (sscanf(command, "query ID=%d", &id) == 1) {
        query_record(id);
    } else {
        printf("Invalid query format. Use: query ID=<id>\n");
    }
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

// Main function
int main(void) {
    show_declaration();  // Show declaration when the program starts

    while (1) {
        print_menu();
        char command[64];
        read_command(command, sizeof(command));

        if (strncmp(command, "open", 4) == 0) {
            action_open();
        } else if (strncmp(command, "show all", 8) == 0) {
            action_show_all();
		} else if (strncmp(command, "show summary", 12) == 0) {
            show_summary();
        } else if (strncmp(command, "query", 5) == 0) {
            action_query(command); 
        } else if (strncmp(command, "delete", 6) == 0) {
            action_delete(command);
        } else if (strncmp(command, "exit", 4) == 0) {
            puts("Exiting. Goodbye!");
            free_records();
            return 0;
        } else {
            puts("Invalid command. Please try again.");
        }
    }
}
