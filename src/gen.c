#pragma GCC diagnostic ignored "-Wwrite-strings"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "gen.h"
#include "symtable.h"
#include "errors.h"

/* A Register Set, or RegSet, is represented with a 16-bit number where
 * the i-th bit is on iff the i-th register is available (for i in
 * the range [0, register_count]). */
typedef int16_t regset_t;

static const uint8_t register_count = 10;
static const regset_t initial_regset = (1 << 10) - 1; // All registers
static const char *register_names[10] = {
  // We don't use rax and rdx because it's used in division (TODO)
  "rbx",
  "rcx",
  "r8",
  "r9",
  "r10",
  "r11",
  "r12",
  "r13",
  "r14",
  "r15"
};

static FILE *out;
static int next_label;
static int next_stack_offset;

/* A struct representing x86 command arguments. Instances of this struct can be
 * constructed with methods below. */
typedef struct {
  enum arg_type_t {
    LIT_ARG,
    REG_ARG,
    MEM_ARG
  } type;

  union {
    int32_t lit;
    struct {
      const char *reg;
      int32_t offset;
    };
  };
} arg_t;

static void init_gen(FILE *);

/* Recursive code generators. */
static void generate_statement(stat_ast_t *, regset_t);
static void generate_expression(expr_ast_t *, regset_t);
static void generate_var_ref(expr_ast_t *, regset_t);
static void generate_binop(expr_ast_t *, regset_t);
static void generate_int_lit(expr_ast_t *, regset_t);
static void generate_func_call(expr_ast_t *, regset_t);

/* Gets the number of the next unused label. The full name of the labels shall
 * be lX, where X is the numbers this function returns. */
static int get_label(void);

/* Register set manipulation and access functions. */
static regset_t consume_reg(regset_t);
static uint8_t next_reg_number(regset_t);
static const char *next_reg_name(regset_t);

/* Constructors for x86 command argument struct instances. They return copies
 * of the constructed structs, because these structs have very short
 * lifespans. */
static arg_t arg_lit(int32_t);
static arg_t arg_reg(const char *);
static arg_t arg_mem(const char *, uint32_t);

/* Helper functions for generating instructions. */
static void gen_arg(arg_t);
static void two_arg_command(const char *, arg_t, arg_t);
static void one_arg_command(const char *, arg_t);
static void save_registers(void);
static void load_registers(void);

/* Functions that generate x86 commands. */
static void mov(arg_t, arg_t);
static void push(arg_t);
static void pop(arg_t);
static void cjmp(operator_t op, int jlabel); // Conditional jump
static void add(arg_t, arg_t);
static void idiv(arg_t, arg_t);
static void imul(arg_t, arg_t);
static void sub(arg_t, arg_t);
static void cmp(arg_t, arg_t);
static void jne(int32_t);
static void je(int32_t);
static void jmp(int32_t);
static void label(int32_t);
static void func_label(char *);
static void call(char *);
static void ret(void);

void generate_code(FILE *file, stat_ast_t *ast) {
  init_gen(file);

  fprintf(out, "\t.text\n\t.globl _main\n");
  generate_statement(ast, initial_regset);
  fprintf(out, "main:\n\tcall _main\n\tret\n");
}

