#include "slab.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

slab_t *create_slab();
static inline bool is_page_aligned(int _)
{
    return (_ % 4096) == 0;
}

// linear allocation works visualized like this:
// [+++++++++++++++++++++++++]                          <- Memory in our case arena (+ means free, * means allocated)
//  ^                                                   <- Pointer to current free address in our case arena_pointer

// After allocating a few bytes, it could look like this:
// [***********++++++++++++++++]
//             ^
__attribute__((always_inline))
static inline void *linear_alloc(uint32_t *arena[], uint32_t arena_pointer[], int which_arena, size_t bytes)
{
    if (arena_pointer[which_arena] + bytes >=  LINEAR_ARENA_SIZE)
    {
        if (which_arena == LINEAR_INTERNAL_ARENA)
            BUG("Allocated more internal memory than there is available!\nIncrease value of the 'ARENA_PADDING' macro\n");

        // If we get here, we have wasted 64 kb of memory on a single cache.
        // Investegate this if it ever happens and bump up the limit
        if (which_arena == LINEAR_MAX_ARENAS)
            return NULL;
        
        which_arena += 1;
    }
    
    arena_pointer[which_arena] += bytes;
    return (void*)((uintptr_t)arena[which_arena] + arena_pointer[which_arena]);
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
        LOG("* Cache size: %ld\n", cache->cache_size);
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

    // We need these to allocate the cache, then they will
    // be copied to the cache's arena & arena_ptr members.
    uint32_t *local_list_arena[LINEAR_MAX_ARENAS];
    uint32_t local_list_arena_ptr[LINEAR_MAX_ARENAS] = {0};
    
    // Index 0 is reserved for internal allocations of nodes and alike, do NOT use this for objects
    local_list_arena[0] = PAGE_ALLOC(1);

    for (size_t i = 1; i < pages_to_alloc+1; i++)
    {
        local_list_arena[i] = PAGE_ALLOC(1);
    }
    
    // When this cache is destroyed, we search for NULL to know when to stop freeing arena memory
    if (pages_to_alloc < LINEAR_MAX_ARENAS)
        local_list_arena[pages_to_alloc + 2] = NULL;
    
    slab_cache_t *cache = (slab_cache_t*)linear_alloc(local_list_arena, local_list_arena_ptr, LINEAR_INTERNAL_ARENA, sizeof(slab_cache_t));
    
    /* Statistics */
    cache->slab_creates = 0;
    cache->slab_destroys = 0;
    cache->slab_allocs = 0;
    cache->slab_frees = 0;
    cache->cache_size = 0;

    /* Cache properties */
    cache->descriptor = descriptor;
    cache->next = NULL;
    cache->prev = get_previous_cache(slab_caches); // TODO: Set the previous node accordingly, so just traverse the slab_caches list until you hit NULL, then set the previous cache.
    cache->constructor = constructor;
    cache->destructor  = destructor;

    /* Append new cache to "global" system cache (slab_caches) */
    append_to_global_cache(cache);

    /* Configure slab states */
    if (num_slabs > MAX_CREATABLE_SLABS_PER_CACHE) num_slabs = MAX_CREATABLE_SLABS_PER_CACHE;
    cache->free    = (slab_state_layer_t*)linear_alloc(local_list_arena, local_list_arena_ptr, LINEAR_INTERNAL_ARENA, sizeof(slab_state_layer_t)*3);
    cache->used    = (slab_state_layer_t*)linear_alloc(local_list_arena, local_list_arena_ptr, LINEAR_INTERNAL_ARENA, sizeof(slab_state_layer_t)*3);
    cache->partial = (slab_state_layer_t*)linear_alloc(local_list_arena, local_list_arena_ptr, LINEAR_INTERNAL_ARENA, sizeof(slab_state_layer_t)*3);
    
    slab_t *free_head = cache->free->head;
    for (size_t i = 0; i < num_slabs; i++, cache->slab_creates++, cache->active_slabs++)
    {
        if (free_head == NULL) free_head = (slab_t*)linear_alloc(local_list_arena, local_list_arena_ptr, LINEAR_INTERNAL_ARENA, sizeof(slab_t));
        free_head = create_slab();
        free_head = free_head->next;
    }

    /* Configure this cache's internal allocation arena */
    for (size_t i = 0; i < pages_to_alloc; i++) {
        cache->list_arena[i] = local_list_arena[i];
        cache->list_arena_ptr[i] = local_list_arena_ptr[i];
    }

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

slab_t *create_slab()
{
    slab_t *slab = PAGE_ALLOC(1);
    for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    {
        slab->objects[i].is_allocated = false;
        slab->objects[i].num_objects++;
        slab->objects[i].size = 0;
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
}