# **Lab for Project: Multi-Threaded Emergency Drone Coordination System**  

## **Phase 1: Synchronized Lists & Simulated Drone Behavior**  
This lab you will implement the synchronized lists necessary for the project. For instance, in Java there are conccurent data structures[see the link](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/util/concurrent/package-summary.html) they provide thread safe datastructures.

**In this thread safe data structures, you still need to use locks when you try to iterate over them.**  
- You are given a partially implemented simulator. You need to implement synchronizations in the list. And change the lock/unlock mechanism in the simulator code.

---

## **Objective**  
Implement a thread-safe, multi-drone simulator where autonomous drones (as threads) coordinate to assist survivors on a map. Students will:  
1. Fix and synchronize a shared list data structure.  
2. Fix the simulator code
3. Simulate drone behavior using threads.  
4. Implement an AI controller to assign missions.  
5. Visualize drone/survivor states with SDL.  

**This phase lays the foundation for Phase 2 (client-server networking)**.  

---

## **Background**  
In disaster scenarios, drone swarms assist survivors by delivering aid. This lab simulates this system using threads and shared data structures. The core challenge is ensuring thread safety and efficient coordination.  

---

## **Tasks**  
### **1. Fix and Synchronize the List Implementation**  
**Purpose**: Ensure thread safety for shared survivor/drone lists.  
**Steps**:  
- Add a `pthread_mutex_t lock` to the `List` struct in `list.h`.  
- Initialize/destroy the mutex in `create_list()` and `destroy()`.  
- Wrap all list operations (`add`, `pop`, etc.) with `pthread_mutex_lock/unlock`.  

**Code Snippet**:  
```c  
// list.c (add function)  
Node* add(List* list, void* data) {  
    pthread_mutex_lock(&list->lock);  
    // ... existing logic ...  
    pthread_mutex_unlock(&list->lock);  
    return node;  
}  
```  

---

### **2. Fix Overflow/Underflow of the List**  

**In list.c, for the overflow/underflow use semaphores or condition variables.**


### **3. Check and correct the synchronization used in the code**

Implementation of the simulation is partially given. 
Since your `list.c` has become thread-safe for add, remove operations;** only the iterations** or direct access to the pointers in the list would need synchronization. 

## Implementation of the simulation and edits required to be done

### Survivors
Survivors are generated randomly by a thread that runs `survivor_generator(void *args)`. You can change its frequence etc.

### Drones (Volunteered helpers)
In current code, the drones are created as separate threads. In Phase-2, instead of `drone_fleet`, you will have client of drones.


**Code Snippet**:  
```c  
// Global drone fleet
Drone *drone_fleet = NULL;
int num_drones = 10; // Default fleet size

void initialize_drones() {
    drone_fleet = malloc(sizeof(Drone) * num_drones);
    srand(time(NULL));

    for(int i = 0; i < num_drones; i++) {
        drone_fleet[i].id = i;
        drone_fleet[i].status = IDLE;
        drone_fleet[i].coord.x = rand() % map.width;
        drone_fleet[i].coord.y = rand() % map.height;
        drone_fleet[i].target = drone_fleet[i].coord; // Initial target=current position
        pthread_mutex_init(&drone_fleet[i].lock, NULL);
        
        // Add to global drone list
        pthread_mutex_lock(&drones->lock);
        drones->add(drones, &drone_fleet[i]);
        pthread_mutex_unlock(&drones->lock);
        
        // Create thread
        pthread_create(&drone_fleet[i].thread_id, NULL, drone_behavior, &drone_fleet[i]);
    }
} 
```  

---

**Drone Thread Behavior**  
If drone is on mission, it is flying toward the target. Otherwise it waits for a mission. 
In Phase2, you will implement this as a client. For this lab, you can change its behavior. For instance, **optional task**: Instead of `while(1)`, you can use condition variable. If there is a task, it would wake-up. 

**Code Snippet**:  
```c  
void* drone_behavior(void *arg) {
    Drone *d = (Drone*)arg;
    
    while(1) {
        pthread_mutex_lock(&d->lock);
        
        if(d->status == ON_MISSION) {
            // Move toward target (1 cell per iteration)
            if(d->coord.x < d->target.x) d->coord.x++;
            else if(d->coord.x > d->target.x) d->coord.x--;
            
            if(d->coord.y < d->target.y) d->coord.y++;
            else if(d->coord.y > d->target.y) d->coord.y--;

            // Check mission completion
            if(d->coord.x == d->target.x && d->coord.y == d->target.y) {
                d->status = IDLE;
                printf("Drone %d: Mission completed!\n", d->id);
            }
        }
        
        pthread_mutex_unlock(&d->lock);
        sleep(1); // Update every second
    }
    return NULL;
}

```  

---

### AI Controller for Mission Assignment  
**Purpose**: Assign idle drones to survivors based on proximity.  

