#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define LOG(...) printf(__VA_ARGS__)
#define PAGE_ALLOC() malloc(4096)

// Todo: Define a bitmask for each slab cache to use
// such as panic if no free mem, allocation type (Kernel, user, dma, etc)

// Todo: We might want to use 
// something like the bufctl here:
// http://src.illumos.org/source/xref/illumos-gate/usr/src/uts/common/sys/kmem_impl.h?r=dfec2ecf#118


#define MAX_FREE_SLABS 5    // TODO check 5

typedef struct slab slab_t;
typedef struct slab_cache slab_cache_t;

// Slab cache (Contains multiple slab_t's)
struct slab_cache
{
    /* Statistics */
    uint64_t slab_creates;   // Total nr of created slabs
    uint64_t slab_destroys;  // Total nr of destroyed slabs
    uint64_t slab_allocs;    // Total nr of allocated slabs
    uint64_t slab_frees;     // Total nr of free'd slabs
    uint64_t cache_size;     // Size of the object

    /* Cache properties */
    const char *descriptor;  // Example: fs, double_buffer, etc
    struct slab_cache *prev;
    struct slab_cache *next;
    void (*constructor)(size_t); // Called when a new object is created

    /* Slab layer */
    struct
    {
        void **mem;      // Memory
        int size;        // Size of the slab in bytes
        int num_objects; // Number of entries in `mem'
    } free;

    struct
    {
        void **mem;
        int size;
        int num_objects;
    } used;

    struct
    {
        void **mem;
        int size;
        int num_objects;
    } partial;
};

void slab_init(void);
void slab_destroy(void);

void slab_cache_alloc(void);
void slab_cache_free(void);

slab_cache_t *slab_cache_create(const char *descriptor, size_t size, void (*constructor)(size_t));

void slab_traverse_cache(slab_cache_t* cache);

#endif // SLAB_H