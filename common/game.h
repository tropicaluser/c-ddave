#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <SDL.h>
#include "variables.h"

/* Forward declarations */
void init_game(struct game_state *);
void init_sdl(SDL_Window **, SDL_Renderer **);
void init_assets(struct game_assets *, SDL_Renderer *);
void run_game_loop(struct game_state *, SDL_Renderer *, struct game_assets *);

void check_input(struct game_state *);
void update_game(struct game_state *);
void render(struct game_state *, SDL_Renderer *, struct game_assets *);

#endif // GAME_H