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
  const u32 FRAME_DELAY = 33; // Frame delay in milliseconds
  u32 timer_begin;
  u32 timer_end;
  u32 delay;

  /* Game loop with fixed time step at 30 FPS*/
  while (!game->quit)
  {
    timer_begin = SDL_GetTicks();

    check_input(game);
    update_game(game);
    render(game, renderer, assets);

    timer_end = SDL_GetTicks();
    delay = FRAME_DELAY - (timer_end - timer_begin);
    delay = delay > FRAME_DELAY ? 0 : delay;
    SDL_Delay(delay);
  }
}

/* Set game and monster properties to default values */
void init_game(struct game_state *game)
{
  int j;
  char fname[13];

  // Initialize game state variables
  game->quit = 0;
  game->current_level = 0;
  game->lives = 3;
  game->view_x = 0;
  game->view_y = 0;
  game->scroll_x = 0;
  game->dave_x = 2;
  game->dave_y = 8;
  game->dave_px = game->dave_x * TILE_SIZE;
  game->dave_py = game->dave_y * TILE_SIZE;
  game->jump_timer = 0;
  game->on_ground = 1;
  game->try_right = 0;
  game->try_left = 0;
  game->try_jump = 0;
  game->dave_right = 0;
  game->dave_left = 0;
  game->dave_jump = 0;
  game->dave_up = 0;
  game->dave_down = 0;
  game->dave_jetpack = 0;

  /* Deactivate all monsters */
  for (j = 0; j < 5; j++)
    game->monster[j].type = 0;
  
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

/* Bring in tileset from tile<xxx>.bmp files from original binary (see TILES.C)*/
void init_assets(struct game_assets *assets, SDL_Renderer *renderer)
{
  int i, j;
  char fname[13];
  char file_num[4];
  char mname[13];
  char mask_num[4];
  SDL_Surface *surface;
  SDL_Surface *mask;
  uint8_t *surf_p;
  uint8_t *mask_p;
  uint8_t mask_offset;

  for (i = 0; i < 158; i++)
  {
    fname[0] = '\0';
    strcat(fname, "tile");
    sprintf(&file_num[0], "%u", i);
    strcat(fname, file_num);
    strcat(fname, ".bmp");

    if ((i >= 53 && i <= 59) || i == 67 || i == 68 || (i >= 71 && i <= 73) || (i >= 77 && i <= 82))
    {
      if (i >= 53 && i <= 59)
        mask_offset = 7;
      if (i >= 67 && i <= 68)
        mask_offset = 2;
      if (i >= 71 && i <= 73)
        mask_offset = 3;
      if (i >= 77 && i <= 82)
        mask_offset = 6;

      mname[0] = '\0';
      strcat(mname, "tile");
      sprintf(&mask_num[0], "%u", i + mask_offset);
      strcat(mname, mask_num);
      strcat(mname, ".bmp");

      surface = SDL_LoadBMP(fname);
      mask = SDL_LoadBMP(mname);

      surf_p = (uint8_t *)surface->pixels;
      mask_p = (uint8_t *)mask->pixels;

      /* Write mask white background to dave tile */
      for (j = 0; j < (mask->pitch * mask->h); j++)
        surf_p[j] = mask_p[j] ? 0xFF : surf_p[j];

      /* Make white mask transparent */
      SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));
      assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);
      SDL_FreeSurface(surface);
      SDL_FreeSurface(mask);
    }
    else
    {
      surface = SDL_LoadBMP(fname);

      /* Monster tiles should use black transparency */
      if ((i >= 89 && i <= 120) || (i >= 129 && i <= 132))
        SDL_SetColorKey(surface, 1, SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));
      assets->graphics_tiles[i] = SDL_CreateTextureFromSurface(renderer, surface);
    }
  };
}

