#include "mymalloc.h"
#include <stdio.h>
#include <unistd.h>

// Define the global variables
Strategy strategy = BEST_FIT;
ListType listtype = ADDR_ORDERED_LIST;
Block *free_list = NULL;
Block *heap_start = NULL;
Block *heap_end = NULL;
Block *last_freed = NULL;

/** finds a block based on strategy,
 * if necessary it splits the block,
 * allocates memory,
 * returns the start addrees of data[]*/
void *mymalloc(size_t size) {
    // Initialize heap if first call
    if (!heap_start) {
        heap_start = (Block *)sbrk(HEAP_SIZE);
        if (heap_start == (Block *)-1) return NULL;
        
        // Create initial block
        heap_start->next = NULL;
        heap_start->prev = NULL;
        heap_start->info.size = (HEAP_SIZE - sizeof(Block)) / 16;
        heap_start->info.isfree = 1;
        
        free_list = heap_start;
        heap_end = heap_start;
        last_freed = heap_start;
    }

    // Calculate required blocks (including header)
    size_t blocks_needed = numberof16blocks(size + sizeof(Block));
    
    // Find suitable block using BEST_FIT strategy
    Block *best = NULL;
    Block *current = free_list;
    size_t min_diff = SIZE_MAX;

    while (current) {
        if (current->info.isfree && current->info.size >= blocks_needed) {
            if (strategy == BEST_FIT) {
                size_t diff = current->info.size - blocks_needed;
                if (diff < min_diff) {
                    min_diff = diff;
                    best = current;
                }
            } else if (strategy == FIRST_FIT) {
                best = current;
                break;
            }
        }
        current = current->next;
    }

    if (!best) return NULL;

    // Split if block is too large
    if (best->info.size > blocks_needed + 1) {
        split_block(best, blocks_needed);
    }

    // Mark block as used
    best->info.isfree = 0;
    
    // Remove from free list
    if (best->prev) best->prev->next = best->next;
    if (best->next) best->next->prev = best->prev;
    if (best == free_list) free_list = best->next;
    
    return best->data;
}

/** frees block,
 * if necessary it coalesces with negihbors,
 * adjusts the free list
 */
void myfree(void *p) {
    if (!p) return;
    
    Block *block = (Block*)((char*)p - sizeof(Block));
    block->info.isfree = 1;

    // Add to free list
    block->next = free_list;
    block->prev = NULL;
    if (free_list) free_list->prev = block;
    free_list = block;
    
    // Attempt coalescing
    block = left_coalesce(block);
    block = right_coalesce(block);
}

/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
 */
Block *split_block(Block *b, size_t size) {
    if (!b || b->info.size <= size) return b;
    
    size_t remaining_size = b->info.size - size;
    if (remaining_size < 2) return b;  // Too small to split

    Block *new_block = (Block*)((char*)b + size * 16);
    new_block->info.size = remaining_size;
    new_block->info.isfree = 1;
    b->info.size = size;

    // Insert new block into free list
    new_block->next = b->next;
    new_block->prev = b->prev;
    if (b->next) b->next->prev = new_block;
    if (b->prev) b->prev->next = new_block;
    if (b == free_list) free_list = new_block;

    return b;
}

/** coalesce b with its left neighbor
 * returns the final block
 */
Block *left_coalesce(Block *b) {
    Block *left = prev_block_in_addr(b);
    if (!left || !left->info.isfree) return b;
    
    left->info.size += b->info.size;
    
    // Update free list
    if (b->next) b->next->prev = b->prev;
    if (b->prev) b->prev->next = b->next;
    if (b == free_list) free_list = b->next;
    
    return left;
}

/** coalesce b with its right neighbor
 * returns the final block
 */
Block *right_coalesce(Block *b) {
    Block *right = next_block_in_addr(b);
    if (!right || !right->info.isfree) return b;
    
    b->info.size += right->info.size;
    
    // Update free list
    if (right->next) right->next->prev = right->prev;
    if (right->prev) right->prev->next = right->next;
    if (right == free_list) free_list = right->next;
    
    return b;
}

/** for a given block returns its next block in the list*/
Block *next_block_in_freelist(Block *b) {
    return NULL;
}

/** for a given block returns its prev block in the list*/
Block *prev_block_in_freelist(Block *b) {
    return NULL;
}

/** for a given block returns its right neghbor in the address*/
Block *next_block_in_addr(Block *b) {
    return NULL;
}

/** for a given block returns its left neghbor in the address*/
Block *prev_block_in_addr(Block *b) {
    return NULL;
}

/**for a given size in bytes, returns number of 16 blocks*/
uint64_t numberof16blocks(size_t size_inbytes) {
    return (size_inbytes + 15) / 16;
}

/** prints meta data of the blocks
 * --------
 * size:
 * free:
 * --------
 */
void printheap() {
    Block *current = heap_start;
    printf("Blocks\n");
    while (current && current <= heap_end) {
        printf("---------------\n");
        printf("Free: %d\n", current->info.isfree);
        printf("Size: %lu\n", current->info.size);
        current = next_block_in_addr(current);
    }
    printf("---------------\n");
}

ListType getlisttype() {
    return listtype;
}

int setlisttype(ListType listtype) {
    return 0;
}

Strategy getstrategy() {
    return strategy;
}

int setstrategy(Strategy strategy) {
    return 0;
}