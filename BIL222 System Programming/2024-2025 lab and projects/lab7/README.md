[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/tHtzcgw7)
# BIL 222 Lab: Memory Allocator
- [BIL 222 Lab: Memory Allocator](#bil-222-lab-memory-allocator)
  - [Key Concepts](#key-concepts)
  - [Overview](#overview)
  - [Details](#details)
    - [Coding Requirements](#coding-requirements)
      - [**`void *mymalloc(size_t size)`**](#void-mymallocsize_t-size)
      - [**`void myfree(void *p)`**](#void-myfreevoid-p)
      - [**`void printheap()`**](#void-printheap)
    - [Optional Enhancements](#optional-enhancements)
  - [Grading (Subject to Adjustment)](#grading-subject-to-adjustment)
  - [Provided Files](#provided-files)
    - [`mymalloc.h`](#mymalloch)
    - [`mymalloc.c`](#mymallocc)
  - [Report](#report)
  - [Notes](#notes)
    - [Notes on the size of blocks-alignment and portability](#notes-on-the-size-of-blocks-alignment-and-portability)
  - [The full content of the given files](#the-full-content-of-the-given-files)
    - [mymalloc.h](#mymalloch-1)
    - [mymalloc.c](#mymallocc-1)

---
## Key Concepts  
- Dynamic memory allocation  
- `sbrk()`  
- Linked lists  
- Structs, `typedef`  
- Memory alignment  
- Heap management  

## Overview  
In this lab, you will implement a memory allocator for C programs. The allocator must support dynamic memory allocation and deallocation using custom versions of `malloc` and `free`, along with heap management utilities.  

---

## Details  
### Coding Requirements  
Implement the following functions in `mymalloc.c` to achieve the functionality described below.  

#### **`void *mymalloc(size_t size)`**  
- **Initialization**:  
  - If called for the first time or if insufficient memory exists, use `sbrk()` to expand the heap by `HEAP_SIZE` bytes. Create a single free block and add it to the free list.  
- **Alignment**:  
  - Round the requested `size` up to the nearest multiple of 16 bytes.  
- **Block Allocation**:  
  - Search the free list for a block that can satisfy the request using **at least one** block-fitting strategy (e.g., First Fit, Best Fit, etc.).  
  - Split the chosen block (if possible) into two: one for allocation and one remaining free block. Return the address of the allocated block’s `data[]`.  

#### **`void myfree(void *p)`**  
- **Deallocation**:  
  - Add the block pointed to by `p` back to the free list.  
  - Perform coalescing (merging) with adjacent free blocks if possible.  
- **Free List Management**:  
  - Support **at least one** free list organization (e.g., address-ordered or LIFO).  

#### **`void printheap()`**  
- Print metadata (free status and size) of all blocks in the heap in the following format:  
  ```  
  Blocks  
  Free: 1  
  Size: xxx  
  ---------------  
  Free: 0  
  Size: xxx  
  ---------------  
  ```  

### Optional Enhancements  
1. **Block-Fitting Strategies**:  
   Implement one or more of the following strategies (optional but encouraged):  
   - First Fit  
   - Best Fit  
   - Worst Fit  
   - Next Fit  

2. **Free List Organization**:  
   Choose to implement either:  
   - Address-ordered list (sorted by memory address)  
   - LIFO (unordered) list  

3. **Coalescing**:  
   Support coalescing with both previous and next blocks (optional but recommended for efficiency).  

---

## Grading (Subject to Adjustment)  
**Base Requirements (70%)**:  
- **`mymalloc` (60%)**:  
  - `sbrk()` initialization and initial block creation: **+5%**  
  - Correct block alignment (16-byte multiples): **-5% if incorrect**  
  - Free block search using at least one strategy: **+20%**  
  - Block splitting (`split_block`): **+10%**  
  - Proper removal from the free list: **+5%**  

- **`myfree` (30%)**:  
  - Adding freed blocks to the free list: **+10%**  
  - Coalescing (partial credit for one-directional merging): **+10%**  

**Optional Enhancements (30%)**:  
- **Additional Block-Fitting Strategies**:  
  - Each implemented strategy: **+5%** (max +15%)  
- **Free List Organization**:  
  - Address-ordered or LIFO support: **+5% each** (max +10%)  
- **Full Coalescing** (both directions): **+5%**  

**Other (10%)**:  
- `printheap` functionality: **+5%**  
- Report analyzing fragmentation for implemented strategies: **+5%**  

**Penalties**:  
- Missing base requirements (e.g., no coalescing): **-10–30%**  
- Code style/indentation issues: **up to -10%**  

---

## Provided Files  
### `mymalloc.h`  
- **Structs**:  
  - `Block`: Contains metadata (`size`, `isfree`), pointers to next/prev free blocks, and the data array.  
  - `Tag`: Handles padding and alignment.  
- **Enums**:  
  - `Strategy`: Defines block-fitting strategies (e.g., `BEST_FIT`).  
  - `ListType`: Defines free list organization (`ADDR_ORDERED_LIST` or `UNORDERED_LIST`).  
- **Global Variables**:  
  - `free_list`, `heap_start`, `heap_end`, `last_freed`.  

### `mymalloc.c`  
- Skeleton code for implementing the allocator.  

---

## Report  
Submit a `Report.txt` file containing:  
- A brief explanation of external fragmentation for the strategies you implemented.  
- Example inputs and outputs demonstrating your allocator’s behavior.  

**Optional features are encouraged but not required for full credit. Prioritize base functionality!**

---

## Notes  
- Use the provided `mymalloc.h` and its structs. Do not modify the header.  
- Focus on correctness first (e.g., alignment, coalescing) before implementing optional features.  
- Test edge cases (e.g., freeing `NULL`, allocating zero bytes).  

### Notes on the size of blocks-alignment and portability
2. **Alignment of `Block` Struct**:  
- **Flexible Array Member**: The `data[]` field in `Block` is declared as a flexible array member (C99-compliant).  
- **Struct Sizes**:  
  -  explicit padding is used to avoid compiler-inserted gaps
     *  `__attribute__((packed, aligned(16)))` works with GCC, `_Alignas` is a C11 alignment specifier.
  -  `Tag` is 16 bytes (`uint64_t` + `uint32_t` + `uint32_t` with `packed` attribute).  
  - `Block` is 32 bytes (assuming 64-bit pointers: `next` + `prev` = 16 bytes, `Tag` = 16 bytes).  
2. **Verifying Layout**
    ```c
    #include <assert.h>

    static_assert(sizeof(Tag) == 16, "Tag must be 16 bytes");
    static_assert(_Alignof(Tag) == 16, "Tag must be 16-byte aligned");

    static_assert(sizeof(Block) == 32, "Block must be 32 bytes");
    static_assert(_Alignof(Block) == 16, "Block must be 16-byte aligned");
    ```
3. **`numberof16blocks` Function**:  
   - The current implementation returns `0`.  
     ```c
     uint64_t numberof16blocks(size_t size_inbytes) { 
         return (size_inbytes + 15) / 16; // Round up to nearest 16-byte multiple
     }
     ```
--- 


## The full content of the given files
### mymalloc.h

```c

/*TIPLER VE TANIMLAMALARDA DEGISIKLIK YAPMAYINIZ:
HATA OlDUGUNU DUSUNUYORSANIZ PIAZZADAN FOLLOW-UPLARDA PAYLASINIZ
*/
#include <inttypes.h>
#include <stdalign.h>
#include <stddef.h>
#define HEAP_SIZE 1024 /*used in sbrk to extend heap*/

typedef enum { BEST_FIT,
               NEXT_FIT,
               FIRST_FIT,
               WORST_FIT } Strategy;
typedef enum { ADDR_ORDERED_LIST,
               UNORDERED_LIST } ListType;
extern Strategy strategy;
extern ListType listtype;

/* There is a padding:|8-byte size|4 byte isfree|padding|*/
typedef struct tag {
    uint64_t size;    /*number of  16 byte blocks*/
    uint32_t isfree;  /* you can make this larger and remove padding: */
    uint32_t padding; /*unused space between boundaries*/
} _Alignas(16) Tag;

/*Block: |8byte(next)|8byte(prev)|8-byte size|4byte isfree|padding|0-byte(data)|*/
typedef struct block {
    struct block *next; /*next free*/
    struct block *prev; /*prev free*/
    Tag info;           /*size and isfree*/
    char data[];        /*start of the allocated memory*/
} _Alignas(16) Block;   /*This works only in GCC*/

extern Block *free_list = 0;  /*start of the free list*/
extern Block *heap_start = 0; /*head of allocated memory from sbrk */

/*end of allocated memory from sbrk: the end block can be extended */
extern Block *heap_end = 0;

/*in LIFO equal to *free_list,
in address ordered it is different*/
extern Block *last_freed = 0;

void *mymalloc(size_t size);
void myfree(void *p);

Block *split_block(Block *b, size_t size);
Block *left_coalesce(Block *b);
Block *right_coalesce(Block *b);
Block *next_block_in_freelist(Block *b);
Block *next_block_in_addr(Block *b);
Block *prev_block_in_freelist(Block *b);
Block *prev_block_in_addr(Block *b);
/**for a given size in bytes, returns number of 16 blocks*/
uint64_t numberof16blocks(size_t size_inbytes);
/* prints heap*/
void printheap();
ListType getlisttype();
int setlisttype(ListType listtype);
Strategy getstrategy();
int setstrategy(Strategy strategy);
```

### mymalloc.c
```c
#include "mymalloc.h"
#include <stdio.h>
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
    return NULL;
}

/** frees block,
 * if necessary it coalesces with negihbors,
 * adjusts the free list
 */
void myfree(void *p) {
}

/** splits block, by using the size(in 16 byte blocks)
 * returns the left block,
 * make necessary adjustments to the free list
 */
Block *split_block(Block *b, size_t size) {
    return NULL;
}

/** coalesce b with its left neighbor
 * returns the final block
 */
Block *left_coalesce(Block *b) {
    return NULL;
}

/** coalesce b with its right neighbor
 * returns the final block
 */
Block *right_coalesce(Block *b) {
    return NULL;
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
```
