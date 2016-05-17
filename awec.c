#include <stdio.h>
#include "gen.h"
#include "lexer.h"
#include "parser.h"

void print_tokens(token_t *token) {
  printf("Tokens:\n");

  for(;token->type != PROGRAM_END_TOK; token++) {
    switch (token->type) {
      case INVALID_TOK:
        printf("[INVALID] ");
      case LPAREN_TOK:
        printf("[LPAREN] ");
        break;
      case RPAREN_TOK:
        printf("[RPAREN] ");
        break;
      case PLUS_TOK:
        printf("[PLUS] ");
        break;
      case STAR_TOK:
        printf("[STAR] ");
        break;
      case FSLASH_TOK:
        printf("[FSLASH] ");
        break;
      case MINUS_TOK:
        printf("[MINUS] ");
        break;
      case RETURN_TOK:
        printf("[RETURN] ");
        break;
      case SCOL_TOK:
        printf("[SCOL] ");
        break;
      case IDENT_TOK:
        printf("[IDENT %s] ", token->name);
        break;
      case INT_LIT_TOK:
        printf("[INT_LIT %d] ", token->ival);
        break;
      default:
        printf("[???] ");
        break;
    }
  }

  printf("\n");
}

char binop_char(operator_t op) {
  switch (op) {
    case ADD:
      return '+';
    case SUBS:
      return '-';
    case MUL:
      return '*';
    case DIV:
      return '/';
    default:
      return '?';
  }
}

void print_expr_ast(expr_ast_t *ast) {
  switch (ast->type) {
    case INVALID_EXPR:
      printf("InvalidExpression");
      break;
    case INT_LIT:
      printf("IntLit(%d)", ast->ival);
      break;
    case BIN_OP:
      printf("BinOp(%c, ", binop_char(ast->op));
      print_expr_ast(ast->left);
      printf(", ");
      print_expr_ast(ast->right);
      printf(")");
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
      printf(")\n");
      break;
    default:
      printf("UKNOWN_STATEMENT\n");
      break;
  }
}


int main () {
  FILE *fin = fopen("test.c", "r");

  token_t *tokens = tokenize(fin);
  print_tokens(tokens);
  
  stat_ast_t *ast = parse(tokens);
  printf("Parsed\n");
  print_stat_ast(ast);

  FILE *fout = fopen("out.s", "w");
  generate_code(fout, ast);

  return 0;
}
