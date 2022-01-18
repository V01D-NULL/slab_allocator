#include "slab.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// TODO: use ctor and dtor

/* Utility functions */
bool is_page_aligned(int n);
bool is_power_of_two(int n);
void append_slab(slab_t **ref, slab_t *new_node);
void append_to_global_cache(slab_cache_t **ref, slab_cache_t *cache);
void remove_from_global_cache(slab_cache_t *cache);
slab_cache_t *get_previous_cache(slab_cache_t *cache);
slab_t *create_slab(size_t size);
void remove_slab_head(slab_state_layer_t *state);
bool is_slab_empty(slab_t *_slab);

/* Linked list of slab caches */
static slab_cache_t *cache_list;
static slab_cache_t *cache_list_head;

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

    cache_list = malloc(sizeof(slab_cache_t));
}

void slab_destroy(slab_cache_t *cache)
{
    if (cache == NULL)
        return;

    // slab_state_layer_t *free_slab_state = cache->free;
    // slab_state_layer_t *used_slab_state = cache->used;
    // slab_state_layer_t *partial_slab_state = cache->partial;

    // for (;;)
    // {
    //     if (free_slab_state->head == NULL)
    //         break;

    //     for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    //     {
    //         free(free_slab_state->head->objects[i].mem);
    //     }

    //     free_slab_state->head = free_slab_state->head->next;
    // }

    // for (;;)
    // {
    //     if (used_slab_state->head == NULL)
    //         break;

    //     for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    //     {
    //         free(used_slab_state->head->objects[i].mem);
    //     }

    //     used_slab_state->head = used_slab_state->head->next;
    // }

    // for (;;)
    // {
    //     if (partial_slab_state->head == NULL)
    //         break;

    //     for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    //     {
    //         free(partial_slab_state->head->objects[i].mem);
    //     }

    //     partial_slab_state->head = partial_slab_state->head->next;
    // }

    // free(cache->free);
    // free(cache->used);
    // free(cache->partial);

    remove_from_global_cache(cache);
}

slab_cache_t *slab_create_cache(const char *descriptor, size_t size, size_t num_slabs, ctor, dtor)
{
    if (!descriptor)
        return NULL;

    if (!is_power_of_two(size) || (size > 4096 && !is_page_aligned(size)) || num_slabs <= 0)
    {
        LOG("slab_cache_create failed because the parameter 'size' (%ld) is:\n1. not a power of two\n2. not page aligned or\n3. num_slabs is zero or negative\n", size);
        return NULL;
    }

    slab_cache_t *cache = malloc(sizeof(slab_cache_t));

    /* Statistics */
    cache->slab_creates = num_slabs;
    cache->slab_allocs = 0;
    cache->slab_frees = 0;

    /* Cache properties */
    cache->descriptor = descriptor;
    cache->next = NULL;
    cache->prev = get_previous_cache(cache_list);
    cache->constructor = constructor;
    cache->destructor = destructor;

    if (!cache->prev)
        cache_list_head = cache;

    /* Append new cache to "global" system cache (cache_list) */
    append_to_global_cache(&cache_list, cache);

    /* Configure slab states */
    if (num_slabs > MAX_CREATABLE_SLABS_PER_CACHE)
        num_slabs = MAX_CREATABLE_SLABS_PER_CACHE;

    cache->free = malloc(sizeof(slab_state_layer_t));
    cache->used = malloc(sizeof(slab_state_layer_t));
    cache->partial = malloc(sizeof(slab_state_layer_t));
    cache->free->head = create_slab(size);

    int pages_to_alloc = size > 4096 ? size / 4096 : 1;
    void *page = PAGE_ALLOC(pages_to_alloc);

    for (int i = 0; i < num_slabs - 1; i++)
    {
        append_slab(&cache->free->head, create_slab(size));
        page += size * MAX_OBJECTS_PER_SLAB;
    }

    cache->used->head = NULL;
    cache->used->next = NULL;
    cache->used->prev = NULL;
    cache->partial->head = NULL;
    cache->partial->next = NULL;
    cache->partial->prev = NULL;

    return cache;
}

