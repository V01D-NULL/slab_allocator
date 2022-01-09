#include "slab.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Linked list of slab caches */
static slab_cache_t *slab_caches;

/* Core functions */
void slab_init(void)
{

	/*  linux kernel

	    #define MAX_NUMNODES 5   // 5 from a userland example

	    #define NUM_INIT_LISTS (2 * MAX_NUMNODES)

	    void __init kmem_cache_init(void)
	    {
	    for (i = 0; i < NUM_INIT_LISTS; i++)
	        kmem_cache_node_init(&init_kmem_cache_node[i]);

	    /* Bootstrap is tricky, because several objects are allocated
	    from caches that do not exist yet:
	    1) initialize the kmem_cache cache: it contains the struct
	     kmem_cache structures of all caches, except kmem_cache itself:
	     kmem_cache is statically allocated.
	     Initially an __init data area is used for the head array and the
	     kmem_cache_node structures, it's replaced with a kmalloc allocated
	     array at the end of the bootstrap.
	    2) Create the first kmalloc cache.
	     The struct kmem_cache for the new cache is allocated normally.
	     An __init data area is used for the head array.
	    3) Create the remaining kmalloc caches, with minimally sized
	     head arrays.
	    4) Replace the __init data head arrays for kmem_cache and the first
	     kmalloc cache with kmalloc allocated arrays.
	    5) Replace the __init data for kmem_cache_node for kmem_cache and
	     the other cache's with kmalloc allocated memory.
	    6) Resize the head arrays of the kmalloc caches to their final sizes.

	    }
	*/

	slab_caches = PAGE_ALLOC(1);
}

void slab_destroy(slab_cache_t *cache)
{
	if (cache == NULL)
	{
		LOG("slab_destroy: Cannot destory invalid cache of type NULL!\n");
		return;
	}

	// TODO:
	// - (increment active_slabs)
	// - increment slab_destroys
}

slab_cache_t *slab_create_cache(const char *descriptor, size_t size, size_t num_slabs, ctor, dtor)
{
	if (!is_power_of_two(size) || (size > 4096 && !is_page_aligned(size)))
	{
		LOG("slab_cache_create failed because %ld is not a power of two or not page aligned\n", size);
		return NULL;
	}

	slab_cache_t *cache = (slab_cache_t *)malloc(sizeof(slab_cache_t));

	/* Statistics */
	cache->slab_creates = num_slabs;
	cache->slab_destroys = 0;
	cache->slab_allocs = 0;
	cache->slab_frees = 0;

	/* Cache properties */
	cache->descriptor = descriptor;
	cache->next = NULL;
	cache->prev = get_previous_cache(slab_caches);
	cache->constructor = constructor;
	cache->destructor = destructor;

	/* Append new cache to "global" system cache (slab_caches) */
	append_to_global_cache(cache);

	/* Configure slab states */
	if (num_slabs > MAX_CREATABLE_SLABS_PER_CACHE)
		num_slabs = MAX_CREATABLE_SLABS_PER_CACHE;

	cache->free = (slab_state_layer_t *)malloc(sizeof(slab_state_layer_t));
	cache->used = (slab_state_layer_t *)malloc(sizeof(slab_state_layer_t));
	cache->partial = (slab_state_layer_t *)malloc(sizeof(slab_state_layer_t));
	cache->free->head = create_slab(size, malloc(size));

	int pages_to_alloc = size > 4096 ? size / 4096 : 1;
	void *page = PAGE_ALLOC(pages_to_alloc);

	for (int i = 0; i < num_slabs - 1; i++)
	{
		append_slab(&cache->free->head, create_slab(size, page));
		page += size;
	}

	cache->used->head = NULL;
	cache->used->tail = NULL;
	cache->used->next = NULL;
	cache->used->prev = NULL;
	cache->partial->head = NULL;
	cache->partial->tail = NULL;
	cache->partial->next = NULL;
	cache->partial->prev = NULL;

	return cache;
}

