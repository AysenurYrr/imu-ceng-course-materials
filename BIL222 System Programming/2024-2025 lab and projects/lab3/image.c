#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"

/*
allocates mem and reads the 0-offset into a memory
returns offset
*/
int read_header(FILE *file, Image *img) {
    // Read pixel data offset (located at byte 10 in BMP header)
    fseek(file, 10, SEEK_SET);
    fread(&img->offset, 4, 1, file);
    img->header = malloc((img->offset + 4) * sizeof(uint8_t));
    if (!img->header) {
        perror("header array allocation failed");
        return -1;
    }
    rewind(file);
    fread(img->header, sizeof(uint8_t), img->offset, file);
    return img->offset;
}

/*reads data from an opened bmp file*/
Image *read_image(FILE *file) {
    // Allocate memory for the Image struct
    Image *img = malloc(sizeof(Image));
    if (!img) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    if (read_header(file, img) < 0) {
        perror("could not read header");
        return NULL;
    }
    // Read width and height (located at bytes 18 and 22 in DIB header)
    uint32_t width, height;
    fseek(file, 18, SEEK_SET);
    fread(&width, 4, 1, file);
    fread(&height, 4, 1, file);

    img->width = width;
    img->height = height;
    img->pixels = malloc(width * height * sizeof(Pixel));
    if (!img->pixels) {
        perror("Pixel array allocation failed");
        return NULL;
    }

    // Read pixel data into the array
    fseek(file, img->offset, SEEK_SET);
    fread(img->pixels, sizeof(Pixel), width * height, file);

    return img;
}

Image *load_image(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    Image *img = read_image(file);

    fclose(file);
    return img;
}

/** free an img */
void free_image(Image *img) {
    /* make sure no memory leaks */
    if (img != NULL) {
        if (img->header != NULL) {
            free(img->header);
        }
        if (img->pixels != NULL) {
            free(img->pixels);
        }
        free(img);
    }
}

void save_image(const Image *img, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to create file");
        return;
    }

    /*
    (For simplicity, copy the original BMP header.
    In practice, you would write it properly.)
    */
    printf("saving img...!\n");
    /* save header*/
    rewind(file);
    fwrite(img->header, sizeof(uint8_t), img->offset, file);

    // Define circle properties
    int center_x = img->width / 2;
    int center_y = img->height / 2;
    int radius = 50;

    // Modify pixels inside the circle to white
    for (uint32_t y = 0; y < img->height; y++) {
        for (uint32_t x = 0; x < img->width; x++) {
            // Check if the pixel is inside the circle using the equation (x - cx)² + (y - cy)² ≤ r²
            int dx = x - center_x;
            int dy = y - center_y;
            if (dx * dx + dy * dy <= radius * radius) {
                img->pixels[y * img->width + x].r = 255;
                img->pixels[y * img->width + x].g = 255;
                img->pixels[y * img->width + x].b = 255;
            }
        }
    }

    // Write pixel data
    rewind(file);
    fseek(file, img->offset, SEEK_SET);
    fwrite(img->pixels, sizeof(Pixel), img->width * img->height, file);

    fclose(file);
}

int test() {
    printf("testing img functions\n");
    Image *img = load_image("cat.bmp");
    if (!img) {
        fprintf(stderr, "Failed to load img\n");
        return 1;
    }

    save_image(img, "output.bmp");
    printf("Image saved successfully!\n");

    free_image(img);
    return 0;
}

