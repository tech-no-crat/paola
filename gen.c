#include <stdio.h>
#include "gen.h"

static void generate_expression(expr_ast_t *ast);
static void generate_binop(expr_ast_t *expr);
static void generate_int_lit(expr_ast_t *expr);
static void init_gen(FILE *file);

static FILE *out;

void generate_code(FILE *file, expr_ast_t *ast) {
  init_gen(file);

  fprintf(out, "\t.text\n\t.globl _main\n_main:\n");
  generate_expression(ast);
  fprintf(out, "\tpopq %%rax\n\tret"); // Return whatever is on top of the stack
}

static void generate_expression(expr_ast_t *expr) {
  switch (expr->type) {
    case BIN_OP:
      generate_binop(expr);
      break;
    case INT_LIT:
      generate_int_lit(expr);
      break;
    default:
      printf("Error: Don't know how to generate code.\n");
      break;
  };
}

static void generate_binop(expr_ast_t *expr) {
  generate_expression(expr->left);
  generate_expression(expr->right);
  fprintf(out, "\tpopq %%rax\n");
  fprintf(out, "\tpopq %%rbx\n");

  switch (expr->op) {
    case PLUS:
      fprintf(out, "\taddq %%rbx, %%rax\n");
      break;
    default:
      printf("Error: Don't know how to translate code for unknown \
          binary operator\n");
      break;
  }

  fprintf(out, "\tpushq %%rax\n");
}

static void generate_int_lit(expr_ast_t *expr) {
  fprintf(out, "\tpushq $%d\n", expr->ival);
}

static void init_gen(FILE *file) {
  out = file;
}
