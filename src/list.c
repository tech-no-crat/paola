#include <assert.h>
#include "list.h"

/* Returns true if ELEM is a head, false otherwise. */
static inline bool is_head(struct list_elem *elem) {
  return elem != NULL && elem->prev == NULL && elem->next != NULL;
}

/* Returns true if ELEM is an interior element,
   false otherwise. */
static inline bool is_interior(struct list_elem *elem) {
  return elem != NULL && elem->prev != NULL && elem->next != NULL;
}

/* Returns true if ELEM is a tail, false otherwise. */
static inline bool is_tail(struct list_elem *elem) {
  return elem != NULL && elem->prev != NULL && elem->next == NULL;
}

/* Initializes LIST as an empty list. */
void list_init(struct list *list) {
  assert(list != NULL);
  list->head.prev = NULL;
  list->head.next = &list->tail;
  list->tail.prev = &list->head;
  list->tail.next = NULL;
}

/* Returns the beginning of LIST.  */
struct list_elem *list_begin(struct list *list) {
  assert(list != NULL);
  return list->head.next;
}

/* Returns the element after ELEM in its list.  If ELEM is the
 * last element in its list, returns the list tail.  Results are
 * undefined if ELEM is itself a list tail. */
struct list_elem *list_next(struct list_elem *elem) {
  assert(is_head(elem) || is_interior(elem));
  return elem->next;
}

/* Returns LIST's tail.
 *
 * list_end() is often used in iterating through a list from
 * front to back.  See the big comment at the top of list.h for
 * an example. */
struct list_elem *list_end(struct list *list) {
  assert(list != NULL);
  return &list->tail;
}

/* Returns the LIST's reverse beginning, for iterating through
 * LIST in reverse order, from back to front. */
struct list_elem *list_rbegin(struct list *list) {
  assert(list != NULL);
  return list->tail.prev;
}

/* Returns the element before ELEM in its list.  If ELEM is the
 * first element in its list, returns the list head.  Results are
 * undefined if ELEM is itself a list head. */
struct list_elem *list_prev(struct list_elem *elem) {
  assert(is_interior(elem) || is_tail(elem));
  return elem->prev;
}

/* Returns LIST's head.
 * list_rend() is often used in iterating through a list in
 * reverse order, from back to front. */
struct list_elem *list_rend(struct list *list) {
  assert(list != NULL);
  return &list->head;
}

/* Return's LIST's head.
 * list_head() can be used for an alternate style of iterating
 * through a list. */
struct list_elem *list_head (struct list *list) {
  assert(list != NULL);
  return &list->head;
}

/* Return's LIST's tail. */
struct list_elem *list_tail(struct list *list) {
  assert(list != NULL);
  return &list->tail;
}

/* Inserts ELEM just before BEFORE, which may be either an
 * interior element or a tail.  The latter case is equivalent to
 * list_push_back(). */
void list_insert(struct list_elem *before, struct list_elem *elem) {
  assert(is_interior(before) || is_tail(before));
  assert(elem != NULL);

  elem->prev = before->prev;
  elem->next = before;
  before->prev->next = elem;
  before->prev = elem;
}

/* Removes elements FIRST though LAST (exclusive) from their
 * current list, then inserts them just before BEFORE, which may
 * be either an interior element or a tail. */
void list_splice(struct list_elem *before, struct list_elem *first,
    struct list_elem *last) {
  assert(is_interior(before) || is_tail(before));
  if(first == last) {
    return;
  }
  last = list_prev(last);

  assert(is_interior(first));
  assert(is_interior(last));

  /* Cleanly remove FIRST...LAST from its current list. */
  first->prev->next = last->next;
  last->next->prev = first->prev;

  /* Splice FIRST...LAST into new list. */
  first->prev = before->prev;
  last->next = before;
  before->prev->next = first;
  before->prev = last;
}

/* Inserts ELEM at the beginning of LIST, so that it becomes the
 * front in LIST. */
void list_push_front(struct list *list, struct list_elem *elem) {
  list_insert(list_begin(list), elem);
}

/* Inserts ELEM at the end of LIST, so that it becomes the
 * back in LIST. */
void list_push_back(struct list *list, struct list_elem *elem) {
  list_insert(list_end(list), elem);
}

/* Removes ELEM from its list and returns the element that
 * followed it.  Undefined behavior if ELEM is not in a list. */
struct list_elem *list_remove(struct list_elem *elem) {
  assert(is_interior(elem));
  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;
  return elem->next;
}

/* Removes the front element from LIST and returns it.
 * Undefined behavior if LIST is empty before removal. */
struct list_elem *list_pop_front(struct list *list) {
  struct list_elem *front = list_front(list);
  list_remove(front);
  return front;
}

/* Removes the back element from LIST and returns it.
 * Undefined behavior if LIST is empty before removal. */
struct list_elem *list_pop_back(struct list *list) {
  struct list_elem *back = list_back(list);
  list_remove(back);
  return back;
}

/* Returns the front element in LIST.
 * Undefined behavior if LIST is empty. */
struct list_elem *list_front(struct list *list) {
  assert(!list_empty(list));
  return list->head.next;
}

/* Returns the back element in LIST.
 * Undefined behavior if LIST is empty. */
struct list_elem *list_back(struct list *list) {
  assert(!list_empty(list));
  return list->tail.prev;
}

/* Returns the number of elements in LIST.
 * Runs in O(n) in the number of elements. */
size_t list_size(struct list *list) {
  struct list_elem *e;
  size_t cnt = 0;

  for (e = list_begin(list); e != list_end(list); e = list_next(e)) {
    cnt++;
  }
  return cnt;
}

/* Returns true if LIST is empty, false otherwise. */
bool list_empty(struct list *list) {
  return list_begin(list) == list_end(list);
}

/* Swaps the `struct list_elem *'s that A and B point to. */
static void swap(struct list_elem **a, struct list_elem **b)  {
  struct list_elem *t = *a;
  *a = *b;
  *b = t;
}

/* Reverses the order of LIST. */
void list_reverse(struct list *list) {
  if (!list_empty(list)) {
    struct list_elem *e;

    for (e = list_begin(list); e != list_end(list); e = e->prev) {
      swap (&e->prev, &e->next);
    }
    swap (&list->head.next, &list->tail.prev);
    swap (&list->head.next->prev, &list->tail.prev->next);
  }
}
