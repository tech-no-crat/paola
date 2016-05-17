#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"

typedef enum {
  INVALID_EXPR,
  INT_LIT,
  BIN_OP
} expr_ast_type_t;

typedef enum {
  INVALID_STAT,
  RETURN_STAT
} stat_ast_type_t;

typedef enum {
  INVALID_OP,
  ADD,
  SUBS,
  MUL,
  DIV
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


typedef struct stat_ast_type {
  stat_ast_type_t type;
  expr_ast_t *expr;
  stat_ast_type *next;
} stat_ast_t;

stat_ast_t *parse(token_t *);

#endif
