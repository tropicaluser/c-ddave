#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <SDL.h>

/* Use extern "C" for C++ compilers to prevent name mangling */
#ifdef __cplusplus
extern "C" {
#endif

void hello_world();
void extract_tile_data(const char* filename, unsigned char* out_data, uint32_t* final_length);
void create_surface_from_tile(SDL_Surface* surface, const uint8_t* tile_data, uint16_t tile_width, uint16_t tile_height, const uint8_t* palette);
void create_bitmap_directory();
void save_tile_bitmap(SDL_Surface* surface, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // COMMON_H