void *slab_alloc(slab_cache_t *cache, const char *descriptor, size_t bytes)
{
	// TODO STEPS
	// 0. check if cache and partial slab are existent
	// 1. if no -> allocate new slab
	// 2. search partial slab for object with size = bytes
	// 3. use mem but before returning
	// 4. check if partial slab is full (all objects is_allocated = true)
	// 5. if yes -> move slab to used slab layer
	// 6. check if used slab layer has slabs left
	// 7. if no -> allocate a new one

	// TODO: Search `slab_caches`

	int num_nodes = 0;

	/* 0. check if cache and partial slab are existent and check parameters */
	if (!cache)
		return NULL;

	if (!is_power_of_two(bytes) || (bytes > 4096 && !is_page_aligned(bytes)))
		return NULL;

	slab_state_layer_t *partial = cache->partial;
	slab_state_layer_t *used = cache->used;
	void *mem = NULL;

	/* 1. if no -> allocate new slab */
	if (!partial->head || partial->is_full)
	{
		LOG("Taking memory from free slab\n");
		slab_t *free = cache->free->head;

		if (free == NULL)
		{
			LOG("No more free slabs!\n");
			return NULL; // we are out of memory
		}

		partial->head = (slab_t *)malloc(sizeof(slab_t));
		memcpy(partial->head, free, sizeof(slab_t));
		partial->head->next = NULL;

		if (partial->is_full)
		{
			partial->prev = partial;
			partial->is_full = false;
		}

		remove_slab_head(cache->free);
	}

	do
	{
		for (;;)
		{
			if (partial->head == NULL || partial->is_full)
				break;

			for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
			{
				if (bytes == partial->head->objects[i].size && !partial->head->objects[i].is_allocated)
				{
					/* 3. use mem */
					LOG("Got memory for partial slab #%d: %p\n", i + 1, partial->head->objects[i].mem);
					mem = partial->head->objects[i].mem;
					partial->head->objects[i].is_allocated = true;

					// This partial slab is at the max, use the next available partial slab
					if (num_nodes == MAX_OBJECTS_PER_SLAB - 1)
					{
						partial->is_full = true;
						// Todo: Set the next partials prev pointer here.
					}

					goto end;
				}

				num_nodes++;
			}

			partial->head = partial->head->next;
		}

		partial = partial->next;
	}
	while (partial != NULL);

	// TODO: Search free slab

	if (!mem)
	{
		fprintf(stderr, "Cache is out of memory!\n");
		return NULL;
	}

end:
	/* 4. check if partial slab is full */
	// if (partial->is_full)
	// {
	//     /* 5. move slab to full slab layer */
	//     append_slab(&used->head, partial->head);
	//     remove_slab_head(partial);
	// }

	cache->active_slabs++;
	cache->slab_allocs++;

	return mem;
}

void slab_free(void)
{
	// TODO:
	// - decrement active_slabs
	// - increment slab_frees
}

/* Utility functions */
bool is_page_aligned(int n)
{
	return (n % 4096) == 0;
}

bool is_power_of_two(int n)
{
	return (n > 0) && ((n & (n - 1)) == 0);
}

void append_slab(slab_t **ref, slab_t *new_node)
{
	slab_t *tail = *ref;

	if (*ref == NULL)
	{
		*ref = new_node;
		return;
	}

	while (tail->next != NULL)
		tail = tail->next;

	tail->next = new_node;
}

void slab_traverse_cache(slab_cache_t *cache)
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
		if (current == NULL)
		{
			current = cache;
			return;
		}

		current = current->next;
	}

	__builtin_unreachable();
}

slab_t *create_slab(size_t size, void *memory)
{
	slab_t *slab = (slab_t *)malloc(sizeof(slab_t));

	for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
	{
		slab->objects[i].is_allocated = false;
		slab->objects[i].num_objects = 0;
		slab->objects[i].size = size;
		slab->objects[i].mem = (void *)((uintptr_t)memory + (size * i));
	}

	return slab;
}

void remove_slab_head(slab_state_layer_t *state)
{
	state->head = state->head->next;
}

// TODO: This is used to test whether the linked list can still be traversed in both directions.
// (When you call this after a few allocations, cache->partial will *not* point to the beginning as it has been modified.
// Instead, it will point to the current slab state. 'next' could be NULL or set, prev *must* be set to the previous node unless it's the very first node (not implemented yet))
void *search(slab_cache_t *cache, void *m)
{
	// slab_state_layer_t *partial = cache->partial;
	// for (;;)
	// {
	// for (int i = 0; i < 5; i++)
	// {
	//     if (!partial->head)
	//         return NULL;

	//     for (int i = 0; i < 5; i++)
	//         if (partial->head->objects[i].mem == m)
	//             return m;

	//     partial->head = partial->head->next;
	// }
	// partial = partial->prev;
	// return NULL;
	// }

	// No compiler warnings pls- thanks :^)
	return NULL;
}
