#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
        puts("Unexpected end of file while parsing string\n");
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
        puts("Unexpected end of file while parsing delimited string\n");
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
        puts("Unexpected end of file while parsing delimited string\n");
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
        puts("Unexpected end of file while parsing delimited string");
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

typedef enum {
  command_append,
  command_prepend,
  command_set,
  command_unknown
} command_name_t;

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

static int execute(char *program, size_t n) {
  char *variable_start, *variable_end;
  char *delim_start, *delim_end;
  char *value_start, *value_end;
  char old_variable_end;
  char old_delim_end;
  char old_value_end;
  char *old_env;
  size_t old_len;
  size_t delim_len;
  size_t value_len;
  size_t new_len;

  struct lexer_t lexer;
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
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      variable_start = program + lexer.token_start;
      variable_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      delim_start = program + lexer.token_start;
      delim_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      value_start = program + lexer.token_start;
      value_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_end_of_command && lexer.token != token_end_of_file)
        return 1;

      // temporarily put some 0 there.
      old_variable_end = *variable_end;
      old_delim_end = *delim_end;
      old_value_end = *value_end;

      // c-stringify
      *variable_end = 0;
      *delim_end = 0;
      *value_end = 0;

      old_env = getenv(variable_start);
      if (old_env == NULL) {
        setenv(variable_start, value_start, 1);
      } else {
        old_len = strlen(old_env);
        delim_len = delim_end - delim_start;
        value_len = value_end - value_start;
        new_len = old_len + delim_len + value_len;
        char *concatenated = malloc(new_len + 1); // trailing null
        if (concatenated == NULL) return 1;
        memcpy(concatenated, old_env, old_len);
        memcpy(concatenated + old_len, delim_start, delim_len);
        memcpy(concatenated + old_len + delim_len, value_start,
               value_len + 1); // null
        setenv(variable_start, concatenated, 1);
        free(concatenated);
      }

      // undo c-stringification
      *variable_end = old_variable_end;
      *delim_end = old_delim_end;
      *value_end = old_value_end;
      break;

    case command_prepend:
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      variable_start = program + lexer.token_start;
      variable_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      delim_start = program + lexer.token_start;
      delim_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      value_start = program + lexer.token_start;
      value_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_end_of_command && lexer.token != token_end_of_file)
        return 1;

      // temporarily put some 0 there.
      old_variable_end = *variable_end;
      old_delim_end = *delim_end;
      old_value_end = *value_end;

      // c-stringify
      *variable_end = 0;
      *delim_end = 0;
      *value_end = 0;

      old_env = getenv(variable_start);
      if (old_env == NULL) {
        setenv(variable_start, value_start, 1);
      } else {
        old_len = strlen(old_env);
        delim_len = delim_end - delim_start;
        value_len = value_end - value_start;
        new_len = old_len + delim_len + value_len;
        char *concatenated = malloc(new_len + 1); // trailing null
        if (concatenated == NULL) return 1;
        memcpy(concatenated, value_start, value_len);
        memcpy(concatenated + value_len, delim_start, delim_len);
        memcpy(concatenated + value_len + delim_len, old_env,
               old_len + 1); // null
        setenv(variable_start, concatenated, 1);
        free(concatenated);
      }

      // undo c-stringification
      *variable_end = old_variable_end;
      *delim_end = old_delim_end;
      *value_end = old_value_end;
      break;

    case command_set:
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      variable_start = program + lexer.token_start;
      variable_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_identifier && lexer.token != token_string)
        return 1;
      value_start = program + lexer.token_start;
      value_end = program + lexer.token_end;
      next_token(&lexer, program, n);
      if (lexer.token != token_end_of_command && lexer.token != token_end_of_file)
        return 1;

      old_variable_end = *variable_end;
      old_value_end = *value_end;
      *variable_end = 0;
      *value_end = 0;
      setenv(variable_start, value_start, 1);
      *variable_end = old_variable_end;
      *value_end = old_value_end;
      break;

    case command_unknown:
      puts("Unknown commmand");
      return 1;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 2)
    return 1;

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
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *str = malloc(fsize + 1);
  if (str == NULL) return 1;
  long num_read = fread(str, 1, fsize, f);
  if (num_read != fsize) return 2;
  fclose(f);
  str[fsize] = 0;

  int result = execute(str, fsize);
  if (result != 0)
    return result;

  int exec_result = execv(real_exe, argv + 1);

  printf("Could not execute %s\n", real_exe);

  return exec_result;
}
