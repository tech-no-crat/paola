#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "parser.h"
#include "list.h"

typedef struct {
  char *name;
  datatype_t datatype;
  list_elem_t elem;
  uint32_t stack_offset;
  //TODO: Also store info about whether it's a function, if it's constant etc.
} symbol_t;

symbol_t *create_symtable_entry(char *, datatype_t);

void symtable_init(void);
symbol_t *symtable_find(char *);
void symtable_insert(symbol_t *);

#endif
