#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void ctor(size_t foo) {}

int main(int argc, char const *argv[])
{
    slab_init();
    slab_cache_t *dummy_cache = slab_cache_create("dummy_cache", 20, ctor);
    slab_traverse_cache(dummy_cache); // Note: Everything except for the cache descriptor string and the cache size
                                      // should be 0 since there have been no calls to slab_cache_alloc().

    return 0;
}