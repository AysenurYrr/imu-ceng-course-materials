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
    map.survivors = survivors;  // 1000 survivor kapasitesi

    printf("Map initialized: %dx%d\n", height, width);
}

void freemap() {
    // Global survivor listesini temizle
    if (map.survivors) {
        map.survivors->destroy(map.survivors);
    }
    printf("Map destroyed\n");
}