void *slab_alloc(slab_cache_t *cache, const char *descriptor, size_t bytes)
{
    // Algorithm:
    // 0. check if cache and partial slab are existent
    // 1. if no -> allocate new slab
    // 2. search partial slab for object with size = bytes
    // 3. use mem but before returning
    // 4. check if partial slab is full (all objects is_allocated = true)
    // 5. if yes -> move slab to used slab layer
    // 6. check if used slab layer has slabs left
    // 7. if no -> allocate a new one

    // TODO: Search `slab_caches`

    /* 0. check if cache and partial slab are existent and check parameters */
    if (!cache)
        return NULL;

    if (!is_power_of_two(bytes) || (bytes > 4096 && !is_page_aligned(bytes)))
        return NULL;

    slab_state_layer_t *partial_slab_state = cache->partial;
    slab_state_layer_t *used_slab_state = cache->used;
    void *mem = NULL;

    /* 1. if no -> allocate new slab */
    if (!partial_slab_state->head || partial_slab_state->is_full)
    {
        LOGV("Taking memory from free slab\n");
        slab_t *free = cache->free->head;

        if (!free)
        {
            LOG("No more free slabs!\n");
            return NULL; // we are out of memory
        }

        partial_slab_state->head = malloc(sizeof(slab_t));

        if (partial_slab_state->is_full)
        {
            partial_slab_state->is_full = false;
        }

        memcpy(partial_slab_state->head, free, sizeof(slab_t));
        partial_slab_state->head->next = NULL;

        remove_slab_head(cache->free);
    }

    for (;;)
    {
        if (partial_slab_state->head == NULL || partial_slab_state->is_full)
            break;

        for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
        {
            if (bytes == partial_slab_state->head->objects[i].size && !partial_slab_state->head->objects[i].is_allocated)
            {
                /* 3. use mem */
                LOGV("Got memory for partial slab #%d: %p\n", i + 1, partial_slab_state->head->objects[i].mem);
                mem = partial_slab_state->head->objects[i].mem;
                partial_slab_state->head->objects[i].is_allocated = true;

                if (i == MAX_OBJECTS_PER_SLAB - 1)
                {
                    partial_slab_state->is_full = true;
                    append_slab(&used_slab_state->head, partial_slab_state->head);
                    remove_slab_head(partial_slab_state);
                    // Note: never worry about prev pointers in partial slabs since the slabs objects
                    // are all the same size and if a new partial slab needs to be created there is a full one; meaning it is in the full slab
                }
                goto end;
            }
        }

        partial_slab_state->head = partial_slab_state->head->next;
    }

    if (!mem)
    {
        fprintf(stderr, "Cache is out of memory!\n");
        return NULL;
    }

end:

    cache->active_slabs++;
    cache->slab_allocs++;

    return mem;
}

int slab_free(slab_cache_t *cache, void *ptr)
{
    /*
    *  Algorithm:
    *   1. Traverse partial slab, search for an object who's 'mem' field contains 'ptr' and verify that it is allocated.
    *     1.1 If we found something in the partial slab, mark the object as free. If the slab is completely free, move it to the free slab, otherwise do nothing and return.
    * 
    *   2. If searching the partial slab didn't yield any result, search the used slab and perform the same checks.
    *     2.2 Mark the object in this slab as free and move it to the partial slab state.
    * 
    *   3. If neither the partial slab, nor the full slab yielded any result, the user attempted to free a free or invalid pointer. Return an error code.
    */

    if (!cache)
        return 1;

    slab_state_layer_t *free_slab_state = cache->free;
    slab_state_layer_t *used_slab_state = cache->used;
    slab_state_layer_t *partial_slab_state = cache->partial;

    /* 1. Traverse partial slab */
    for (;;)
    {
        if (partial_slab_state->head == NULL)
            break;

        /* 1. Search for an object who's 'mem' field contains 'ptr' and verify that it is allocated */
        for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
        {
            if (ptr == partial_slab_state->head->objects[i].mem && partial_slab_state->head->objects[i].is_allocated)
            {
                /* 1.1 If we found something in the partial slab, mark the object as free */

                partial_slab_state->head->objects[i].is_allocated = false;

                cache->active_slabs--;
                cache->slab_frees++;

                if (is_slab_empty(partial_slab_state->head))
                {
                    /* 1.1 If the slab is completely free, move it to the free slab */
                    append_slab(&free_slab_state->head, partial_slab_state->head);
                    remove_slab_head(partial_slab_state);
                }

                return 0;
            }
        }

        partial_slab_state->head = partial_slab_state->head->next;
    }

    /* 2. Because searching the partial slab didn't yield any result, search the used slab and perform the same checks. */
    for (;;)
    {
        if (used_slab_state->head == NULL)
            break;

        for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
        {

            if (ptr == used_slab_state->head->objects[i].mem && used_slab_state->head->objects[i].is_allocated)
            {
                /* 2.2 Mark the object in this slab as free */

                used_slab_state->head->objects[i].is_allocated = false;

                cache->active_slabs--;
                cache->slab_frees++;

                if (i == MAX_OBJECTS_PER_SLAB - 1)
                {
                    /* 2.2. If the slab is completely free, move it to the partial slab state. */
                    append_slab(&partial_slab_state->head, used_slab_state->head);
                    remove_slab_head(used_slab_state);
                }

                return 0;
            }
        }
    }

    /* 3. If neither the partial slab, nor the full slab yielded any result, the user attempted to free a free or invalid pointer. Return an error code. */
    return 1;
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

    if (!(*ref))
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

        // for (size_t i = 0; i < cache->active_slabs; i++)
        // {
        // LOG("* Free slab nr:\n");
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
        // }

        LOG("=== Cache statistics ===\n");
        LOG("* No. Active slabs: %5ld\n", cache->active_slabs);
        LOG("* No. Created slabs: %4ld\n", cache->slab_creates);
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
        {
            // This is the first node in the list, it has no prev pointer
            if (current == cache)
            {
                LOGV("There is no prev node -> NULL\n");
                return NULL;
            }

            LOGV("Found prev node (%s)\n", current->descriptor);
            return current;
        }

        current = current->next;
    }

    __builtin_unreachable();
}

