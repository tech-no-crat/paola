#ifndef UTILS_H
#define UTILS_H
#include "gen.h"
#include "lexer.h"
#include "parser.h"
#include "errors.h"

void print_tokens(token_t *);
void print_expr_ast(expr_ast_t *);
void print_stat_ast(stat_ast_t *);

#endif
