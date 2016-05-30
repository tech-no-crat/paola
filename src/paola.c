#include <stdio.h>
#include <stdlib.h>
#include "gen.h"
#include "lexer.h"
#include "parser.h"
#include "semcheck.h"
#include "errors.h"
#include "utils.h"

void if_errors_exit() {
  if (error_count() > 0) {
    print_messages(stdout);
    exit(-1);
  }
}

int main (int argc, char **argv) {
  FILE *fin = fopen(argv[1], "r");
  errors_init();

  token_t *tokens = tokenize(fin);
  print_tokens(tokens);

  if_errors_exit();

  stat_ast_t *ast = parse(tokens);

  if_errors_exit();

  print_stat_ast(ast);
  printf("\n");

  semcheck(ast);

  if_errors_exit();

  FILE *fout = fopen("out.s", "w");
  generate_code(fout, ast);

  if_errors_exit();

  print_messages(stdout); // Maybe we have warnings

  return 0;
}
