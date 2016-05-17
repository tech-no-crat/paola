#include <stdio.h>
#include "gen.h"

static void generate_statement(stat_ast_t *ast);
static void generate_expression(expr_ast_t *ast);
static void generate_binop(expr_ast_t *expr);
static void generate_int_lit(expr_ast_t *expr);
static void init_gen(FILE *file);

static FILE *out;

void generate_code(FILE *file, stat_ast_t *ast) {
  init_gen(file);

  fprintf(out, "\t.text\n\t.globl _main\n_main:\n");
  generate_statement(ast);
}

static void generate_statement(stat_ast_t *stat) {
  switch (stat->type) {
    case RETURN_STAT:
      /* Generate code that will calculate the result of the expression and
       * push it on top of the stack. */
      generate_expression(stat->expr);

      /* Now return whatever is on top of the stack. */
      fprintf(out, "\tpopq %%rax\n\tret\n");
      break;
    default:
      printf("Error: Don't know how to generate code for statement %d.\n", stat->type);
      break;
  }

  if (stat->next) {
    generate_statement(stat->next);
  }
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
      printf("Error: Don't know how to generate code for expression.\n");
      break;
  };
}

static void generate_binop(expr_ast_t *expr) {
  generate_expression(expr->left);
  generate_expression(expr->right);
  fprintf(out, "\tpopq %%rbx\n");
  fprintf(out, "\tpopq %%rax\n");

  switch (expr->op) {
    case ADD:
      fprintf(out, "\taddq %%rbx, %%rax\n");
      break;
    case SUBS:
      fprintf(out, "\tsubq %%rbx, %%rax\n");
      break;
    case DIV:
      /* When dividing rdx and rax are concatinated, so we need to zero rdx
       * first. */
      fprintf(out, "\tmov $0, %%rdx\n");
      fprintf(out, "\tidivq %%rbx, %%rax\n");
      break;
    case MUL:
      fprintf(out, "\timulq %%rbx, %%rax\n");
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
