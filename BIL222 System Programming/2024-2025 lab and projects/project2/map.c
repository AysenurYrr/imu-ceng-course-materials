#include "headers/map.h"
#include "headers/list.h"
#include <stdlib.h>
#include <stdio.h>
#include "headers/globals.h"
// Global map instance (defined here, declared extern in map.h)
Map map;

void init_map(int height, int width) {
    map.height = height;
    map.width = width;

    // Tek bir global survivor listesi oluştur
    printf("Map initialized: %dx%d\n", height, width);
}

