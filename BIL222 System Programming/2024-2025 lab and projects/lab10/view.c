// #include "headers/view.h"
#include <SDL2/SDL.h>

#include "headers/drone.h"
#include "headers/map.h"
#include "headers/survivor.h"
#include "headers/globals.h"

#define CELL_SIZE 20  // Pixels per map cell

// SDL globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
int window_width, window_height;

// Colors
const SDL_Color BLACK = {0, 0, 0, 255};
const SDL_Color RED = {255, 0, 0, 255};      // Bekleyen survivor'lar
const SDL_Color YELLOW = {255, 255, 0, 255}; // Atanmış survivor'lar
const SDL_Color BLUE = {0, 0, 255, 255};     // Boşta olan drone'lar
const SDL_Color GREEN = {0, 255, 0, 255};    // Görevdeki drone'lar
const SDL_Color WHITE = {255, 255, 255, 255};// Grid çizgileri
const SDL_Color GRAY = {128, 128, 128, 255}; // Drone hedefleri

int init_sdl_window() {
    window_width = map.width * CELL_SIZE;
    window_height = map.height * CELL_SIZE;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window =
        SDL_CreateWindow("Drone Simulator", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, window_width,
                         window_height, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n",
                SDL_GetError());
        SDL_Quit();
        return 1;
    }

    return 0;
}

void draw_cell(int x, int y, SDL_Color color) {
    SDL_Rect rect = {.x = y * CELL_SIZE,
                     .y = x * CELL_SIZE,
                     .w = CELL_SIZE,
                     .h = CELL_SIZE};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b,
                           color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_drones() {
    printf("[DEBUG] draw_drones başladı\n");
    // printf("[DEBUG] drones listesi adresi: %p\n", drones);
    // printf("[DEBUG] drones->head adresi: %p\n", drones->head);
    
    if (drones == NULL || drones->head == NULL) {
        printf("[DEBUG] drones listesi veya head NULL\n");
        return;
    }
    
    Node *current = drones->head;
    int drone_count = 0;
    
    while (current != NULL) {
        // printf("[DEBUG] Drone %d işleniyor, node adresi: %p\n", drone_count, current);
        // printf("[DEBUG] Node data adresi: %p\n", current->data);
        
        if (current->data == NULL) {
            printf("[DEBUG] Node data NULL\n");
            current = current->next;
            continue;
        }
        
        Drone **d_ptr = (Drone **)current->data;  // Pointer'ın pointer'ı
        // printf("[DEBUG] Drone pointer adresi: %p\n", d_ptr);
        
        if (d_ptr == NULL) {
            printf("[DEBUG] Drone pointer NULL\n");
            current = current->next;
            continue;
        }
        
        Drone *d = *d_ptr;  // Pointer'ı al
        // printf("[DEBUG] Drone struct adresi: %p\n", d);
        
        if (d == NULL) {
            printf("[DEBUG] Drone struct NULL\n");
            current = current->next;
            continue;
        }
        
        // Drone'u çiz
        SDL_Color color = (d->status == IDLE) ? BLUE : GREEN;
        draw_cell(d->coord.x, d->coord.y, color);

        // Drone'un hedefini çiz
        if (d->status == ON_MISSION) {
            // Hedef noktasını çiz
            SDL_SetRenderDrawColor(renderer, GRAY.r, GRAY.g, GRAY.b, GRAY.a);
            SDL_Rect target_rect = {
                .x = d->target.y * CELL_SIZE + CELL_SIZE/4,
                .y = d->target.x * CELL_SIZE + CELL_SIZE/4,
                .w = CELL_SIZE/2,
                .h = CELL_SIZE/2
            };
            SDL_RenderFillRect(renderer, &target_rect);
            
            // Drone'dan hedefe çizgi çiz
            SDL_SetRenderDrawColor(renderer, GREEN.r, GREEN.g, GREEN.b, GREEN.a);
            SDL_RenderDrawLine(
                renderer,
                d->coord.y * CELL_SIZE + CELL_SIZE/2,
                d->coord.x * CELL_SIZE + CELL_SIZE/2,
                d->target.y * CELL_SIZE + CELL_SIZE/2,
                d->target.x * CELL_SIZE + CELL_SIZE/2
            );
        }
        
        drone_count++;
        current = current->next;
    }
    printf("[DEBUG] draw_drones bitti, toplam %d drone çizildi\n", drone_count);
}

void draw_survivors() {
    printf("[DEBUG] draw_survivors başladı\n");
    // Global survivor listesini kontrol et
    Node *current = survivors->head;
    while (current != NULL) {
        Survivor *s = (Survivor *)current->data;
        printf("[DEBUG] Survivor çiziliyor: (%d,%d) status=%d\n", s->coord.x, s->coord.y, s->status);
        // Survivor durumuna göre renk seç
        SDL_Color color = (s->status == WAITING) ? RED : YELLOW;
        printf("[DEBUG] Survivor durumu: %d, renk: (%d,%d,%d)\n", s->status, color.r, color.g, color.b);
        draw_cell(s->coord.x, s->coord.y, color);
        current = current->next;
    }
    printf("[DEBUG] draw_survivors bitti\n");
}

void draw_grid() {
    SDL_SetRenderDrawColor(renderer, WHITE.r, WHITE.g, WHITE.b, WHITE.a);
    
    // Yatay çizgiler
    for (int i = 0; i <= map.height; i++) {
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, window_width, i * CELL_SIZE);
    }
    
    // Dikey çizgiler
    for (int j = 0; j <= map.width; j++) {
        SDL_RenderDrawLine(renderer, j * CELL_SIZE, 0, j * CELL_SIZE, window_height);
    }
}

int draw_map() {
    printf("[DEBUG] draw_map çağrıldı\n");
    // Ekranı temizle
    SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, BLACK.a);
    SDL_RenderClear(renderer);

    // Grid'i çiz
    draw_grid();
    
    // Survivor'ları çiz
    draw_survivors();
    
    // Drone'ları çiz
    draw_drones();

    // Ekranı güncelle
    SDL_RenderPresent(renderer);
    return 0;
}

int check_events() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return 1;
        if (event.type == SDL_KEYDOWN &&
            event.key.keysym.sym == SDLK_ESCAPE)
            return 1;
    }
    return 0;
}

void quit_all() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}