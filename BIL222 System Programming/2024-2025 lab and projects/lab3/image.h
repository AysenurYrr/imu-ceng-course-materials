#ifndef IMAGE_H
#define IMAGE_H
#include <stdint.h>

/**
 * Pixel structure
 * */
typedef union {
    uint32_t value; /* Raw 32-bit pixel value (BGRX) */
    struct {
        uint8_t b; /*  Blue */
        uint8_t g; /* Green */
        uint8_t r; /* Red */
        uint8_t a; /* Alpha (unused in BMP)*/
    } __attribute__((packed));
} Pixel;

typedef struct {
    void *header; /* header-offset data       */
    uint32_t offset;
    uint32_t width;  /* Image width (pixels)     */
    uint32_t height; /* Image height (pixels)    */
    Pixel *pixels;   /* Array of pixels (size = width * height) */
} Image;
extern int read_header(FILE *file, Image *img);
extern Image *read_image(FILE *file);
extern Image *load_image(const char *filename);
extern void free_image(Image *img);
extern void save_image(const Image *img, const char *filename);
#endif
