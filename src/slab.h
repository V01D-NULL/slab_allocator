#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#define LOG(...) printf(__VA_ARGS__)
#ifdef LOG_VERBOSE
#define LOGV(...) printf("\e[1;34m"); printf(__VA_ARGS__); printf("\e[0m")
#else
#define LOGV(...)
#endif

#define BUG(...) { printf("\033[31mBUG:\033[39m "); printf(__VA_ARGS__); exit(2); }
#define PAGE_ALLOC(pages) malloc(4096 * pages)
#define PAGE_FREE(ptr) free(ptr);

#define MAX_SLABS_PER_STATE  5 // 5 slabs per slab_state_t
#define MAX_OBJECTS_PER_SLAB 5 // 5 objects per slab_t
#define MAX_CREATABLE_SLABS_PER_CACHE 8 // 8 slabs per cache are the max limit
#define SLAB_FREE_ENTRY (void*)-1

/*
An simple implementation of Slab Memory Allocator. A logic structure can be 
simplified like this:
             
             |-- kmem_cache 
             |
kmem_chain --|-- kmem_cache   |-- slabs_full
             |                |
             |-- kmem_cache --|-- slabs_partial |-- slab
                              |                 |
                              |-- slabs_empty --|-- slab   |-- chunk
                                                |          |
                                                |-- slab --|-- chunk
                                                           |
                                                           |-- chunk

*/

typedef struct slab_cache slab_cache_t;

// Represents an object of a given slab
typedef struct
{
    void *mem;
    int num_objects;

    /* Flags */
    bool is_allocated;
} slab_object_t;

// Represents a slab itself, a slab state (full,partial,used) may have multiple struct slab's (i.e. a linked list)
typedef struct _slab
{
    slab_object_t objects[MAX_OBJECTS_PER_SLAB];
    struct _slab *next;
} slab_t;

// Represents a slab *state*, i.e. partial, free or full.
typedef struct slab_state
{
    slab_t *head;
    bool is_full; // True if this slab state is full, use the next one if available.
    bool is_empty;
    struct slab_state *prev;
    struct slab_state *next;
} slab_state_layer_t;

// Slab cache, also known as kmem_cache in linux
struct slab_cache
{
    uint32_t size;

    /* Statistics */
    uint64_t active_slabs;   // 
    uint64_t slab_creates;   // Total nr of created slabs
    uint64_t slab_allocs;    // Total nr of allocated slabs
    uint64_t slab_frees;     // Total nr of free'd slabs
    
    /* Cache properties */
    struct slab_cache *prev;
    struct slab_cache *next;

    /* Slab layer */
    slab_state_layer_t *free    __attribute__((aligned(16)));
    slab_state_layer_t *used    __attribute__((aligned(16)));
    slab_state_layer_t *partial __attribute__((aligned(16)));
};

/* Core functions */
void slab_init(void);
void slab_destroy(slab_cache_t *cache);
slab_cache_t *slab_create_cache(size_t size, size_t num_slabs);
void *slab_alloc(slab_cache_t *cache, size_t bytes);
int slab_free(slab_cache_t *cache, void *ptr);

/* Utility functions */
void slab_traverse_cache(slab_cache_t *cache);
void print_slabs(slab_state_layer_t *t);
void print_caches(void);

#endif // SLAB_H
