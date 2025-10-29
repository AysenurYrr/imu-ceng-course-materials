#ifndef SURVIVOR_H
#define SURVIVOR_H

#include "map.h"
#include <time.h>
#include "list.h"
#include "coord.h"
// Survivor durumları
#define WAITING 0    // Yardım bekliyor
#define ASSIGNED 1   // Drone atanmış
#define RESCUED 2    // Kurtarıldı

typedef struct survivor {
    Coord coord;
    char info[25];
    struct tm discovery_time;
    int status;  // WAITING, ASSIGNED, veya RESCUED
} Survivor;

// Global survivor lists (extern)
extern List *survivors;          // Survivors awaiting help
extern List *helpedsurvivors;    // Helped survivors

// Functions
Survivor *create_survivor(Coord *coord, char *info, struct tm *discovery_time);
void *survivor_generator(void *args);
void *survivor_cleanup(void *arg);
void survivor_finished(int x, int y);
void init_survivor_system();
void cleanup_survivor_system();

#endif