/* Checks input and sets flags. First step of the game loop */
void check_input(struct game_state *game)
{
  SDL_Event event;
  SDL_PollEvent(&event);

  const u8 *keystate = SDL_GetKeyboardState(NULL);
  if (keystate[SDL_SCANCODE_RIGHT])
    game->try_right = 1;
  if (keystate[SDL_SCANCODE_LEFT])
    game->try_left = 1;
  if (keystate[SDL_SCANCODE_UP])
    game->try_jump = 1;
  if (keystate[SDL_SCANCODE_DOWN])
    game->try_down = 1;
  if (keystate[SDL_SCANCODE_LCTRL])
    game->try_fire = 1;
  if (keystate[SDL_SCANCODE_LALT])
    game->try_jetpack = 1;

  if (event.type == SDL_QUIT)
    game->quit = 1;
}

/* Updates world, entities, and handles input flags .
   Second step of the game loop */
void update_game(struct game_state *game)
{
  check_collision(game);
  pickup_item(game, game->check_pickup_x, game->check_pickup_y);
  update_dbullet(game);
  update_ebullet(game);
  verify_input(game);
  move_dave(game);
  move_monsters(game);
  fire_monsters(game);
  scroll_screen(game);
  apply_gravity(game);
  update_level(game);
  clear_input(game);
}

/* Renders the world. First step of the game loop */
void render(struct game_state *game, SDL_Renderer *renderer, struct game_assets *assets)
{
  /* Clear back buffer with black */
  SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
  SDL_RenderClear(renderer);

  /* Draw world elements */
  draw_world(game, assets, renderer);
  draw_dave(game, assets, renderer);
  draw_monsters(game, assets, renderer);
  draw_dave_bullet(game, assets, renderer);
  draw_monster_bullet(game, assets, renderer);

  /* Swaps display buffers (puts above drawing on the screen)*/
  SDL_RenderPresent(renderer);
}

/* Updates dave's collision point state */
void check_collision(struct game_state *game)
{
  /* Updates 8 points around Dave */
  game->collision_point[0] = is_clear(game, game->dave_px + 4, game->dave_py - 1, 1);
  game->collision_point[1] = is_clear(game, game->dave_px + 10, game->dave_py - 1, 1);
  game->collision_point[2] = is_clear(game, game->dave_px + 11, game->dave_py + 4, 1);
  game->collision_point[3] = is_clear(game, game->dave_px + 11, game->dave_py + 12, 1);
  game->collision_point[4] = is_clear(game, game->dave_px + 10, game->dave_py + 16, 1);
  game->collision_point[5] = is_clear(game, game->dave_px + 4, game->dave_py + 16, 1);
  game->collision_point[6] = is_clear(game, game->dave_px + 3, game->dave_py + 12, 1);
  game->collision_point[7] = is_clear(game, game->dave_px + 3, game->dave_py + 4, 1);

  /* Is dave on the ground? */
  game->on_ground = (!game->collision_point[4] && !game->collision_point[5]);
}

/* Clear flags set by keyboard input */
void clear_input(struct game_state *game)
{
  game->try_jump = 0;
  game->try_right = 0;
  game->try_left = 0;
  game->try_fire = 0;
  game->try_jetpack = 0;
  game->try_down = 0;
  game->try_up = 0;
}

void pickup_item(struct game_state *game, u8 grid_x, u8 grid_y)
{
	u8 type;

	/* No pickups outside of the world (or you'll lbe eaten by the grue) */
	if (!grid_x || !grid_y)
		return;

	/* Get the type */
	type = game->level[game->current_level].tiles[grid_y * 100 + grid_x];

	/* Handle the type */
	switch (type)
	{
	  /* Jetpack pickup */
		case 4: game->jetpack = 0xFF; break;
		/* Trophy pickup */
		case 10:
		{
      game->score += 1000;
			game->trophy = 1;
		} break;
		/* Gun pickup */
		case 20: game->gun = 1; break;
		default: break;
	}

	/* Clear the pickup tile */
	game->level[game->current_level].tiles[grid_y * 100 + grid_x] = 0;

	/* Clear the pickup handler */
	game->check_pickup_x = 0;
	game->check_pickup_y = 0;
}

