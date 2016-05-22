#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "gen.h"

static void generate_statement(stat_ast_t *, uint16_t);
static void generate_expression(expr_ast_t *, uint16_t);
static void generate_binop(expr_ast_t *, uint16_t);
static void generate_int_lit(expr_ast_t *, uint16_t);
static void init_gen(FILE *file);
static int get_label(void);
static uint16_t consume_reg(uint16_t);
static uint8_t next_reg_number(uint16_t);
static const char *next_reg_name(uint16_t);

static FILE *out;
static int next_label;

static const uint8_t register_count = 11;
static const uint16_t initial_regset = (1 << register_count) - 1; // All registers
static const char *register_names[11] = {
  "rax",
  "rbx",
  "rcx",
  // We don't use rdx because it's sometimes used in division (TODO)
  "r8",
  "r9",
  "r10",
  "r11",
  "r12",
  "r13",
  "r14",
  "r15"
};

void generate_code(FILE *file, stat_ast_t *ast) {
  init_gen(file);

  fprintf(out, "\t.text\n\t.globl _main\n_main:\n");
  generate_statement(ast, initial_regset);
}

static void generate_statement(stat_ast_t *stat, uint16_t regset) {
  switch (stat->type) {
    case RETURN_STAT: {
      /* Generate code that will calculate the result of the expression to the
       * first free register of regset. */
      generate_expression(stat->expr, regset);

      /* Copy the result to rax and return. */
      const char *result_reg = next_reg_name(regset);
      if (strcmp(result_reg, "rax") != 0) {
        fprintf(out, "\tmov %%%s %%rax\n", result_reg);
      }

      fprintf(out, "\tret\n");
      break;
    } case IF_STAT: {
      int label = get_label();
      const char *cond_reg = next_reg_name(regset);
      generate_expression(stat->expr, regset);
      fprintf(out, "\tcmpq $0, %%%s\n\tje l%d\n", cond_reg, label);

      generate_statement(stat->tstat, regset);
      fprintf(out, "l%d:\n", label);

      if (stat->fstat) {
        generate_statement(stat->fstat, regset);
      }
      break;
    } case BLOCK_STAT:
      for (list_elem_t *e = list_begin(&(stat->stats)); e != list_end(&(stat->stats));
          e = list_next(e)) {
        stat_ast_t *s = list_entry(e, stat_ast_t, block_elem);
        generate_statement(s, regset);
      }
      break;
    default:
      printf("Error: Don't know how to generate code for statement %d.\n", stat->type);
      break;
  }
}

static void generate_expression(expr_ast_t *expr, uint16_t regset) {
  switch (expr->type) {
    case BIN_OP:
      generate_binop(expr, regset);
      break;
    case INT_LIT:
      generate_int_lit(expr, regset);
      break;
    default:
      printf("Error: Don't know how to generate code for expression.\n");
      break;
  };
}

static void generate_binop(expr_ast_t *expr, uint16_t regset) {
  const char *left_reg = next_reg_name(regset);
  generate_expression(expr->left, regset);
  regset = consume_reg(regset);

  const char *right_reg = next_reg_name(regset);
  generate_expression(expr->right, regset);

  switch (expr->op) {
    case ADD:
      fprintf(out, "\taddq %%%s, %%%s\n", right_reg, left_reg);
      break;
    case SUBS:
      fprintf(out, "\tsubq %%%s, %%%s\n", right_reg, left_reg);
      break;
    case DIV:
      /* When dividing rdx and rax are concatinated, so we need to zero rdx
       * first. */
      fprintf(out, "\tmov $0, %%rdx\n");
      fprintf(out, "\tidivq %%%s, %%%s\n", right_reg, left_reg);
      break;
    case MUL:
      fprintf(out, "\timulq %%%s, %%%s\n", right_reg, left_reg);
      break;
    default:
      printf("Error: Don't know how to translate code for unknown \
          binary operator\n");
      break;
  }
}

static void generate_int_lit(expr_ast_t *expr, uint16_t regset) {
  const char *reg = next_reg_name(regset);
  fprintf(out, "\tmovq $%d, %%%s\n", expr->ival, reg);
}

static int get_label() {
  return next_label++;
}

static void init_gen(FILE *file) {
  out = file;
  next_label = 0;
}

static uint16_t consume_reg(uint16_t regset) {
  return regset & (~(1 << (next_reg_number(regset) - 1)));
}

static const char *next_reg_name(uint16_t regset) {
  return register_names[next_reg_number(regset) - 1];
}

static uint8_t next_reg_number(uint16_t regset) {
  assert(__builtin_popcount(regset) >= 1);
  return (uint8_t) ffsl(regset);
}

