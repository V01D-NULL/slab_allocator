#include "list.h"

list_head_t my_head;

typedef struct my_node {
    int data;
    int key;
    list_head_t list;
} my_node_t;

int main(int argc, char const *argv[])
{
    LOG("+++ list.h testing +++\n");

    my_node_t node1;
    my_node_t node2;

    INIT_LIST_HEAD(&my_head);


    node1.data = 1337;
    node1.key = 1;

    node2.data = 3141;
    node2.key = 2;

    list_append_right(&node1.list, &my_head);
    list_append_right(&node2.list, &my_head);
}