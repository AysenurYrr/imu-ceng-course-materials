[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Ckx1vpHM)
## **Project: Emergency Drone Coordination System (Client-Server app)**  

**Objective**:  
Transition from a simulator to a real client-server system where drones (clients) communicate with a central server to coordinate emergency aid for survivors (simulated by random generations). Students will implement synchronization, thread-safe data structures, and networked communication, mirroring real-world drone swarm coordination systems.

---

### **Phased Implementation Plan**  
#### **Phase 1 (given as a lab lab): Thread-Safe Lists & Simulated Clients/Servers**  
**Objective**: Implement synchronization and simulate client-server interactions using threads **(no sockets yet, drones are simulated as threads)**.  
**Duration**: 1 week  

**Tasks**:  
1. **Fix and Enhance the List Implementation**:  
   - **Mandatory Fixes**:  
     - Add `pthread_mutex_t` to the `List` struct in `list.h` and synchronize all operations (`add`, `remove`, `pop`).  
     - Fix memory leaks in `destroy()` by removing redundant `memset`.  
     - Replace `sprintf` with `snprintf` in `listtest.c` to prevent buffer overflows.  
   - **Optional Efficiency Improvements**:  
     - Implement a *free list* to reuse nodes and reduce allocation overhead.  

2. **Simulate Drones and Survivors with Threads**:  
   - **Drone Simulator Thread**:  
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
   - **Survivor Generator Thread**:  
     ```c  
     void* survivor_generator(void* arg) {  
         while (1) {  
             // Generate random survivors  
             // Add to synchronized `survivor_list`  
             sleep(2);  
         }  
     }  
     ```  

3. **Visualize with SDL**:  
   - Update `view.c` to show:  
     - Survivors as **red cells**, idle drones as **blue dots**, and missions as **green lines**.  

---

#### **Phase 2: Basic Socket Communication**  
see [communication protocol details](communication-protocol.md)

**Objective**: Replace simulated threads with real socket-based clients and servers.  
**Duration**: 1 week (after socket lectures)  

**Tasks**:  
1. **Server Implementation**:  
   - Create a multi-threaded server that:  
     - Listens for incoming drone connections (TCP).  
     - Maintains a synchronized list of active drones and survivors.  
   - Example server loop:  
     ```c  
     while (1) {  
         int drone_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);  
         pthread_create(&thread_id, NULL, handle_drone, (void*)&drone_fd);  
     }  
     ```  

2. **Drone Client Implementation**:  
   - Drones connect to the server and send periodic status updates (JSON format):  
     ```json  
     { "drone_id": "D1", "status": "idle", "location": [10, 20] }  
     ```  
   - Receive mission assignments (e.g., `{"type": "mission", "target": [x, y]}`).  

3. **AI Controller Logic**:  
   - Assign closest idle drone to the oldest unhelped survivor (prioritize fairness).  

---

#### **Phase 3: Advanced Features & Testing**  
**Objective**: Optimize performance and handle edge cases.  
**Duration**: 1 week  

**Tasks**:  
1. **Load Testing**:  
   - Simulate 50+ drones and measure server throughput.  
2. **Fault Tolerance**:  
   - Handle disconnected drones (e.g., reassign missions after timeout).  
3. **Optional Extensions**:  
   - Add a web-based dashboard using WebSocket for real-time monitoring.
     - Or you can use view.c as a client version
     - Server sends the changes to the client or the whole data and client shows it
   - Implement QoS (quality of service) e.g., prioritize critically injured survivors.  

---

### **Technical Components**  
#### **Client (Drone)**  
- **Key Functions**:  
  - `connect_to_server()`: Establishes TCP connection.  
  - `send_status_update()`: Periodically reports drone state.  
  - `navigate_to_target()`: Simulates movement toward survivor coordinates.  

#### **Server**  
- **Key Functions**:  
  - `handle_drone()`: Thread to manage each drone connection.  
  - `assign_missions()`: AI logic to match drones and survivors.  
  - `log_performance()`: Track average survivor wait time and drone utilization.  

#### **Communication Protocol (see [communcation-protocol.md](communication-protocol.md))**  
- **Messages**:  
  - Drone → Server: `STATUS_UPDATE`, `MISSION_COMPLETE`.  
  - Server → Drone: `ASSIGN_MISSION`, `HEARTBEAT`.  
- **Format**: JSON (simple, human-readable).  

---

### **Learning Outcomes**  
1. **Core Skills**:  
   - Thread synchronization (mutexes), socket programming, and client-server design.  
2. **Real-World Relevance**:  
   - Mirrors systems like Zipline’s medical delivery drones or Amazon’s delivery swarms.  
3. **Problem Solving**:  
   - Debug race conditions, optimize latency, and handle scalability.  

---

### **Grading Criteria**  
- **Code (50%)**:  
  - Thread-safe lists, working socket communication, AI logic.  
- **Report (25%)**:  
  - Explain design choices, synchronization strategy, and performance analysis.  
- **Demo (25%)**:  
  - Show handling of 10+ concurrent drones with real-time SDL visualization.  

---

### **Tools & Starter Code**  
- **Provided**:  
  -  `list.h/c` (needs to be fixed), SDL visualization framework, a simulator code as skeleton.  
- **Recommended Libraries**:  
  - `json-c` (message parsing), `SDL2` (visualization).  


