#include "common.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#include <iostream>
#include <fstream>
using std::ifstream;
#define FILE_TYPE ifstream *
#else
#include <stdio.h>
#define FILE_TYPE FILE *
#endif

void hello_world()
{
    printf("Hello, World!\n");
}

FILE_TYPE open_exe_file(const char *filename)
{
#ifdef __cplusplus
    // C++: Use ifstream for file handling
    ifstream *fin = new ifstream(filename, std::ios::binary);

    // Check if the file was opened successfully
    if (!(*fin))
    {
        std::cerr << "Failed to open " << filename << ". Please ensure the file is in the same directory." << std::endl;
        delete fin;     // Clean up dynamically allocated memory
        return nullptr; // Return nullptr to indicate failure
    }

    return fin; // Return the pointer to the opened file
#else
    // C: Use fopen for file handling
    FILE *fin = fopen(filename, "rb");
    if (!fin)
    {
        printf("Failed to open %s. Please ensure the file is in the same directory.\n", filename);
        return NULL; // Return NULL on failure
    }
    return fin; // Return the opened file pointer
#endif
}

/**
 * @brief Create a tileset directory object
 *
 * @param path file path
 */
void create_directory(const char *path)
{
    struct stat st = {0};
    if (stat(path, &st) == -1)
    {
#ifdef _WIN32
        mkdir(path);
#else
        mkdir(path, 0700);
#endif
    }
}

/**
 * @brief read VGA palette data
 *
 * @param fin pointer to file to read the data
 * @param vga_pal_addr address where VGA pixel starts
 * @param palette VGA palette data
 */
void read_vga_palette(FILE_TYPE fin, uint32_t vga_pal_addr, uint8_t *palette)
{
#ifdef __cplusplus
    // C++: Use ifstream methods
    fin->seekg(vga_pal_addr, std::ios::beg);
    fin->read(reinterpret_cast<char *>(palette), 768);
#else
    // C: Use FILE methods
    fseek(fin, vga_pal_addr, SEEK_SET);
    fread(palette, 1, 768, fin);
#endif

    // Adjust each color value (convert from 6-bit to 8-bit RGB).
    for (uint32_t i = 0; i < 768; i++)
    {
        palette[i] <<= 2;
    }
}

/**
 * @brief Get the tile indices object
 * Read in the file position of each tile.
       Do it so we know when we done with each tile
      Many are arbitrary sizes
 *
 * @param out_data
 * @param tile_index
 * @param tile_count
 */
void get_tile_indices(unsigned char *out_data, uint32_t *tile_index, uint32_t tile_count)
{
#ifdef __cplusplus
    // C++: Read the 4 bytes starting at the appropriate index directly into tile_index
    for (uint32_t i = 0; i < tile_count; i++)
    {
        tile_index[i] = *reinterpret_cast<uint32_t *>(&out_data[i * 4 + 4]);
    }
#else
    // C: Use a simple cast to read 4 bytes into tile_index
    for (uint32_t i = 0; i < tile_count; i++)
    {

        tile_index[i] = *(uint32_t *)(&out_data[i * 4 + 4]);
        /* tile_index[i] |= out_data[i * 4 + 4];
        tile_index[i] |= out_data[i * 4 + 5] << 8;
        tile_index[i] |= out_data[i * 4 + 6] << 16;
        tile_index[i] |= out_data[i * 4 + 7] << 24; */
    }

#endif
}

void get_tile_dimensions(uint32_t *current_byte, uint16_t *tile_width, uint16_t *tile_height, unsigned char *out_data)
{
    /* Skip unusual byte */
    if (*current_byte > 65280)
        (*current_byte)++;

    /* Read first 4 bytes for possible custom dimensions */
    if (out_data[*current_byte + 1] == 0 && out_data[*current_byte + 3] == 0)
    {
        if (out_data[*current_byte] > 0 && out_data[*current_byte] < 0xBf &&
            out_data[*current_byte + 2] > 0 && out_data[*current_byte + 2] < 0x64)
        {
            *tile_width = out_data[*current_byte];
            *tile_height = out_data[*current_byte + 2];
            *current_byte += 4;
        }
    }
}

/**
 * @brief Create a surface object
 *
 * @param width
 * @param height
 * @return SDL_Surface*
 */
SDL_Surface *create_surface(uint32_t width, uint32_t height)
{
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    if (!surface)
    {
        printf("Error: Failed to create SDL surface. SDL_Error: %s\n", SDL_GetError());
    }
    return surface;
}

/**
 * @brief Create and fill surface object
 *
 * @param out_data
 * @param current_byte
 * @param width
 * @param height
 * @param palette
 * @return SDL_Surface*
 */
SDL_Surface *create_and_fill_surface(unsigned char *out_data, uint32_t *current_byte, uint16_t width, uint16_t height, uint8_t *palette)
{
    SDL_Surface *surface = create_surface(width, height);
    if (!surface)
        return NULL;

    uint8_t *dst_byte = (uint8_t *)surface->pixels;
    uint8_t src_byte;

    // Fill the surface with pixel data
    for (uint32_t current_tile_byte = 0; current_tile_byte < width * height; current_tile_byte++)
    {
        src_byte = out_data[*current_byte];
        uint8_t blue_p = palette[src_byte * 3 + 2];
        uint8_t green_p = palette[src_byte * 3 + 1];
        uint8_t red_p = palette[src_byte * 3];

        dst_byte[current_tile_byte * 4] = blue_p;      // Blue
        dst_byte[current_tile_byte * 4 + 1] = green_p; // Green
        dst_byte[current_tile_byte * 4 + 2] = red_p;   // Red
        dst_byte[current_tile_byte * 4 + 3] = 0xff;    // Alpha

        (*current_byte)++; // Move to the next byte
    }

    return surface;
}

