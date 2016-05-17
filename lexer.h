#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef enum {
  PROGRAM_END_TOK,
  INVALID_TOK,
  INT_LIT_TOK,
  PLUS_TOK,
  LPAREN_TOK,
  RPAREN_TOK
} token_type_t;

typedef struct token {
  token_type_t type;
  int ival;
} token_t;

token_t *tokenize(FILE *);

#endif