/* Move Dave's bullets */
void update_dbullet(struct game_state *game)
{
  u8 i, grid_x, grid_y, mx, my;

  /* No bullet in world - Not active */
  if (!game->dbullet_px || !game->dbullet_py)
    return;

  game->dbullet_px += game->dbullet_dir * 4;

  /* Bullet hit something - deactivate */
  if (!is_clear(game, game->dbullet_px, game->dbullet_py, 0))
    game->dbullet_px = game->dbullet_py = 0;

  grid_x = game->dbullet_px / TILE_SIZE;
  grid_y = game->dbullet_py / TILE_SIZE;

  /* Bullet left room - deactivate */
  if (grid_x - game->view_x < 1 || grid_x - game->view_x > 20)
    game->dbullet_px = game->dbullet_py = 0;

    /* Bullet active */
  if (game->dbullet_px)
  {
    game->dbullet_px += game->dbullet_dir * 4;

    /* Check all monster positions */
    for (i = 0; i < 5; i++)
    {
      if (game->monster[i].type)
      {
        mx = game->monster[i].monster_x;
        my = game->monster[i].monster_y;

        if ((grid_y == my || grid_y == my + 1) && (grid_x == mx || grid_x == mx + 1))
        {
          /* Dave's bullet hits monster - destroy bullet and monster */
          game->dbullet_px = game->dbullet_py = 0;
          game->monster[i].dead_timer = 30;
        }
      }
    }
  }
}

/* Move Monster's bullets */
void update_ebullet(struct game_state *game)
{
  /* No bullet in world - Not active */
  if (!game->ebullet_px || !game->ebullet_py)
    return;

  /* Bullet hit something - deactivate */
  if (!is_clear(game, game->ebullet_px, game->ebullet_py, 0))
    game->ebullet_px = game->ebullet_py = 0;

  /* Bullet left room - deactivate */
  if (!is_visible(game, game->ebullet_px))
    game->ebullet_px = game->ebullet_py = 0;

  if (game->ebullet_px)
  {
    u8 grid_x, grid_y;
    game->ebullet_px += game->ebullet_dir * 4;

    grid_x = game->ebullet_px / TILE_SIZE;
    grid_y = game->ebullet_py / TILE_SIZE;

    /* Compare with Dave's position */
    if (grid_y == game->dave_y && grid_x == game->dave_x)
    {
      /* Monster's bullet hits Dave - remove bullet & dave dead */
      game->ebullet_px = game->ebullet_py = 0;
      game->dave_dead_timer = 30;
    }
  }
}

/* Start a new level */
void start_level(struct game_state *game)
{
  u8 i;

  /* Reset dave position */
  restart_level(game);

  /* Deactivate monsters */
  for (i = 0; i < 5; i++)
  {
    game->monster[i].type = 0;
  }

  switch (game->current_level)
  {
  case 2:
  {
    game->monster[0].type = 89;
    game->monster[0].path_index = 0;
    game->monster[0].dead_timer = 0;
    game->monster[0].monster_px = 44 * TILE_SIZE;
    game->monster[0].monster_py = 4 * TILE_SIZE;
    game->monster[0].next_px = 0;
    game->monster[0].next_py = 0;

    game->monster[1].type = 89;
    game->monster[1].path_index = 0;
    game->monster[1].dead_timer = 0;
    game->monster[1].monster_px = 59 * TILE_SIZE;
    game->monster[1].monster_py = 4 * TILE_SIZE;
    game->monster[1].next_px = 0;
    game->monster[1].next_py = 0;
  }
  break;
  case 3:
  {
    game->monster[0].type = 93;
    game->monster[0].path_index = 0;
    game->monster[0].dead_timer = 0;
    game->monster[0].monster_px = 32 * TILE_SIZE;
    game->monster[0].monster_py = 2 * TILE_SIZE;
    game->monster[0].next_px = 0;
    game->monster[0].next_py = 0;
  }
  default:
    break;
  }

  /* Sets Dave start position in a level */
  game->dave_px = game->dave_x * TILE_SIZE;
  game->dave_py = game->dave_y * TILE_SIZE;

  /* Reset various state variables at the start of each level */
  game->dave_jetpack = 0;
  game->dave_fire = 0;
  game->trophy = 0;
  game->gun = 0;
  game->jetpack = 0;
  game->dave_dead_timer = 0;
  game->check_door = 0;
  game->view_x = 0;
  game->view_y = 0;
  game->jump_timer = 0;
  game->last_dir = 0;
  game->dbullet_px = 0;
  game->dbullet_py = 0;
  game->ebullet_px = 0;
  game->ebullet_py = 0;
}

