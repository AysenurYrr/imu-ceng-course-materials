#include "headers/drone.h"
#include "headers/globals.h"
#include "headers/map.h"
#include "headers/survivor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

// Global drone fleet
Drone *drone_fleet = NULL;
int num_drones = 10; // Default fleet size

// Global semaphore for drone control
static sem_t drone_control_sem;

void init_drone_system() {
    sem_init(&drone_control_sem, 0, 1);
}

void initialize_drones() {
    printf("[DEBUG] initialize_drones başladı\n");
    drone_fleet = malloc(sizeof(Drone) * num_drones);
    printf("[DEBUG] drone_fleet malloc tamam\n");
    srand(time(NULL));

    for(int i = 0; i < num_drones; i++) {
        printf("[DEBUG] Drone %d başlatılıyor\n", i);
        drone_fleet[i].id = i;
        drone_fleet[i].status = IDLE;
        drone_fleet[i].coord = (Coord){rand() % map.width, rand() % map.height};
        drone_fleet[i].target = drone_fleet[i].coord; // Initial target=current position
        pthread_mutex_init(&drone_fleet[i].lock, NULL);
        printf("[DEBUG] Drone %d struct init tamam\n", i);
        // Add to global drone list (pointer olarak ekle)
        printf("[DEBUG] Drone %d global listeye ekleniyor\n", i);
        Drone *drone_ptr = &drone_fleet[i];
        drones->add(drones, &drone_ptr);  // Pointer'ın adresini gönder
        printf("[DEBUG] Drone %d thread oluşturuluyor\n", i);
        pthread_create(&drone_fleet[i].thread_id, NULL, drone_behavior, &drone_fleet[i]);
        printf("[DEBUG] Drone %d thread başlatıldı\n", i);
    }
    printf("[DEBUG] initialize_drones bitti\n");
}

Drone *create_drone(int id, Coord *coord) {
    Drone *d = malloc(sizeof(Drone));
    if (!d) return NULL;

    memset(d, 0, sizeof(Drone));
    d->id = id;
    d->coord = *coord;
    d->target = *coord;  // Başlangıçta hedef mevcut konum
    d->status = IDLE;
    pthread_mutex_init(&d->lock, NULL);
    return d;
}

void *drone_behavior(void *arg) {
    Drone *d = (Drone *)arg;
    while (1) {
        pthread_mutex_lock(&d->lock);
        
        if (d->status == ON_MISSION) {
            // Hedefe doğru hareket et (her iterasyonda 1 hücre)
            if (d->coord.x < d->target.x) d->coord.x++;
            else if (d->coord.x > d->target.x) d->coord.x--;
            
            if (d->coord.y < d->target.y) d->coord.y++;
            else if (d->coord.y > d->target.y) d->coord.y--;

            // Görev tamamlandı mı kontrol et
            if (d->coord.x == d->target.x && d->coord.y == d->target.y){
                d->status = IDLE;
                //To do: Survivor gloal listesine bakılacak. eşleşen x,y varsa 
                // Survivor status rescued olacak.
                survivor_finished(d->target.x, d->target.y); // Survivor'ı kurtarma işlemi 

                printf("Drone %d: Görev tamamlandı!\n", d->id);
            }
        }
        
        pthread_mutex_unlock(&d->lock);
        usleep(100000);  // 100ms bekle
    }
    return NULL;
}

void assign_mission(Drone *drone, Coord target) {
    pthread_mutex_lock(&drone->lock);
    drone->target = target;
    drone->status = ON_MISSION;
    pthread_mutex_unlock(&drone->lock);
}

void cleanup_drones() {
    for(int i = 0; i < num_drones; i++) {
        pthread_cancel(drone_fleet[i].thread_id);
        pthread_mutex_destroy(&drone_fleet[i].lock);
    }
    free(drone_fleet);
}

void cleanup_drone_system() {
    sem_destroy(&drone_control_sem);
}