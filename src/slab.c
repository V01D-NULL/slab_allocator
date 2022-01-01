#include "slab.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

slab_t *create_slab(size_t size);
static inline bool is_page_aligned(int _)
{
    return (_ % 4096) == 0;
}

static inline void append(slab_t **ref, slab_t *new_node)
{
    slab_t *tail = *ref;

    if (*ref == NULL)
    {
        *ref = new_node;
        return;
    }
    
    while (tail->next != NULL) {
        tail = tail->next;
    }
    
    tail->next = new_node;
}

/* Linked list of slab caches */
static slab_cache_t *slab_caches;

void slab_traverse_cache(slab_cache_t* cache)
{
    /* Just log this cache */
    if (cache != NULL)
    {
        LOG("=== Logging info for the slab cache \"%s\" ===\n", cache->descriptor);
        LOG("=== Dumping %ld slabs ===\n", cache->active_slabs);

        for (size_t i = 0; i < cache->active_slabs; i++)
        {
            LOG("* Free slab nr:\n");
            // LOG("* #%ld Total objects: %8d\n", i, cache->free[i].num_objects);
            // LOG("* #%ld Assigned memory: %15p\n", i, cache->free[i].mem[i] == SLAB_FREE_ENTRY ? 0 : cache->free[i].mem[i]);
            // LOG("* #%ld Size: %d\n\n", i, cache->free[i].size);

            // LOG("* Used slab:\n");
            // LOG("* #%ld Total objects: %8d\n", i, cache->used[i].num_objects);
            // LOG("* #%ld Assigned memory: %15p\n", i, cache->used[i].mem[i] == SLAB_FREE_ENTRY ? 0 : cache->used[i].mem[i]);
            // LOG("* #%ld Size: %d\n\n", i, cache->used[i].size);

            // LOG("* Partial slab:\n");
            // LOG("* #%ld Total objects: %8d\n", i, cache->partial[i].num_objects);
            // LOG("* #%ld Assigned memory: %15p\n", i, cache->partial[i].mem[i] == SLAB_FREE_ENTRY ? 0 : cache->partial[i].mem[i]);
            // LOG("* #%ld Size: %d\n\n", i, cache->partial[i].size);

        }

        LOG("=== Cache statistics ===\n");
        LOG("* No. Active slabs: %5ld\n", cache->active_slabs);
        LOG("* No. Created slabs: %4ld\n", cache->slab_creates);
        LOG("* No. Destroyed slabs: %2ld\n", cache->slab_destroys);
        LOG("* No. Allocations: %6ld\n", cache->slab_allocs);
        LOG("* No. Free's: %11ld\n", cache->slab_frees);
        LOG("* Has next cache: %8s\n", cache->next == NULL ? "no" : "yes");
        LOG("* Has ctor: %14s\n", cache->constructor == NULL ? "no" : "yes");
        LOG("* Has dtor: %14s\n\n", cache->destructor == NULL ? "no" : "yes");
    }
    /* Log all slab caches */
    else
    {
        for (;;) // TODO
        {
            
        }
    }
}

void slab_init(void)
{
    slab_caches = PAGE_ALLOC(1);
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

void slab_destroy(slab_cache_t *cache)
{
    if (cache == NULL) {
        LOG("slab_destroy: Cannot destory invalid cache of type NULL!\n");
        return;
    }
}

slab_cache_t *slab_cache_create(const char *descriptor, size_t size, size_t num_slabs, ctor, dtor)
{
    // Any slab size greater than 4096 bytes must be page aligned.
    size_t pages_to_alloc = 1;
    if (size > 4096)
        if (is_page_aligned(size))
            pages_to_alloc = size / 4096;
        else
            return NULL;
    
    slab_cache_t *cache = (slab_cache_t*)malloc(sizeof(slab_cache_t));
    
    /* Statistics */
    cache->slab_creates = 0;
    cache->slab_destroys = 0;
    cache->slab_allocs = 0;
    cache->slab_frees = 0;

    /* Cache properties */
    cache->descriptor = descriptor;
    cache->next = NULL;
    cache->prev = get_previous_cache(slab_caches);
    cache->constructor = constructor;
    cache->destructor  = destructor;

    /* Append new cache to "global" system cache (slab_caches) */
    append_to_global_cache(cache);

    /* Configure slab states */
    if (num_slabs > MAX_CREATABLE_SLABS_PER_CACHE) num_slabs = MAX_CREATABLE_SLABS_PER_CACHE;
    cache->free    = (slab_state_layer_t*)malloc(sizeof(slab_state_layer_t));
    cache->used    = (slab_state_layer_t*)malloc(sizeof(slab_state_layer_t));
    cache->partial = (slab_state_layer_t*)malloc(sizeof(slab_state_layer_t));
    
    cache->free->head = (slab_t*)malloc(sizeof(slab_t));
    slab_t *free_head = cache->free->head = create_slab(size);

    for (size_t i = 0; i < num_slabs; i++, cache->slab_creates++)
        append(&cache->free->head, create_slab(size));

    cache->used->head = NULL;
    cache->used->tail = NULL;
    cache->used->next = NULL;
    cache->partial->head = NULL;
    cache->partial->tail = NULL;
    cache->partial->next = NULL;

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

    __builtin_unreachable();
}

void append_to_global_cache(slab_cache_t *cache)
{
	slab_cache_t *current = slab_caches;

	for (;;)
	{
		if (current == NULL) {
			current = cache;
            return;
        }

		current = current->next;
	}

    __builtin_unreachable();
}

slab_t *create_slab(size_t size)
{
    slab_t *slab = (slab_t*)malloc(sizeof(slab_t));
    for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    {
        slab->objects[i].is_allocated = false;
        slab->objects[i].num_objects = 0;
        slab->objects[i].size = size;
        slab->objects[i].mem = malloc(size);
    }
    return slab;
}

void *slab_cache_alloc(slab_cache_t *cache, const char *descriptor, size_t bytes)
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
    // Todo: Search `slab_caches`
    if (!cache) return NULL;



    slab_t* partial = cache->partial->head;
    if (!partial)
    {
        slab_t *free = cache->free->head;
        if (free == NULL)
            return NULL; // OOM
        
        partial = (slab_t*)malloc(sizeof(slab_t));
        memcpy(partial, free, sizeof(slab_t));
        // remove_head(&cache->free->head);
    }
}