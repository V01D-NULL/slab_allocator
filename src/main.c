#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void my_ctor(size_t foo) {}
void my_dtor(size_t foo) {}

int main(int argc, char const *argv[])
{
	slab_init();

	size_t size = 2;
	slab_cache_t *dummy_cache1 = slab_cache_create("first dummy_cache", size, 3, my_ctor, my_dtor);

	if (dummy_cache1 == NULL)
	{
		fprintf(stderr, "slab_cache_create returned NULL\n");
		exit(1);
	}

	// Demo: Allocate memory from a partial slab. Take a free slab if
	// the partial slab is empty. Exits the program if no free slab is available.
	for (;;)
		if (slab_cache_alloc(dummy_cache1, NULL, size) == NULL)
			exit(0);

	// Uncomment to test prev pointers in the slab state layer list.
	// These 5 allocations are going into a single slab state (i.e. cache->partial->next is NULL)
	// void *addr = slab_cache_alloc(dummy_cache1, NULL, size);
	// slab_cache_alloc(dummy_cache1, NULL, size);
	// slab_cache_alloc(dummy_cache1, NULL, size);
	// slab_cache_alloc(dummy_cache1, NULL, size);
	// slab_cache_alloc(dummy_cache1, NULL, size);

	// This allocation goes into the next available slab state (cache->partial->next)
	// slab_cache_alloc(dummy_cache1, NULL, size);

	// Note: The linked list now points to cache->partial->next so you cannot simply search
	// for addr anymore without implementing the prev pointer in the linked list.
	// LOG("addr = %p\n", addr);
	// LOG("search() = %p\n", search(dummy_cache1, addr));

	return 0;
}
