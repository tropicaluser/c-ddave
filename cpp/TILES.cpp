#include <iostream> // For input/output
#include <fstream>  // For file input/output
#include <string>   // For string manipulation
#include <cstdint>  // For fixed-width data types (C++11)
#include <cstdlib>
#include <SDL.h>      // Using SDL data structure
#include <sys/stat.h> // For creating directories
#include "../common/common.h"

/**
 * @brief Reads and decodes VGA pixel data from a file using RLE (Run-Length Encoding).
 *
 * This function moves to the specified VGA data address within a file, reads the RLE-compressed data,
 * decodes it, and stores the decompressed pixel data in the provided output buffer. The compression
 * follows the Keen 1-3 RLE format, as described in the links below:
 *
 * - Compression Overview: https://moddingwiki.shikadi.net/wiki/Dangerous_Dave_Tileset_Format
 * - Keen 1-3 RLE Compression Details: https://moddingwiki.shikadi.net/wiki/Keen_1-3_RLE_compression
 *
 * @param fin Pointer to the file from which the VGA data is to be read.
 * @param vga_data_addr The address where the VGA data begins within the file.
 * @param out_data A buffer to store the decoded pixel data.
 * @return The size (in bytes) of the decoded pixel data.
 */
uint32_t decode_vga_data(std::ifstream *fin, uint32_t vga_data_addr, unsigned char *out_data)
{
    fin->seekg(vga_data_addr, std::ios::beg); /* Move the file pointer to the specified VGA data address. */
    uint32_t final_length = 0;                /* Variable to track the length of the currently decoded data.*/

    fin->read(reinterpret_cast<char *>(&final_length), sizeof(final_length));

    // Variable to track the length of the currently decoded data.
    uint32_t current_length = 0;
    uint8_t byte_buffer;

    // Continue decoding until the entire uncompressed length is reached.
    while (current_length < final_length)
    {
        // Read the next byte from the file.
        fin->read(reinterpret_cast<char *>(&byte_buffer), 1);

        // If the high bit (0x80) is set, this indicates a run of unique bytes.
        if (byte_buffer & 0x80)
        {
            // Clear the high bit and increment the value by 1 to get the count of bytes.
            byte_buffer &= 0x7F;
            byte_buffer++;

            // Read and store the specified number of unique bytes directly into out_data.
            while (byte_buffer--)
            {
                if (!fin->read(reinterpret_cast<char *>(&out_data[current_length++]), 1))
                {
                    std::cerr << "Error reading byte data." << std::endl;
                    return current_length;
                }
            }
        }

        else /* Otherwise, it's a run-length encoded sequence */
        {
            byte_buffer += 3; /* Add 3 to the value, which gives the count of repeated bytes. */
            char next;        /* Read the next byte, which will be repeated for the specified count. */
            if (!fin->read(&next, 1))
            {
                std::cerr << "Error reading repeated byte." << std::endl;
                return current_length;
            }

            while (byte_buffer--) /* Store the repeated byte in out_data for the specified count.*/
            {
                out_data[current_length++] = next;
            }
        }
    }

    // Return the total length of the decoded data.
    return final_length;
}

int main(int argc, char *argv[])
{
    /* https://moddingwiki.shikadi.net/wiki/Dangerous_Dave - File formats */
    printf("Starting the extraction process...\n");

    // variables
    const uint32_t vga_data_addr = 0x120f0; /* 0x120f0 - Dangerous Dave Tileset Format - VGA tiles */
    const uint32_t vga_pal_addr = 0x26b0a;  /* 0x26b0a - VGA Palette	6-bit RGB */
    unsigned char out_data[150000] = {0};   /* Buffer to hold all pixel data */
    uint8_t palette[768];
    std::ifstream *fin;      /* File pointer for the EXE file */
    uint32_t final_length;   /* final length of decoded data */
    uint32_t tile_count = 0; /* number of tiles */

    // Assign values to constants
    fin = open_exe_file("DAVE.EXE");                              /* File pointer for the EXE file */
    final_length = decode_vga_data(fin, vga_data_addr, out_data); /* Undo RLE and read all pixel data */
    read_vga_palette(fin, vga_pal_addr, palette);                 /* Read in VGA Palette. 256-color of 3 bytes (RGB) */

    fin->close(); /* Close the file */
    delete fin;   /* Clean up dynamically allocated memory */

    printf("Decoded %d bytes of data.\n", final_length);
    printf("palette[0] = %d\n", palette[6]);

    create_directory(FOLDER_TILESET); /* Create the tileset directory if it doesn't exist */

    tile_count = get_tile_count(out_data); /* Get number of tiles */

    /* offset index for each tile */
    uint32_t tile_index[500] = {0};
    get_tile_indices(out_data, tile_index, tile_count); /* Get tile indices */

    /* To determine when the current tile ends, we use the address of the next tile.
    Therefore, an extra index is needed to mark the end of the file (EOF). */

    /* The last tile ends at EOF */
    tile_index[tile_count] = final_length;

    /* Save each tile as it own file. Instead of sprite sheet.
        Easier to get started without managing sprite sheets.
    Go through each tile and create separate file */
    // Process each tile
    for (uint32_t current_tile = 0; current_tile < tile_count; current_tile++)
    {
        uint32_t current_byte = tile_index[current_tile];
        uint16_t tile_width, tile_height;

        // Get the dimensions for the current tile
        get_tile_dimensions(out_data, &current_byte, &tile_width, &tile_height);

        // Create and fill the surface
        SDL_Surface *surface = create_and_fill_surface(out_data, &current_byte, tile_width, tile_height, palette);
        if (surface)
        {
            // Save the tile to file
            save_tile_to_file(surface, current_tile);
        }
    }

    std::cout << "Extraction complete." << std::endl;
    return 0;
}
