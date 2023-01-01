#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION "0.1.0-dev"

typedef enum {
  lexer_token_start,
  lexer_identifier,
  lexer_string_end,
  lexer_comment,
  lexer_identifier_or_delimited_string_quote,
  lexer_delimited_string_start,
  lexer_inline_delimited_string,
  lexer_inline_delimited_string_end,
} state_t;

typedef enum {
  token_identifier,
  token_string,
  token_end_of_command,
  token_end_of_file,
  token_fatal_error,
} token_t;

typedef enum {
  command_append,
  command_prepend,
  command_set,
  command_unknown
} command_name_t;

struct lexer_t {
  state_t state;
  char closing_delimiter;
  size_t token_start;
  size_t token_end;
  size_t heredoc_start;
  size_t heredoc_size;
  size_t heredoc_pos;
  size_t index;
  int end_of_file;
  token_t token;
};

static void init_lexer(struct lexer_t *p) {
  p->state = lexer_token_start;
  p->index = 0;
  p->end_of_file = 0;
}

static void next_token(struct lexer_t *lexer, char *input, size_t n) {
  while (1) {
    char c;
    if (lexer->index < n) {
      c = input[lexer->index];
    } else {
      lexer->end_of_file = 1;
    }

    switch (lexer->state) {

    case lexer_token_start: {
      if (lexer->end_of_file) {
        lexer->token = token_end_of_file;
        return;
      }

      // Ignore comments
      if (c == '#') {
        lexer->state = lexer_comment;
        ++lexer->index;
        continue;
      }

      // Begin of a string
      if (c == '"') {
        lexer->state = lexer_string_end;
        lexer->token_start = lexer->index + 1;
        ++lexer->index;
        continue;
      }

      // Either the start of a string r"(...)" or just an identifier.
      if (c == 'r') {
        lexer->state = lexer_identifier_or_delimited_string_quote;
        lexer->token_start = lexer->index;
        ++lexer->index;
        continue;
      }

      // End of a command
      if (c == '\n') {
        ++lexer->index;
        lexer->token = token_end_of_command;
        return;
      }

      // Ignore whitespace
      if (isspace(c)) {
        ++lexer->index;
        continue;
      }

      // Anything else is valid identifier
      lexer->state = lexer_identifier;
      lexer->token_start = lexer->index;
      ++lexer->index;
      break;
    }

    case lexer_identifier: {
      // emit end of identifier
      if (lexer->end_of_file || c == '\n') {
        lexer->token = token_identifier;
        lexer->state = lexer_token_start;
        lexer->token_end = lexer->index;
        return;
      }

      // emit identifier and step
      if (isspace(c)) {
        lexer->token = token_identifier;
        lexer->state = lexer_token_start;
        lexer->token_end = lexer->index;
        ++lexer->index;
        return;
      }

      // part of the identifier
      ++lexer->index;
      break;
    }

    case lexer_comment: {
      // comment end
      if (lexer->end_of_file) {
        lexer->token = token_end_of_file;
        return;
      }

      // comment end
      if (c == '\n') {
        lexer->token = token_end_of_command;
        lexer->state = lexer_token_start;
        ++lexer->index;
        return;
      }

      // consume comment
      ++lexer->index;
      break;
    }

    case lexer_string_end: {
      // unexpected end of file
      if (lexer->end_of_file) {
        fprintf(stderr, "Unexpected end of file while parsing string\n");
        lexer->token = token_fatal_error;
        return;
      }

      if (c == '"') {
        lexer->state = lexer_token_start;
        lexer->token = token_string;
        lexer->token_end = lexer->index;
        ++lexer->index;
        return;
      }

      // string continues;
      ++lexer->index;
      break;
    }

    case lexer_identifier_or_delimited_string_quote: {
      // end of an identifier
      if (lexer->end_of_file || c == '\n') {
        lexer->state = lexer_token_start;
        lexer->token = token_identifier;
        lexer->token_end = lexer->index;
        return;
      }

      // end of identifier
      if (isspace(c)) {
        lexer->state = lexer_token_start;
        lexer->token = token_identifier;
        lexer->token_end = lexer->index;
        ++lexer->index;
        return;
      }

      // begin of r"(...)" delimited string
      if (c == '"') {
        lexer->state = lexer_delimited_string_start;
        lexer->heredoc_size = 0;
        lexer->heredoc_start = lexer->index + 1;
        ++lexer->index;
        continue;
      }

      // identifier
      lexer->state = lexer_identifier;
      ++lexer->index;
      break;
    }

    case lexer_delimited_string_start: {
      // unexpected end of file
      if (lexer->end_of_file) {
        fprintf(stderr, "Unexpected end of file while parsing delimited string\n");
        lexer->token = token_fatal_error;
        return;
      }

      // parse r"{...
      if (c == '{' || c == '[' || c == '(' || c == '<') {
        lexer->state = lexer_inline_delimited_string;
        lexer->closing_delimiter = c == '{' ? '}' : c == '[' ? ']' : c == '(' ? ')' : '>';
        lexer->token_start = lexer->index + 1;
        ++lexer->index;
        continue;
      }

      ++lexer->heredoc_size;
      ++lexer->index;
      continue;
    }

    case lexer_inline_delimited_string: {
      // unexpected end of file
      if (lexer->end_of_file) {
        fprintf(stderr, "Unexpected end of file while parsing delimited string\n");
        lexer->token = token_fatal_error;
        return;
      }

      // closing delimiter
      if (c == lexer->closing_delimiter) {
        lexer->state = lexer_inline_delimited_string_end;
        lexer->heredoc_pos = 0;
        lexer->token_end = lexer->index;
        ++lexer->index;
        continue;
      }

      ++lexer->index;
      break;
    }

    case lexer_inline_delimited_string_end: {
      if (lexer->end_of_file) {
        fprintf(stderr, "Unexpected end of file while parsing delimited string\n");
        lexer->token = token_fatal_error;
        return;
      }


      if (lexer->heredoc_pos < lexer->heredoc_size) {
        // Matching heredoc character first.
        if (c == input[lexer->heredoc_start + lexer->heredoc_pos]) {
          ++lexer->heredoc_pos;
          ++lexer->index;
          continue;
        }

        // Mismatch! Reset heredoc pos.
        lexer->heredoc_pos = 0;

        // Are we again in string closing mode
        if (c == lexer->closing_delimiter) {
          ++lexer->index;
          continue;
        }

        // If not, it's string territory.
        lexer->state = lexer_inline_delimited_string;
        ++lexer->index;
        continue;
      }

      // Finished the heredoc
      if (c == '"') {
        lexer->state = lexer_token_start;
        lexer->token = token_string;
        ++lexer->index;
        return;
      }

      // Repeated closing character, maybe the next one ends the string.
      if (c == lexer->closing_delimiter) {
        ++lexer->index;
        continue;
      }

      // still a string.
      lexer->state = lexer_inline_delimited_string;
      ++lexer->index;
      break;
    }
    }
  }
}

