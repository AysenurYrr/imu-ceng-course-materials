// controller.c
#include "headers/globals.h"
#include "headers/map.h"
#include "headers/drone.h"
#include "headers/survivor.h"
#include "headers/ai.h"
#include "headers/list.h"
#include "headers/view.h"
#include <stdio.h>
#include <time.h>
List *survivors, *helpedsurvivors, *drones;


int main() {
    printf("[DEBUG] main başladı\n");
    // Initialize global lists
    survivors = create_list(sizeof(Survivor), 1000);     // Survivors waiting for help
    helpedsurvivors = create_list(sizeof(Survivor), 1000); // Helped survivors
    drones = create_list(sizeof(Drone *), 100);            // Active drones (pointer)
    printf("[DEBUG] Listeler oluşturuldu\n");

    // Initialize map (depends on survivors list for cells)
    init_map(40, 30); // Example: 40x30 grid
    printf("[DEBUG] Map oluşturuldu\n");

    // Initialize drones (spawn threads)
    initialize_drones();
    printf("[DEBUG] Dronelar başlatıldı\n");

    srand(time(NULL)); // Rastgelelik için

    // for (int i = 0; i < 6; i++) {
    //     Coord hedef;
    //     // Hedef, mevcut konumdan farklı olsun
    //     do {
    //         hedef.x = rand() % map.width;
    //         hedef.y = rand() % map.height;
    //     } while (hedef.x == drone_fleet[i].coord.x && hedef.y == drone_fleet[i].coord.y);

    //     printf("[TEST] Drone %d için hedef: (%d, %d) (mevcut: %d,%d)\n", 
    //         i, hedef.x, hedef.y, drone_fleet[i].coord.x, drone_fleet[i].coord.y);

    //     assign_mission(&drone_fleet[i], hedef);
    // }

    // Start survivor generator thread
    pthread_t survivor_thread;
    int survivor_thread_result = pthread_create(&survivor_thread, NULL, survivor_generator, NULL);
    printf("[DEBUG] Survivor thread başlatıldı, result=%d\n", survivor_thread_result);

    // Start AI controller thread
    pthread_t ai_thread;
    int ai_thread_result = pthread_create(&ai_thread, NULL, ai_controller, NULL);
    printf("[DEBUG] AI thread başlatıldı, result=%d\n", ai_thread_result);

     // *** NEW: start the cleanup thread ***
     pthread_t cleanup_thread;
     int cleanup_thread_result = pthread_create(&cleanup_thread, NULL,
                                                survivor_cleanup, NULL);
     printf("[DEBUG] Survivor cleanup thread başlatıldı, result=%d\n",
            cleanup_thread_result); 

    // Start SDL visualization
    int sdl_result = init_sdl_window();
    printf("[DEBUG] SDL window başlatıldı, result=%d\n", sdl_result);

    printf("[DEBUG] Ana döngüye giriliyor\n");
    while (!check_events()) {
        draw_map();
        SDL_Delay(100);
    }
    printf("Exiting...\n");
    // Cleanup
    freemap();
    survivors->destroy(survivors);
    helpedsurvivors->destroy(helpedsurvivors);
    drones->destroy(drones);
    quit_all();
    return 0;
}