#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

// void my_ctor(size_t foo) {}
// void my_dtor(size_t foo) {}

int main(int argc, char const *argv[])
{
    slab_init();

    for (;;)
        if (slab_alloc(NULL, 2) == NULL)
            break;

    // slab_cache_t *dummy_cache1 = NULL;
    // slab_cache_t *dummy_cache2 = NULL;
    // slab_cache_t *dummy_cache3 = NULL;
    // slab_cache_t *dummy_cache4 = NULL;

    // if ((dummy_cache1 = slab_create_cache(2, 3)) == NULL)
    // {
    //     fprintf(stderr, "slab_cache_create returned NULL\n");
    //     exit(1);
    // }

    // LOG("breakpoint 0.1\n");

    // if ((dummy_cache2 = slab_create_cache(4, 3)) == NULL)
    // {
    //     fprintf(stderr, "slab_cache_create returned NULL\n");
    //     exit(1);
    // }

    // LOG("breakpoint 0.2\n");

    // if ((dummy_cache3 = slab_create_cache(8, 3)) == NULL)
    // {
    //     fprintf(stderr, "slab_cache_create returned NULL\n");
    //     exit(1);
    // }

    // LOG("breakpoint 0.3\n");

    // if ((dummy_cache4 = slab_create_cache(16, 3)) == NULL)
    // {
    //     fprintf(stderr, "slab_cache_create returned NULL\n");
    //     exit(1);
    // }

    // LOG("breakpoint 0.4\n");

    // print_caches();

    // LOG("breakpoint 1\n");

    // void *ptr = slab_alloc(dummy_cache1, 2);

    // for (;;)
    //     if (slab_alloc(dummy_cache1, 2) == NULL)
    //         break;

    // LOG("breakpoint 2\n");

    // slab_destroy(dummy_cache4);

    // LOG("breakpoint 3\n");

    // print_caches();

    // LOG("breakpoint 4\n");

    return 0;
}
