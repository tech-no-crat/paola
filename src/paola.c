#include <stdio.h>
#include <stdlib.h>
#include "gen.h"
#include "lexer.h"
#include "parser.h"
#include "semcheck.h"
#include "errors.h"
#include "utils.h"

enum { // Different exit codes denote different error type
  OTHER_ERR = 1,
  LEXER_ERR,
  PARSE_ERR,
  SEM_ERR,
  GEN_ERR
};

void if_errors_exit(int code) {
  if (error_count() > 0) {
    print_messages(stdout);
    exit(code);
  }
}

int main (int argc, char **argv) {
  options_t options = parse_options(argc, argv);
  if (options.input_file == 0) {
    printf("No input file specified.\n");
    exit(1);
  }

  FILE *fin = fopen(options.input_file, "r");
  errors_init();

  /* Lexer */
  token_t *tokens = tokenize(fin);
  if_errors_exit(LEXER_ERR);

  if (options.print_tokens) {
    print_tokens(tokens);
  }

  /* Parser */
  stat_ast_t *ast = parse(tokens);
  if_errors_exit(PARSE_ERR);

  if (options.print_ast) {
    print_stat_ast(ast);
  }

  /* Semantic checker */
  semcheck(ast);
  if_errors_exit(SEM_ERR);

  /* Code generation */
  FILE *fout = fopen(options.output_file, "w");
  generate_code(fout, ast);
  fclose(fout);
  if_errors_exit(GEN_ERR);

  print_messages(stdout);
  return 0;
}
