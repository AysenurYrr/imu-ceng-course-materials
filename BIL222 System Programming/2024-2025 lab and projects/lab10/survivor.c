#include "headers/survivor.h"
#include "headers/map.h"
#include "headers/globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "headers/list.h"      // for List, Node

// Global survivor list (extern)
extern List *survivors;
extern List *helpedsurvivors;

void *survivor_generator(void *arg) {
    srand(time(NULL));
    
    while (1) {
        // Rastgele koordinat oluştur
        Coord coord = {
            .x = rand() % map.height,
            .y = rand() % map.width
        };
        
        // Yeni survivor oluştur
        Survivor *s = malloc(sizeof(Survivor));
        s->coord = coord;
        s->status = WAITING; // Başlangıçta atanmış
        
        // Global survivor listesine ekle
        survivors->add(survivors, s);
        
        printf("Yeni survivor oluşturuldu: (%d,%d)\n", coord.x, coord.y);
        
        // 1-3 saniye bekle
        sleep(rand() % 2 + 1);
    }
    
    return NULL;
}
void survivor_finished(int x, int y) {
    // Lock the list so nobody mutates while we search
    pthread_mutex_lock(&survivors->lock);

    Node *curr = survivors->head;
    while (curr) {
        Survivor *s = (Survivor*)curr->data;
        if (s->coord.x == x && s->coord.y == y) {
            s->status = RESCUED;
            printf("[DRONE] Survivor at (%d,%d) marked RESCUED\n", x, y);
            break;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&survivors->lock);
}

void *survivor_cleanup(void *arg) {
    while (1) {
        sleep(1);
        pthread_mutex_lock(&survivors->lock);
        Node *curr = survivors->head;
        while (curr) {
            Node *next = curr->next; // Silmeden önce next'i al!
            Survivor *s = (Survivor*)curr->data;
            if (s->status == RESCUED) {
                Survivor rescued = *s;
                pthread_mutex_unlock(&survivors->lock); // --- UNLOCK ET!
                survivors->removedata(survivors, &rescued);
                helpedsurvivors->add(helpedsurvivors, &rescued);
                printf("[CLEANUP] Survivor rescued and moved: (%d, %d)\n", rescued.coord.x, rescued.coord.y);
                pthread_mutex_lock(&survivors->lock); // --- TEKRAR LOCK ET!
            }
            curr = next;
        }
        pthread_mutex_unlock(&survivors->lock);
    }
    return NULL;
}
