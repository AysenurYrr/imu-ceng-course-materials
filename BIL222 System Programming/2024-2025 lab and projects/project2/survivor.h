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

// Priority levels
#define PRIORITY_LOW            "low"
#define PRIORITY_MEDIUM         "medium"
#define PRIORITY_HIGH           "high"

typedef struct survivor {
    Coord coord;
    char info[25];
    struct tm discovery_time;
    int status;  // WAITING, ASSIGNED, veya RESCUED
    time_t creation_time;  // <— timestamp we’ll set
    char priority[8];      // one of PRIORITY_LOW / _MEDIUM / _HIGH
} Survivor;

// Global survivor lists (extern)
extern List *survivors;          // Survivors awaiting help
extern List *helpedsurvivors;    // Helped survivors

// Functions
Survivor *create_survivor(const Coord *coord,
    const char *info,
    const struct tm *discovery_time,
    const char *priority_str);
void *survivor_generator(void *args);
void *survivor_cleanup(void *arg);
void survivor_finished(int x, int y);
void init_survivor_system();
void cleanup_survivor_system();

#endif