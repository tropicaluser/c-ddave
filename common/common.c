#include "common.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

void hello_world() {
  printf("Hello, World!\n");
}

void extract_tile_data(const char* filename, unsigned char* out_data, uint32_t* final_length) {
    FILE* fin = fopen(filename, "rb");
    if (!fin) {
        printf("Failed to open %s. Please ensure the file is in the same directory.\n", filename);
        exit(1);
    }

    // Read file length - first 4 bytes LE
    *final_length = 0;
    *final_length |= fgetc(fin);
    *final_length |= fgetc(fin) << 8;
    *final_length |= fgetc(fin) << 16;
    *final_length |= fgetc(fin) << 24;

    // Read each byte and un-encode
    uint32_t current_length = 0;
    uint8_t byte_buffer;
    while (current_length < *final_length) {
        byte_buffer = fgetc(fin);
        if (byte_buffer & 0x80) {
            byte_buffer &= 0x7F;
            byte_buffer++;
            while (byte_buffer) {
                out_data[current_length++] = fgetc(fin);
                byte_buffer--;
            }
        } else {
            byte_buffer += 3;
            char next = fgetc(fin);
            while (byte_buffer) {
                out_data[current_length++] = next;
                byte_buffer--;
            }
        }
    }
    fclose(fin);
}

void create_surface_from_tile(SDL_Surface* surface, const uint8_t* tile_data, uint16_t tile_width, uint16_t tile_height, const uint8_t* palette) {
    uint8_t* dst_byte = (uint8_t*)surface->pixels;
    uint32_t current_tile_byte = 0;

    for (uint32_t i = 0; i < tile_width * tile_height; i++) {
        uint8_t src_byte = tile_data[i];
        uint8_t red_p = palette[src_byte * 3];
        uint8_t green_p = palette[src_byte * 3 + 1];
        uint8_t blue_p = palette[src_byte * 3 + 2];

        dst_byte[current_tile_byte * 4] = blue_p;
        dst_byte[current_tile_byte * 4 + 1] = green_p;
        dst_byte[current_tile_byte * 4 + 2] = red_p;
        dst_byte[current_tile_byte * 4 + 3] = 0xff;

        current_tile_byte++;
    }
}

void create_bitmap_directory() {
    struct stat st = {0};
    if (stat("bitmaps", &st) == -1) {
        #ifdef _WIN32
            mkdir("bitmaps");
        #else 
            mkdir("bitmaps", 0700);
        #endif
    }
}

void save_tile_bitmap(SDL_Surface* surface, const char* filename) {
    printf("Saving %s\n", filename);
    SDL_SaveBMP(surface, filename);
}
