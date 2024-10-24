#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <SDL.h>
#include "variables.h"

/* Forward declarations */
void init_game(struct game_state *);
void init_sdl(SDL_Window **, SDL_Renderer **);
void init_assets(struct game_assets *, SDL_Renderer *);
void start_level(struct game_state *);
void run_game_loop(struct game_state *, SDL_Renderer *, struct game_assets *);

void check_input(struct game_state *);
void update_game(struct game_state *);

void check_collision(struct game_state *);
void clear_input(struct game_state *);
void pickup_item(struct game_state *, u8, u8);
void update_dbullet(struct game_state *);
void update_ebullet(struct game_state *);
void verify_input(struct game_state *);
void move_dave(struct game_state *);
void move_monsters(struct game_state *);
void fire_monsters(struct game_state *);
void scroll_screen(struct game_state *);
void apply_gravity(struct game_state *);
void update_level(struct game_state *);
void restart_level(struct game_state *);

void render(struct game_state *, SDL_Renderer *, struct game_assets *);
u8 update_frame(struct game_state *, u8, u8);

void draw_world(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_dave(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_dave_bullet(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_monster_bullet(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_monsters(struct game_state *, struct game_assets *, SDL_Renderer *);
void draw_ui(struct game_state *, struct game_assets *, SDL_Renderer *);

u8 is_clear(struct game_state *, u16, u16, u8);
u8 is_visible(struct game_state *, u16);
void add_score(struct game_state *, u16);

#endif // GAME_H