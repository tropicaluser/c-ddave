#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <SDL.h>

#ifdef __cplusplus
#include <iostream>
#include <fstream>
using std::ifstream;
#define FILE_TYPE ifstream *
#else
#include <stdio.h>
#define FILE_TYPE FILE *
#endif

#define FOLDER_TILESET "tileset"
#define FOLDER_TILEMAP "tilemap"
#define MAX_TILES 158
#define TILE_SIZE 16
#define TILE_SIZE 16
#define MAP_WIDTH (100 * TILE_SIZE)
#define MAP_HEIGHT (100 * TILE_SIZE)

/* Use extern "C" for C++ compilers to prevent name mangling */
#ifdef __cplusplus
extern "C"
{
#endif

  void hello_world();

  /* tiles file functions */
  FILE_TYPE open_exe_file(const char *filename);
  void create_directory(const char *path);
  void read_vga_palette(FILE_TYPE fin, uint32_t vga_pal_addr, uint8_t *palette);
  void get_tile_indices(unsigned char *out_data, uint32_t *tile_index, uint32_t tile_count);
  void get_tile_dimensions(uint32_t *current_byte, uint16_t *tile_width, uint16_t *tile_height, unsigned char *out_data);
  SDL_Surface *create_and_fill_surface(unsigned char *out_data, uint32_t *current_byte, uint16_t width, uint16_t height, uint8_t *palette);
  void save_tile_to_file(SDL_Surface *surface, uint32_t tile_index);
  uint32_t get_tile_count(unsigned char *out_data);

  /* LEVEL file functions */

  /* https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Level_format - Level structure */
  struct dave_level
  {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
  };

  void free_tiles(SDL_Surface **tiles);
  void save_map(SDL_Surface *map);
  void create_tile_map(SDL_Surface *tiles[], struct dave_level *level, SDL_Surface *map);
  SDL_Surface **load_tiles();
  FILE *open_input_file(const char *input_filename); /* works in c and cpp */
  void stream_levels(FILE *fin, struct dave_level *level);
  void write_levels_to_files(FILE *fin, struct dave_level *level);

#ifdef __cplusplus
}
#endif

#endif // COMMON_H
