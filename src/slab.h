#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>

// TODO: Define a bitmask for each slab cache to use
// such as panic if no free mem, allocation type (Kernel, user, dma, etc)

void slab_init(void);
void slab_destroy(void);

void slab_cache_alloc(void);
void slab_cache_free(void);

typedef struct
{
    // Do we need this/ how does this work?? Why not use a void *object?
    void (*ctor)(void *obj); // defines the constructor of the object which we want to allocate.
    int size; // Size of the object
} slab_object_t;

// The slab itself. (Contains multiple slab_object_t's)
typedef struct
{
    char descriptor[25];    // Name of the slab
    slab_object_t *objects;
} slab_t;

// Keep's track of the free, used and partial states of a specific cache.
typedef struct
{
    // Linked list of slabs
    slab_t *free;
    slab_t *used;
    slab_t *partial;
} slab_states_t;

// Slab cache (Contains multiple slab_t's)
typedef struct slab_cache
{
    struct slab_cache *prev;
    struct slab_cache *next;
    
    slab_states_t states;
} slab_cache_t;

#endif // SLAB_H