#ifndef LIST_H
#define LIST_H

/* Doubly linked list. This is a modified version of the linked
 * list implementation of the PintOS educational operating
 * system. The modifications I made are mostly simplifications
 * and the removal of features that are not needed here, as well
 * code style changes.
 *
 * This implementation of a doubly linked list does not require
 * use of dynamically allocated memory.  Instead, each structure
 * that is a potential list element must embed a struct list_elem
 * member.  All of the list functions operate on these `struct
 * list_elem's.  The list_entry macro allows conversion from a
 * struct list_elem back to a structure object that contains it. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* List element. */
typedef struct list_elem {
  struct list_elem *prev;     /* Previous list element. */
  struct list_elem *next;     /* Next list element. */
} list_elem_t;

/* List. */
typedef struct list 
{
  struct list_elem head;      /* List head. */
  struct list_elem tail;      /* List tail. */
} list_t;

#define list_entry(LIST_ELEM, STRUCT, MEMBER)           \
        ((STRUCT *) ((uint8_t *) &(LIST_ELEM)->next     \
                     - offsetof (STRUCT, MEMBER.next)))

#define LIST_INITIALIZER(NAME) { { NULL, &(NAME).tail }, \
                                 { &(NAME).head, NULL } }

void list_init (struct list *);

/* List traversal. */
struct list_elem *list_begin (struct list *);
struct list_elem *list_next (struct list_elem *);
struct list_elem *list_end (struct list *);

struct list_elem *list_rbegin (struct list *);
struct list_elem *list_prev (struct list_elem *);
struct list_elem *list_rend (struct list *);

struct list_elem *list_head (struct list *);
struct list_elem *list_tail (struct list *);

/* List insertion. */
void list_insert (struct list_elem *, struct list_elem *);
void list_splice (struct list_elem *before,
                  struct list_elem *first, struct list_elem *last);
void list_push_front (struct list *, struct list_elem *);
void list_push_back (struct list *, struct list_elem *);

/* List removal. */
struct list_elem *list_remove (struct list_elem *);
struct list_elem *list_pop_front (struct list *);
struct list_elem *list_pop_back (struct list *);

/* List elements. */
struct list_elem *list_front (struct list *);
struct list_elem *list_back (struct list *);

/* List properties. */
size_t list_size (struct list *);
bool list_empty (struct list *);

/* Miscellaneous. */
void list_reverse (struct list *);

#endif
