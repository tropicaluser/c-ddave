#include <stdlib.h>
#include <string.h>
#include "game.h"

#ifdef __cplusplus
#include <fstream>
#include <iostream>
using namespace std;
#endif

void init_sdl(SDL_Window **window, SDL_Renderer **renderer)
{
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO))
    SDL_Log("SDL error: %s", SDL_GetError());

  // Create a window and renderer
  if (SDL_CreateWindowAndRenderer(320 * DISPLAY_SCALE, 200 * DISPLAY_SCALE, 0, window, renderer))
    SDL_Log("Window/Renderer error: %s", SDL_GetError());

  /* Easy onversion between original world (320x200) and current screen size */
  SDL_RenderSetScale(*renderer, DISPLAY_SCALE, DISPLAY_SCALE);

  /* Clear screen */
  SDL_SetRenderDrawColor(*renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(*renderer);
}

void run_game_loop(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets)
{
  u32 timer_begin;
  u32 timer_end;
  u32 delay;

  while (!game->quit)
  {
    timer_begin = SDL_GetTicks();

    check_input(game);
    update_game(game);
    render(game, renderer, assets);

    timer_end = SDL_GetTicks();
    delay = 33 - (timer_end - timer_begin);
    delay = delay > 33 ? 0 : delay;
    SDL_Delay(delay);
  }
}

/* Set game and monster properties to default values */
void init_game(struct game_state *game)
{
  char fname[13];

  game->quit = 0;
  game->current_level = 0;
  game->view_x = 0;
  game->view_y = 0;
  game->scroll_x = 0;

  /* Load each level from level<xxx>.dat. (see LEVEL.c utility) */
  for (int j = 0; j < 10; j++)
  {
    /* Make new file name */
    snprintf(fname, sizeof(fname), "level%d.dat", j);

#ifdef __cplusplus
    // C++ File Handling
    ifstream file_level(fname, ios::binary);
    if (!file_level)
    {
      cerr << "Failed to open " << fname << endl;
      continue;
    }

    // Read data into game struct
    file_level.read(reinterpret_cast<char *>(game->level[j].path), sizeof(game->level[j].path));
    file_level.read(reinterpret_cast<char *>(game->level[j].tiles), sizeof(game->level[j].tiles));
    file_level.read(reinterpret_cast<char *>(game->level[j].padding), sizeof(game->level[j].padding));

#else
    // C File Handling
    FILE *file_level = fopen(fname, "rb");
    if (!file_level)
    {
      fprintf(stderr, "Failed to open %s\n", fname);
      continue;
    }

    // Stream bytes into game struct
    fread(game->level[j].path, sizeof(game->level[j].path), 1, file_level);
    fread(game->level[j].tiles, sizeof(game->level[j].tiles), 1, file_level);
    fread(game->level[j].padding, sizeof(game->level[j].padding), 1, file_level);

    fclose(file_level);
#endif
  }
}

void init_assets(struct game_assets *assets, SDL_Renderer *renderer)
{
  int i;
  char fname[13];
  char file_num[4];

  for (i = 0; i < 158; i++)
  {
    fname[0] = '\0';
    strcat(fname, "tile");
    sprintf(&file_num[0], "%u", i);
    strcat(fname, file_num);
    strcat(fname, ".bmp");

    // Check if the file exists before trying to load it
    assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(fname));
  };
}

void check_input(struct game_state *game)
{
  SDL_Event event;
  SDL_PollEvent(&event);
  {
    if (event.key.keysym.sym == SDLK_RIGHT)
      game->scroll_x = 15;
    if (event.key.keysym.sym == SDLK_LEFT)
      game->scroll_x = -15;
    if (event.key.keysym.sym == SDLK_DOWN)
      game->current_level++;
    if (event.key.keysym.sym == SDLK_UP)
      game->current_level--;
  }
}

void update_game(struct game_state *game)
{
  // overflow check
  if (game->current_level == 0xFF)
    game->current_level = 0;

  if (game->current_level > 9)
    game->current_level = 9;

  if (game->scroll_x > 0)
  {
    if (game->view_x == 80)
      game->view_x = 0;
    else
    {
      game->view_x++;
      game->scroll_x--;
    }
  }

  if (game->scroll_x < 0)
  {
    if (game->view_x == 80)
      game->view_x = 0;
    else
    {
      game->view_x--;
      game->scroll_x++;
    }
  }
}

void render(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets)
{
  SDL_Rect dest;
  dest.w = 16;
  dest.h = 16;

  for (int j = 0; j < 10; j++)
  {
    dest.y = j * 16;

    for (int i = 0; i < 20; i++)
    {
      dest.x = i * 16;
      uint8_t tile_index = game->level[game->current_level].tiles[j * 100 + game->view_x + i];
      SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
    }
  }

  SDL_RenderPresent(renderer);
}