static void generate_statement(stat_ast_t *stat, regset_t regset) {
  switch (stat->type) {
    case RETURN_STAT: {
      /* Generate code that will calculate the result of the expression to the
       * first free register of regset. */
      generate_expression(stat->expr, regset);

      /* Copy the result to rax and return. */
      const char *result_reg = next_reg_name(regset);
      if (strcmp(result_reg, "rax") != 0) {
        mov(
          arg_reg(result_reg),
          arg_reg("rax")
        );
      }

      ret();
      break;
    } case IF_STAT: {
      int flabel = get_label();
      const char *cond_reg = next_reg_name(regset);
      generate_expression(stat->expr, regset);

      cmp(
        arg_lit(0),
        arg_reg(cond_reg)
      );
      je(flabel);

      generate_statement(stat->tstat, regset);

      label(flabel);
      if (stat->fstat) {
        generate_statement(stat->fstat, regset);
      }
      break;
    } case WHILE_STAT: {
      int cond_label = get_label(), body_label = get_label();
      jmp(cond_label);

      label(body_label);
      generate_statement(stat->body, regset);

      label(cond_label);
      const char *cond_reg = next_reg_name(regset);
      generate_expression(stat->expr, regset);

      cmp(
        arg_lit(0),
        arg_reg(cond_reg)
      );
      jne(body_label);
      break;
    } case FOR_STAT: {
      int cond_label = get_label(), body_label = get_label();

      generate_expression(stat->init, regset);
      jmp(cond_label);

      label(body_label);
      generate_statement(stat->body, regset);
      generate_expression(stat->iter, regset);

      label(cond_label);
      const char *cond_reg = next_reg_name(regset);
      generate_expression(stat->cond, regset);
      cmp(arg_lit(0), arg_reg(cond_reg));
      jne(body_label);
      break;
    } case BLOCK_STAT: {
      for (list_elem_t *e = list_begin(&(stat->stats)); e != list_end(&(stat->stats));
          e = list_next(e)) {
        stat_ast_t *s = list_entry(e, stat_ast_t, block_elem);
        generate_statement(s, regset);
      }
      break;
    } case DECL_STAT: {
      symbol_t *symbol = stat->symbol;
      assert(symbol != 0);

      if (stat->is_func) {
        func_label(stat->target);
        generate_statement(stat->func_body, regset);
      } else {
        int datatype_size = 8; //TODO: Should depend on sizeof(type)
        symbol->stack_offset = next_stack_offset;
        next_stack_offset += datatype_size;
      }
      break;
    } case EXPR_STAT: {
      generate_expression(stat->expr, regset);
      break;
    } case SKIP_STAT: {
      break;
    } default:
      error(0, "Don't know how to generate code for statement %s.\n",
          stat_t_to_str(stat->type));
      break;
  }
}

static void generate_expression(expr_ast_t *expr, regset_t regset) {
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
    case FUNC_CALL:
      generate_func_call(expr, regset);
      break;
    default:
      error(0, "Don't know how to generate code for expression %s.",
          expr_t_to_str(expr->type));
      break;
  };
}

static void generate_var_ref(expr_ast_t *expr, regset_t regset) {
  symbol_t *symbol = expr->symbol;
  assert(symbol != 0);
  
  if (expr->assign) { // Leave the address at the destination register
    const char *dst = next_reg_name(regset);
    mov(
      arg_lit(-symbol->stack_offset),
      arg_reg(dst)
    );

    add(
      arg_reg("rsp"),
      arg_reg(dst)
    );
  } else { // Leave the value at the destination register
    mov(
      arg_mem("rsp", -symbol->stack_offset),
      arg_reg(next_reg_name(regset))
    );
  }
}

static void generate_func_call(expr_ast_t *expr, regset_t regset) {
  save_registers();
  call(expr->name);
  load_registers();

  mov(
    arg_reg("rax"),
    arg_reg(next_reg_name(regset))
  );
}

static void save_registers() {
  for (int i = 0; i < register_count; i++) {
    push(arg_reg(register_names[i]));
  }
}

static void load_registers() {
  for (int i = register_count - 1; i >= 0; i--) {
    pop(arg_reg(register_names[i]));
  }
}

static void generate_binop(expr_ast_t *expr, regset_t regset) {
  const char *left_reg = next_reg_name(regset);
  generate_expression(expr->left, regset);
  regset = consume_reg(regset);

  const char *right_reg = next_reg_name(regset);
  generate_expression(expr->right, regset);

  switch (expr->op) {
    case ADD:
      add(arg_reg(right_reg), arg_reg(left_reg));
      break;
    case SUBS:
      sub(arg_reg(right_reg), arg_reg(left_reg));
      break;
    case DIV:
      /* When dividing rdx and rax are concatinated, so we need to zero rdx
       * first. */
      mov(
        arg_lit(0),
        arg_reg("rdx")
      );
      idiv(
        arg_reg(right_reg),
        arg_reg(left_reg)
      );
      break;
    case MUL:
      imul(
        arg_reg(right_reg),
        arg_reg(left_reg)
      );
      break;
    case ASSIGN: {
      mov(
        arg_reg(right_reg),
        arg_mem(left_reg, 0)
      );
      break;
    } case EQ:
      case GT:
      case GTE:
      case LT:
      case LTE: {
      cmp(
        arg_reg(right_reg),
        arg_reg(left_reg)
      );
      
      mov(
        arg_lit(1),
        arg_reg(left_reg)
      );

      int t_label = get_label();

      cjmp(
        expr->op,
        t_label
      );

      mov(
        arg_lit(0),
        arg_reg(left_reg)
      );

      label(t_label);

      break;
    } default:
      error(0, "Don't know how to translate code for binary operator %s.",
          oper_to_str(expr->op));
      break;
  }
}

