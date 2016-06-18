#include "errors.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static void create_message(position_t *, const char *, char *, va_list *);
static void add_message(char *);

typedef struct {
  char *str;
  list_elem_t elem;
} message_t;

static list_t messages;
static int errors, warnings;

const char *token_t_to_str(token_type_t type) {
  switch (type) {
    case INVALID_TOK:
      return "INVALID";
    case LPAREN_TOK:
      return "LPAREN";
    case RPAREN_TOK:
      return "RPAREN";
    case PLUS_TOK:
      return "PLUS";
    case STAR_TOK:
      return "STAR";
    case FSLASH_TOK:
      return "FSLASH";
    case MINUS_TOK:
      return "MINUS";
    case RETURN_TOK:
      return "RETURN";
    case SCOL_TOK:
      return "SCOL";
    case IDENT_TOK:
      return "IDENT";
    case INT_LIT_TOK:
      return "INT_LIT";
    case LBRACE_TOK:
      return "LBRACE";
    case RBRACE_TOK:
      return "RBRACE";
    case IF_TOK:
      return "IF";
    case WHILE_TOK:
      return "WHILE";
    case FOR_TOK:
      return "FOR";
    case ELSE_TOK:
      return "ELSE";
    case EQ_TOK:
      return "EQ";
    default:
      return "UNKNOWN";
  }
}
const char *oper_to_str(operator_t op) {
  switch (op) {
    case ADD:
      return "+";
    case SUBS:
      return "-";
    case MUL:
      return "*";
    case DIV:
      return "/";
    case ASSIGN:
      return "=";
    default:
      return "?";
  }
}

const char *datatype_to_str(datatype_t datatype) {
  switch (datatype) {
    case INVALID_DT:
      return "INVALID_TYPE";
    case INT_DT:
      return "INT_TYPE";
    default:
      return "UNKNOWN_TYPE";
  }
}

const char *stat_t_to_str(stat_ast_type_t stat_type) {
  switch (stat_type) {
    case INVALID_STAT:
      return "InvalidStatement";
    case RETURN_STAT:
      return "ReturnStatement";
    case IF_STAT:
      return "IfStatement";
    case WHILE_STAT:
      return "WhileStatement";
    case FOR_STAT:
      return "ForStatement";
    case BLOCK_STAT:
      return "BlockStatement";
    case EXPR_STAT:
      return "ExpressionStatement";
    case DECL_STAT:
      return "DeclarationStatement";
    default:
      return "UnknownStatement";
  }
}

const char *expr_t_to_str(expr_ast_type_t expr_type) {
  switch (expr_type) {
    case INVALID_EXPR:
      return "InvalidExpression";
    case INT_LIT:
      return "IntLiteralExpression";
    case BIN_OP:
      return "BinaryOperandExpression";
    case VAR_REF:
      return "VariableReferenceExpression";
    default:
      return "UnknownExpression";
  }
}

static void create_message(position_t *pos, const char *beginning, char *format, va_list *args) {
  char str[1024];
  int line = 0, column = 0, len = 0;

  if (pos) {
    line = pos->line;
    column = pos->character;
  }

  sprintf(str, "%s line %d:%d: ", beginning, line, column);

  len = strlen(str);
  vsprintf(str + len, format, *args);

  len = strlen(str);
  sprintf(str + len, "\n");

  add_message(str);
}

void error(position_t *pos, char *format, ...) {
  errors++;

  va_list args;
  va_start(args, format);
  create_message(pos, "Error", format, &args);
  va_end(args);
}

void warning(position_t *pos, char *format, ...) {
  warnings++;

  va_list args;
  va_start(args, format);
  create_message(pos, "Warning", format, &args);
  va_end(args);
}

static void add_message(char *str) {
  int len = strlen(str);
  message_t *message = (message_t *) malloc(sizeof(message_t));
  message->str = (char *) malloc(sizeof(char) * (len + 1));
  strcpy(message->str, str);

  list_push_back(&messages, &message->elem);
}

void errors_init(void) {
  warnings = 0;
  errors = 0;
  list_init(&messages);
}

int error_count(void) {
  return errors;
}

int warning_count(void) {
  return warnings;
}

void print_messages(FILE *out) {
  for (list_elem_t *e = list_begin(&messages); e != list_end(&messages);
      e = list_next(e)) {
    message_t *msg = list_entry(e, message_t, elem);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
    fprintf(out, msg->str);
#pragma GCC diagnostic pop
  }
}
