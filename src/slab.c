#include "slab.h"
#include <stdlib.h>
#include <string.h>

/* Linked list of slab caches */
static slab_cache_t *slab_caches;

void slab_traverse_cache(slab_cache_t* cache)
{
    /* Just log this cache */
    if (cache != NULL)
    {
        LOG("=== Logging info for the slab cache \"%s\" ===\n", cache->descriptor);
        LOG("* Free slab objects: %8d\n", cache->free.num_objects);
        LOG("* Used slab objects: %8d\n", cache->used.num_objects);
        LOG("* Partial slab objects: %5d\n\n", cache->partial.num_objects);

        LOG("=== Cache statistics ===\n");
        LOG("* Cache size: %11ld\n", cache->cache_size);
        LOG("* No. Created slabs: %4ld\n", cache->slab_creates);
        LOG("* No. Destroyed slabs: %2ld\n", cache->slab_destroys);
        LOG("* No. Allocations: %6ld\n", cache->slab_allocs);
        LOG("* No. Free's: %11ld\n", cache->slab_frees);
        LOG("* Has previous cache: %4s\n", cache->prev == NULL ? "no" : "yes");
        LOG("* Has next cache: %8s\n", cache->next == NULL ? "no" : "yes");
        LOG("* Has ctor: %14s\n\n", cache->constructor == NULL ? "no" : "yes");
    }
    /* Log all slab caches */
    else
    {
    }
}

void slab_init(void)
{
    slab_caches = PAGE_ALLOC();
}

/* linux kernel

#define MAX_NUMNODES 5   // 5 from a userland example

#define NUM_INIT_LISTS (2 * MAX_NUMNODES)

void __init kmem_cache_init(void)
{
    for (i = 0; i < NUM_INIT_LISTS; i++)
            kmem_cache_node_init(&init_kmem_cache_node[i]);
    
    /* Bootstrap is tricky, because several objects are allocated
	 * from caches that do not exist yet:
	 * 1) initialize the kmem_cache cache: it contains the struct
	 *    kmem_cache structures of all caches, except kmem_cache itself:
	 *    kmem_cache is statically allocated.
	 *    Initially an __init data area is used for the head array and the
	 *    kmem_cache_node structures, it's replaced with a kmalloc allocated
	 *    array at the end of the bootstrap.
	 * 2) Create the first kmalloc cache.
	 *    The struct kmem_cache for the new cache is allocated normally.
	 *    An __init data area is used for the head array.
	 * 3) Create the remaining kmalloc caches, with minimally sized
	 *    head arrays.
	 * 4) Replace the __init data head arrays for kmem_cache and the first
	 *    kmalloc cache with kmalloc allocated arrays.
	 * 5) Replace the __init data for kmem_cache_node for kmem_cache and
	 *    the other cache's with kmalloc allocated memory.
	 * 6) Resize the head arrays of the kmalloc caches to their final sizes.
	 *
}
*/

void slab_destroy(void)
{
    // blah
}

slab_cache_t *slab_cache_create(const char *descriptor, size_t size, void (*constructor)(size_t))
{
    slab_cache_t *cache = (slab_cache_t*)PAGE_ALLOC();
    
    /* Statistics */
    cache->slab_creates = 0;
    cache->slab_destroys = 0;
    cache->slab_allocs = 0;
    cache->slab_frees = 0;
    cache->cache_size = size;

    /* Cache properties */
    cache->descriptor = descriptor;
    cache->next = NULL;
    cache->prev = get_previous_cache(slab_caches); // Todo: Set the previous node accordingly, so just traverse the slab_caches list until you hit NULL, then set the previous cache.
    cache->constructor = constructor;

    /* Append new cache to "global" system cache (slab_caches) */
    slab_caches->next = cache;

    /* Configure slab states */
    cache->free.size = 0;
    cache->free.mem = NULL;
    
    cache->used.size = 0;   // No used memory yet.
    cache->used.mem = NULL; // No memory to store

    cache->partial.size = 0;   // No used memory yet.
    cache->partial.mem = NULL; // No memory to store
    
    return cache;
}

slab_cache_t *get_previous_cache(slab_cache_t *cache)
{
	slab_cache_t *current = cache;

	for (;;)
	{
		if (current->next == NULL)
			return current;

		current = current->next;
	}

    return NULL;    // TODO: check if it even can get called
}

void *slab_cache_alloc(slab_cache_t *cache, const char *descriptor)
{
    /*
        To allocate an object:
        
        if (there's an object in the cache)
        {
            take it (no construction required);
        }
        else
        {
            allocate memory;
            construct the object;
        }
    */
    // allocate object from the cache and return it to the caller
	
	if (cache == NULL)
	{
		cache = find_in_linked_list(slab_caches, descriptor);
	}

	void *mem = find_free_slab(cache);

	organize_slab_states(cache);

	return mem;
}

slab_cache_t *find_in_linked_list(slab_cache_t *cache, const char *descriptor)
{
	slab_cache_t *current = cache;

	while (current != NULL)
	{
		if (current->descriptor == descriptor)
			return current;

		current = current->next;
	}

    return NULL;    // TODO: check if it even can get called
}

void *find_free_slab(slab_cache_t *cache)
{
	//
}

void organize_slab_states(slab_cache_t *cache)
{
    //
}

void slab_cache_free(void)
{
    // free object and return it to the cache
}