**Steps**:  
- Iterate through the survivor list.  
- Find the closest idle drone using Manhattan distance.  
- Update the drone’s target and status.  
  
You will also use this in Phase-2

**Code Snippet**:  
```c  
void* ai_controller(void *arg) {
    while (1) {
        pthread_mutex_lock(&survivors->lock); // List mutex
        Survivor *s = survivors->peek(survivors);
        if (s) {
            pthread_mutex_lock(&drones->lock); // List mutex
            Drone *closest = find_closest_idle_drone(s->coord);
            pthread_mutex_unlock(&drones->lock);

            if (closest) {
                assign_mission(closest, s->coord); // Uses drone->lock
                survivors->pop(survivors, NULL);
            }
        }
        pthread_mutex_unlock(&survivors->lock);
        sleep(1);
    }
}
```  

---

### **5. SDL Visualization**  
**Purpose**: Visualize drones, survivors, and missions in real time.  

**Steps**:  
- Update `view.c` to draw drones (blue = idle, green = on mission).  
- Draw lines between drones and their targets.  

The current code only draws the survivors and drones.

You should change this to show 
- drones target (their flying destination), 
- survivors that are assigned drones
- survivors that are waiting for help
  
**Code Snippet**:  
```c  
void draw_drones() {
    for (int i = 0; i < num_drones; i++) {
        pthread_mutex_lock(&drone_fleet[i].lock);
        SDL_Color color = (drone_fleet[i].status == IDLE) ? BLUE : GREEN;
        draw_cell(drone_fleet[i].coord.x, drone_fleet[i].coord.y, color);
        
        // Draw mission line if on mission
        if (drone_fleet[i].status == ON_MISSION) {
            SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
            SDL_RenderDrawLine(renderer,
                drone_fleet[i].coord.y * CELL_SIZE + CELL_SIZE/2,
                drone_fleet[i].coord.x * CELL_SIZE + CELL_SIZE/2,
                drone_fleet[i].target.y * CELL_SIZE + CELL_SIZE/2,
                drone_fleet[i].target.x * CELL_SIZE + CELL_SIZE/2);
        }
        pthread_mutex_unlock(&drone_fleet[i].lock);
    }
}

void draw_survivors() {
    for (int i = 0; i < map.height; i++) {
        for (int j = 0; j < map.width; j++) {
            pthread_mutex_lock(&map.cells[i][j].survivors->lock);
            if (map.cells[i][j].survivors->number_of_elements > 0) {
                draw_cell(i, j, RED);
            }
            pthread_mutex_unlock(&map.cells[i][j].survivors->lock);
        }
    }
}
```  

---

## **Testing & Debugging Tips**  
1. **Test Thread Safety**:  
   - Run multiple survivor/drone generators concurrently.  
   - Use `valgrind --tool=helgrind` to detect race conditions.  
   - E.g. survivors are malloced but not freed.
2. **Verify Movement Logic**:  
   - Ensure drones reach targets within expected time (e.g., distance/speed).  
3. **Check Visualization**:  
   - Confirm idle/active drones and mission lines render correctly.  

---

## **Deliverables**  
1. **Code**:  
   - Thread-safe `list.c/h`.  
   - Multi-threaded drone simulation in `model: survivor.c, drone.c, ai.c`.  
   - Updated SDL visualization in `view.c, map.c`.  
2. **Report**:  
   - Explain synchronization strategy.  
   - Analyze performance (e.g., missions completed per minute).  

---

## **Grading Criteria**  
| **Criteria**               | **Weight** |  
|-----------------------------|------------|  
| Thread-safe list            | 30%        |  
| Working drone threads       | 30%        |  
| AI controller logic         | 20%        |  
| Visualization & documentation | 20%      |  

---

## **Connection to Future Project Phases**  
- **Phase 2**: Replace simulated drone threads with socket-based clients. The synchronized lists will manage incoming drone connections and survivor requests.
  1. **Phase 1 (Simulator)**  
     - **`drone.c`** owns all drone-related logic:  
       - Thread creation  
       - Behavior simulation  
       - Memory management  
     - **`controller.c`** only orchestrates high-level flow.

  2. **Phase 2 (Client-Server Transition)**  
     - **Minimal Changes**:  
       - Replace `initialize_drones()` in `controller.c` with server-side client acceptance.  
       - `drone.c` evolves into:  
         - **Server-side**: `drone_manager.c` (tracks connected drones).  
         - **Client-side**: `drone_client.c` (replaces thread behavior with network ops).  

        **Example Phase 2 Controller**
        ```c
        // controller.c (Phase 2)
        #include "drone_manager.h" // Replaces drone.h

        int main() {
            // Initialize server (instead of drone threads)
            start_drone_server(8080); 

            // Rest remains similar
            while (!quit) {
                handle_connections(); // Accept new drones
                update_simulation(); // AI controller logic
                draw_map();
            }
        }
    ```
    
- **Phase 3**: Extend the AI controller to handle network latency and drone failures.  

