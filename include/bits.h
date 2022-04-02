#ifndef BITS_H
#define BITS_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    size_t bit_size;
    size_t current;
    uint8_t* bits;
} BitStream;

BitStream* bits_init(size_t byte_size, FILE* fp);
int bits_read_one_bit(BitStream* bits);
uint8_t bits_read_le_u8(BitStream* bits);
uint16_t bits_read_le_u16(BitStream* bits);
uint32_t bits_read_le_u32(BitStream* bits);
float bits_read_le_f32(BitStream* bits);
uint32_t bits_read_bits(size_t bit_size, BitStream* bits);

size_t bits_read_bytes(char* buf, size_t len, BitStream* bits);
char* bits_read_str(size_t* len, BitStream* bits);

void bits_free(BitStream* bits);

#endif