#ifndef MAP_H
#define MAP_H

#include "coord.h"
#include "list.h"
#include "survivor.h"

typedef struct {
    int width;
    int height;
    List *survivors;  // Tek bir global survivor listesi
} Map;

// Global map instance (defined in map.c)
extern Map map;

// Function declarations
void init_map(int height, int width);
void freemap();

#endif