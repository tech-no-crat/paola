#include "symtable.h"
#include "list.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static symbol_t *symtable_find_in_scope(scope_t *, char *);
static void empty_scope(scope_t *);
static scope_t *get_current_scope();

// The symtable is a list of scopes, and each scope is a list of symbols.
// TODO: should be a list of hashtables instead of a list of lists
static list_t symtable; 

symbol_t *create_symtable_entry(char *name, datatype_t datatype) {
  symbol_t *entry = (symbol_t *) malloc(sizeof(symbol_t));
  entry->name = name;
  entry->datatype = datatype;
  return entry;
}

void symtable_init(void) {
  list_init(&symtable);
}

void symtable_open_scope(char *name) {
  scope_t *scope = (scope_t *) malloc(sizeof(scope_t));
  scope->name = name;
  list_init(&scope->symbols);
  list_push_back(&symtable, &scope->symtable_elem);
}

void symtable_close_scope() {
  list_elem_t *e = list_pop_back(&symtable); 
  scope_t *scope = list_entry(e, scope_t, symtable_elem);
  empty_scope(scope);
  free(scope);
}

symbol_t *symtable_find(char *needle) {
  // Search each scope, starting from the inner-most one. Return the first result,
  // or 0 if the symbol was not found in any scopes.
  for (list_elem_t *e = list_begin(&symtable); e != list_end(&symtable);
    e = list_next(e)) {
    scope_t *scope = list_entry(e, scope_t, symtable_elem);
    symbol_t *result = symtable_find_in_scope(scope, needle);

    if (result) {
      return result;
    }
  }

  return 0;
}

void symtable_insert(symbol_t *symbol) {
  scope_t *current_scope = get_current_scope();

  // The symbol must not exist in the current scope
  assert(symtable_find_in_scope(current_scope, symbol->name) == 0);
  list_push_back(&current_scope->symbols, &symbol->scope_elem);
}

// Search a single scope for a symbol
static symbol_t *symtable_find_in_scope(scope_t *scope, char *needle) { // TODO: Sloooow, O(N)
  for (list_elem_t *e = list_begin(&scope->symbols); e != list_end(&scope->symbols);
    e = list_next(e)) {
    symbol_t *symbol = list_entry(e, symbol_t, scope_elem);

    if (strcmp(symbol->name, needle) == 0) {
      return symbol;
    }
  }

  return 0;
}

// De-allocates the symbols in a scope
static void empty_scope(scope_t *scope) {
  for (list_elem_t *e = list_begin(&scope->symbols); e != list_end(&scope->symbols);
    e = list_next(e)) {
    symbol_t *symbol = list_entry(e, symbol_t, scope_elem);
    free(symbol);
  }
}

static scope_t *get_current_scope() {
  list_elem_t *e = list_back(&symtable); 
  scope_t *scope = list_entry(e, scope_t, symtable_elem);
  return scope;
}