void append_to_global_cache(slab_cache_t **ref, slab_cache_t *cache)
{
    if (!(*ref))
        BUG("append_to_global_cache: Paremeter 'ref' is NULL"); // This should never happen

    while ((*ref)->next != NULL)
        *ref = (*ref)->next;

    (*ref)->next = cache;
}

void remove_from_global_cache(slab_cache_t *cache)
{
    // The 'cache_list' is updated passively
    // by changing next & prev pointers accordingly:
    //
    // cache -> cache     | cache-to-be-removed |       cache
    //              ^-----------------------------------^

    // Is this the head node?
    if (!cache->prev)
    {
        // Yes, the cache is now empty.
        LOGV("cache '%s' is the head node\n", cache->descriptor);

        if (cache->next)
        {
            LOGV("There is a next cache (%s)\n", cache->next->descriptor);
            cache->next->prev = NULL; // the next node is now the head
            cache = cache->next;
            cache_list_head = cache;
        }
        else
        {
            memset(cache, 0, sizeof(slab_cache_t));
        }

        free(cache);
        return;
    }

    if (cache->next)
    {
        cache->prev->next = cache->next;
        cache->next->prev = cache->prev;
    }
    else
    {
        // We know there is prev pointer.
        cache->prev->next = NULL;
    }

    free(cache);
}

slab_t *create_slab(size_t size)
{
    slab_t *slab = malloc(sizeof(slab_t));

    for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
    {
        slab->objects[i].is_allocated = false;
        slab->objects[i].num_objects = 0;
        slab->objects[i].size = size;
        slab->objects[i].mem = malloc(size);
    }

    return slab;
}

void remove_slab_head(slab_state_layer_t *state)
{
    state->head = state->head->next;
}

void print_slabs(slab_state_layer_t *t)
{
    slab_state_layer_t *type = malloc(sizeof(t));
    memcpy(type, t, sizeof(t));

    for (;;)
    {
        for (int i = 0; i < MAX_SLABS_PER_STATE; i++)
        {
            if (!type->head)
                goto quit;

            for (int j = 0; j < MAX_OBJECTS_PER_SLAB; j++)
                LOG("[slab#%d] type->head[%d] = %p (is free = %d)\n", i, j, type->head->objects[j].mem, !type->head->objects[j].is_allocated);

            type->head = type->head->next;
        }
    }
quit:
    free(type);
}

void print_caches(void)
{
    slab_cache_t *type = malloc(sizeof(slab_cache_t));
    memcpy(type, cache_list_head, sizeof(slab_cache_t));

    for (;;)
    {
        if (!type)
            goto quit;

        LOG("Found cache '%s'\n", type->descriptor);
        type = type->next;
    }
quit:
    free(type);
}

bool is_slab_empty(slab_t *_slab)
{
    slab_t *slab = malloc(sizeof(_slab));
    memcpy(slab, _slab, sizeof(_slab));

    for (;;)
    {
        if (!slab)
        {
            free(slab);
            return true;
        }

        for (int i = 0; i < MAX_OBJECTS_PER_SLAB; i++)
        {
            if (slab->objects[i].is_allocated)
            {
                free(slab);
                return false;
            }
        }

        slab = slab->next;
    }
}
