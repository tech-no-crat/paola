#include "symtable.h"
#include "list.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static list_t symtable; //TODO: should be a list of hashtables

symbol_t *create_symtable_entry(char *name, datatype_t datatype) {
  symbol_t *entry = (symbol_t *) malloc(sizeof(symbol_t));
  entry->name = name;
  entry->datatype = datatype;
  return entry;
}

void symtable_init(void) {
  list_init(&symtable);
}

symbol_t *symtable_find(char *needle) { // Slooow - O(N)
  for (list_elem_t *e = list_begin(&symtable); e != list_end(&symtable);
    e = list_next(e)) {
    symbol_t *symbol = list_entry(e, symbol_t, elem);

    if (strcmp(symbol->name, needle) == 0) {
      return symbol;
    }
  }

  return 0;
}

void symtable_insert(symbol_t *symbol) {
  assert(symtable_find(symbol->name) == 0); // Sloooow - O(N)
  list_push_back(&symtable, &symbol->elem);
}

