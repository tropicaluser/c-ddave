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

void check_collision(struct game_state *);
void verify_input(struct game_state *);
void move_dave(struct game_state *);
void scroll_screen(struct game_state *);
void apply_gravity(struct game_state *);
void clear_input(struct game_state *);

void render(struct game_state *, SDL_Renderer *, struct game_assets *);

void draw_world(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_dave(struct game_state *, struct game_assets *, SDL_Renderer *);

u8 is_clear(struct game_state *, u16, u16);

#endif // GAME_H