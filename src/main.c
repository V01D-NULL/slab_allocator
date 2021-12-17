#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void ctor1(size_t foo) {}
void ctor2(size_t foo) {}

int main(int argc, char const *argv[])
{
    slab_init();

    slab_cache_t *dummy_cache1 = slab_cache_create("first dummy_cache", 20, ctor1);
    slab_traverse_cache(dummy_cache1); // Note: Everything except for the cache descriptor string and the cache size
                                      // should be 0 since there have been no calls to slab_cache_alloc().
    
    slab_cache_t *dummy_cache2 = slab_cache_create("second dummy_cache", sizeof(int), ctor1);
    slab_traverse_cache(dummy_cache2); // Note: Everything except for the cache descriptor string and the cache size
                                      // should be 0 since there have been no calls to slab_cache_alloc().

    return 0;
}