/* Extracts level information from the Dangerous Dave binary
 *  Data is stored in levelx.dat
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h> // For creating directories
#include <SDL.h>
#include "../common/common.h"

int main(int argc, char *argv[])
{
    create_directory(FOLDER_TILEMAP); /* Create the "tilemap" directory if it doesn't exist */

    /* https://moddingwiki.shikadi.net/wiki/Dangerous_Dave - File formats */

    const uint32_t level_addr = 0x26e0a;
    struct dave_level level[10];             /* Allocate space for 10 game levels */
    const char *input_filename = "DAVE.EXE"; /* Determine the input filename */

    /* Open EXE file and go to level data */
    FILE *fin = open_input_file(input_filename);
    if (!fin)
        return 1; // Exit if opening file failed

    fseek(fin, level_addr, SEEK_SET);

    stream_levels(fin, level); /* Read level data from the input file */

    write_levels_to_files(fin, level); /* Write levels to output files */

    fclose(fin);

    /* Load tileset from ../tileset folder */
    SDL_Surface **tiles = load_tiles();
    if (tiles == NULL)
    {
        return 1; // Exit if loading tiles failed
    }

    /* Create map of the world by level, row, and column */

    /* Each level consists of 100x10 tiles, with 10 levels in total,
        making it 100x100 tiles overall. Since each tile is 16x16 pixels,
        we need a 1600x1600 surface. */
    SDL_Surface *map = SDL_CreateRGBSurface(0, MAP_WIDTH, MAP_HEIGHT, 32, 0, 0, 0, 0);
    create_tile_map(tiles, level, map);
    save_map(map);

    SDL_FreeSurface(map); // Free the map surface after saving
    free_tiles(tiles);    // Free the tile surfaces

    return 0;
}
