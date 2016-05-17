#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"

static token_t next_token(void);
static token_t create_token(token_type_t);
static token_t create_int_lit_token(int);
static bool is_digit(char);
static bool is_whitespace(char);
static int consume_int_literal(void);
static void consume_char(void);
static void init_tokenizer(FILE *);
static void skip_whitespace();

static const int MAX_TOKENS = 1000; //TODO: We don't want this.
static char next_char; /* The next character to be tokenized. */
static FILE *input;

//TODO: Return a linked list (or resizable vector) of tokens
token_t *tokenize(FILE *file) {
  init_tokenizer(file);

  token_t *tokens = (token_t *) malloc(sizeof(token_t) * MAX_TOKENS);
  int i = 0;
  
  do {
    tokens[i] = next_token();
  } while (tokens[i++].type != PROGRAM_END_TOK);

  return tokens;
}

/* Consumes a character from the input file, and parses the token starting at
 * that character. It may consumer more characters during this process, if the
 * token consists of more than one characters. */
token_t next_token() {
  token_t token;
  skip_whitespace();

  if (is_digit(next_char)) {
    token = create_int_lit_token(consume_int_literal());
    // We don't need to consume a char, consume_int has already done that for us.
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
      default:
        printf("Warning: Unknown character %c (%d)\n", next_char, next_char);
        token = create_token(INVALID_TOK);
        break;
    }

    consume_char(); // Consume the char we just read
  }

  return token;
}

static token_t create_token(token_type_t type) {
  token_t token;
  token.type = type;

  return token;
}

static token_t create_int_lit_token(int val) {
  token_t token;
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
  while (is_digit(next_char) && next_char != EOF) {
    val = (next_char-'0') + val * 10;
    consume_char();
  }
  return val;
}

/* Takes a single char from the input file and stores it in next_char. */
static void consume_char(void) {
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

static void init_tokenizer(FILE *file) {
  input = file;
  next_char = 0;
  consume_char();
}
