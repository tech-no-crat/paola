#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "errors.h"
#include "lexer.h"

static token_t next_token(void);
static token_t create_token(token_type_t);
static position_t create_position(void);
static token_t create_int_lit_token(int);
static token_t create_identifier_token(char *);
static bool is_digit(char);
static bool is_whitespace(char);
static int consume_int_literal(void);
static void consume_char(void);
static void init_tokenizer(FILE *);
static void skip_whitespace();
static bool is_alphabetic(char);
static bool is_alphanumeric(char);
static token_t create_ident_or_keyword_token(char *);
static char *consume_string(void);
static void skip_line();
static char peek();

static const int MAX_TOKENS = 10000; //TODO: We don't want this.
static const int MAX_STRING_SIZE = 64; //TODO: We don't want this either.

static char next_char; /* The next character to be tokenized. */
static FILE *input;
static int line, character;

//TODO: Return a linked list (or resizable vector) of tokens
token_t *tokenize(FILE *file) {
  init_tokenizer(file);

  token_t *tokens = (token_t *) malloc(sizeof(token_t) * MAX_TOKENS);
  int i = 0;
  int token_count = 0;
  
  do {
    tokens[i] = next_token();
    if (tokens[i].type != INVALID_TOK) token_count++;
  } while (tokens[i++].type != PROGRAM_END_TOK);

  token_t *ret = (token_t *) malloc(sizeof(token_t) * token_count);
  int c = 0;
  for (int j = 0; j < i; j++) {
    if (tokens[j].type != INVALID_TOK) {
      ret[c++] = tokens[j];
    }
  }
  free(tokens);

  return ret;
}

/* Consumes a character from the input file, and parses the token starting at
 * that character. It may consumer more characters during this process, if the
 * token consists of more than one characters. */
token_t next_token() {
  token_t token;
  skip_whitespace();

  if (is_alphabetic(next_char)) {
    token = create_ident_or_keyword_token(consume_string());
    // We don't need to consume a char, consume_string has already done that.
  }
  else if (is_digit(next_char)) {
    token = create_int_lit_token(consume_int_literal());
    // We don't need to consume a char, consume_int has already done that
  } else {
    switch (next_char) {
      case EOF:
        token = create_token(PROGRAM_END_TOK);
        break;
      case '(':
        token = create_token(LPAREN_TOK);
        break;
      case ')':
        token = create_token(RPAREN_TOK);
        break;
      case '+':
        token = create_token(PLUS_TOK);
        break;
      case '-':
        token = create_token(MINUS_TOK);
        break;
      case '*':
        token = create_token(STAR_TOK);
        break;
      case '/':
        if (peek() == '/') {
          token = create_token(INVALID_TOK);
          skip_line();
        } else {
          token = create_token(FSLASH_TOK);
        }
        break;
      case '{':
        token = create_token(LBRACE_TOK);
        break;
      case '}':
        token = create_token(RBRACE_TOK);
        break;
      case ';':
        token = create_token(SCOL_TOK);
        break;
      case '=':
        token = create_token(EQ_TOK);
        break;
      default: {
        position_t pos = create_position();
        warning(&pos, "Unknown token %c (%d)\n", next_char, next_char);
        token = create_token(INVALID_TOK);
        break;
      }
    }

    consume_char(); // Consume the char we just read
  }

  return token;
}


static token_t create_token(token_type_t type) {
  token_t token;
  token.type = type;
  token.pos = create_position();

  return token;
}

static position_t create_position() {
  position_t pos;
  pos.line = line;
  pos.character = character;
  return pos;
}

static token_t create_int_lit_token(int val) {
  token_t token = create_token(INT_LIT_TOK);
  token.type = INT_LIT_TOK;
  token.ival = val;

  return token;
}

static bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

static bool is_whitespace(char c) {
  return c == '\t' || c == ' ' || c == '\n'; // TODO: Add other whitespace characters
}

/* Returns the integer starting at the current character stored in next_char.
 * When it returns, next_char will not be a digit. */
static int consume_int_literal(void) {
  int val = 0;
  while (is_digit(next_char)) {
    val = (next_char-'0') + val * 10;
    consume_char();
  }
  return val;
}

/* Takes a single char from the input file and stores it in next_char. */
static void consume_char(void) {
  if (next_char == '\n') {
    line++;
    character = 0;
  } else {
    character++;
  }

  next_char = fgetc(input);
}

/* Skips all whitespace. When called, next_char must be a whitespace character,
 * otherwise it will do nothing. When it returns, next_char will be the next
 * non-whitespace character.  */
static void skip_whitespace(void) {
  while (is_whitespace(next_char)) {
    consume_char();
  }
}

static void skip_line() {
  while (next_char != '\n') {
    consume_char();
  }
}

static char peek() {
  char c = fgetc(input);
  ungetc(c, input);
  return c;
}

/* Returns true if the char is in a-z, A-Z or an underscore (_). */
static bool is_alphabetic(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static bool is_alphanumeric(char c) {
  return is_alphabetic(c) || is_digit(c);
}

static char *consume_string(void) {
  char *str = (char *) malloc(sizeof(char) * MAX_STRING_SIZE);
  int i = 0;
  while (is_alphanumeric(next_char)) {
    str[i++] = next_char;
    consume_char();
  }
  str[i] = 0; /* End of string */
  return str;
}

static token_t create_ident_or_keyword_token(char *str) {
  if (strcmp("return", str) == 0) {
    free(str); /* We don't need the string anymore. */
    return create_token(RETURN_TOK);
  } else if (strcmp("if", str) == 0) {
    free(str);
    return create_token(IF_TOK);
  } else if (strcmp("else", str) == 0) {
    free(str);
    return create_token(ELSE_TOK);
  } else {
    /* It's not a reserved keyword, so it's an identifier. */
    return create_identifier_token(str);
  }
}

static token_t create_identifier_token(char *str) {
  token_t token = create_token(IDENT_TOK);
  token.name = str;

  return token;
}

static void init_tokenizer(FILE *file) {
  input = file;
  next_char = 0;
  line = 1;
  character = 1;
  consume_char();
}
