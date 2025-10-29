
#ifndef VIEW_H
#define VIEW_H
#include <SDL2/SDL.h>
/*view.c*/
extern int init_sdl_window();
extern void draw_cell(int x, int y, SDL_Color color);
extern void draw_drones();
extern void draw_survivors();
extern void draw_grid();
extern int draw_map();
extern int check_events();
extern int quit_all();

#endif