#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "image.h"
#include "lsbstegan.h"
/*TODO: change to hide more than 1-bit*/
#define BIT_PER_CHANNEL 1

int main() {
    
    // Load image
    Image *img = load_image("cat.bmp");
    if (!img) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }

    // Encode message
    const char *message = "Hello, AI!";
    if (encode_message(img, message, BIT_PER_CHANNEL) != 0) {
        fprintf(stderr, "Message encoding failed\n");
        free(img->pixels);
        free(img);
        return 1;
    }

    // Save modified image
    save_image(img, "encoded.bmp");

    // Decode message
    char *decoded = decode_message(img, BIT_PER_CHANNEL);
    printf("Decoded message: %s\n", decoded);

    // Cleanup
    free(decoded);
    free_image(img);
    return 0;
}