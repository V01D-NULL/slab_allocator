#include <stdio.h>
#include "list.h"

list_head_t my_head;

typedef struct my_node {
    int data;
    int key;
    list_head_t list;
} my_node_t;

void list_traverse(list_head_t *head)
{
    list_head_t *current = head;
    
    for (;;)
    {
        if (current->next == head)
        {
            
            return;
        }

        current = current->next;
    }
}

int main(int argc, char const *argv[])
{
    printf("+++ list.h testing +++\n");

    my_node_t node1;
    my_node_t node2;

    INIT_LIST_HEAD(&my_head);

    node1.data = 1337;
    node1.key = 1;

    node2.data = 3141;
    node2.key = 2;

    printf("* Is empty: %14s\n", list_is_empty(&my_head) ? "yes" : "no");
    printf("* Length: %d\n", list_get_length(&my_head));

    list_append_right(&node1.list, &my_head);

    printf("* Length: %d\n", list_get_length(&my_head));

    list_add(&node2.list, &my_head);

    printf("* Length: %d\n", list_get_length(&my_head));
    printf("* Is empty: %s\n", list_is_empty(&my_head) ? "yes" : "no");
}