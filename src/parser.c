#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "parser.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "errors.h"

static void init_parser(token_t *tokens);

static expr_ast_t *parse_expr(void);
static expr_ast_t *parse_subexpr(void);
static stat_ast_t *parse_stat(void);
static expr_ast_t *parse_term(void);
static expr_ast_t *parse_factor(void);
static expr_ast_t *parse_int_lit(void);
static stat_ast_t *parse_declaration(void);
static stat_ast_t *parse_block_stat(void);

static expr_ast_t *create_binop_expr(operator_t, expr_ast_t *, expr_ast_t *);
static stat_ast_t *create_return_stat(expr_ast_t *);
static expr_ast_t *create_invalid_expr(void);
static stat_ast_t *create_invalid_stat(void);
static stat_ast_t *create_block_stat(void);
static stat_ast_t *create_if_stat(expr_ast_t *, stat_ast_t *, stat_ast_t *);
static stat_ast_t *create_while_stat(expr_ast_t *cond, stat_ast_t *stat);
static stat_ast_t *create_for_stat(expr_ast_t *, expr_ast_t *, expr_ast_t *, stat_ast_t *);
static stat_ast_t *create_stat(void);
static expr_ast_t *create_expr(void);

static expr_ast_t *create_variable_ref(char *);
static expr_ast_t *create_function_call(char *);
static stat_ast_t *create_declaration(datatype_t, char *);
static stat_ast_t *create_expr_statement(expr_ast_t *);
static stat_ast_t *create_skip_statement(void);

static datatype_t match_datatype(token_t *token);
static operator_t match_binop(void);
static bool is_factor_binop(token_t *token);
static bool is_term_binop(token_t *token);
static bool is_expr_binop(token_t *token);
static bool is_type_ident(token_t *token);
static void match_token(token_type_t);

static token_t *next_token;

stat_ast_t *parse(token_t *tokens) {
  init_parser(tokens);

  stat_ast_t *program = create_block_stat();
  while (next_token->type != PROGRAM_END_TOK) {
    stat_ast_t *stat = parse_stat();
    list_push_back(&program->stats, &stat->block_elem);
  }

  return program;
}

stat_ast_t *parse_stat() {
  stat_ast_t *stat = 0;
  switch (next_token->type) {
    case RETURN_TOK: {
      match_token(RETURN_TOK);
      expr_ast_t *expr = parse_expr();
      stat = create_return_stat(expr);
      match_token(SCOL_TOK);
      break;
    } case IF_TOK: {
      match_token(IF_TOK);
      match_token(LPAREN_TOK);
      expr_ast_t *cond = parse_expr();
      match_token(RPAREN_TOK);
      stat_ast_t *tstat = parse_stat();
      stat_ast_t *fstat = 0;

      if(next_token->type == ELSE_TOK) {
        match_token(ELSE_TOK);
        fstat = parse_stat();
      }

      stat = create_if_stat(cond, tstat, fstat);
      break;
    } case WHILE_TOK: {
      match_token(WHILE_TOK);
      match_token(LPAREN_TOK);
      expr_ast_t *cond = parse_expr();
      match_token(RPAREN_TOK);
      stat_ast_t *body = parse_stat();

      stat = create_while_stat(cond, body);
      break;
    } case FOR_TOK: {
      match_token(FOR_TOK);
      match_token(LPAREN_TOK);

      expr_ast_t *init = parse_expr();
      match_token(SCOL_TOK);

      expr_ast_t *cond = parse_expr();
      match_token(SCOL_TOK);

      expr_ast_t *iter = parse_expr();
      match_token(RPAREN_TOK);

      stat_ast_t *body = parse_stat();
      
      stat = create_for_stat(init, cond, iter, body);
      break;
    } case LBRACE_TOK:
      stat = parse_block_stat();
            break;
    case IDENT_TOK:
      if (is_type_ident(next_token)) {
        stat = parse_declaration();
      } else {
        stat = create_expr_statement(parse_expr());
        match_token(SCOL_TOK);
      }
      break;
    case INT_LIT_TOK:
      stat = create_expr_statement(parse_expr());
      match_token(SCOL_TOK);
      break;
    case SCOL_TOK:
      stat = create_skip_statement();
      match_token(SCOL_TOK);
      break;
    default:
      error(&next_token->pos, "Expected start of statement, but found token %s.",
          token_t_to_str(next_token->type));
      while (next_token->type != SCOL_TOK && next_token->type != PROGRAM_END_TOK) {
        match_token(next_token->type);
      }
      match_token(SCOL_TOK);
      return create_invalid_stat(); 
  }

  return stat;
}

