#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "crud.h"
#include "config.h"
#define DEBUG_MODE 1
#define RETURNCODE unsigned long

#ifdef DEBUG_MODE
void debug(char *_msg) {
  puts(_msg);
}
#else
void debug(char _msg) {}
#endif
/*
  Primitive functions:
  - Get next row
  - Get next column
    - Get cell value of row and column
  - Update row
  - Insert row
  - Create column
  - Update column
  - Create table
  - Delete table
  - Create database

  Direct access:
  - Get/Update Table name
  - Get/Update Database name
 */

RETURNCODE find_record_by_id(Record *records,
                             size_t record_count,
                             int id,
                             Record **filtered_record) {
  for (int i = 0; i < record_count; i++) {
    if (records[i].id == id) {
      *filtered_record = &records[i];
      return 0;
    }
  }
  return -1;
}

/*
RETURNCODE find_record(void *records,
                       size_t record_count,
                       size_t target_offset,
                       size_t container_size,
                       void* value,
                       void*(*comparator(void*,size_t,void*)),
                       void **filtered_record) {
  for (int i = 0; i < record_count; i++) {
    if (records[i].id == id) {
      *filtered_record = &records[i];
      return 0;
    }
  }
  return -1;
  }*/

RETURNCODE update_record(void *record,
                         size_t target_offset,
                         void* value,
                         size_t container_size) {
  printf("\nupdate_record called!,"
         "\n\trecord name: %s"
         "\n\ttarget_offset: %lu"
         "\n\tvalue: %s"
         "\n\tcontainer_size: %lu"
         "\n",
         ((Record *)record)->name, target_offset, (char *)value, container_size);
  size_t value_size = sizeof(value);
  size_t copy_size = value_size > container_size ? container_size : value_size;
  //printf("\n\toffsetof recalced: %lu", offsetof(Record, name));
  //printf("\n\trecord name offset-based: %s", (char *)record + offsetof(Record, name));
  //printf("\n\tcopy_size: %lu", copy_size);
  //printf("\n\trecord+target_offset (OLD): %s", (char *)record + target_offset);
  memcpy((char *)record + target_offset, value, copy_size);
  //printf("\n\trecord+target_offset (NEW): %s", (char *)record + target_offset);
  return 0;
}

/*
  Update a record in-place by ID.
  memory-format: CMSv1Store
 */
RETURNCODE update_record_by_id(Record *records,
                               size_t record_count,
                               int id,
                               size_t target_offset,
                               void* value,
                               size_t container_size) {
  Record *filtered_record;
  if (find_record_by_id(records,
                        record_count,
                        id,
                        &filtered_record) < 0) {
    printf("The record with ID=%d does not exist.", id);
    return -1;
  }

  return update_record(filtered_record,
                       target_offset,
                       value,
                       container_size) << 4;
}

typedef struct TargetValue {
  char *target;
  void *value;
  size_t container_size;
} TargetValue;

typedef enum _DataType {
  INT32,
  FLOAT,
  STRING,
} DataType;

typedef struct _Column {
  char *name;
  DataType data_type;
  size_t data_type_size;
} Column;

typedef struct _Table {
  char *name;
  Column *columns;
  size_t column_count;
} Table;

static inline RETURNCODE parse_command(char *command,
                                       char **_action,
                                       char **_filter_column,
                                       char **_filter_value,
                                       char **_target_column,
                                       char **_target_value) {
  // Rudemntary parser.
  // Limitations:
  // - Filter and target names must not have space
  // - Filter column value must not have space
  // - Filter column must be "ID" (downstream limitation)
  char *action = strtok(command, " ");
  char *filter_column = strtok(NULL, "=");
  char *filter_value = strtok(NULL, " ");
  char *target_column = strtok(NULL, "=");
  char *target_value = strtok(NULL, "=");
  size_t target_column_compare_length = strlen(target_column) < sizeof("ID") ? strlen(target_column) : sizeof("ID");
  if (!filter_column || !filter_value || !target_column || !target_value)
    return -1;
  if (!strncmp(target_column, "ID", target_column_compare_length))
    return -2;

  *_action = action;
  *_filter_column = filter_column;
  *_filter_value = filter_value;
  *_target_column = target_column;
  *_target_value = target_value;
  return 0;
  
  //if(sscanf(command, "update ID=%d", id) == 1) {
  //  return 0;
  //}
  //return -1;
}

/*
  memory-format: CMSv1Store
 */
static inline RETURNCODE get_container_offset_size(char *target_column,
                                                   size_t *offset,
                                                   size_t *size) {
  if (strncmp(target_column, "NAME", 4) == 0) {
    *offset = offsetof(Record, name);
    *size = sizeof((Record){}.name);
  } else if (strncmp(target_column, "MARK", 4) == 0) {
    *offset = offsetof(Record, mark);
    *size = sizeof((Record){}.mark);
  } else {
    return -1;
  }
  return 0;
}

RETURNCODE action_update(Record *records,
                         size_t record_count,
                         char *command) {
  char *action;
  char *filter_column;
  char *filter_value;
  char *target_column;
  char *target_value;
  size_t offset;
  size_t container_size;
  int id;
  if (parse_command(command,
                    &action,
                    &filter_column,
                    &filter_value,
                    &target_column,
                    &target_value) < 0)
    puts("Invalid query format. Use: update ID=<id> "
         "<target_column>=<target_value>");
  id = atoi(filter_value);
  for (size_t i = 0; i < strlen(target_column); i++) {
    target_column[i] = toupper(target_column[i]);
  }
  get_container_offset_size(target_column, &offset, &container_size);
  return update_record_by_id(records,
                             record_count,
                             id,
                             offset,
                             target_value,
                             container_size) << 4;
}
