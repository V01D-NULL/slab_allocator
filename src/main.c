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

    if ((dummy_cache1 = slab_create_cache("first dummy_cache", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    if ((dummy_cache2 = slab_create_cache("second dummy_cache", size, 3, my_ctor, my_dtor)) == NULL)
    {
        fprintf(stderr, "slab_cache_create returned NULL\n");
        exit(1);
    }

    // Demo: Allocate memory from a partial slab. Take a free slab if
    // the partial slab is empty. Exits the program if no free slab is available.
	
	void *ptr = slab_alloc(dummy_cache1, NULL, size);

    for (;;)
        if (slab_alloc(dummy_cache1, NULL, size) == NULL)
            break;
	
	LOGV("starting slab_traverse_cache\n");
	slab_traverse_cache(dummy_cache1);
	LOGV("slab_traverse_cache done\n\n");

	LOGV("starting print_slabs (free)\n");
	print_slabs(dummy_cache1->free);
	LOGV("print_slabs (free) done\n\n");

	LOGV("starting print_slabs (used)\n");
	print_slabs(dummy_cache1->used);
	LOGV("print_slabs (used) done\n\n");

	LOGV("starting print_slabs (partial)\n");
	print_slabs(dummy_cache1->partial);
	LOGV("print_slabs (partial) done\n\n");

	LOGV("starting print_caches\n");
	print_caches();
	LOGV("print_caches done\n\n");

	slab_free(dummy_cache1, ptr);

	LOGV("starting slab_traverse_cache\n");
	slab_traverse_cache(dummy_cache1);
	LOGV("slab_traverse_cache done\n\n");

	LOGV("starting print_slabs (free)\n");
	print_slabs(dummy_cache1->free);
	LOGV("print_slabs (free) done\n\n");

	LOGV("starting print_slabs (used)\n");
	print_slabs(dummy_cache1->used);
	LOGV("print_slabs (used) done\n\n");

	LOGV("starting print_slabs (partial)\n");
	print_slabs(dummy_cache1->partial);
	LOGV("print_slabs (partial) done\n\n");

	LOGV("starting print_caches\n");
	print_caches();
	LOGV("print_caches done\n\n");


    return 0;
}
