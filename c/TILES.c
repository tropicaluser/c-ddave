#include <stdio.h>  /* Opening Files */
#include <string.h> /* Manipulating Strings */
#include <stdint.h> /* Fixed-width data types (C99) */
#include <stdlib.h>
#include <SDL.h>      /* Using SDL data structure */
#include <sys/stat.h> /* For creating directories */
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
uint32_t decode_vga_data(FILE *fin, uint32_t vga_data_addr, unsigned char *out_data)
{
    fseek(fin, vga_data_addr, SEEK_SET); /* Move the file pointer to the specified VGA data address. */
    uint32_t final_length = 0;           /* Variable to track the length of the currently decoded data.*/

    /* Read the first 4 bytes to get the length of the uncompressed data.
        bitwise OR combined with left bit shift */
    /* final_length |= fgetc(fin);       // Read the first byte (least significant byte).
    final_length |= fgetc(fin) << 8;  // Read the second byte and shift it left by 8 bits.
    final_length |= fgetc(fin) << 16; // Read the third byte and shift it left by 16 bits.
    final_length |= fgetc(fin) << 24; // Read the fourth byte and shift it left by 24 bits. */
    fread(&final_length, sizeof(final_length), 1, fin);

    // Variable to track the length of the currently decoded data.
    uint32_t current_length = 0;
    uint8_t byte_buffer;

    // Continue decoding until the entire uncompressed length is reached.
    while (current_length < final_length)
    {
        // Read the next byte from the file.
        byte_buffer = fgetc(fin);

        // If the high bit (0x80) is set, this indicates a run of unique bytes.
        if (byte_buffer & 0x80)
        {
            // Clear the high bit and increment the value by 1 to get the count of bytes.
            byte_buffer &= 0x7F;
            byte_buffer++;

            while (byte_buffer) /* Read and store the specified number of unique bytes directly into out_data. */
            {
                out_data[current_length++] = fgetc(fin);
                byte_buffer--;
            }
        }

        else /* Otherwise, it's a run-length encoded sequence */
        {
            byte_buffer += 3;       /* Add 3 to the value, which gives the count of repeated bytes. */
            char next = fgetc(fin); /* Read the next byte, which will be repeated for the specified count. */

            while (byte_buffer) /* Store the repeated byte in out_data for the specified count.*/
            {
                out_data[current_length++] = next;
                byte_buffer--;
            }
        }
    }

    // Return the total length of the decoded data.
    return final_length;
}

/**
 * @brief open exe file
 *
 * @param filename
 * @return FILE*
 */

int main(int argc, char *argv[])
{
    /* https://moddingwiki.shikadi.net/wiki/Dangerous_Dave - File formats */
    printf("Starting the extraction process...\n");

    // variables
    const uint32_t vga_data_addr = 0x120f0; /* 0x120f0 - Dangerous Dave Tileset Format - VGA tiles */
    const uint32_t vga_pal_addr = 0x26b0a;  /* 0x26b0a - VGA Palette	6-bit RGB */
    unsigned char out_data[150000] = {0};   /* Buffer to hold all pixel data */
    uint8_t palette[768];
    FILE *fin;             /* File pointer for the EXE file */
    uint32_t final_length; /* final length of decoded data */
    uint32_t tile_count;   /* number of tiles */

    // Assign values to constants
    fin = open_exe_file("DAVE.EXE");                              /* Open EXE File and go to VGA pixel data */
    final_length = decode_vga_data(fin, vga_data_addr, out_data); /* Undo RLE-compression and read all pixel data */
    read_vga_palette(fin, vga_pal_addr, palette);                 /* Read in VGA Palette. 256-color of 3 bytes (RGB) */

    fclose(fin);

    printf("Decoded %d bytes of data.\n", final_length);
    printf("palette[0] = %d\n", palette[6]);

    create_directory(FOLDER_TILESET); /* Create the tileset directory if it doesn't exist */

    tile_count = get_tile_count(out_data); /* Get number of tiles */

    uint32_t tile_index[500] = {0};
    get_tile_indices(out_data, tile_index, tile_count); /* Get tile indices */

    tile_index[tile_count] = final_length; /* The last tile ends at EOF.*/

    // Process each tile 
    for (uint32_t current_tile = 0; current_tile < tile_count; current_tile++)
	{
		uint32_t current_byte = tile_index[current_tile];

		/* Assume 16x16 */
		uint16_t tile_width = 16;
		uint16_t tile_height = 16;

		/* Skip unusual byte */
		get_tile_dimensions(&current_byte, &tile_width, &tile_height, out_data);

		// Create and fill the surface
        SDL_Surface *surface = create_and_fill_surface(out_data, &current_byte, tile_width, tile_height, palette);
        if (surface)
        {
            // Save the tile to file
            save_tile_to_file(surface, current_tile);
        }
	}

    printf("Extraction complete.\n");
    return 0;
}
