#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void my_ctor(size_t foo) {}
void my_dtor(size_t foo) {}

int main(int argc, char const *argv[])
{
    slab_init();

    slab_cache_t *dummy_cache1 = slab_cache_create("first dummy_cache", 4096 * 2, 3, my_ctor, my_dtor);
    if (dummy_cache1 == NULL) {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }
    slab_traverse_cache(dummy_cache1);
    slab_cache_alloc(dummy_cache1, NULL, 10);
    slab_cache_alloc(dummy_cache1, NULL, 12);

    return 0;
}