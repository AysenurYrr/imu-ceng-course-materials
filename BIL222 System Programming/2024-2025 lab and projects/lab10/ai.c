#include "headers/ai.h"
#include "headers/globals.h"
#include "headers/map.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <limits.h>

// Manhattan mesafesi hesaplama
static int manhattan_distance(Coord a, Coord b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// En yakın boşta olan drone'u bul
static Drone *find_closest_idle_drone(Coord target) {
    Drone *closest = NULL;
    int min_distance = INT_MAX;
    
    pthread_mutex_lock(&drones->lock);
    Node *current = drones->head;
    while (current != NULL) {
        Drone *d = *(Drone **)current->data;
        pthread_mutex_lock(&d->lock);
        
        if (d->status == IDLE) {
            int distance = manhattan_distance(d->coord, target);
            if (distance < min_distance) {
                min_distance = distance;
                closest = d;
            }
        }
        
        pthread_mutex_unlock(&d->lock);
        current = current->next;
    }
    pthread_mutex_unlock(&drones->lock);
    
    return closest;
}

void *ai_controller(void *arg) {
    (void)arg;
    
    while (1) {
        // Survivor listesini kontrol et
        pthread_mutex_lock(&survivors->lock);
        Node *current = survivors->head;
        
        while (current != NULL) {
            Survivor *s = (Survivor *)current->data;
            
            // Sadece bekleyen survivor'ları kontrol et
            if (s->status == WAITING) {
                // En yakın boşta olan drone'u bul
                Drone *closest = find_closest_idle_drone(s->coord);
                
                if (closest) {
                    // Drone'a görev ata
                    assign_mission(closest, s->coord);
                    s->status = ASSIGNED;
                    printf("Survivor %s için Drone %d atandı\n", 
                           s->info, closest->id);
                }
            }
            
            current = current->next;
        }
        
        pthread_mutex_unlock(&survivors->lock);
        usleep(100000);  // 100ms bekle
    }
    
    return NULL;
}