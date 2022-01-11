#include <stdio.h>
#include "slab.h"
#include <stdlib.h>

void my_ctor(size_t foo) {}
void my_dtor(size_t foo) {}

int main(int argc, char const *argv[])
{
	slab_init();

	size_t size = 2;
    slab_cache_t *dummy_cache1 = NULL;
	if ((dummy_cache1 = slab_create_cache("first dummy_cache", size, 3, my_ctor, my_dtor)) == NULL)
    {
		fprintf(stderr, "slab_cache_create returned NULL\n");
		exit(1);
	}

    LOG("Dumping used slabs...\n");
    // print_slabs(dummy_cache1->used);


    LOG("Allocating all memory...\n");
	// Demo: Allocate memory from a partial slab. Take a free slab if
	// the partial slab is empty. Exits the program if no free slab is available.
	for (;;)
		if (slab_alloc(dummy_cache1, NULL, size) == NULL)
			break;

    LOG("Dumping used slabs...\n");
    slab_state_layer_t *l = dummy_cache1->used;
    print_slabs(l);
    LOG("\n\n\n");
    print_slabs(l);

	return 0;
}
