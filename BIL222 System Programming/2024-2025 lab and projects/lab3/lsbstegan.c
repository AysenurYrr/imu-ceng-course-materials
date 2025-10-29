#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lsbstegan.h"
#include "image.h"


uint8_t set_msb2lsb(uint8_t dest, uint8_t src, uint8_t nbits) {
    uint8_t mask = (1 << nbits) - 1;
    return (dest & ~mask) | (src & mask);  // Directly take LSBs
}

/** Get the n MSB of byte */
uint8_t get_nmsb(uint8_t byte, uint8_t nbits) {
    return byte & ((1 << nbits) - 1);  // Extract LSBs instead of MSBs
}


/** Set the n LSBs of a byte to 0 */
uint8_t set_nlsb(uint8_t byte, uint8_t nbits) {
    return byte & (~((1 << nbits) - 1));
}

/** Encode message into an image using n bits per channel */
int encode_message(Image *img, const char *message, uint8_t nbit_per_channel) {
    size_t msg_len = strlen(message) + 1;
    size_t total_bits = msg_len * 8;
    size_t total_pixels_needed = (total_bits + (3 * nbit_per_channel - 1)) / (3 * nbit_per_channel);

    if (total_pixels_needed > img->width * img->height) {
        fprintf(stderr, "Message too long!\n");
        return -1;
    }

    size_t bit_index = 0;
    for (size_t i = 0; i < msg_len; ++i) {
        char c = message[i];
        for (int j = 8 - nbit_per_channel; j >= 0; j -= nbit_per_channel) {
            uint8_t bits = (c >> j) & ((1 << nbit_per_channel) - 1);
            size_t pixel_idx = bit_index / (3 * nbit_per_channel);
            int channel = (bit_index / nbit_per_channel) % 3;

            Pixel *p = &img->pixels[pixel_idx];
            switch (channel) {
                case 0: p->b = set_msb2lsb(p->b, bits, nbit_per_channel); break;
                case 1: p->g = set_msb2lsb(p->g, bits, nbit_per_channel); break;
                case 2: p->r = set_msb2lsb(p->r, bits, nbit_per_channel); break;
            }
            bit_index += nbit_per_channel;
        }
    }
    return 0;
}

char *decode_message(const Image *img, uint8_t nbit_per_channel) {
    size_t max_chars = (img->width * img->height * 3 * nbit_per_channel) / 8;
    char *buffer = malloc(max_chars + 1); // Allocate space for NULL terminator
    if (!buffer) return NULL;

    size_t bit_index = 0;
    uint8_t current_byte = 0;
    int bits_collected = 0;
    
    for (size_t i = 0; i < img->width * img->height; ++i) {
        Pixel p = img->pixels[i];
        for (int channel = 0; channel < 3; ++channel) {
            uint8_t bits;
            switch (channel) {
                case 0: bits = get_nmsb(p.b, nbit_per_channel); break;
                case 1: bits = get_nmsb(p.g, nbit_per_channel); break;
                case 2: bits = get_nmsb(p.r, nbit_per_channel); break;
            }
            current_byte = (current_byte << nbit_per_channel) | bits;
            bits_collected += nbit_per_channel;

            if (bits_collected >= 8) {
                buffer[bit_index / 8] = current_byte;
                if (current_byte == '\0') {
                    buffer[bit_index / 8] = '\0';
                    return buffer;  // Stop decoding when NULL terminator is reached
                }
                bits_collected = 0;
                current_byte = 0;
            }
            bit_index += nbit_per_channel;
        }
    }

    buffer[max_chars] = '\0';
    return buffer;
}
