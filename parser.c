#include "parser.h"
#include <stdbool.h>
#include <stdlib.h>

static void init_parser(token_t *tokens);
static expr_ast_t *parse_expr(void);
static stat_ast_t *parse_stat(void);
static expr_ast_t *parse_term(void);
static expr_ast_t *parse_factor(void);
static expr_ast_t *parse_int_lit(void);
static expr_ast_t *create_binop_expr(operator_t, expr_ast_t *, expr_ast_t *);
static stat_ast_t *create_return_stat(expr_ast_t *);
static expr_ast_t *create_invalid_expr(void);
static stat_ast_t *create_invalid_stat(void);
static operator_t match_binop(void);
static bool is_factor_binop(token_t *token);
static bool is_term_binop(token_t *token);
static void match_token(token_type_t);

static token_t *next_token;

stat_ast_t *parse(token_t *tokens) {
  init_parser(tokens);
  return parse_stat();
}

stat_ast_t *parse_stat() {
  if (next_token->type != RETURN_TOK) {
    printf("Error: Expected return statement\n");
    return create_invalid_stat(); 
  }
  match_token(RETURN_TOK);
  expr_ast_t *expr = parse_expr();
  match_token(SCOL_TOK);

  stat_ast_t *stat = create_return_stat(expr);

  if (next_token->type != PROGRAM_END_TOK) {
    stat->next = parse_stat();
  }

  return stat;
}

static expr_ast_t *parse_expr() {
  expr_ast_t *expr = parse_term();

  while (is_term_binop(next_token)) {
    operator_t op = match_binop();

    expr_ast_t *t = parse_term();
    expr = create_binop_expr(op, expr, t);
  }

  return expr;
}

static expr_ast_t *parse_term() {
  expr_ast_t *expr = parse_factor();

  while (is_factor_binop(next_token)) {
    operator_t op = match_binop();

    expr_ast_t *t = parse_term();
    expr = create_binop_expr(op, expr, t);
  }

  return expr;
}

static expr_ast_t *parse_factor() {
  if (next_token->type == LPAREN_TOK) {
    match_token(LPAREN_TOK);
    expr_ast_t *expr = parse_expr();
    match_token(RPAREN_TOK);
    return expr;
  }

  return parse_int_lit();
}

static expr_ast_t *parse_int_lit() {
  if (next_token->type != INT_LIT_TOK) {
    printf ("Error: Expected integer literal.\n");
    return create_invalid_expr();
  }

  expr_ast_t *expr = (expr_ast_t *) malloc(sizeof(expr_ast_t));
  expr->type = INT_LIT;
  expr->ival = next_token->ival;
  match_token(INT_LIT_TOK);

  return expr;
}

static expr_ast_t *create_binop_expr(operator_t op, expr_ast_t *left,
    expr_ast_t *right) {
  expr_ast_t *expr = (expr_ast_t *) malloc(sizeof(expr_ast_t));
  expr->type = BIN_OP;
  expr->op = op;
  expr->left = left;
  expr->right = right;

  return expr;
}

static operator_t match_binop(void) {
  switch (next_token->type) {
    case PLUS_TOK:
      match_token(PLUS_TOK);
      return ADD;
    case MINUS_TOK:
      match_token(MINUS_TOK);
      return SUBS;
    case STAR_TOK:
      match_token(STAR_TOK);
      return MUL;
    case FSLASH_TOK:
      match_token(FSLASH_TOK);
      return DIV;
    default:
      printf("Error: expected binary operator.\n");
      return INVALID_OP;
  }
}

static void match_token(token_type_t type) {
  if (type == next_token->type) {
    if (type != PROGRAM_END_TOK) {
      next_token++;
    }
  } else {
    printf("Error: expected some other token.\n");
  }
}

static bool is_factor_binop(token_t *token) {
  return token->type == STAR_TOK || token->type == FSLASH_TOK;
}

static bool is_term_binop(token_t *token) {
  return token->type == PLUS_TOK || token->type == MINUS_TOK;
}

static expr_ast_t *create_invalid_expr(void) {
  expr_ast_t *expr = (expr_ast_t *) malloc(sizeof(expr_ast_t));
  expr->type = INVALID_EXPR;
  return expr;
}

static stat_ast_t *create_invalid_stat(void) {
  stat_ast_t *stat = (stat_ast_t *) malloc(sizeof(stat_ast_t));
  stat->type = INVALID_STAT;
  stat->next = 0;
  return stat;
}

static stat_ast_t *create_return_stat(expr_ast_t *expr) {
  stat_ast_t *stat = (stat_ast_t *) malloc(sizeof(stat_ast_t));
  stat->type = RETURN_STAT;
  stat->expr = expr;
  stat->next = 0;
  return stat;
}

static void init_parser(token_t *tokens) {
  next_token = tokens;
}
