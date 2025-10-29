#include <SDL2/SDL.h>

#include "headers/drone.h"
#include "headers/map.h"
#include "headers/survivor.h"
#include "headers/globals.h"
#include "headers/view.h"
#define DEBUG_PRINT 0
#define CELL_SIZE 20  // Pixels per map cell

// SDL globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
int window_width, window_height;

// Color struct alias
typedef SDL_Color Color;

// Colors
const Color BLACK      = {  0,   0,   0, 255 };
const Color WHITE      = {255, 255, 255, 255 };
const Color BLUE       = {  0,   0, 255, 255 };
const Color GREEN      = {  0, 255,   0, 255 };
// Survivor priority reds
const Color DARK_RED   = {150,   0,   0, 255}; // high priority
const Color MEDIUM_RED = {200,   0,   0, 255}; // medium priority
const Color LIGHT_RED  = {255, 100, 100,255}; // low priority
const Color GRAY       = {128, 128, 128,255};

int init_sdl_window() {
    window_width  = map.width  * CELL_SIZE;
    window_height = map.height * CELL_SIZE;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow(
        "Drone Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    return 0;
}

void draw_cell(int x, int y, Color color) {
    SDL_Rect rect = {
        .x = y * CELL_SIZE,
        .y = x * CELL_SIZE,
        .w = CELL_SIZE,
        .h = CELL_SIZE
    };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_drones() {
    if (!drones || !drones->head) return;
    Node *current = drones->head;
    while (current) {
        Drone **d_ptr = (Drone **)current->data;
        if (d_ptr) {
            Drone *d = *d_ptr;
            if (d) {
                Color color = (d->status == IDLE) ? BLUE : GREEN;
                draw_cell(d->coord.x, d->coord.y, color);

                if (d->status == ON_MISSION) {
                    // Draw target square
                    SDL_Rect target_rect = {
                        .x = d->target.y * CELL_SIZE + CELL_SIZE/4,
                        .y = d->target.x * CELL_SIZE + CELL_SIZE/4,
                        .w = CELL_SIZE/2,
                        .h = CELL_SIZE/2
                    };
                    SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, GRAY.a);
                    SDL_RenderFillRect(renderer, &target_rect);

                    // Draw line from drone to target
                    SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
                    SDL_RenderDrawLine(
                        renderer,
                        d->coord.y * CELL_SIZE + CELL_SIZE/2,
                        d->coord.x * CELL_SIZE + CELL_SIZE/2,
                        d->target.y * CELL_SIZE + CELL_SIZE/2,
                        d->target.x * CELL_SIZE + CELL_SIZE/2
                    );
                }
            }
        }
        current = current->next;
    }
}

void draw_survivors() {
    if (!survivors) return;

    // 1) Snapshot: en fazla 128 survivor  
    Survivor snap[128];
    int cnt = 0;

    pthread_mutex_lock(&survivors->lock);
    for (Node *n = survivors->head; n && cnt < 128; n = n->next) {
        Survivor *s = (Survivor*)n->data;
        if (s->status == WAITING || s->status == ASSIGNED) {
            // coord, status, priority alanlarını kopyala
            snap[cnt].coord  = s->coord;
            snap[cnt].status = s->status;
            strncpy(snap[cnt].priority,
                    s->priority,
                    sizeof(snap[cnt].priority));
            cnt++;
        }
    }
    pthread_mutex_unlock(&survivors->lock);

    // 2) Kilitsiz olarak çiz
    for (int i = 0; i < cnt; i++) {
        Survivor *s = &snap[i];
        Color color;
        if (s->status == WAITING) {
            if      (!strcmp(s->priority, PRIORITY_HIGH))   color = DARK_RED;
            else if (!strcmp(s->priority, PRIORITY_MEDIUM)) color = MEDIUM_RED;
            else                                            color = LIGHT_RED;
        } 
        draw_cell(s->coord.x, s->coord.y, color);
    }
}


void draw_grid() {
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    for (int i = 0; i <= map.height; i++) {
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, window_width, i * CELL_SIZE);
    }
    for (int j = 0; j <= map.width; j++) {
        SDL_RenderDrawLine(renderer, j * CELL_SIZE, 0, j * CELL_SIZE, window_height);
    }
}

int draw_map() {
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);

    draw_grid();
    draw_survivors();
    draw_drones();

    SDL_RenderPresent(renderer);
    return 0;
}

int check_events() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return 1;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return 1;
    }
    return 0;
}

int quit_all() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
