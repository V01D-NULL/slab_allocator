#include "list.h"

void list_add(list_head_t *new, list_head_t *head)
{
    list_head_t *prev = head;
    list_head_t *next = head->next;

    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_del(list_head_t *head)
{
    list_head_t *prev = head->prev;
    list_head_t *next = head->next;

    next->prev = prev;
    prev->next = next;

    head->next = NULL;  // TODO: check this
    head->prev = NULL;  // TODO: check this
}

bool list_is_empty(list_head_t *head)
{
    return head->next == head;
}

int list_get_length(list_head_t *head)
{
    int length = 0;

    for (list_head_t *current = head->next; current != head; current = current->next)
        length++;
    
    return length;
}