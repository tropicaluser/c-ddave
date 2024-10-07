#include <iostream>   // For input/output
#include <fstream>    // For file input/output
#include <string>     // For string manipulation
#include <cstdint>    // For fixed-width data types (C++11)
#include <cstdlib>
#include <SDL.h>      // Using SDL data structure
#include <sys/stat.h> // For creating directories
#include "../common/common.h"

int main(int argc, char* argv[])
{
    hello_world();

    std::cout << "Starting the extraction process..." << std::endl;

    const uint32_t vga_data_addr = 0x120f0;
    const uint32_t vga_pal_addr = 0x26b0a;

    // Open EXE File and go to VGA pixel data
    std::ifstream fin("DAVE.EXE", std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open DAVE.EXE. Please ensure the file is in the same directory." << std::endl;
        return -1;
    }
    fin.seekg(vga_data_addr, std::ios::beg);

    // Undo RLE and read all pixel data
    // Read file length - first 4 bytes LE
    uint32_t final_length = 0;
    fin.read(reinterpret_cast<char*>(&final_length), sizeof(final_length));

    // Read each byte and un-encode
    uint32_t current_length = 0;
    unsigned char out_data[150000] = {0};
    uint8_t byte_buffer;

    while (current_length < final_length) {
        fin.read(reinterpret_cast<char*>(&byte_buffer), 1);
        if (byte_buffer & 0x80) {
            byte_buffer &= 0x7F;
            byte_buffer++;
            while (byte_buffer--) {
                fin.read(reinterpret_cast<char*>(&out_data[current_length++]), 1);
            }
        } else {
            byte_buffer += 3;
            char next;
            fin.read(&next, 1);
            while (byte_buffer--) {
                out_data[current_length++] = next;
            }
        }
    }

    // Read in VGA Palette. 256-color of 3 bytes (RGB)
    fin.seekg(vga_pal_addr, std::ios::beg);
    uint8_t palette[768];
    fin.read(reinterpret_cast<char*>(palette), 768);
    for (uint32_t i = 0; i < 768; i++) {
        palette[i] <<= 2;
    }

    fin.close();

    // Create the bitmaps directory if it doesn't exist
    struct stat st = {0};
    if (stat("bitmaps", &st) == -1) {
        #ifdef _WIN32
            mkdir("bitmaps");
        #else 
            mkdir("bitmaps", 0700);
        #endif
    }

    // Get total tile count from first byte
    uint32_t tile_count = 0;
    tile_count |= out_data[3] << 24;
    tile_count |= out_data[2] << 16;
    tile_count |= out_data[1] << 8;
    tile_count |= out_data[0];

    // Each in offset index for each tile
    uint32_t tile_index[500] = {0};
    for (uint32_t i = 0; i < tile_count; i++) {
        tile_index[i] |= out_data[i * 4 + 4];
        tile_index[i] |= out_data[i * 4 + 5] << 8;
        tile_index[i] |= out_data[i * 4 + 6] << 16;
        tile_index[i] |= out_data[i * 4 + 7] << 24;
        //std::cout << tile_index[i] << std::endl;
    }

    // The last tile ends at EOF
    tile_index[tile_count] = final_length;

    // Go through each tile and create separate file
    uint16_t tile_width;
    uint16_t tile_height;
    uint32_t current_byte;
    uint32_t current_tile_byte;
    uint8_t current_tile;

    for (current_tile = 0; current_tile < tile_count; current_tile++) {
        current_tile_byte = 0;
        current_byte = tile_index[current_tile];

        // Assume 16x16
        tile_width = 16;
        tile_height = 16;

        // Skip unusual byte
        if (current_byte > 65280)
            current_byte++;

        // Read first 4 bytes for possible custom dimensions
        if (out_data[current_byte + 1] == 0 && out_data[current_byte + 3] == 0) {
            if (out_data[current_byte] > 0 && out_data[current_byte] < 0xBf &&
                out_data[current_byte + 2] > 0 && out_data[current_byte + 2] < 0x64) {
                tile_width = out_data[current_byte];
                tile_height = out_data[current_byte + 2];
                current_byte += 4;
            }
        }

        // Create an SDL Surface
        SDL_Surface* surface;
        uint8_t* dst_byte;
        surface = SDL_CreateRGBSurface(0, tile_width, tile_height, 32, 0, 0, 0, 0);
        if (!surface) {
            std::cerr << "Error: Failed to create SDL surface. SDL_Error: " << SDL_GetError() << std::endl;
            return -1;
        }
        dst_byte = (uint8_t*)surface->pixels;

        // Go through the data, matching to palette and write to surface
        uint8_t src_byte;
        for (; current_byte < tile_index[current_tile + 1]; current_byte++) {
            src_byte = out_data[current_byte];
            dst_byte[current_tile_byte * 4] = palette[src_byte * 3 + 2];     // Blue
            dst_byte[current_tile_byte * 4 + 1] = palette[src_byte * 3 + 1]; // Green
            dst_byte[current_tile_byte * 4 + 2] = palette[src_byte * 3];     // Red
            dst_byte[current_tile_byte * 4 + 3] = 0xff;                      // Alpha

            current_tile_byte++;
        }

        // Create output file name
        std::string fout = "bitmaps/tile" + std::to_string(current_tile) + ".bmp";
        std::cout << "Saving " << fout << " as a bitmap (" << tile_width << " x " << tile_height << ")" << std::endl;

        // Save and free
        SDL_SaveBMP(surface, fout.c_str());
        SDL_FreeSurface(surface);
    }

    std::cout << "Extraction complete." << std::endl;
    return 0;
}