/**
 * @brief save tile to file
 *
 * @param surface
 * @param tile_index
 */
void save_tile_to_file(SDL_Surface *surface, uint32_t tile_index)
{
    char fout[20];
    snprintf(fout, sizeof(fout), "%s/tile%u.bmp", FOLDER_TILESET, tile_index);
    SDL_SaveBMP(surface, fout);
    SDL_FreeSurface(surface);
}

/**
 * @brief Get the tile count object - Same order as RLE-decompression
 * - https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Tileset_Format - File structure
    The |= operator is a bitwise OR assignment operator.
        It means tile_count = tile_count | (out_data[3] << 24);

        Shifting left by 24 positions effectively moves the value of out_data[3]
        into the 4th byte (the highest byte) of a 32-bit integer.
        For example, if out_data[3] has a value of 0x01,
        then out_data[3] << 24 becomes.
 *
 * @param out_data
 * @return uint32_t
 */
uint32_t get_tile_count(unsigned char *out_data)
{
    return out_data[3] << 24 | out_data[2] << 16 | out_data[1] << 8 | out_data[0];
}

/** LEVEL file structure */
void free_tiles(SDL_Surface **tiles)
{
    for (int i = 0; i < MAX_TILES; i++)
    {
        if (tiles[i] != NULL)
        {
            SDL_FreeSurface(tiles[i]);
        }
    }
    free(tiles); // Free the tile surface array
}

void save_map(SDL_Surface *map)
{
    char output_fname[50];
    snprintf(output_fname, sizeof(output_fname), "%s/map.bmp", FOLDER_TILEMAP);
    SDL_SaveBMP(map, output_fname);
}

void create_tile_map(SDL_Surface *tiles[], struct dave_level *level, SDL_Surface *map)
{
    SDL_Rect dest;
    uint8_t tile_index;

    const int tilePerLevel = 10;                                        // Size of each tile (assuming this represents the number of tiles in a layer)
    const int tilesPerColumn = 10;                                      // Number of layers
    const int tilesPerRow = 100;                                        // Number of tiles in each row
    const int totalTiles = tilePerLevel * tilesPerColumn * tilesPerRow; // Total number of tiles to process

    for (int index = 0; index < totalTiles; index++)
    {

        // Calculate `k`, `j`, and `i` based on the flattened index
        int k = index / (tilesPerColumn * tilesPerRow);
        int j = (index / tilesPerRow) % tilesPerColumn;
        int i = index % tilesPerRow;

        tile_index = level[k].tiles[j * tilesPerRow + i];

        // Set up the destination rectangle for the tile
        dest.x = i * TILE_SIZE;
        dest.y = k * 160 + j * TILE_SIZE;
        dest.w = TILE_SIZE;
        dest.h = TILE_SIZE;

        // Blit the tile onto its expected location
        SDL_BlitSurface(tiles[tile_index], NULL, map, &dest);
    }
}

SDL_Surface **load_tiles()
{
// Cast only when compiling as C++ for compatibility
#ifdef __cplusplus
    SDL_Surface **tiles = (SDL_Surface **)malloc(MAX_TILES * sizeof(SDL_Surface *));
#else
    SDL_Surface **tiles = malloc(MAX_TILES * sizeof(SDL_Surface *));
#endif

    for (int i = 0; i < MAX_TILES; i++)
    {
        char fname[50];
        snprintf(fname, sizeof(fname), "./%s/tile%d.bmp", FOLDER_TILESET, i);
        tiles[i] = SDL_LoadBMP(fname);
        if (tiles[i] == NULL)
        {
            printf("Error loading tile %s: %s\n", fname, SDL_GetError());
            free(tiles); // Free allocated memory
            return NULL;
        }
    }
    return tiles;
}

FILE *open_input_file(const char *input_filename)
{
    FILE *fin = fopen(input_filename, "rb");
    if (!fin)
    {
        printf("Error: Could not open file %s\n", input_filename);
    }
    return fin;
}

void stream_levels(FILE *fin, struct dave_level *level)
{
    for (uint32_t j = 0; j < 10; j++)
    {
        /* Read path data */
        fread(level[j].path, sizeof(uint8_t), sizeof(level[j].path), fin);

        /* Read tile indices */
        fread(level[j].tiles, sizeof(uint8_t), sizeof(level[j].tiles), fin);

        /* Read padding */
        fread(level[j].padding, sizeof(uint8_t), sizeof(level[j].padding), fin);
    }
}

void write_level_data(FILE *fout, struct dave_level *level, int level_index)
{
    // Write path data
    fwrite(level[level_index].path, sizeof(uint8_t), sizeof(level[level_index].path), fout);

    // Write tile indices
    fwrite(level[level_index].tiles, sizeof(uint8_t), sizeof(level[level_index].tiles), fout);

    // Write padding
    fwrite(level[level_index].padding, sizeof(uint8_t), sizeof(level[level_index].padding), fout);
}

void write_levels_to_files(FILE *fin, struct dave_level *level)
{
    FILE *fout;
    char fname[50]; // Increased size to accommodate longer path

    for (uint32_t j = 0; j < 10; j++)
    {
        /* Make new file */
        snprintf(fname, sizeof(fname), "%s/level%d.dat", FOLDER_TILEMAP, j); // Use snprintf to avoid overflow

        fout = fopen(fname, "wb");
        if (!fout)
        {
            printf("Error: Could not open output file %s\n", fname);
            fclose(fin); // Ensure the input file is closed
            return;      // Return without error code if output file cannot be opened
        }

        // Write the level data to the output file
        write_level_data(fout, level, j); // Pass level array and index
        fclose(fout);
    }
}