command_name_t parse_command(char const *start, char const *end) {
  size_t len = end - start;
  if (len == 6 && strncmp(start, "append", 6) == 0) {
    return command_append;
  }

  if (len == 7 && strncmp(start, "prepend", 7) == 0) {
    return command_prepend;
  }

  if (len == 3 && strncmp(start, "set", 3) == 0) {
    return command_set;
  }

  return command_unknown;
}

struct parsed_string_t {
  char * start;
  char * end;
  size_t len;
};

static int next_identifier_or_string(struct lexer_t *lexer, struct parsed_string_t *str, char *program, size_t n) {
  next_token(lexer, program, n);

  if (lexer->token != token_identifier && lexer->token != token_string) return -1;

  str->start = program + lexer->token_start;
  str->end = program + lexer->token_end;
  str->len = str->end - str->start;
  return 0;
}

static int next_end_of_command(struct lexer_t *lexer, char *program, size_t n) {
  next_token(lexer, program, n);
  if (lexer->token != token_end_of_command && lexer->token != token_end_of_file)
    return 1;
  return 0;
}

static int run_command_set(struct parsed_string_t *variable, struct parsed_string_t *value) {
  char var_end = *variable->end;
  char val_end = *value->end;
  *variable->end = 0;
  *value->end = 0;
  setenv(variable->start, value->start, 1);
  *variable->end = var_end;
  *value->end = val_end;
  return 0;
}