static expr_ast_t *parse_expr() {
  expr_ast_t *expr = parse_subexpr();

  if (is_expr_binop(next_token)) {
    operator_t op = match_binop();
    expr_ast_t *t = parse_subexpr();
    expr->assign = (op == ASSIGN);
    expr = create_binop_expr(op, expr, t);
  }

  return expr;
}

static expr_ast_t *parse_subexpr() {
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

  if (next_token->type == IDENT_TOK) {
    char *name = next_token->name;
    match_token(IDENT_TOK);
    if (next_token->type == LPAREN_TOK) {
      match_token(LPAREN_TOK);
      match_token(RPAREN_TOK);
      return create_function_call(name);
    } else {
      return create_variable_ref(name);
    }
  } else if (next_token->type == INT_LIT_TOK) {
    return parse_int_lit();
  } else {
    error(&next_token->pos, "Expected integer literal or identifier.");
    return create_invalid_expr();
  }
}

static expr_ast_t *parse_int_lit() {
  if (next_token->type != INT_LIT_TOK) {
    error(&next_token->pos, "Expected integer literal.");
    return create_invalid_expr();
  }

  expr_ast_t *expr = create_expr();
  expr->assign = false;
  expr->type = INT_LIT;
  expr->ival = next_token->ival;
  match_token(INT_LIT_TOK);

  return expr;
}

static stat_ast_t *parse_declaration() {
  datatype_t type = match_datatype(next_token);
  match_token(IDENT_TOK);
  char *target = next_token->name;
  match_token(IDENT_TOK);
  stat_ast_t *decl_stat = create_declaration(type, target);

  if (next_token->type == LPAREN_TOK) { // Function declaration
    match_token(LPAREN_TOK);
    match_token(RPAREN_TOK);
    decl_stat->is_func = true;
    if (next_token->type == LBRACE_TOK) {
      decl_stat->func_body = parse_block_stat();
    } else {
      match_token(SCOL_TOK);
    }
  } else { // Variable declaration
    decl_stat->is_func = false;
    if (next_token->type == ASSIGN_TOK) {
      match_token(ASSIGN_TOK);
      decl_stat->value = parse_expr();
    }
    match_token(SCOL_TOK);
  }

  return decl_stat;
}

static stat_ast_t *parse_block_stat() {
  match_token(LBRACE_TOK);
  stat_ast_t *stat = create_block_stat();
  while (next_token->type != RBRACE_TOK) {
    stat_ast_t *x = parse_stat();
    list_push_back(&stat->stats, &x->block_elem);
  }
  match_token(RBRACE_TOK);

  return stat;
}

static expr_ast_t *create_binop_expr(operator_t op, expr_ast_t *left,
    expr_ast_t *right) {
  expr_ast_t *expr = create_expr();
  expr->assign = false;
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
    case ASSIGN_TOK:
      match_token(ASSIGN_TOK);
      return ASSIGN;
    case EQ_TOK:
      match_token(EQ_TOK);
      return EQ;
    case GT_TOK:
      match_token(GT_TOK);
      return GT;
    case GTE_TOK:
      match_token(GTE_TOK);
      return GTE;
    case LT_TOK:
      match_token(LT_TOK);
      return LT;
    case LTE_TOK:
      match_token(LTE_TOK);
      return LTE;
    default:
      error(&next_token->pos, "Expected binary operator.");
      return INVALID_OP;
  }
}

static datatype_t match_datatype(token_t *token) {
  if (strcmp(next_token->name, "int") == 0) {
    return INT_DT;
  } else {
    error(&next_token->pos, "Expected datatype, found %s.", next_token->name);
    return INVALID_DT;
  }
}

