#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct node {
   int data;
   int key;
   struct node *next;
} list_node_t;

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    
}

void list_append_left(int key, int data);
void list_append_right(int key, int data);

void list_pop_left(void);
void list_pop_right(void);

void list_delete(int key);

bool list_is_emtpy(void);

int list_get_length(void);

list_node_t *list_find(int key);

#endif // LIST_H