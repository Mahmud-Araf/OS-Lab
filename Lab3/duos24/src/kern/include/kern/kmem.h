#ifndef __KMEM_H
#define __KMEM_H

#include <types.h>

// Initialize memory management system
void __init_memory_manager(void);

// Allocate memory from the heap
void *__kmem_malloc(uint32_t size);

// Free allocated memory
int __kmem_free(void *ptr);

#endif /* __KMEM_H */ 