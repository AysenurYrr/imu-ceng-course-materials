#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "demonslayer.h"

 // Global game state
Player **players;
Demon **demons;
int demon_count = 0;
int player_count = 0;

void *player(void *properties) {
    Player *p = (Player *)properties;
    // Implement player logic:
    // - Movement strategy (you can add this to struct player)
    // - Combat resolution
    // - State updates
    return NULL;
}

void *demon(void *properties) {
    Demon *d = (Demon *)properties;
    // Implement demon logic:
    // - Strength growth
    // - Combat resolution
    // - State updates
    return NULL;
}

void *player_generator(void *arg) {
    // Implement player spawning:
    // - Creation interval
    // - Initial properties setup
    // - Thread creation
    return NULL;
}

void *demon_generator(void *arg) {
    // Implement demon spawning:
    // - Creation interval
    // - Initial properties setup
    // - Thread creation
    return NULL;
}

Cell generate_random_location() {
    // Implement random coordinate generation
}

int calculate_strength(int base) {
    // Implement strength calculation formula
}
void update_scoreboard(Player **players, int player_count) {
    // Sort and display players by score
    printf("\n=== Scoreboard ===\n");
    for (int i = 0; i < player_count; i++) {
        printf("%s: %d\n", players[i]->name, players[i]->score);
    }
    printf("==================\n");
}
int main() {
    srand(time(NULL));

    // Initialize game state
    players = malloc(sizeof(Player *) * MAX_PLAYERS);
    demons = malloc(sizeof(Demon *) * MAX_DEMONS);

    init_view();

    // Create generator threads
    pthread_t pg_tid, dg_tid;
    pthread_create(&pg_tid, NULL, player_generator, NULL);
    pthread_create(&dg_tid, NULL, demon_generator, NULL);

    // Main game loop
    while (1) {
        update_view(players, player_count, demons, demon_count);
        update_scoreboard(players, player_count);
        usleep(16666);  // ~60 FPS
    }

    close_view();
    return 0;
}