/* Check if keyboard input is valid. If so, set action variable */
void verify_input(struct game_state *game)
{
   /* Dave is dead. No input is valid */
  if (game->dave_dead_timer)
    return;
  
  /* Dave can move right if there are no obstructions */
  if (game->try_right && game->collision_point[2] && game->collision_point[3])
    game->dave_right = 1;

  /* Dave can move left if there are no obstructions */
  if (game->try_left && game->collision_point[6] && game->collision_point[7])
    game->dave_left = 1;

  /* Dave can jump if he's on the ground and not using the jeypack, also no double-jumping*/
  if (game->try_jump && game->on_ground && !game->dave_jump && !game->dave_jetpack && game->collision_point[0] && game->collision_point[1])
    game->dave_jump = 1;

    /* Dave and fire if he has the gun and isn't already firing */
  if (game->try_fire && game->gun && !game->dbullet_py && !game->dbullet_px)
    game->dave_fire = 1;

  /* Dave can toggle the jetpack if he has one and he didn't recently toggle it */
  if (game->try_jetpack && game->jetpack && !game->jetpack_delay)
  {
    game->dave_jetpack = !game->dave_jetpack;
    game->jetpack_delay = 10;
  }

  /* Dave can move downward if he is climbing or has a jetpack */
  if (game->try_down && game->dave_jetpack && game->collision_point[4] && game->collision_point[5])
    game->dave_down = 1;

  /* Dave can move up if he has jetpack */
  if (game->try_jump && game->dave_jetpack && game->collision_point[0] && game->collision_point[1])
    game->dave_up = 1;
}

/* Move dave around the world */
void move_dave(struct game_state *game)
{
  game->dave_x = game->dave_px / TILE_SIZE;
  game->dave_y = game->dave_py / TILE_SIZE;

  /* Move Dave right */
  if (game->dave_right)
  {
    game->dave_px += 2;
    game->last_dir = 1;
    game->dave_right = 0;
  }

  /* Move Dave left */
  if (game->dave_left)
  {
    game->dave_px -= 2;
    game->last_dir = -1;
    game->dave_left = 0;
  }

  /* Move Dave down */
  if (game->dave_down)
  {
    game->dave_py += 2;
    game->dave_down = 0;
  }

  /* Move Dave up */
  if (game->dave_up)
  {
    game->dave_py -= 2;
    game->dave_up = 0;
  }

  /* Make Dave jump */
  if (game->dave_jump)
  {
    if (!game->jump_timer)
    {
      game->jump_timer = 30;
      // return to neutral position after jump
      game->last_dir = 0;
    }

    /* if space above dave is clear */
    if (game->collision_point[0] && game->collision_point[1])
    {
      /* Dave should move up at a decreasing rate, then float for a moment */
      if (game->jump_timer > 16)
        game->dave_py -= 2;

      if (game->jump_timer >= 12 && game->jump_timer <= 15)
        game->dave_py -= 1;
    }

    game->jump_timer--;

    /* Stop jump if timer is zero */
    if (!game->jump_timer)
      game->dave_jump = 0;
  }

  /* Fire Dave's gun */
  if (game->dave_fire)
  {
    game->dbullet_dir = game->last_dir;

    /* Bullet should match Dave's direction */
    if (!game->dbullet_dir)
      game->dbullet_dir = 1;

    /* Bullet should start in front of Dave */
    if (game->dbullet_dir == 1)
      game->dbullet_px = game->dave_px + 18;

    if (game->dbullet_dir == -1)
      game->dbullet_px = game->dave_px - 8;

    game->dbullet_py = game->dave_py + 8;

    /* Reset fire flag */
    game->dave_fire = 0;
  }
}

