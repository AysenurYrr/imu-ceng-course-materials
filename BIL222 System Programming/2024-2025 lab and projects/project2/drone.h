#ifndef DRONE_H
#define DRONE_H

#include "coord.h"
#include <time.h>
#include <pthread.h>
#include "list.h"

// Drone durumları
typedef enum {
    IDLE,       // Boşta
    ON_MISSION, // Görevde
    CHARGING    // Şarj oluyor
} DroneStatus;

// Drone yapısı
typedef struct {
    char id[32];       // Drone ID
    Coord coord;       // Mevcut konum
    Coord target;      // Hedef konum (görevdeyse)
    DroneStatus status;// Durum
} Drone;

// Global drone list (extern)
extern List *drones;
extern Drone *drone_fleet; // Array of drones
extern int num_drones;    // Number of drones in the fleet
// Functions
void initialize_drones();
void assign_mission(Drone *drone, Coord target);

#endif