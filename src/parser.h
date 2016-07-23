#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "list.h"

typedef enum {
  INVALID_EXPR,
  INT_LIT,
  BIN_OP,
  VAR_REF
} expr_ast_type_t;

typedef enum {
  INVALID_DT,
  INT_DT
} datatype_t;

typedef enum {
  INVALID_STAT,
  RETURN_STAT,
  IF_STAT,
  WHILE_STAT,
  FOR_STAT,
  BLOCK_STAT,
  DECL_STAT,
  EXPR_STAT,
  SKIP_STAT
} stat_ast_type_t;

typedef enum {
  INVALID_OP,
  ADD,
  SUBS,
  MUL,
  DIV,
  ASSIGN,
  EQ,
  GT,
  GTE,
  LT,
  LTE
} operator_t;

typedef struct expr_ast_type {
  expr_ast_type_t type;
  bool assign;
  union {
    int ival; // INT_LIT
    struct {  // BIN_OP
      operator_t op;
      struct expr_ast_type *left, *right;
    };
    char *name; // VAR_REF
  };
} expr_ast_t;

typedef struct stat_ast_type {
  stat_ast_type_t type;

  union {
    expr_ast_t *expr; // RETURN_STAT, EXPR_STAT

    struct { // IF_STAT, WHILE_STAT, FOR_STAT
      expr_ast_t *cond; // IF_STAT, WHILE_STAT, FOR_STAT
      struct stat_ast_type *body; // IF_STAT, WHILE_STAT, FOR_STAT

      union {
        struct { // IF_STAT
          struct stat_ast_type *tstat, *fstat;
        };

        struct { // FOR_STAT
          expr_ast_t *init, *iter;
        };
      };
    };

    list_t stats; // BLOCK_STAT

    struct { // DECL_STAT
      datatype_t datatype;
      char *target;
      expr_ast_t *value;
    };
  };
  list_elem_t block_elem;
} stat_ast_t;

stat_ast_t *parse(token_t *);

#endif
