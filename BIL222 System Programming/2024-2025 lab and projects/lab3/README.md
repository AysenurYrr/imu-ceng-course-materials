[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/wFYgG2Nh)
### Lab Document: Hiding Messages in Images Using Least Significant Bits (LSB)

#### Lab Objectives:
1. Understand the structure of BMP files and pixel manipulation.
2. Use `typedef` to define pixel and image structures.
3. Practice memory allocation and file I/O for image processing.
4. Encode/decode hidden messages using LSB of RGB values.

---

### Part 1: Pixel Structure and Image Memory Allocation
**Task-1:** You are given a program that copies an image (see the below explanations). Change this program to implement so that it draws nested circles on an image and saves a copy of the image.

#### Step 1: Understanding the Pixel Structure
To store a pixel, we will use a `typedef` with a union to directly access RGB components and the raw 32-bit pixel value (BGRX format, 4 bytes per pixel).  
**Add to your code (image.h):**
```c
#include <stdint.h>
// Define Pixel structure using a union
typedef union {
    uint32_t value; // Raw 32-bit pixel value (BGRX)
    struct {
        uint8_t b;  // Blue
        uint8_t g;  // Green
        uint8_t r;  // Red
        uint8_t a;  // Alpha (unused in BMP)
    } __attribute__((packed));
} Pixel;
```
**Understanding little endian representation:**
Write an example main function and test the following code to understand little endian representation   
```C
Pixel p;
p.vaue = 0x12345678; 
//TODO: print b, g, r, a in hexadecimal
``` 

#### Step 2: Define the Image Structure
Create an `Image` struct to store image metadata and pixel data. Note that image data startts from `offset` value. 
**Add to your code:**
```c
typedef struct {
    void *header; /* header-offset data       */
    uint32_t offset;
    uint32_t width;  /* Image width (pixels)     */
    uint32_t height; /* Image height (pixels)    */
    Pixel *pixels;   /* Array of pixels (size = width * height) */
} Image;
```

