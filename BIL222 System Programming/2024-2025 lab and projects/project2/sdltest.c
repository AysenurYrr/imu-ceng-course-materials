/*
compiling:
for macos:
    copy SDL frameworks into Library/Frameworks

    then
    gcc sdltest.c -F/Library/Frameworks -framework SDL2

for linux:
    gcc sdltest.c -lSDL2
*/
#include "SDL2/SDL.h"

#define DELAY 3000
/* Sets constants */
#define WIDTH 800
#define HEIGHT 600
int main(int argc, char **argv) {
    /* Initialises data */
    SDL_Window *window = NULL;

    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using
     * SDL_GetError().
     */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    /* Creates a SDL window */
    window =
        SDL_CreateWindow("SDL Example",           /* Title of the SDL window */
                         SDL_WINDOWPOS_UNDEFINED, /* Position x of the window */
                         SDL_WINDOWPOS_UNDEFINED, /* Position y of the window */
                         WIDTH,  /* Width of the window in pixels */
                         HEIGHT, /* Height of the window in pixels */
                         0);     /* Additional flag(s) */

    /* Checks if window has been created; if not, exits program */
    if (window == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n",
                SDL_GetError());
        return 1;
    }

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN) {
                quit = 1;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                quit = 1;
            }
        }
    }
    /* Pauses all SDL subsystems for a variable amount of milliseconds */
    SDL_Delay(DELAY);
    /* Frees memory */
    SDL_DestroyWindow(window);

    /* Shuts down all SDL subsystems */
    SDL_Quit();

    return 0;
}
