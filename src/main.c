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

	slab_cache_t *cool_cache = slab_create_cache("cool cache", size, 3, NULL, NULL);

    // Demo: Allocate memory from a partial slab. Take a free slab if
    // the partial slab is empty. Exits the program if no free slab is available.
    // for (;;)
    //     if (slab_alloc(fs, NULL, size) == NULL)
    //         break;

    // FIXME: print_caches doesn't print 'dummy_cache1', something about slab_cache_create breaks or removes a head node (?)
	// FIXME: UPDATE on this: all the caches exist but they are somehow linked wrong
	// FIXME: probably get_previous_cache or print_caches or append_to_global_cache is wrong
	// FIXME: interestingly, print_caches only prints the last two caches in cache_list
    // print_caches();
    // slab_destroy(dummy_cache1);
    // slab_destroy(fs);
    // print_caches();

    
    // slab_alloc(fs, NULL, size);
	

	// LOGV("starting slab_traverse_cache\n");
	// slab_traverse_cache(dummy_cache1);
	// LOGV("slab_traverse_cache done\n");

	LOGV("starting print_slabs (free)\n");
	print_slabs(dummy_cache1->free);
	LOGV("print_slabs (free) done\n");

	LOGV("starting print_slabs (used)\n");
	print_slabs(dummy_cache1->used);
	LOGV("print_slabs (used) done\n");

	LOGV("starting print_slabs (partial)\n");
	print_slabs(dummy_cache1->partial);
	LOGV("print_slabs (partial) done\n");

	LOGV("prev logging\n");
	
	LOG("%p\n", dummy_cache1->prev);
	LOG("%s\n", fs->prev->descriptor);
	LOG("%s\n", never_gonna_give_you_up->prev->descriptor);
	LOG("%s\n", cool_cache->prev->descriptor);
	// -> get_previous_cache seems to work

	LOGV("prev logging done\n");

	LOGV("starting print_caches\n");
	print_caches();
	LOGV("print_caches done\n");


    return 0;
}
