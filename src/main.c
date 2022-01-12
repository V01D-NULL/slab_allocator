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

    LOG("Allocating all memory...\n");

    // Demo: Allocate memory from a partial slab. Take a free slab if
    // the partial slab is empty. Exits the program if no free slab is available.
    // for (;;)
    //     if (slab_alloc(dummy_cache1, NULL, size) == NULL)
    //         break;

    LOG("meooow\n");
    slab_destroy(dummy_cache1);
    LOG("wuuuf\n");
    
    void *ptr1 = slab_alloc(dummy_cache1, NULL, size);
    for (;;)
        if (slab_alloc(dummy_cache1, NULL, size) == NULL)
            break;

    LOG("\n++++++Dumping slab informations++++++\n");
    slab_traverse_cache(dummy_cache1);

    LOG("\n++++++Before alloc++++++\n");

    LOG("Dumping partial slabs...\n");
    print_slabs(dummy_cache1->partial);
    LOG("\nDumping used slabs...\n");
    print_slabs(dummy_cache1->used);
    LOG("\nDumping free slabs...\n");
    print_slabs(dummy_cache1->free);

    LOG("\n++++++After alloc / Before free++++++\n");

    LOG("Freeing first alloc ptr...\n");
    int return_code = slab_free(dummy_cache1, ptr1);
    LOG("Slab free returned: %d\n", return_code);

    LOG("\n++++++After free++++++\n");

    LOG("Dumping partial slabs...\n");
    print_slabs(dummy_cache1->partial);
    LOG("\nDumping used slabs...\n");
    print_slabs(dummy_cache1->used);
    LOG("\nDumping free slabs...\n");
    print_slabs(dummy_cache1->free);

    return 0;
}

