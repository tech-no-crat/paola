#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"

typedef enum {
  INVALID,
  INT_LIT,
  BIN_OP
} expr_ast_type_t;

typedef enum {
  INVALID_OP,
  PLUS
} operator_t;

typedef struct expr_ast_type {
  expr_ast_type_t type;
  union {
    int ival; // INT_LIT case
    struct {  // BIN_OP case
      operator_t op;
      expr_ast_type *left, *right;
    };
  };
} expr_ast_t;

expr_ast_t *parse(token_t *);
#endif