#### Step 3: Read BMP Headers and Allocate Memory
An image (bmp) file should be first opened in binary reading format:
```C
FILE *file = fopen(filename, "rb");
```
Then we can read any part of the file, by simply setting cursor to the desired position.
For instance, we can read the offset info of the bmp file by simply reading the integer value starting from byte 10 (see [wikipedia BMP for header info](https://en.wikipedia.org/wiki/BMP_file_format)).
```C
    uint32_t offset;
    fseek(file, 10, SEEK_SET);
    fread(&offset, 4, 1, file);
```
Similarly, we can read the width and height of the image:
```C
    uint32_t width, height;
    fseek(file, 18, SEEK_SET);
    fread(&width, 4, 1, file);
    fread(&height, 4, 1, file);
```

Now we can implement a function to read BMP headers and allocate memory for the image.  
```c
nt read_header(FILE *file, Image *img) {
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
``` 

#### Step 4: Saving Image to BMP File
To save a copy of the iamge, we write `Image` struct back to a BMP file.  
**Complete the following code:**
```c
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

    // Write pixel data
    rewind(file);
    fseek(file, img->offset, SEEK_SET);
    fwrite(img->pixels, sizeof(Pixel), img->width * img->height, file);

    fclose(file);
}
```

#### Step 5: Test Loading and Saving
Test your code by loading an image and saving it unchanged. Use a 32-bit BMP file for testing.  
**Example `main` function:**
```c
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
int main() {
    printf("testing img functions\n");
    Image *img = load_image("input.bmp");
    if (!img) {
        fprintf(stderr, "Failed to load img\n");
        return 1;
    }

    save_image(img, "output.bmp");
    printf("Image saved successfully!\n");

    free_image(img);
    return 0;
}
```

### Step-6: Edit main function 
**task-1:**
Change the main function to implement so that it draws nested circles on an image and saves a copy of the image.

---

### Notes for Part 1:
1. **Error Handling**: Always check `malloc`, `fopen`, and `fread` for failures (as shown in the code).
2. **BMP Structure**: The code assumes the input is a 32-bit BMP with 4 bytes per pixel (BGRX format). 
3. **Endianness**: The `Pixel` union handles little-endian byte order automatically.

---

### Next Steps:
In Part 2, you will modify the least significant bits (LSB) of the RGB values to encode hidden messages. The given code hides only 1 bit per channel, you will change this to hide nbit perchannel. Ensure Part 1 works correctly before proceeding!


### Part 2: Encoding and Decoding Messages Using Least Significant Bits (LSB)
**Task-2:** The given code hides only 1 bit per channel, you will change this to hide nbit perchannel

---

#### Step 1: Understanding LSB Steganography  
The least significant bit (LSB) of a pixel’s RGB components can be modified to store hidden data. Since the LSB contributes minimally to color perception, changes are visually subtle. Each pixel can store **3 bits** (one in each R, G, and B component).  

**Example**:  
- Character `'A'` = `0x41` = `01000001` in binary.  
- Split into 8 bits: `0 1 0 0 0 0 0 1`.  
- Store these bits across 3 pixels:  
  ```
  Pixel 1: B=0, G=1, R=0  
  Pixel 2: B=0, G=0, R=0  
  Pixel 3: B=0, G=1, R=0 (last bit padded with 0)  
  ```

---

#### Step 2: Bit Manipulation Functions  
Add helper functions to set/get the LSB of a byte:  
```c
// Set the LSB of a byte to a bit value (0 or 1)
uint8_t set_lsb(uint8_t byte, uint8_t bit) {
    return (byte & 0xFE) | (bit & 1);
}

// Get the LSB of a byte
uint8_t get_lsb(uint8_t byte) {
    return byte & 1;
}
```

---

#### Step 3: Encode a Message into the Image  
The given functions `encode_message` hides a null-terminated string in the image and `decode_message` extracts the hidden message:
```c
int encode_message(Image *img, const char *message,  uint8_t nbit_per_channel);
char *decode_message(const Image *img, uint8_t nbit_per_channel)
```

**Taks-2:** In the given versions, they encode and decode only 1 bit perchannel. Your task is to change this into `n-bit` per channel.

---

#### Notes for Part 2:  
1. **Message Capacity**:  
   - Maximum message length = `(width * height * 3) / 8 - 1` bytes (subtract 1 for the null terminator).  
   - Example: A 100x100 image can store `(100*100*3)/8 ≈ 3750` characters.  

2. **Testing**:  
   - Use simple messages first (e.g., "TEST").  
   - Verify the output image visually and by decoding.  

3. **Limitations**:  
   - Only works with 32-bit BMP files.  
   - Large messages may exceed image capacity.  

---

### Next Steps:  
1. **Advanced Features**:  
   - Add a GTK GUI interface
     - 1 button to upload an image
     - 1 button to save it
     - Textbox for messages
     - Encode/decode buttons 
   - You can also add image as a background to GTK drawing and save image with user drawings
   - Encrypt messages before encoding.  
   - Hide a smaller image into another image
2. **Interaction with AI**:  
   - You can submit encoded images to AI and receive decoded responses.  
   - Example task: Hide a question in an image, let AI answer, and decode the response.  

---

### Lab Submission:  
1. Code for Parts 1 and 2. 
   1. for task-1 you can add a new function to image.c
   2. for task-2 you should update the functions (you can copy the older version into different names) 
2. Before/after images showing encoded messages.  
3. A report explaining their approach and challenges.

---

## The given files

image.h
```C
#ifndef IMAGE_H
#define IMAGE_H
#include <stdint.h>

/** Pixel structure */
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
```
image.c
```c
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

    // Write pixel data
    rewind(file);
    fseek(file, img->offset, SEEK_SET);
    fwrite(img->pixels, sizeof(Pixel), img->width * img->height, file);

    fclose(file);
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
```

lsbstegan.h
```c
#ifndef LSBSTEGAN_H
#define LSBSTEGAN_H
#include <stdint.h>
#include "image.h"

extern int encode_message(Image *img, const char *message,uint8_t nbit_per_channel);
extern uint8_t set_lsb(uint8_t byte, uint8_t bit);
extern uint8_t get_lsb(uint8_t byte);
extern char *decode_message(const Image *img,uint8_t nbit_per_channel);
#endif
```

lsbstegan.c
```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lsbstegan.h" 
#include "image.h"

/** TODO:Set the nbits of MSB of a src to LSB of dest */
uint8_t set_msb2lsb(uint8_t dest, uint8_t src, uint8_t nbits) {
    return 0;
}

/** TODO: get the n MSB of byte:
 * shifts MSB to right 0000MSB*/
uint8_t get_nmsb(uint8_t byte, uint8_t nbits) {
    return 0;
}

/** TODO: Set the n LS bits to given value*/
uint8_t set_nlsb(uint8_t byte, uint8_t nbits) {
    return 0;
}

/* Set the LSB of a byte to a bit value (0 or 1) */
uint8_t set_lsb(uint8_t byte, uint8_t bit) {
    return (byte & 0xFE) | (bit & 1);
}

/**Get the LSB of a byte */
uint8_t get_lsb(uint8_t byte) {
    return byte & 1;
}

/**
 *  TODO: convert this code so hide nbit per channel
 *
 */
int encode_message(Image *img, const char *message,
                   uint8_t nbit_per_channel) {
    size_t msg_len = strlen(message) + 1;  // Include null terminator
    size_t total_bits = msg_len * 8;
    size_t total_pixels_needed = (total_bits + 2) / 3;  // Ceiling division

    // Check if the image can hold the message
    if (total_pixels_needed > img->width * img->height) {
        fprintf(stderr, "Message too long! Required pixels: %zu, Available: %zu\n",
                total_pixels_needed, img->width * img->height);
        return -1;
    }

    size_t bit_index = 0;
    for (size_t i = 0; i < msg_len; ++i) {
        char c = message[i];
        // Process each bit of the character (MSB to LSB)
        for (int j = 7; j >= 0; --j) {
            uint8_t bit = (c >> j) & 1;

            size_t pixel_idx = bit_index / 3;
            int channel = bit_index % 3;  // 0=B, 1=G, 2=R

            Pixel *p = &img->pixels[pixel_idx];
            switch (channel) {
            case 0:
                p->b = set_lsb(p->b, bit);
                break;
            case 1:
                p->g = set_lsb(p->g, bit);
                break;
            case 2:
                p->r = set_lsb(p->r, bit);
                break;
            }
            bit_index++;
        }
    }
    return 0;
}

/**
 *  TODO: convert this code to get nbit from per channel
 *
 */
char *decode_message(const Image *img, uint8_t nbit_per_channel) {
    size_t max_chars = (img->width * img->height * 3) / 8;
    char *buffer = malloc(max_chars + 1);  // +1 for null terminator
    if (!buffer) {
        perror("Memory allocation failed");
        return NULL;
    }

    size_t bit_index = 0;
    uint8_t current_byte = 0;
    int bits_collected = 0;

    for (size_t i = 0; i < img->width * img->height; ++i) {
        Pixel p = img->pixels[i];
        // Process B, G, R channels in order
        for (int channel = 0; channel < 3; ++channel) {
            uint8_t bit;
            switch (channel) {
            case 0:
                bit = get_lsb(p.b);
                break;
            case 1:
                bit = get_lsb(p.g);
                break;
            case 2:
                bit = get_lsb(p.r);
                break;
            }

            current_byte = (current_byte << 1) | bit;
            bits_collected++;

            if (bits_collected == 8) {
                buffer[bit_index / 8] = current_byte;
                if (current_byte == '\0') {
                    // Null terminator found
                    buffer[bit_index / 8] = '\0';
                    return buffer;
                }
                current_byte = 0;
                bits_collected = 0;
            }
            bit_index++;
        }
    }
    // If no null terminator found, add it
    buffer[max_chars] = '\0';
    return buffer;
}
```

main.c
```c
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
```
