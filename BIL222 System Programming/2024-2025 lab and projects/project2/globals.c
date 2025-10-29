#include "headers/globals.h"
#include "headers/map.h"

// Global listelerin tanımları
List *survivors = NULL;       // Kurtarılmayı bekleyen hayatta kalanlar
List *helpedsurvivors = NULL; // Kurtarılmış hayatta kalanlar
List *drones = NULL;          // Aktif drone'lar