#include "headers/survivor.h"
#include "headers/map.h"
#include "headers/globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "headers/list.h"      // for List, Node
#include <string.h>
#define DEBUG_PRINT 0
// Global survivor list (extern)
extern List *survivors;
extern List *helpedsurvivors;


Survivor *create_survivor(const Coord *coord,
    const char *info,
    const struct tm *discovery_time,
    const char *priority_str)
{
Survivor *s = malloc(sizeof(*s));
if (!s) return NULL;

// copy basic fields
s->coord = *coord;
strncpy(s->info, info, sizeof(s->info)-1);
s->info[sizeof(s->info)-1] = '\0';
s->discovery_time = *discovery_time;
s->status = WAITING;

s->creation_time = time(NULL);

// copy the priority string
strncpy(s->priority, priority_str, sizeof(s->priority)-1);
s->priority[sizeof(s->priority)-1] = '\0';

return s;
}

void *survivor_generator(void *arg) {
    // Example: randomly pick string priorities
    const char *prio_array[3] = {
        PRIORITY_LOW,
        PRIORITY_MEDIUM,
        PRIORITY_HIGH
    };

    srand(time(NULL));
    while (1) {
        Coord coord = { rand() % map.height, rand() % map.width };
        struct tm now_tm;
        time_t now = time(NULL);
        localtime_r(&now, &now_tm);
        // Example: random priority (you might base this on real data!)

        int r = rand() % 100;  
        int idx;
        if (r < 95) {
            idx = 0;         // LOW 
        } else {
            idx = 2;         // HIGH (kritik)
        }


        Survivor *s = create_survivor(&coord, "auto-gen", &now_tm, prio_array[idx]);
        survivors->add(survivors, s);
        usleep(500000); // 500ms bekle
        //sleep(1);
    }
    return NULL;
}

void survivor_finished(int x, int y) {
    if (DEBUG_PRINT) {
    printf("[DEBUG] survivor_finished çağrıldı: (%d,%d)\n", x, y);
    }
    // Lock the list so nobody mutates while we search
    pthread_mutex_lock(&survivors->lock);
    if (DEBUG_PRINT) {
    printf("[DEBUG] survivors listesi kilitlendi, eleman sayısı: %d\n", survivors->number_of_elements);
    }
    
    Node *curr = survivors->head;
    int found = 0;
    while (curr) {
        Survivor *s = (Survivor*)curr->data;
        if (DEBUG_PRINT) {
        printf("[DEBUG] Survivor kontrol ediliyor: (%d,%d) status=%d\n", s->coord.x, s->coord.y, s->status);
        }
        
        if (s->coord.x == x && s->coord.y == y) {
            s->status = RESCUED;
            if (DEBUG_PRINT) {
            printf("[DRONE] Survivor at (%d,%d) marked RESCUED\n", x, y);
            }
            found = 1;
            break;
        }
        curr = curr->next;
    }

    if (!found) {
        if (DEBUG_PRINT) {
        printf("[DEBUG] Belirtilen konumda (%d,%d) survivor bulunamadı!\n", x, y);
        }
    }
    
    pthread_mutex_unlock(&survivors->lock);
    if (DEBUG_PRINT) {
    printf("[DEBUG] survivors listesi kilidi açıldı, eleman sayısı: %d\n", survivors->number_of_elements);
    }
}

void *survivor_cleanup(void *arg) {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&survivors->lock);
        
        if (DEBUG_PRINT) {
        printf("[DEBUG] survivor_cleanup çalışıyor, survivors sayısı: %d\n", survivors->number_of_elements);
        }
        
        Node *curr = survivors->head;
        int rescued_count = 0;
        
        while (curr) {
            Node *next = curr->next; // Silmeden önce next'i al!
            Survivor *s = (Survivor*)curr->data;
            if (DEBUG_PRINT) {
            printf("[DEBUG] Survivor kontrol ediliyor: (%d,%d) status=%d\n", s->coord.x, s->coord.y, s->status);
            }
            
            if (s->status == RESCUED) {
                rescued_count++;
                Survivor rescued = *s;
                pthread_mutex_unlock(&survivors->lock); // --- UNLOCK ET!
                survivors->removedata(survivors, &rescued);
                helpedsurvivors->add(helpedsurvivors, &rescued);
                printf("[CLEANUP] Survivor rescued and moved: (%d, %d)\n", rescued.coord.x, rescued.coord.y);
                pthread_mutex_lock(&survivors->lock); // --- TEKRAR LOCK ET!
            }
            curr = next;
        }
        
        if (rescued_count > 0) {
            if (DEBUG_PRINT) {
            printf("[DEBUG] %d adet kurtarılmış survivor temizlendi\n", rescued_count);
            printf("[CLEANUP] survivor_cleanup bitti, survivors sayısı: %d\n", survivors->number_of_elements);
            }
        }
        
        pthread_mutex_unlock(&survivors->lock);
    }
    return NULL;
}