static void match_token(token_type_t type) {
  if (type == next_token->type) {
    if (type != PROGRAM_END_TOK) {
      next_token++;
    }
  } else {
    error(&next_token->pos, "Expected token %s, but found token %s.",
        token_t_to_str(type), token_t_to_str(next_token->type));
  }
}

static bool is_factor_binop(token_t *token) {
  return token->type == STAR_TOK || token->type == FSLASH_TOK;
}

static bool is_term_binop(token_t *token) {
  return token->type == PLUS_TOK || token->type == MINUS_TOK;
}


static bool is_expr_binop(token_t *token) {
  return token->type == ASSIGN_TOK || token->type == EQ_TOK
      || token->type == GT_TOK || token->type == GTE_TOK
      || token->type == LT_TOK || token->type == LTE_TOK;
}

static expr_ast_t *create_invalid_expr(void) {
  expr_ast_t *expr = create_expr();
  expr->assign = false;
  expr->type = INVALID_EXPR;
  return expr;
}

static stat_ast_t *create_invalid_stat(void) {
  stat_ast_t *stat = create_stat();
  stat->type = INVALID_STAT;
  return stat;
}

static stat_ast_t *create_return_stat(expr_ast_t *expr) {
  stat_ast_t *stat = create_stat();
  stat->type = RETURN_STAT;
  stat->expr = expr;
  return stat;
}

static stat_ast_t *create_expr_statement(expr_ast_t *expr) {
  stat_ast_t *stat = create_stat();
  stat->type = EXPR_STAT;
  stat->expr = expr;
  return stat;
}

static stat_ast_t *create_skip_statement() {
  stat_ast_t *stat = create_stat();
  stat->type = SKIP_STAT;
  return stat;
}

static stat_ast_t *create_block_stat(void) {
  stat_ast_t *stat = create_stat();
  stat->type = BLOCK_STAT;
  list_init(&stat->stats);

  return stat;
}

static stat_ast_t *create_if_stat(expr_ast_t *cond, stat_ast_t *tstat,
    stat_ast_t *fstat) {
  stat_ast_t *stat = create_stat();
  stat->type = IF_STAT;
  stat->cond = cond;
  stat->tstat = tstat;
  stat->fstat = fstat;

  return stat;
}

static stat_ast_t *create_while_stat(expr_ast_t *cond, stat_ast_t *body) {
  stat_ast_t *stat = create_stat();
  stat->type = WHILE_STAT;
  stat->cond = cond;
  stat->body = body;

  return stat;
}

static stat_ast_t *create_for_stat(expr_ast_t *init, expr_ast_t *cond,
    expr_ast_t *iter, stat_ast_t *body) {
  stat_ast_t *stat = create_stat();
  stat->type = FOR_STAT;
  stat->init = init;
  stat->cond = cond;
  stat->iter = iter;
  stat->body = body;

  return stat;
}

static expr_ast_t *create_variable_ref(char *name) {
  expr_ast_t *expr = create_expr();
  expr->assign = false;
  expr->type = VAR_REF;
  expr->name = name;
  expr->symbol = 0;
  return expr;
}

static expr_ast_t *create_function_call(char *name) {
  expr_ast_t *expr = create_expr();
  expr->type = FUNC_CALL;
  expr->name = name;
  return expr;
}

static stat_ast_t *create_declaration(datatype_t datatype, char *target) {
  stat_ast_t *stat = create_stat();
  stat->type = DECL_STAT;
  stat->datatype = datatype;
  stat->target = target;
  stat->symbol = 0;
  return stat;
}

static stat_ast_t *create_stat() {
  stat_ast_t *stat = (stat_ast_t *) malloc(sizeof(stat_ast_t));
  stat->pos = next_token->pos;
  return stat;
}

static expr_ast_t *create_expr() {
  expr_ast_t *expr = (expr_ast_t *) malloc(sizeof(expr_ast_t));
  expr->pos = next_token->pos;
  return expr;

}

static bool is_type_ident(token_t *token) {
  return (token->type == IDENT_TOK && strcmp(token->name, "int") == 0);
}

static void init_parser(token_t *tokens) {
  next_token = tokens;
}
