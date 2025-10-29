[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Y95fVcDx)
# BIL 222 Lab: Memory Allocator
- [BIL 222 Lab: Memory Allocator](#bil-222-lab-memory-allocator)
- [Lab Assignment: Multithreaded Demon Slayer Game](#lab-assignment-multithreaded-demon-slayer-game)
  - [Key Concepts](#key-concepts)
  - [Objectives](#objectives)
  - [Problem Statement and Overview](#problem-statement-and-overview)
  - [Code Structure Guidelines](#code-structure-guidelines)
  - [Building and Running](#building-and-running)
  - [Assignment Tasks](#assignment-tasks)
    - [1. Core Functionality Implementation](#1-core-functionality-implementation)
    - [2. Visualization Implementation](#2-visualization-implementation)
    - [3. Memory Management](#3-memory-management)
    - [4. Game Balancing](#4-game-balancing)
    - [5. Advanced Features (Optional)](#5-advanced-features-optional)
  - [Submission Requirements](#submission-requirements)
  - [Notes](#notes)
  - [The provided file contents](#the-provided-file-contents)
    - [demonslayer.h](#demonslayerh)
    - [main.c](#mainc)
    - [view.c](#viewc)

---


# Lab Assignment: Multithreaded Demon Slayer Game

## Key Concepts  
- posix threads
- **Synchronization is not included.**

## Objectives
1. Implement multithreaded game entities using POSIX threads
2. Practice managing shared game state without synchronization
3. Work with basic SDL2 visualization
4. Develop different AI strategies for player movement

## Problem Statement and Overview
Implement a game where AI-controlled players and dynamically generated demons compete on a 2D grid. Players gain strength by defeating demons, while demons grow stronger over time and can defeat players.
* **Overall player logic**
  * a player thread uses a given strategy to slay demons.
    * Initial strength can be given randomly or fixed. Your choice.
  * The player can be started at a random position or you can  adjust parameters to get spesific locations.
  * It moves to a new location based on given strategy.
  * If there is a demon at the location and its strength less than the demon, it kills the demon.
    * The demon's strength is added to the player's strength.
    * If the demon is stronger, it is killed by the demon.
* **Overall demon logic**
  * A demon starts at random location with a given initial strength.
  * The longer it lives (stays on the map), the greater its strength.
  * If it kills a player, it gets also the player's strength.

## Code Structure Guidelines
```bash
.
├── Makefile          # Build instructions
├── main.c            # Game logic and thread management
├── view.c            # SDL2 visualization implementation
└── demonslayer.h     # header file
```
## Building and Running
```bash
# Installation
sudo apt install libsdl2-dev
# sudo apt install libsdl2-ttf-dev

# Compilation
gcc -o game main.c view.c -lpthread -lSDL2 -lSDL2_ttf

# Execution
./game
```

## Assignment Tasks
- The provided code is intentionally incomplete and the required tasks are indicated on the code. Below is the summary
### 1. Core Functionality Implementation
- Implement combat resolution in both player and demon threads:
  ```c
  // Sample combat pseudo-code
  if(player->score > demon->score) {
      player->score += demon->score;
      remove_demon(demon);
  } else {
      demon->score += player->score;
      remove_player(player);
  }
  ```
- Add different movement strategies (at least 3):
  - Random walk (implemented)
  - Demon-seeking behavior
  - Strength-based area control
  - Pattern-based movement

### 2. Visualization Implementation
- Complete `update_view()` in view.c:
  - Render players as blue squares with size proportional to strength
  - Render demons as red circles with growing radius
  - Add text rendering for entity names and strengths
  - Implement color intensity based on entity strength

### 3. Memory Management
- Add proper cleanup handlers
- Implement `remove_player()` and `remove_demon()` functions
- Handle array resizing when entities are removed

### 4. Game Balancing
- Adjust strength growth rates
- Implement meaningful initial strength values
- Add game termination conditions

### 5. Advanced Features (Optional)
- Implement player teams with different colors
- Add power-up items that randomly spawn
- Create different demon types with special abilities
- Add sound effects using SDL_mixer

## Submission Requirements
1. Complete all core functionality tasks
2. Submit all source files with your implementations
3. Include a Makefile that builds the project
4. Provide a short video demo (1-2 minutes)
5. Submit a README report explaining your design choices

**Points:** 100 (80 core functionality + 20 visualization)

## Notes

- Global variables are used for simplicity - in real applications you would want better encapsulation
- SDL2 text rendering may require SDL2_ttf library if you use texts on the views
- Remember to handle SDL events for clean termination

This assignment will test your understanding of:
- POSIX thread management
- Resource management in multithreaded environments
- Basic game loop architecture
- Simple graphics programming concepts
- Dynamic data structure management
 
## The provided file contents
### demonslayer.h
```c
#ifndef DEMONSLAYER_H
#define DEMONSLAYER_H

#include <SDL2/SDL.h>
#include <pthread.h>

#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define MAX_PLAYERS 10
#define MAX_DEMONS 20

typedef struct cell {
    int x;
    int y;
} Cell;

typedef struct player {
    char name[15];
    int score;
    Cell location;
} Player;

typedef struct demon {
    char name[15];
    int score;
    Cell location;
} Demon;

// View functions
void init_view();
void update_view(Player **players, int player_count, Demon **demons, int demon_count);
void close_view();
void update_scoreboard(Player **players, int player_count);

// Thread functions
void *player(void *properties);
void *demon(void *properties);
void *player_generator(void *arg);
void *demon_generator(void *arg);

// Utility functions
Cell generate_random_location();
int calculate_strength(int base);

#endif
```

### main.c
```c
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
    // TODO: Sort and display players by score
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

    // TODO: Create generator threads


    // Main game loop
    while (1) {
        update_view(players, player_count, demons, demon_count);
        update_scoreboard(players, player_count);
        usleep(16666);  // ~60 FPS
    }

    close_view();
    return 0;
}
```

### view.c
```c
#include "demonslayer.h"
#include <SDL2/SDL.h> // you can also use SDL3
#include <stdio.h>

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
```