static void generate_int_lit(expr_ast_t *expr, regset_t regset) {
  mov(
    arg_lit(expr->ival),
    arg_reg(next_reg_name(regset))
  );
}

static int get_label() {
  return next_label++;
}

static void init_gen(FILE *file) {
  out = file;
  next_label = 0;
  next_stack_offset = 8;
}

static regset_t consume_reg(regset_t regset) {
  return regset & (~(1 << (next_reg_number(regset) - 1)));
}

static const char *next_reg_name(regset_t regset) {
  return register_names[next_reg_number(regset) - 1];
}

static uint8_t next_reg_number(regset_t regset) {
  assert(__builtin_popcount(regset) >= 1);
  return (uint8_t) __builtin_ffsl(regset);
}

static arg_t arg_lit(int32_t lit) {
  arg_t arg;
  arg.type = LIT_ARG;
  arg.lit = lit;
  return arg;
}

static arg_t arg_reg(const char *reg) {
  arg_t arg;
  arg.type = REG_ARG;
  arg.reg = reg;
  return arg;
}

static arg_t arg_mem(const char *reg, uint32_t offset) {
  arg_t arg;
  arg.type = MEM_ARG;
  arg.reg = reg;
  arg.offset = offset;
  return arg;
}

static void gen_arg(arg_t arg) {
  switch(arg.type) {
    case LIT_ARG:
      fprintf(out, "$%d", arg.lit);
      break;
    case REG_ARG:
      fprintf(out, "%%%s", arg.reg); 
      break;
    case MEM_ARG:
      if (arg.offset) {
        fprintf(out, "%d", arg.offset);
      }
      fprintf(out, "(%%%s)", arg.reg); 
      break;
    default:
      error(0, "Don't know how to generate code for argument type %d.\n", arg.type);
  }
}

static void two_arg_command(const char *command, arg_t src, arg_t dst) {
  fprintf(out, "\t%s ", command);
  gen_arg(src);
  fprintf(out, ", ");
  gen_arg(dst);
  fprintf(out, "\n");
}

static void one_arg_command(const char *command, arg_t src) {
  fprintf(out, "\t%s ", command);
  gen_arg(src);
  fprintf(out, "\n");
}

static void mov(arg_t src, arg_t dst) {
  two_arg_command("mov", src, dst);
}

static void push(arg_t arg) {
  one_arg_command("push", arg);
}

static void pop(arg_t arg) {
  one_arg_command("pop", arg);
}

static void cjmp(operator_t op, int jlabel) { // Conditional jump
  const char *command;

  switch (op) {
    case EQ:
      command = "je";
      break;
    case GT:
      command = "jg";
      break;
    case GTE:
      command = "jge";
      break;
    case LT:
      command = "jl";
      break;
    case LTE:
      command = "jle";
      break;
    default:
      assert(false);
      return;
  }

  fprintf(out, "\t%s l%d\n", command, jlabel);
}

static void add(arg_t src, arg_t dst) {
  two_arg_command("add", src, dst);
}

static void sub(arg_t src, arg_t dst) {
  two_arg_command("sub", src, dst);
}

static void imul(arg_t src, arg_t dst) {
  two_arg_command("imul", src, dst);
}

static void idiv(arg_t src, arg_t dst) {
  mov(
    dst,
    arg_reg("rax")
  );

  mov(
    arg_lit(0),
    arg_reg("rdx")
  );

  fprintf(out, "\tidivq ");
  gen_arg(src);
  fprintf(out, "\n");

  mov(
    arg_reg("rax"),
    dst
  );
}

static void cmp(arg_t src, arg_t dst) {
  two_arg_command("cmp", src, dst);
}

static void jne(int32_t label_id) {
  fprintf(out, "\tjne l%d\n", label_id);
}

static void je(int32_t label_id) {
  fprintf(out, "\tje l%d\n", label_id);
}

static void jmp(int32_t label_id) {
  fprintf(out, "\tjmp l%d\n", label_id);
}

static void ret(void) {
  fprintf(out, "\tret\n");
}

static void label(int32_t label_id) {
  fprintf(out, "l%d:\n", label_id);
}

static void func_label(char *name) {
  fprintf(out, "_%s:\n", name);
}

static void call(char *name) {
  fprintf(out, "\tcall _%s\n", name);
}
