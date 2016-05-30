#include "utils.h"
#include <string.h>

options_t parse_options(int argc, char **argv) {
  options_t opt = {0, 0, false, false};

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') opt.input_file = argv[i];
    else if (strcmp(argv[i], "--print-tokens") == 0) opt.print_tokens = true;
    else if (strcmp(argv[i], "--print-ast") == 0) opt.print_ast = true;
    else if (strcmp(argv[i], "-o") == 0) {
      i++;
      if (i < argc) {
        opt.output_file = argv[i];
      }
    }
  }

  if (opt.output_file == 0) { // Default
    opt.output_file = "out.s";
  }

  return opt;
}

void print_tokens(token_t *token) {
  printf("Input tokens:\n");

  for(;token->type != PROGRAM_END_TOK; token++) {
    if (token->type == INT_LIT_TOK) {
      printf("[%s %d]", token_t_to_str(token->type), token->ival);
    } else if(token->type == IDENT_TOK) {
      printf("[%s %s]", token_t_to_str(token->type), token->name);
    } else {
      printf("[%s]", token_t_to_str(token->type));
    }
  }

  printf("\n");
}

void print_expr_ast(expr_ast_t *ast) {
  if (!ast) {
    printf("null expr\n");
    return;
  }
  switch (ast->type) {
    case INVALID_EXPR:
      printf("InvalidExpression");
      break;
    case INT_LIT:
      printf("IntLit(%d)", ast->ival);
      break;
    case BIN_OP:
      printf("BinOp(%s, ", oper_to_str(ast->op));
      print_expr_ast(ast->left);
      printf(", ");
      print_expr_ast(ast->right);
      printf(")");
      break;
    case VAR_REF:
      printf("VarRef(%s)", ast->name);
      break;
    default:
      printf("UNKNOWN_EXPRESSION");
      break;
  }
}

void print_stat_ast(stat_ast_t *ast) {
  switch (ast->type) {
    case INVALID_STAT:
      printf("InvalidStatement");
      break;
    case RETURN_STAT:
      printf("Return(");
      print_expr_ast(ast->expr);
      printf(")");
      break;
    case IF_STAT:
      printf("If(");
      print_expr_ast(ast->cond);
      printf(", ");
      print_stat_ast(ast->tstat);
      printf(", ");
      if (ast->fstat) {
        print_stat_ast(ast->fstat);
      } else {
        printf("Skip");
      }
      printf(")");
      break;
    case BLOCK_STAT:
      printf("Block[");
      for (list_elem_t *e = list_begin(&(ast->stats)); e != list_end(&(ast->stats));
          e = list_next(e)) {
        stat_ast_t *stat = list_entry(e, stat_ast_t, block_elem);
        print_stat_ast(stat);
        printf(", ");
      }
      printf("Skip]");
      break;
    case EXPR_STAT:
      printf("Expression(");
      print_expr_ast(ast->expr);
      printf(")");
      break;
    case DECL_STAT:
      printf("Declaration(%s, %s, ", datatype_to_str(ast->datatype), ast->target);
      if (ast->value) {
        print_expr_ast(ast->value);
      } else {
        printf("Skip");
      }
      printf(")");
      break;
    default:
      printf("UNKNOWN_STATEMENT");
      break;
  }
}
