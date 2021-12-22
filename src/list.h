#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>

// typedef struct node {
//    int data;
//    int key;
//    list_node_t *next;
// } list_node_t;

typedef struct list_head {
   list_head_t *next;
   list_head_t *prev;
} list_head_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) list_head_t name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(list_head_t *list)
{
   list->next = list;
   list->prev = list;
}

void list_add(list_head_t *new, list_head_t *head);

// void list_append_left(list_head_t *new, list_head_t *head);
// void list_append_right(list_head_t *new, list_head_t *head);

// void list_pop_left(list_head_t *head);
// void list_pop_right(list_head_t *head);

// void list_delete(int key);

bool list_is_emtpy(list_head_t *head);

int list_get_length(list_head_t *head);

// list_node_t *list_find(int key);

#endif // LIST_H