#include <SDL2/SDL.h> /* you can also use SDL3 */
#include <stdio.h>
#include "demonslayer.h"

#define CELL_SIZE 30

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void init_view() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Demon Slayer",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              800, 600,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                      SDL_RENDERER_PRESENTVSYNC);
}

void update_view(Player **players, int player_count,
                 Demon **demons, int demon_count) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Draw grid
    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    for (int x = 0; x <= 800; x += CELL_SIZE) {
        SDL_RenderDrawLine(renderer, x, 0, x, 600);
    }
    for (int y = 0; y <= 600; y += CELL_SIZE) {
        SDL_RenderDrawLine(renderer, 0, y, 800, y);
    }

    // TODO: Implement entity rendering
    // Should draw players and demons with:
    // - Different colors
    // - Strength-based size/color intensity
    // - Names/strength text

    SDL_RenderPresent(renderer);
}

void close_view() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
