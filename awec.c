#include <stdio.h>
#include "gen.h"
#include "lexer.h"
#include "parser.h"

void print_tokens(token_t *token) {
  printf("Tokens:\n");

  for(;token->type != PROGRAM_END_TOK; token++) {
    if (token->type == INT_LIT_TOK) {
      printf("[INT %d] ", token->ival); 
    }

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

void print_ast(expr_ast_t *ast) {
  switch (ast->type) {
    case INVALID:
      printf("invalid");
      break;
    case INT_LIT:
      printf("IntLit(%d)", ast->ival);
      break;
    case BIN_OP:
      printf("BinOp(%c, ", binop_char(ast->op));
      print_ast(ast->left);
      printf(", ");
      print_ast(ast->right);
      printf(")");
      break;
  }
}

int main () {
  FILE *fin = fopen("test.c", "r");

  token_t *tokens = tokenize(fin);
  print_tokens(tokens);
  
  expr_ast_t *ast = parse(tokens);
  printf("Parsed\n");
  print_ast(ast);

  FILE *fout = fopen("out.s", "w");
  generate_code(fout, ast);

  return 0;
}
