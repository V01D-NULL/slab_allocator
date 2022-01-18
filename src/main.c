#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void my_ctor(size_t foo) {}
void my_dtor(size_t foo) {}

int main(int argc, char const *argv[])
{
    slab_init();

    size_t size = 2;
    slab_cache_t *dummy_cache1 = NULL;
    slab_cache_t *dummy_cache2 = NULL;
    slab_cache_t *dummy_cache3 = NULL;
    slab_cache_t *dummy_cache4 = NULL;

    if ((dummy_cache1 = slab_create_cache("1", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    if ((dummy_cache2 = slab_create_cache("2", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    if ((dummy_cache3 = slab_create_cache("3", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    if ((dummy_cache4 = slab_create_cache("4", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }


    void *ptr = slab_alloc(dummy_cache1, NULL, size);

    for (;;)
        if (slab_alloc(dummy_cache1, NULL, size) == NULL)
            break;

    slab_destroy(dummy_cache4);

    print_caches();

    return 0;
}