static void concat_3(char *dst, char *a, size_t a_len, char *b, size_t b_len, char *c, size_t c_len) {
  memcpy(dst, a, a_len);
  memcpy(dst + a_len, b, b_len);
  memcpy(dst + a_len + b_len, c, c_len);
}

static int run_command_append_or_prepend(struct parsed_string_t *variable, struct parsed_string_t *delim, struct parsed_string_t *value, int append) {
  // temporarily C-stringify
  char old_variable_end = *variable->end;
  char old_delim_end = *delim->end;
  char old_value_end = *value->end;
  *variable->end = 0;
  *delim->end = 0;
  *value->end = 0;

  char * old_val = getenv(variable->start);
  if (old_val == NULL) {
    setenv(variable->start, value->start, 1);
  } else {
    size_t old_len = strlen(old_val);
    size_t new_len = old_len + delim->len + value->len;
    char *new_val = malloc(new_len + 1); // trailing null
    if (new_val == NULL) return 1;
    if (append)
      concat_3(new_val, old_val, old_len, delim->start, delim->len, value->start, value->len + 1);
    else
      concat_3(new_val, value->start, value->len, delim->start, delim->len, old_val, old_len + 1);
    setenv(variable->start, new_val, 1);
    free(new_val);
  }

  // undo c-stringification
  *variable->end = old_variable_end;
  *delim->end = old_delim_end;
  *value->end = old_value_end;
  return 0;
}

static int execute(char *program, size_t n) {
  struct lexer_t lexer;
  struct parsed_string_t variable, delim, value;
  init_lexer(&lexer);

  while (1) {
    // Parse a new command.
    next_token(&lexer, program, n);

    if (lexer.token == token_fatal_error)
      return 1;

    if (lexer.token == token_end_of_file)
      return 0;

    // Skip over lines without commands
    if (lexer.token == token_end_of_command)
      continue;
    
    // Commands are identifiers
    if (lexer.token != token_identifier) {
      printf("Expected a command got %d\n", lexer.token);
      return 1;
    }

    // Identifier should be a command
    command_name_t cmd =
        parse_command(program + lexer.token_start, program + lexer.token_end);
    switch (cmd) {
    case command_append:
    case command_prepend:
      if (next_identifier_or_string(&lexer, &variable, program, n) != 0) return 1;
      if (next_identifier_or_string(&lexer, &delim, program, n) != 0) return 1;
      if (next_identifier_or_string(&lexer, &value, program, n) != 0) return 1;
      if (next_end_of_command(&lexer, program, n) != 0) return 1;
      if (run_command_append_or_prepend(&variable, &delim, &value, cmd == command_append) != 0) return 1;
      break;

    case command_set:
      if (next_identifier_or_string(&lexer, &variable, program, n) != 0) return 1;
      if (next_identifier_or_string(&lexer, &value, program, n) != 0) return 1;
      if (next_end_of_command(&lexer, program, n) != 0) return 1;
      if (run_command_set(&variable, &value) != 0) return 1;
      break;

    case command_unknown:
      puts("Unknown commmand");
      return 1;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("Provide a script");
    return 1;
  }

  // Quickly scan for --version flag
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-' && strcmp(argv[i], "--version") == 0) {
      puts(VERSION);
      return 0;
    }
  }

  // The wrapper file
  char *name = argv[1];
  FILE *f = fopen(name, "rb");
  if (f == NULL)
    return 1;

  // Get the real exe: [wrapper]-real
  char real_exe[4096];
  int file_len = strlen(name);
  memcpy(real_exe, name, file_len);
  strcpy(real_exe + file_len, "-real");

  // Read file into memory
  if (fseek(f, 0, SEEK_END) != 0) return 1;
  long int fsize = ftell(f);
  if (fsize == -1) return 1;
  if (fseek(f, 0, SEEK_SET) != 0) return 1;
  char *str = malloc(fsize + 1);
  if (str == NULL) return 1;
  size_t num_read = fread(str, 1, fsize, f);
  if (num_read != (size_t) fsize) return 2;
  fclose(f);
  str[fsize] = 0;

  int result = execute(str, fsize);
  if (result != 0)
    return result;

  int exec_result = execv(real_exe, argv + 1);
  printf("Could not execute %s\n", real_exe);
  return exec_result;
}
