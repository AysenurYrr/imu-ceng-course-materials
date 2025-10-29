## **Detailed communication protocol** 

To standardize interactions between drones (clients) and the server, you should implement the following protocol. 
- This protocol ensures interoperability between your projects while leaving room for custom AI logic (e.g., mission prioritization). You can extend it with optional fields like `emergency_level` or `weather_conditions`.

---

### **Communication Protocol**  
**Transport**: TCP (reliable, ordered delivery).  
**Encoding**: JSON (UTF-8).  
**Message Types**:  

| **Direction**       | **Message Type**       | **Purpose**                                                                 |
|----------------------|------------------------|-----------------------------------------------------------------------------|
| **Drone → Server**   | `HANDSHAKE`            | Register drone with server (initial connection).                           |
|                      | `STATUS_UPDATE`        | Periodic updates (location, battery, status).                              |
|                      | `MISSION_COMPLETE`     | Notify server of mission completion.                                       |
|                      | `HEARTBEAT_RESPONSE`   | Acknowledge server’s heartbeat.                                            |
| **Server → Drone**   | `HANDSHAKE_ACK`        | Confirm drone registration.                                                |
|                      | `ASSIGN_MISSION`       | Assign a mission (target coordinates).                                     |
|                      | `HEARTBEAT`            | Check if drone is alive (sent periodically).                               |
| **Either → Either**  | `ERROR`                | Report protocol violations, invalid missions, or connection issues.        |

---

### **1. Message Formats**  
#### **Drone → Server**  
**A. `HANDSHAKE` (Initial Connection)**  
```json
{
  "type": "HANDSHAKE",
  "drone_id": "D1",
  "capabilities": {
    "max_speed": 30,
    "battery_capacity": 100,
    "payload": "medical"
  }
}
```

**B. `STATUS_UPDATE` (Periodic Updates)**  
```json
{
  "type": "STATUS_UPDATE",
  "drone_id": "D1",
  "timestamp": 1620000000,
  "location": {"x": 10, "y": 20},
  "status": "idle",  // "idle", "busy", "charging"
  "battery": 85,
  "speed": 5
}
```

**C. `MISSION_COMPLETE`**  
```json
{
  "type": "MISSION_COMPLETE",
  "drone_id": "D1",
  "mission_id": "M123",
  "timestamp": 1620000000,
  "success": true,
  "details": "Delivered aid to survivor."
}
```

**D. `HEARTBEAT_RESPONSE`**  
```json
{
  "type": "HEARTBEAT_RESPONSE",
  "drone_id": "D1",
  "timestamp": 1620000000
}
```

---

#### **Server → Drone**  
**A. `HANDSHAKE_ACK`**  
```json
{
  "type": "HANDSHAKE_ACK",
  "session_id": "S123",
  "config": {
    "status_update_interval": 5,  // in seconds
    "heartbeat_interval": 10
  }
}
```

**B. `ASSIGN_MISSION`**  
```json
{
  "type": "ASSIGN_MISSION",
  "mission_id": "M123",
  "priority": "high",  // "low", "medium", "high"
  "target": {"x": 45, "y": 30},
  "expiry": 1620003600,  // mission expiry timestamp
  "checksum": "a1b2c3"   // optional data integrity check
}
```

**C. `HEARTBEAT`**  
```json
{
  "type": "HEARTBEAT",
  "timestamp": 1620000000
}
```

**D. `ERROR`**  
```json
{
  "type": "ERROR",
  "code": 404,
  "message": "Mission M123 not found.",
  "timestamp": 1620000000
}
```

---

### **2. Sequence Diagram**  
```plaintext
Drone                   Server
|----HANDSHAKE--------->|      // Drone registers
|<---HANDSHAKE_ACK------|      // Server confirms
|----STATUS_UPDATE----->|      // Periodic updates
|<---ASSIGN_MISSION-----|      // Server assigns mission
|----MISSION_COMPLETE-->|      // Drone completes mission
|<-------HEARTBEAT------|      // Server checks liveness
|----HEARTBEAT_RESPONSE>|      // Drone responds
```

---

### **3. Rules & Conventions**  
1. **Timestamps**: Unix epoch time (UTC).  
2. **Coordinates**: Grid-based (`x`, `y` as integers).  
3. **Mission IDs**: Unique strings (e.g., `M123`).  
4. **Heartbeats**: If a drone misses 3 heartbeats, mark it `disconnected`.  
5. **Error Codes**:  
   - `400`: Invalid JSON.  
   - `404`: Mission not found.  
   - `503`: Server overloaded.  

---

### **4. Example Workflow**  
1. **Drone Registration**:  
   - Drone sends `HANDSHAKE`.  
   - Server replies with `HANDSHAKE_ACK` and configuration.  

2. **Mission Assignment**:  
   - Server sends `ASSIGN_MISSION` to closest idle drone.  
   - Drone acknowledges by starting navigation.  

3. **Mission Completion**:  
   - Drone sends `MISSION_COMPLETE` on success/failure.  
   - Server updates survivor list and drone status.  

---