/* Move monsters along their predefined path
  Path data - https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Level_format */
void move_monsters(struct game_state *game)
{
  u8 i;

  for (i = 0; i < 5; i++)
  {
    struct monster_state *m;
    m = &game->monster[i];

    /* Only move if monster is alive */
    if (m->type && !m->dead_timer)
    {
      /* Check if monster is at end of path */
      if (!m->next_px && !m->next_py)
      {
        /* Get the next path waypoint */
        m->next_px = game->level[game->current_level].path[m->path_index];
        m->next_py = game->level[game->current_level].path[m->path_index + 1];

        /* x and y is stored in pair */
        m->path_index += 2;
      }

      /* End of path = 0xEA -- reset monster to start of path */
      if (m->next_px == (signed char)0xEA && m->next_py == (signed char)0xEA)
      {
        m->next_px = game->level[game->current_level].path[0];
        m->next_py = game->level[game->current_level].path[1];
        m->path_index = 2;
      }

      /* Move monster left */
      if (m->next_px < 0)
      {
        m->monster_px -= 1;
        m->next_px++;
      }

      /* Move monster right */
      if (m->next_px > 0)
      {
        m->monster_px += 1;
        m->next_px--;
      }

      /* Move monster up */
      if (m->next_py < 0)
      {
        m->monster_py -= 1;
        m->next_py++;
      }

      /* Move monster down */
      if (m->next_py > 0)
      {
        m->monster_py += 1;
        m->next_py--;
      }

      /* Update monster grid position */
      m->monster_x = m->monster_px / TILE_SIZE;
      m->monster_y = m->monster_py / TILE_SIZE;
    }
  }
}

/* Monster shooting */
void fire_monsters(struct game_state *game)
{
  int i;

  if (!game->ebullet_px && !game->ebullet_py)
  {
    for (i = 0; i < 5; i++)
    {
      /* Monster's shoot if they're active and visible */
      if (game->monster[i].type && is_visible(game, game->monster[i].monster_px) && !game->monster[i].dead_timer)
      {
        /* Shoot towards Dave */
        game->ebullet_dir = game->dave_px < game->monster[i].monster_px ? -1 : 1;

        if (!game->ebullet_dir)
          game->ebullet_dir = 1;

        /* Start bullet in front of monster */
        if (game->ebullet_dir == 1)
          game->ebullet_px = game->monster[i].monster_px + 18;

        if (game->ebullet_dir == -1)
          game->ebullet_px = game->monster[i].monster_px - 8;

        /* vertical align it to the center of the title */
        game->ebullet_py = game->monster[i].monster_py + 8;
      }
    }
  }
}

/* Scroll the screen when Dave is near the edge
   Game view is 20 grid units wide */
void scroll_screen(struct game_state *game)
{
  /* Scroll right if Dave reaches view position 18 */
  if (game->dave_x - game->view_x >= 18)
    game->scroll_x = 15;

  /* Scroll left if Dave reaches view position 2 */
  if (game->dave_x - game->view_x < 2)
    game->scroll_x = -15;

  if (game->scroll_x > 0)
  {
    /* Cap right side at 80 (each level is 100 wide) */
    if (game->view_x == 80)
      game->view_x = 0;
    else
    {
      game->view_x++;
      game->scroll_x--;
    }
  }

  /* Cap left side at 0*/
  if (game->scroll_x < 0)
  {
    if (game->view_x == 0)
      game->view_x = 0;
    else
    {
      game->view_x--;
      game->scroll_x++;
    }
  }
}

