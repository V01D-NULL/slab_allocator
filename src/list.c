#include "list.h"

void list_add(list_head_t *new, list_head_t *head)
{
    // TODO: validate list

    list_head_t *new = new;
    list_head_t *prev = head;
    list_head_t *next = head->next;

    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

bool list_is_empty(list_head_t *head)
{
    return head->next == head;
}

int list_get_length(list_head_t *head)
{
    int length = 0; // TODO: should this be 0 or 1

    for (list_head_t *current = head->next; current != head; current = current->next)
        length++;
    
    return length;
}