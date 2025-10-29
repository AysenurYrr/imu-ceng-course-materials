[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/QcCo2zDa)
# Lab: Synchronized Demon Slayer Game with Thread Pools 
- [Lab: Synchronized Demon Slayer Game with Thread Pools](#lab-synchronized-demon-slayer-game-with-thread-pools)
  - [Key Concepts](#key-concepts)
  - [Objectives](#objectives)
  - [Problem Statement](#problem-statement)
  - [Required Modifications](#required-modifications)
    - [1. Thread-Safe Data Structures](#1-thread-safe-data-structures)
    - [2. Thread Pool Implementation](#2-thread-pool-implementation)
    - [3. Required Function Modifications](#3-required-function-modifications)
  - [Implementation Tasks](#implementation-tasks)
    - [Core Requirements](#core-requirements)
    - [Advanced Requirements (Optional)](#advanced-requirements-optional)
  - [Submission Requirements](#submission-requirements)
  - [Notes](#notes)
  - [Recommended Implementation Order](#recommended-implementation-order)

---

**Previous Lab Dependency:** [Lab 8: Multithread Demon Slayer Game](https://github.com/adaskin/bil222-sysprog25/tree/main/labs/8lab-multithread-demonslayer-game.md)  

## Key Concepts  
- POSIX threads  
- Thread pools [threads-II.md](https://github.com/adaskin/bil222-sysprog25/blob/main/lectures/6threads-ii.md) 
- **Synchronization:** Mutexes and Condition Variables [synchronization.md](https://github.com/adaskin/bil222-sysprog25/blob/main/lectures/7synchronization.md)

## Objectives  
1. Implement thread-safe data structures using mutexes and condition variables  
2. Design a thread pool for managing player entities  
3. Practice proper synchronization for shared game state management  
4. Convert naive threading implementation to task queue model  

## Problem Statement  
In this enhanced version of Lab 8's demon slayer game, you will:  
1. Replace direct thread-per-entity model with thread pools  
2. Implement synchronized access to all shared resources  
   1. Demon/player arrays
   2. Per demon/player entities
   3. Each read/write **must** be synchronized.
3. Design proper producer-consumer patterns for entity management  

## Required Modifications  
### 1. Thread-Safe Data Structures  
```c
// demonslayer.h
typedef struct safe_queue {
    void **items;          // Array of queue items
    int capacity;          // Max queue size
    int size;              // Current item count
    int front;             // Front index
    int rear;              // Rear index
    pthread_mutex_t lock;  // Queue access mutex
    pthread_cond_t not_empty; // Condition variable
} SafeQueue;

typedef struct player {
    pthread_mutex_t lock;  // Per-player mutex
    char name[15];
    int score;
    Cell location;
} Player;

typedef struct demon {
    pthread_mutex_t lock;  // Per-demon mutex
    char name[15];
    int score;
    Cell location;
    time_t spawn_time;     // For strength growth
} Demon;
```

### 2. Thread Pool Implementation  
```c
// Thread pool structure
typedef struct thread_pool {
    pthread_t *workers;     // Array of worker threads
    SafeQueue *task_queue;  // Shared task queue
    int pool_size;          // Number of worker threads
    bool shutdown;          // Termination flag
} ThreadPool;
```

### 3. Required Function Modifications  
1. **Player Management**  
   - Convert `Player **players` to `ThreadPool *player_pool`  
   - Implement `player_pool_init()`, `player_pool_submit()`, `player_pool_shutdown()`  
   - Add `player_task_handler()` for thread pool workers  

2. **Demon Management**  
   - Replace `Demon **demons` with `SafeQueue *demon_queue`  
   - Implement `demon_updater()` thread function that:  
     ```c
     void* demon_updater(void *arg) {
         while(!shutdown_flag) {
             // 1. Lock demon_queue
             // 2. Update all demons' strength based on age
             // 3. Check for demon-player collisions
             // 4. Unlock demon_queue
         }
         return NULL;
     }
     ```

3. **Synchronization Points**  
   - Add mutex protection for:  
     - Queue access (both player pool and demon queue)  
     - Individual player/demon state modifications  
     - View updates and scoreboard access  
   - Use condition variables for:  
     - Task notification in thread pools  
     - Resource availability in queues  

## Implementation Tasks  
### Core Requirements  
1. **Thread Pool Implementation** (6 points)  
   - Create thread pool with 6 worker threads  
   - Implement task queue with proper synchronization  
   - Handle player tasks through pool instead of direct threads  

2. **Synchronized Data Structures** (8 points)  
   - Convert `players` array to thread-safe task queue  
   - Convert `demons` array to synchronized queue  
   - Add per-entity mutexes for state changes  

3. **Demon Update System** (6 points)  
   - Implement single `demon_updater()` thread instead of per-demon threads  
   - Add aging-based strength calculation:  
     ```c
     demon->score += (time(NULL) - demon->spawn_time) * GROWTH_RATE;
     ```

4. **Combat System Synchronization** (6 points)  
   - Implement thread-safe combat resolution:  
   ```c
   void resolve_combat(Player *p, Demon *d) {
       pthread_mutex_lock(&p->lock);
       pthread_mutex_lock(&d->lock);
       
       if(p->score > d->score) {
           p->score += d->score;
           remove_demon(d);
       } else {
           d->score += p->score;
           remove_player(p);
       }
       
       pthread_mutex_unlock(&d->lock);
       pthread_mutex_unlock(&p->lock);
   }
   ```

### Advanced Requirements (Optional)  
1. Implement load-balanced thread pool with work stealing  
2. Add condition variable-based rendering throttling  
3. Implement priority queue for high-score players  

## Submission Requirements  
1. Modified `demonslayer.h` with synchronization structures  
2. Thread pool implementation in `pool.c`/`pool.h`  
3. Updated `main.c` with synchronized game logic  
4. Demonstration of deadlock-free operation  
5. Performance comparison (a brief note) with previous lab's implementation  

## Notes  
- Maintain the original game mechanics from Lab 8  
- All shared resource accesses **must** be properly synchronized  
- Use recursive mutexes where nested locking is required  
- Implement proper error handling for synchronization primitives  

## Recommended Implementation Order  
1. Implement thread-safe queue structure  
2. Create thread pool for player entities  
3. Convert demon system to single updater thread  
4. Add per-entity mutexes  
5. Implement synchronized combat system  
6. Update visualization to handle locked resources  
 
**Total Points:** 26 (Core) + 8 (Advanced)  