/* Apply gravity to Dave */
void apply_gravity(struct game_state *game)
{
  /* If he's not jumping or on the ground, apply gravity */
  if (!game->dave_jump && !game->on_ground && !game->dave_jetpack)
  {
    /* If above is clear, move dave up*/
    if (is_clear(game, game->dave_px + 4, game->dave_py + 17, 1))
      game->dave_py += 2;
    else
    {
      u8 not_align;
      not_align = game->dave_py % TILE_SIZE;

      /* If Dave is not level aligned, lock him to nearest tile*/
      if (not_align)
      {
        game->dave_py = not_align < 8 ? game->dave_py - not_align : game->dave_py + TILE_SIZE - not_align;
      }
    }
  }
}

/* Handle level-wide events */
void update_level(struct game_state *game)
{
  u8 i;

  /* Decrement jetpack delay */
  if (game->jetpack_delay)
    game->jetpack_delay--;
  
  /* Check if Dave completes level */
  if (game->check_door)
  {
    if (game->trophy)
    {
      if (game->current_level < 9)
      {
        game->current_level++;
        start_level(game);
      }
      else
      {
        printf("You won with %u points!\n", game->score);
        game->quit = 1;
      }
    }
    else
      game->check_door = 0;
  }

  /* Reset level when Dave is dead */
  if (game->dave_dead_timer)
  {
    game->dave_dead_timer--;
    if (!game->dave_dead_timer)
    {
      if (game->lives)
      {
        game->lives--;
        restart_level(game);
      }
      else
        game->quit = 1;
    }
  }

  /* Check monster timers */
  for (i = 0; i < 5; i++)
  {
    if (game->monster[i].dead_timer)
    {
      game->monster[i].dead_timer--;
      if (!game->monster[i].dead_timer)
        game->monster[i].type = 0;
    }
    else
    {
      /* Check Dave/Monster collisions */
      if (game->monster[i].type)
      {
        if (game->monster[i].monster_x == game->dave_x && game->monster[i].monster_y == game->dave_y)
        {
          game->monster[i].dead_timer = 30;
          game->dave_dead_timer = 30;
        }
      }
    }
  }
}

void restart_level(struct game_state *game)
{
  /* start position in https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Level_format
  Activate monsters based on level current_level counting starts at 0 (i.e Level 3 is case 2) */
  switch (game->current_level)
	{
		case 0: game->dave_x = 2; game->dave_y = 8; break;
		case 1: game->dave_x = 1; game->dave_y = 8; break;
		case 2: game->dave_x = 2; game->dave_y = 5; break;
		case 3: game->dave_x = 1; game->dave_y = 5; break;
		case 4: game->dave_x = 2; game->dave_y = 8; break;
		case 5: game->dave_x = 2; game->dave_y = 8; break;
		case 6: game->dave_x = 1; game->dave_y = 2; break;
		case 7: game->dave_x = 2; game->dave_y = 8; break;
		case 8: game->dave_x = 6; game->dave_y = 1; break;
		case 9: game->dave_x = 2; game->dave_y = 8; break;
	}

  game->dave_px = game->dave_x * TILE_SIZE;
  game->dave_py = game->dave_y * TILE_SIZE;
}

/* Render the world */
void draw_world(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer)
{
  SDL_Rect dest;
  u8 tile_index;
  u8 i, j;

  /* Draw each tile in row-major */
  for (j = 0; j < 10; j++)
  {

    dest.y = j * TILE_SIZE;
    dest.w = TILE_SIZE;
    dest.h = TILE_SIZE;

    for (i = 0; i < 20; i++)
    {
      dest.x = i * TILE_SIZE;
      tile_index = game->level[game->current_level].tiles[j * 100 + game->view_x + i];
      SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
    }
  }
}

