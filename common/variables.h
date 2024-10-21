#ifndef LMDAVE_H
#define LMDAVE_H

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;

#define TILE_SIZE 16
#define DISPLAY_SCALE 3

/* Format of the level information
 * -path is used for monster movement
 * -tiles contain tileset indices
 * -padding unused but included for capatibility
 */
struct dave_level
{
  i8 path[256];
  u8 tiles[1000];
  u8 padding[24];
};

/* Game state information in kitchen-sink format
 *    (Refactor me please!!!)
 */
struct game_state
{
  u8 quit;
  u8 current_level;
  u8 score;
	u8 lives;
  u8 view_x;
  u8 view_y;
  i8 scroll_x;
  u8 dave_x;
	u8 dave_y;
	u16 dave_px;
	u16 dave_py;
  u8 on_ground;
  i8 last_dir;

  u8 try_right;
  u8 try_left;
  u8 try_jump;
  u8 try_fire;
	u8 try_jetpack;
	u8 try_down;
	u8 try_up;
  u8 dave_right;
  u8 dave_left;
  u8 dave_jump;
  u8 dave_fire;
	u8 dave_jetpack;
	u8 dave_down;
	u8 dave_up;
  u8 jump_timer;
  u8 check_pickup_x;
	u8 check_pickup_y;
  u8 check_door;
	u8 trophy;
	u8 gun;
	u8 jetpack;

  u16 dbullet_px;
	u16 dbullet_py;
	i8 dbullet_dir;
	u16 ebullet_px;
	u16 ebullet_py;
	i8 ebullet_dir;

  u8 collision_point[9];

  struct dave_level level[10];
};

/* Game asset structure
 * Only tileset data for now
 * Could include music/sounds, etc
 */
struct game_assets
{
  SDL_Texture *graphics_tiles[158];
};

#endif