#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "parser.h"
#include "list.h"

typedef struct {
  list_t symbols;
  list_elem_t symtable_elem;
  char *name;
} scope_t;

symbol_t *create_symtable_entry(char *, datatype_t);

void symtable_init(void);
symbol_t *symtable_find(char *);
void symtable_insert(symbol_t *);
void symtable_open_scope(char *);
void symtable_close_scope(void);

#endif