/* Render dave */
void draw_dave(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer)
{
  SDL_Rect dest;
  u8 tile_index;

  dest.x = game->dave_px - game->view_x * TILE_SIZE;
  dest.y = game->dave_py;
  dest.w = 20;
  dest.h = 16;

  tile_index = 56;

  if (game->dave_jetpack)
    tile_index = game->last_dir >= 0 ? 77 : 80;
  else
  {
    if (game->dave_jump || !game->on_ground)
      tile_index = game->last_dir >= 0 ? 67 : 68;
  }

  /* dead sprite */
  if (game->dave_dead_timer)
    tile_index = 129;

  SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
}

/* Render Dave's bullets */
void draw_dave_bullet(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer)
{
  SDL_Rect dest;
  u8 tile_index;

  /* If no bullet, exit */
  if (!game->dbullet_px || !game->dbullet_py)
    return;

  dest.x = game->dbullet_px - game->view_x * TILE_SIZE;
  dest.y = game->dbullet_py;
  dest.w = 12;
  dest.h = 3;
  tile_index = game->dbullet_dir > 0 ? 127 : 128;

  SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
}

/* Render Monster bullets */
void draw_monster_bullet(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer)
{
  SDL_Rect dest;
  u8 tile_index;

  /* If no bullet, exit */
  if (!game->ebullet_px || !game->ebullet_py)
    return;

  dest.x = game->ebullet_px - game->view_x * TILE_SIZE;
  dest.y = game->ebullet_py;
  dest.w = 12;
  dest.h = 3;
  tile_index = game->ebullet_dir > 0 ? 121 : 124;

  SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
}

/* Render monster */
void draw_monsters(struct game_state *game, struct game_assets *assets, SDL_Renderer *renderer)
{
  SDL_Rect dest;
  u8 tile_index;
  u8 i;

  /* loop through all monsters */
  for (i = 0; i < 5; i++)
  {

    struct monster_state *m;
    m = &game->monster[i];

    if (m->type)
    {
      dest.x = m->monster_px - game->view_x * TILE_SIZE;
      dest.y = m->monster_py;
      dest.w = 20;
      dest.h = 16;

      tile_index = m->dead_timer ? 129 : m->type;

      SDL_RenderCopy(renderer, assets->graphics_tiles[tile_index], NULL, &dest);
    }
  }
}

/* Checks if designated grid has an obstruction or pickup
   1 means clear */
u8 is_clear(struct game_state *game, u16 px, u16 py, u8 is_dave)
{
  u8 grid_x;
  u8 grid_y;
  u8 type;

  grid_x = px / TILE_SIZE;
  grid_y = py / TILE_SIZE;

  if (grid_x > 99 || grid_y > 9)
    return 1;

  type = game->level[game->current_level].tiles[grid_y * 100 + grid_x];

  if (type == 1) { return 0; }
	if (type == 3) { return 0; }
	if (type == 5) { return 0; }
	if (type == 15) { return 0; }
	if (type == 16) { return 0; }
	if (type == 17) { return 0; }
	if (type == 18) { return 0; }
	if (type == 19) { return 0; }
	if (type == 21) { return 0; }
	if (type == 22) { return 0; }
	if (type == 23) { return 0; }
	if (type == 24) { return 0; }
	if (type == 29) { return 0; }
	if (type == 30) { return 0; }

  if (is_dave)
  {
    /* Dave-only collision checks (pickups) */
    switch (type)
    {
    case 2:
      game->check_door = 1;
      break;
    case 4:  /* jetpack */
    case 10: /* trophy */
    case 20: /* gun */
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    {
      game->check_pickup_x = grid_x;
      game->check_pickup_y = grid_y;
    }
    break;
    case 6:
    case 25:
    case 36:
    {
      if (!game->dave_dead_timer)
        game->dave_dead_timer = 30;
    }
    break;
    default:
      break;
    }
  }

  return 1;
}

/* Checks if an input pixel position is currently visible */
inline u8 is_visible(struct game_state *game, u16 px)
{
  u8 pos_x;
  pos_x = px / TILE_SIZE;
  return (pos_x - game->view_x < 20 && pos_x - game->view_x >= 0);
}
