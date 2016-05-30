#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "gen.h"
#include "symtable.h"
#include "errors.h"

static void generate_statement(stat_ast_t *, uint16_t);
static void generate_expression(expr_ast_t *, uint16_t);
static void generate_var_ref(expr_ast_t *expr, uint16_t regset);
static void generate_binop(expr_ast_t *, uint16_t);
static void generate_int_lit(expr_ast_t *, uint16_t);
static void init_gen(FILE *file);
static int get_label(void);
static uint16_t consume_reg(uint16_t);
static uint8_t next_reg_number(uint16_t);
static const char *next_reg_name(uint16_t);

static FILE *out;
static int next_label;
static int next_stack_offset;

static const uint8_t register_count = 11;
static const uint16_t initial_regset = (1 << 11) - 1; // All registers
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

  fprintf(out, "\t.text\n\t.globl main\nmain:\n");
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
    } case BLOCK_STAT: {
      for (list_elem_t *e = list_begin(&(stat->stats)); e != list_end(&(stat->stats));
          e = list_next(e)) {
        stat_ast_t *s = list_entry(e, stat_ast_t, block_elem);
        generate_statement(s, regset);
      }
      break;
    } case DECL_STAT: {
      symbol_t *symbol = symtable_find(stat->target);
      assert(symbol != 0);
      assert(symbol->datatype == INT_DT);

      int datatype_size = 8; //TODO: Should depend on sizeof(type)
      symbol->stack_offset = next_stack_offset;
      next_stack_offset += datatype_size;
      break;
    } case EXPR_STAT: {
      generate_expression(stat->expr, regset);
      break;
    } default:
      error(0, "Don't know how to generate code for statement %s.\n",
          stat_t_to_str(stat->type));
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
    case VAR_REF:
      generate_var_ref(expr, regset);
      break;
    default:
      error(0, "Don't know how to generate code for expression %s.",
          expr_t_to_str(expr->type));
      break;
  };
}

static void generate_var_ref(expr_ast_t *expr, uint16_t regset) {
  symbol_t *symbol = symtable_find(expr->name);
  assert(symbol != 0);
  assert(symbol->datatype == INT_DT);
  
  if (expr->assign) { // Leave the address at the destination register
    const char *dst = next_reg_name(regset);
    fprintf(out, "\tmovq $-%d, %%%s\n", symbol->stack_offset, dst); // - stack offset -> dst
    fprintf(out, "\taddq %%rsp, %%%s\n", dst); // dst = dst + stack pointer
  } else { // Leave the value at the destination register
    const char *dst = next_reg_name(regset);
    
    fprintf(out, "\tmovq -%d(%%rsp), %%%s\n", symbol->stack_offset, dst);
  }
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
    case ASSIGN: {
      fprintf(out, "\tmovq %%%s, (%%%s)\n", right_reg, left_reg);
      break;
    } default:
      error(0, "Don't know how to translate code for binary operator %s.",
          oper_to_str(expr->op));
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
  next_stack_offset = 8;
}

static uint16_t consume_reg(uint16_t regset) {
  return regset & (~(1 << (next_reg_number(regset) - 1)));
}

static const char *next_reg_name(uint16_t regset) {
  return register_names[next_reg_number(regset) - 1];
}

static uint8_t next_reg_number(uint16_t regset) {
  assert(__builtin_popcount(regset) >= 1);
  return (uint8_t) __builtin_ffsl(regset);
}

