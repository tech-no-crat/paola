#ifndef ERRORS_H
#define ERRORS_
#include "lexer.h"
#include "parser.h"

const char *token_t_to_str(token_type_t);
const char *oper_to_str(operator_t);
const char *stat_t_to_str(stat_ast_type_t);
const char *expr_t_to_str(expr_ast_type_t);
const char *datatype_to_str(datatype_t datatype);

void error(position_t *, char *, ...);
void warning(position_t *, char *, ...);
void errors_init(void);

int error_count(void);
int warning_count(void);

void print_messages(FILE *out);

#endif
