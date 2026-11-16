#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// MACROS
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)
#define TABLE_MAX_PAGES 100

// TYPE DEFINATIONS
typedef struct {
  char *buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum {
  META_COMMAND_SUCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  StatementType type;
  Row row_to_insert;
} Statement;

// CONSTANTS

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

// FUNCTION DEFINATIONS

void serialize_row(Row *src, void *dest) {
  memcpy(dest + ID_OFFSET, &(src->id), ID_SIZE);
  memcpy(dest + USERNAME_OFFSET, &(src->username), USERNAME_SIZE);
  memcpy(dest + EMAIL_OFFSET, &(src->email), EMAIL_SIZE);
}

void deserialize_row(void *src, Row *dest) {
  memcpy((&dest->id), src + ID_OFFSET, ID_SIZE);
  memcpy((&dest->username), src + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy((&dest->email), src + EMAIL_OFFSET, EMAIL_SIZE);
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement) {

  if (strncasecmp(input_buffer->buffer, "select", 6) == 0) {
    statement->type = STATEMENT_SELECT;
    return PREPARE_SUCESS;
  } else if (strncasecmp(input_buffer->buffer, "insert", 6) == 0) {
    statement->type = STATEMENT_INSERT;
    int params = sscanf(
        input_buffer->buffer, "insert %d %s %s", &statement->row_to_insert.id,
        statement->row_to_insert.username, statement->row_to_insert.email);
    if (params != 3) {
      return PREPARE_SYNTAX_ERROR;
    }
    return PREPARE_SUCESS;
  }
  return PREPARE_UNRECOGNIZED_STATEMENT;
}

InputBuffer *new_input_buffer() {
  InputBuffer *input_buffer = (InputBuffer *)malloc(sizeof(InputBuffer));

  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void close_input_buffer(InputBuffer *ib) {
  free(ib->buffer);
  free(ib);
}

void print_prompt() { printf("db > "); }

void read_input(InputBuffer *ip_buf) {
  ssize_t bytes_read =
      getline(&(ip_buf->buffer), &(ip_buf->buffer_length), stdin);
  if (bytes_read <= 0) {
    printf("Error: Unable to read the input\n");
    exit(EXIT_FAILURE);
  }
  ip_buf->buffer_length = bytes_read - 1;
  ip_buf->buffer[bytes_read - 1] = 0;
}

MetaCommandResult do_meta_commands(InputBuffer *input_buffer) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    close_input_buffer(input_buffer);
    exit(EXIT_SUCCESS);
  } else {
    return META_COMMAND_UNRECOGNIZED_COMMAND;
  }
}

void execute_command(Statement *statement) {
  switch (statement->type) {

  case STATEMENT_INSERT:
    printf("We will run INSERT in this case\n");
    break;
  case STATEMENT_SELECT:
    printf("We will run SELECT now\n");
    break;
  }
}

int main() {
  InputBuffer *input_buffer = new_input_buffer();

  while (1) {
    print_prompt();
    read_input(input_buffer);

    // caters the meta commands
    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_commands(input_buffer)) {
      case META_COMMAND_SUCESS:
        continue;
      case META_COMMAND_UNRECOGNIZED_COMMAND:
        printf("Unrecogined MetaCommand '%s'. \n", input_buffer->buffer);
        continue;
      }
    }

    // caters commands other then meta commands
    Statement statement;
    switch (prepare_statement(input_buffer, &statement)) {
    case PREPARE_SUCESS:
      break;
    case PREPARE_UNRECOGNIZED_STATEMENT:
      printf("Unrecogined Command '%s'. \n", input_buffer->buffer);
      continue;
    case PREPARE_SYNTAX_ERROR:
      printf("Syntax Error '%s'. \n", input_buffer->buffer);
      continue;
    }

    execute_command(&statement);
    printf("Executed the command Sucessfully\n");
  }
}
