#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

void kmalloc(size_t bytes);
void kfree();

#endif // KMALLOC_H