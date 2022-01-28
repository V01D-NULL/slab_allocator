#include "kmalloc.h"

// used when small memory buffers are required
void kmalloc(size_t bytes)
{

}

void kzalloc()
{
    // slab_cache_alloc with zeros
}

void krealloc()
{
    // resize cache allocation
}

void kfree()
{
    // slab_cache_free
}

void kzfree()
{
    // slab_cache_free with zero
}