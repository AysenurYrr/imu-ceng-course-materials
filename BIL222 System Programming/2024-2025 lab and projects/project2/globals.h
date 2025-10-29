/**
 * @file globals.h
 * @brief Global değişkenler ve yapılar için tanımlamalar
 */
#ifndef GLOBALS_H
#define GLOBALS_H

#include "map.h"
#include "drone.h"
#include "survivor.h"
#include "list.h"
#include "coord.h"

// Harita boyutları
#define MAP_HEIGHT 40
#define MAP_WIDTH 30

extern volatile int server_should_exit;
extern List *survivors;       // Kurtarılmayı bekleyen hayatta kalanlar
extern List *helpedsurvivors; // Kurtarılmış hayatta kalanlar
extern List *drones;          // Aktif drone'lar

extern pthread_mutex_t drone_list_mutex;

#endif