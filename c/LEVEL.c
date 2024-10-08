/* Extracts level information from the Dangerous Dave binary
 *  Data is stored in levelx.dat
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h> // For creating directories
#include <SDL.h>

/* Level format structure */
struct dave_level {
    uint8_t path[256];
    uint8_t tiles[1000];
    uint8_t padding[24];
};

int main(int argc, char* argv[]) {
    const uint32_t level_addr = 0x26e0a;

     // Create the "level" directory if it doesn't exist
    struct stat st = {0};
    if (stat("level", &st) == -1) {
      #ifdef _WIN32
          mkdir("level");
      #else 
          mkdir("level", 0700);
      #endif
    }

    /* Allocate space for 10 game levels */
    struct dave_level level[10];

    /* Determine the input filename */
    const char* input_filename = "DAVE.EXE"; // Default filename
    if (argc > 1) {
        input_filename = argv[1]; // Use provided filename
    }

    /* Open EXE file and go to level data */
    FILE *fin = fopen(input_filename, "rb");
    if (!fin) {
        printf("Error: Could not open file %s\n", input_filename);
        return 1; // Return with error code if file cannot be opened
    }
    
    fseek(fin, level_addr, SEEK_SET);

    /* Stream level data to memory and output files */
    FILE *fout;
    char fname[50];  // Increased size to accommodate longer path
    uint32_t i, j, k;

    for (j = 0; j < 10; j++) {
        /* Make new file */
        snprintf(fname, sizeof(fname), "level/level%d.dat", j);  // Safer alternative

        fout = fopen(fname, "wb");
        if (!fout) {
            printf("Error: Could not open output file %s\n", fname);
            fclose(fin); // Ensure the input file is closed
            return 1; // Return with error code if output file cannot be opened
        }

        /* Stream path data */
        for (i = 0; i < sizeof(level[j].path); i++) {
            level[j].path[i] = fgetc(fin);
            fputc(level[j].path[i], fout);
        }

        /* Stream tile indices */
        for (i = 0; i < sizeof(level[j].tiles); i++) {
            level[j].tiles[i] = fgetc(fin);
            fputc(level[j].tiles[i], fout);
        }

        /* Stream padding */
        for (i = 0; i < sizeof(level[j].padding); i++) {
            level[j].padding[i] = fgetc(fin);
            fputc(level[j].padding[i], fout);
        }

        printf("Saving %s as level data\n", fname);
        fclose(fout);
    }
    fclose(fin);

    /* Load tileset from ../bitmaps folder */
    SDL_Surface *tiles[158];
    for (i = 0; i < 158; i++) {
        snprintf(fname, sizeof(fname), "./bitmaps/tile%d.bmp", i);  // Use snprintf to avoid overflow

        tiles[i] = SDL_LoadBMP(fname);
        if (tiles[i] == NULL) {
            printf("Error loading tile %s: %s\n", fname, SDL_GetError());
            return 0;
        }
    }

    /* Create map of the world by level, row, and column */
    SDL_Surface *map;
    SDL_Rect dest;
    uint8_t tile_index;
    map = SDL_CreateRGBSurface(0, 1600, 1600, 32, 0, 0, 0, 0);
    for (k = 0; k < 10; k++) {
        for (j = 0; j < 10; j++) {
            for (i = 0; i < 100; i++) {
                tile_index = level[k].tiles[j * 100 + i];
                dest.x = i * 16;
                dest.y = k * 160 + j * 16;
                dest.w = 16;
                dest.h = 16;
                SDL_BlitSurface(tiles[tile_index], NULL, map, &dest);
            }
        }
    }

    char output_fname[50];
    snprintf(output_fname, sizeof(output_fname), "level/map.bmp"); // Update the file path
    SDL_SaveBMP(map, output_fname);

    SDL_FreeSurface(map); // Free the map surface after saving
    for (i = 0; i < 158; i++) { // Free each tile surface if loaded
        if (tiles[i] != NULL) {
            SDL_FreeSurface(tiles[i]);
        }
    }

    return 0;
}
