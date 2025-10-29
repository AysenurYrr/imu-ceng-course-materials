/* ------------------------------------------------------------------
 * server_ai.c  –  AI that assigns missions to idle drones
 *
 *  – No snapshot/array collection.
 *  – Each idle drone is inspected in turn; we lock the survivors list,
 *    search for the best WAITING survivor (priority >> distance),
 *    mark it ASSIGNED, unlock, then send the mission.
 *  – Fine‑grained locking keeps the critical sections short while
 *    guaranteeing consistency.
 * ------------------------------------------------------------------*/

 #include "headers/server_ai.h"
 #include "headers/drone_manager.h"
 #include "headers/survivor.h"
 #include "headers/globals.h"
 #include "headers/protocol.h"
 
 #include <pthread.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <limits.h>
 #include <time.h>
 
 /*––––––––––––––––––––  Helpers  –––––––––––––––––––––––––––––––––––––––––––*/
 static int chebyshev(Coord a, Coord b) {
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return (dx > dy) ? dx : dy;
}
 
 static int priority_rank(const char *p) {
     if (strcmp(p, PRIORITY_HIGH)   == 0) return 0;
     if (strcmp(p, PRIORITY_MEDIUM) == 0) return 1;
     return 2; /* low */
 }
 
 static pthread_t  ai_thread;
 static volatile int ai_running = 0;
 
 /*––––––––––––––––––––  Promotion of old LOWs  ––––––––––––––––––––––––––––*/
 static void promote_old_lows(void)
 {
     const time_t now = time(NULL);
     pthread_mutex_lock(&survivors->lock);
     for (Node *n = survivors->head; n; n = n->next) {
         Survivor *s = (Survivor*)n->data;
         if (s->status == WAITING &&
             strcmp(s->priority, PRIORITY_LOW) == 0 &&
             now - s->creation_time > 20)
         {
             strncpy(s->priority, PRIORITY_MEDIUM, sizeof(s->priority)-1);
             s->priority[sizeof(s->priority)-1] = '\0';
         }
     }
     pthread_mutex_unlock(&survivors->lock);
 }
 
 /*––––––––––––––––––––  Find best WAITING survivor for drone d  ––––––––––*/
 static Survivor *select_best_survivor(ServerDrone *d)
 {
     Survivor *best = NULL;
     int best_rank = 3;            /* 0=high (best) */
     int best_dist = INT_MAX;
 
     for (Node *n = survivors->head; n; n = n->next) {
         Survivor *s = (Survivor*)n->data;
         if (s->status != WAITING) continue;
 
         int rank = priority_rank(s->priority);
         if (rank > best_rank) continue;   /* daha düşük öncelik */
 
         int dist = chebyshev(d->location, s->coord);
         if (rank < best_rank || (rank == best_rank && dist < best_dist)) {
             best_rank = rank;
             best_dist = dist;
             best      = s;
         }
     }
     return best;
 }
 
 /*––––––––––––––––––––  Main AI loop  –––––––––––––––––––––––––––––––––––––*/
 static void *ai_controller(void *arg)
 {
     List *conn = (List*)arg;     /* connected_drones listesi */
 
     while (ai_running) {
         /* 0) Önce eski LOW'ları MEDIUM'a terfi ettir */
         promote_old_lows();
 
         /* 1) Tüm bağlantılı droneları sırayla kontrol et */
         pthread_mutex_lock(&drone_list_mutex);
         for (Node *dn = conn->head; dn; dn = dn->next) {
             ServerDrone *d = *(ServerDrone**)dn->data;
 
             pthread_mutex_lock(&d->lock);
             int is_idle = (strcmp(d->status, DRONE_STATUS_IDLE) == 0 &&
                            strcmp(d->status, DRONE_STATUS_DISCONNECTED) != 0 &&
                            d->location.x >= 0 && d->location.y >= 0);
             pthread_mutex_unlock(&d->lock);
             if (!is_idle) continue;
 
             /* 2) Bu drone için en iyi survivor'ı bul */
             pthread_mutex_lock(&survivors->lock);
             Survivor *target = select_best_survivor(d);
             if (target) target->status = ASSIGNED;  /* optimistically mark */
             pthread_mutex_unlock(&survivors->lock);
 
             if (!target) continue;   /* hiçbir bekleyen kalmadı */
 
             /* 3) Görevi ilet */
             if (assign_mission_to_drone(d, target->coord, target->priority) == 0) {
                 printf("AI: %s ← drone %s  (%d,%d)\n",
                        target->priority, d->drone_id,
                        target->coord.x, target->coord.y);
             } else {
                 /* iletim başarısız → statüyü geri al */
                 pthread_mutex_lock(&survivors->lock);
                 if (target->status == ASSIGNED)
                     target->status = WAITING;
                 pthread_mutex_unlock(&survivors->lock);
             }
         }
         pthread_mutex_unlock(&drone_list_mutex);
 
         /* 4) Heartbeat & kısa uyku */
         send_heartbeats_to_all();
         sleep(2);   /* 100 ms */
     }
     return NULL;
 }
 
 /*––––––––––––––––––––  Start / Stop  –––––––––––––––––––––––––––––––––––––*/
 int start_server_ai(List *drone_list)
 {
     if (ai_running) return -1;
     ai_running = 1;
     if (pthread_create(&ai_thread, NULL, ai_controller, drone_list) != 0) {
         ai_running = 0;
         perror("ai_controller thread");
         return -1;
     }
     printf("Server AI controller started\n");
     return 0;
 }
 
 void stop_server_ai(void)
 {
     if (!ai_running) return;
     ai_running = 0;
     pthread_cancel(ai_thread);
     pthread_join(ai_thread, NULL);
     printf("Server AI controller stopped\n");
 }
 