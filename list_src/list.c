#include "list.h"
#include <stdio.h>

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

// append new node to the last node of the linked list
// [head] -> [next] -> [next] -> [tail]
// becomes
// [head] -> [next] -> [next] -> [next aka previous tail] -> [tail aka new node]
void list_append_right(list_head_t *new, list_head_t *head)
{
    // TODO: validate list

    list_head_t *current = head;

    for (;;)
    {
        if (current->next == head)
        {
            list_head_t *new = new;
            list_head_t *prev = current;
            list_head_t *next = current->next;

            next->prev = new;
            new = malloc(sizeof(list_head_t));
            new->next = next;
            new->prev = prev;
            prev->next = new;

            return;
        }

        current = current->next;
    }
}

// append new node in front of the head of the linked list
// [head] -> [next] -> [next] -> [tail]
// becomes
// [head aka new node] -> [next aka previous head] -> [next] -> [next] -> [tail]
void list_append_left(list_head_t *new, list_head_t *head)
{
    new->next = head;
    new->prev = new;
    head->prev = new;
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