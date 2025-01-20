#include <types.h>
#include <kmain.h>
#include <stddef.h>
#include <kstdio.h>

// Global memory table
static mem_table_t mem_table;

// External symbols from linker script
extern uint8_t _sheap;    // Start of heap
extern uint8_t _eheap;    // End of heap

void __init_memory_manager(void) {
    // Initialize first block to cover entire heap section
    mem_block_t *first_block = (mem_block_t *)&_sheap;
    
    kprintf("Memory manager: heap starts at 0x%x\n", (unsigned int)&_sheap);
    kprintf("Memory manager: heap ends at 0x%x\n", (unsigned int)&_eheap);
    kprintf("Memory manager: first block at 0x%x\n", (unsigned int)first_block);
    
    // Calculate total heap size
    uint32_t total_heap_size = (uint32_t)&_eheap - (uint32_t)&_sheap;
    
    // Calculate the data area address (after the block header)
    void *data_start = (void *)((uint8_t *)first_block + sizeof(mem_block_t));
    kprintf("Memory manager: data starts at 0x%x\n", (unsigned int)data_start);
    
    // Initialize the first block
    first_block->addr = data_start;
    first_block->size = total_heap_size - sizeof(mem_block_t);
    first_block->pid = 0;
    first_block->is_free = 1;
    first_block->next = NULL;

    // Initialize memory table
    mem_table.head = first_block;
    mem_table.total_size = total_heap_size;
    mem_table.free_size = first_block->size;
    
    kprintf("Memory manager: total heap size %d bytes, usable size %d bytes\n", 
            mem_table.total_size, mem_table.free_size);
}

int __kmem_free(void *ptr) {
    if (!ptr) {
        kprintf("Free: NULL pointer\n");
        return -1;
    }

    // Validate pointer is within heap bounds
    if ((uint8_t *)ptr < &_sheap || (uint8_t *)ptr >= &_eheap) {
        kprintf("Free: pointer outside heap bounds\n");
        return -1;
    }

    kprintf("Free: attempting to free address 0x%x\n", (unsigned int)ptr);

    // Find the memory block for this pointer
    mem_block_t *block = mem_table.head;
    while (block) {
        kprintf("Free: checking block at 0x%x, data at 0x%x\n", 
                (unsigned int)block, (unsigned int)block->addr);
        
        if (block->addr == ptr && !block->is_free) {
            // Found the block, mark it as free
            block->is_free = 1;
            block->pid = 0;
            mem_table.free_size += block->size;
            
            kprintf("Free: freed block of size %d\n", block->size);

            // Try to merge with next block if it's free
            if (block->next && block->next->is_free) {
                block->size += block->next->size + sizeof(mem_block_t);
                block->next = block->next->next;
                kprintf("Free: merged with next block, new size %d\n", block->size);
            }

            return 0;
        }
        block = block->next;
    }

    kprintf("Free: block not found\n");
    return -1;
}

void *__kmem_malloc(uint32_t size) {
    if (size == 0) {
        kprintf("Malloc: zero size requested\n");
        return NULL;
    }

    // Align size to 8 bytes for better memory access
    size = (size + 7) & ~7;
    kprintf("Malloc: aligned size is %d bytes\n", size);

    // Check if we have enough free memory
    if (size > mem_table.free_size) {
        kprintf("Malloc: not enough memory (need %d, have %d)\n", 
                size, mem_table.free_size);
        return NULL;
    }

    // Find a suitable free block using first-fit strategy
    mem_block_t *block = mem_table.head;
    while (block) {
        kprintf("Malloc: checking block at 0x%x (size=%d, free=%d)\n", 
                (unsigned int)block, block->size, block->is_free);
        
        if (block->is_free && block->size >= size) {
            // Found a suitable block
            if (block->size >= size + sizeof(mem_block_t) + 8) {
                // Split the block if there's enough space for a new block
                mem_block_t *new_block = (mem_block_t *)((uint8_t *)block->addr + size);
                
                // Verify new block is within heap bounds
                if ((uint8_t *)new_block >= &_eheap) {
                    kprintf("Malloc: split would exceed heap bounds\n");
                    return NULL;
                }
                
                kprintf("Malloc: splitting block, new block at 0x%x\n", 
                        (unsigned int)new_block);
                
                // Initialize the new block
                new_block->size = block->size - size - sizeof(mem_block_t);
                new_block->is_free = 1;
                new_block->pid = 0;
                new_block->next = block->next;
                new_block->addr = (void *)((uint8_t *)new_block + sizeof(mem_block_t));
                
                // Update the current block
                block->size = size;
                block->next = new_block;
                
                kprintf("Malloc: split sizes: %d and %d\n", 
                        block->size, new_block->size);
            }

            // Mark the block as used
            block->is_free = 0;
            block->pid = 0;
            mem_table.free_size -= block->size;

            kprintf("Malloc: allocated block at 0x%x, data at 0x%x, size %d\n", 
                    (unsigned int)block, (unsigned int)block->addr, block->size);
            
            // Verify the address is within heap bounds
            if ((uint8_t *)block->addr < &_sheap || 
                (uint8_t *)block->addr >= &_eheap) {
                kprintf("Malloc: WARNING - address outside heap bounds!\n");
                return NULL;
            }
            
            return block->addr;
        }
        block = block->next;
    }

    kprintf("Malloc: no suitable block found\n");
    return NULL;
} 