#ifndef LSBSTEGAN_H
#define LSBSTEGAN_H
#include <stdint.h>
#include "image.h"

extern int encode_message(Image *img, const char *message,uint8_t nbit_per_channel);
extern uint8_t set_lsb(uint8_t byte, uint8_t bit);
extern uint8_t get_lsb(uint8_t byte);
extern char *decode_message(const Image *img,uint8_t nbit_per_channel);
#endif