#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>
#include "gen.h"
#include "lexer.h"
#include "parser.h"
#include "errors.h"

typedef struct {
   const char *input_file, *output_file;
   bool print_tokens, print_ast;
} options_t;

void print_tokens(token_t *);
void print_expr_ast(expr_ast_t *);
void print_stat_ast(stat_ast_t *);

options_t parse_options(int, char **);

#endif
