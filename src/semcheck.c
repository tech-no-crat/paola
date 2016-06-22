#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "semcheck.h"
#include "list.h"
#include "symtable.h"
#include "errors.h"

static void semcheck_stat(stat_ast_t *);
static datatype_t semcheck_expr(expr_ast_t *);
static datatype_t semcheck_binop(expr_ast_t *);
static void type_error(datatype_t, datatype_t, char const *);
static void init_semcheck(void);
static bool is_const(expr_ast_t *);

void semcheck(stat_ast_t *ast) {
  init_semcheck();
  semcheck_stat(ast);
}

static void semcheck_stat(stat_ast_t *stat) {
  switch (stat->type) {
    case INVALID_STAT: {
      break;
    } case RETURN_STAT: {
      datatype_t return_type = semcheck_expr(stat->expr);
      if (return_type != INT_DT) {
        type_error(INT_DT, return_type, "return expression");
      }
      break;
    } case IF_STAT: {
      datatype_t condition_type = semcheck_expr(stat->cond);
      if (condition_type != INT_DT) {
        type_error(INT_DT, condition_type, "if condition expression");
      }

      semcheck_stat(stat->tstat);
      if (stat->fstat) {
        semcheck_stat(stat->fstat);
      }
      break;
    } case WHILE_STAT: {
      datatype_t condition_type = semcheck_expr(stat->cond);
      if (condition_type != INT_DT) {
        type_error(INT_DT, condition_type, "while condition expression");
      }

      semcheck_stat(stat->body);
      break;
    } case FOR_STAT: {
      datatype_t condition_type = semcheck_expr(stat->cond);
      if (condition_type != INT_DT) {
        type_error(INT_DT, condition_type, "for condition expression");
      }

      semcheck_expr(stat->init);
      semcheck_expr(stat->iter);
      semcheck_stat(stat->body);
      break;
    } case BLOCK_STAT: {
      for (list_elem_t *e = list_begin(&(stat->stats)); e != list_end(&(stat->stats));
          e = list_next(e)) {
        stat_ast_t *s = list_entry(e, stat_ast_t, block_elem);
        semcheck_stat(s);
      }
      break;
    } case DECL_STAT: {
      symbol_t *symbol = symtable_find(stat->target);
      if (symbol) {
        error(0, "%s is already defined in this scope.", stat->target);
        break;
      }

      if (stat->value) {
        datatype_t value_type = semcheck_expr(stat->value);
        if (value_type != stat->datatype) {
          type_error(stat->datatype, value_type, "variable initialization");
          // Intentionally do not break here, continue to add the variable to
          // the symbol table.
        }
      }

      symtable_insert(create_symtable_entry(stat->target, stat->datatype));
      break;
    } case EXPR_STAT: {
      semcheck_expr(stat->expr);
      break;
    } default: {
      error(0, "Don't know how to semantically check statement %s.",
          stat_t_to_str(stat->type));
    }
  }
}

static datatype_t semcheck_expr(expr_ast_t *expr) {
  switch (expr->type) {
    case INVALID_EXPR: {
      return INVALID_DT;
    } case INT_LIT: {
      return INT_DT;
    } case BIN_OP: {
      return semcheck_binop(expr);
    } case VAR_REF: {
      symbol_t *symbol = symtable_find(expr->name);
      if (!symbol) {
        error(0, "Variable %s not defined in current scope.", expr->name);
        return INVALID_DT;
      }
      return symbol->datatype;
    } default: {
      error(0, "Don't know how to semantically check expression %s.",
          expr_t_to_str(expr->type));
      return INVALID_DT;
    }
  }
}

static datatype_t semcheck_binop(expr_ast_t *expr) {
  switch (expr->op) {
    case INVALID_OP: {
      return INVALID_DT;
    } case ADD:
    case SUBS:
    case MUL:
    case DIV: {
      datatype_t left_type = semcheck_expr(expr->left);
      datatype_t right_type = semcheck_expr(expr->right);

      if (left_type != INT_DT) {
        type_error(INT_DT, left_type, "left binop operand");
        return INVALID_DT;
      }
      if (right_type != INT_DT) {
        type_error(INT_DT, right_type, "right binop operand");
        return INVALID_DT;
      }

      return INT_DT;
    } case ASSIGN: {
      if (is_const(expr->left)) {
        error(0, "Left assignment operand is constant.");
        return INVALID_DT;
      }

      datatype_t left_type = semcheck_expr(expr->left);
      datatype_t right_type = semcheck_expr(expr->right);
      if (left_type != right_type) {
        type_error(left_type, right_type, "assignment");
      }

      return left_type;
    } case EQ:
      case GT:
      case GTE:
      case LT:
      case LTE: {
      datatype_t left_type = semcheck_expr(expr->left);
      datatype_t right_type = semcheck_expr(expr->right);
      if (left_type != right_type) {
        type_error(left_type, right_type, "comparison operator");
      }

      return INT_DT;
    } default: {
      error(0, "Don't know how to semantically check operator %s.\n",
          oper_to_str(expr->op));
      return INVALID_DT;
    }
  }
}

static bool is_const(expr_ast_t *expr) {
  return expr->type != VAR_REF;
}

static void type_error(datatype_t expected, datatype_t actual, char const *desc) {
  if (expected == INVALID_DT || actual == INVALID_DT) {
    // Either the expected or actual datatype is invalid, so this was caused by an error
    // we've already reported. Do not throw another error.
    return;
  }
  error(0, "Expected type %s, but found type %s in %s.",
      datatype_to_str(expected), datatype_to_str(actual), desc);
}

void init_semcheck() {
  symtable_init();
}
