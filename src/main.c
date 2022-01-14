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

    if ((dummy_cache1 = slab_create_cache("first dummy_cache", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    slab_cache_t *fs = slab_create_cache("fs", size, 3, NULL, NULL);
    if (!fs)
    {
        LOG("cache creation failed\n");
        exit(1);
    }
    
    slab_cache_t *never_gonna_give_you_up = slab_create_cache("get rickrolled :D", size, 3, NULL, NULL);
    if (!never_gonna_give_you_up)
    {
        LOG("cache creation failed\n");
        exit(1);
    }

    // Demo: Allocate memory from a partial slab. Take a free slab if
    // the partial slab is empty. Exits the program if no free slab is available.
    // for (;;)
    //     if (slab_alloc(fs, NULL, size) == NULL)
    //         break;

    // FIXME: print_caches doesn't print 'dummy_cache1', something about slab_cache_create breaks or removes a head node (?)
    print_caches();
    // slab_destroy(dummy_cache1);
    // slab_destroy(fs);
    // print_caches();

    
    // slab_alloc(fs, NULL, size);

    return 0;
}
