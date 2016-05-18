#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "list.h"

typedef enum {
  INVALID_EXPR,
  INT_LIT,
  BIN_OP
} expr_ast_type_t;

typedef enum {
  INVALID_STAT,
  RETURN_STAT,
  IF_STAT,
  BLOCK_STAT
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
    int ival; // INT_LIT
    struct {  // BIN_OP
      operator_t op;
      expr_ast_type *left, *right;
    };
  };
} expr_ast_t;


typedef struct stat_ast_type {
  stat_ast_type_t type;
  union {
    expr_ast_t *expr; // RETURN_STAT
    
    struct { // IF_STAT
      expr_ast_t *cond;
      stat_ast_type *tstat, *fstat;
    };

    list_t stats; // BLOCK_STAT
  };
  list_elem_t block_elem;
} stat_ast_t;

stat_ast_t *parse(token_t *);

#endif
