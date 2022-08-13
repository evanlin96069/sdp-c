#ifndef BITS_H
#define BITS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t bit_size;
    size_t byte_size;
    size_t current;
    uint64_t fetch;
    uint8_t offset;
    uint8_t* bits;
} BitStream;

#define COORD_INTEGER_BITS 14
#define COORD_FRACTIONAL_BITS 5
typedef struct {
    bool exists;
    bool has_int;
    bool has_frac;
    bool sign;
    uint16_t int_value;
    uint16_t frac_value;
} BitCoord;

typedef struct {
    BitCoord x, y, z;
} VectorCoord;

BitStream* bits_init(uint8_t* data, size_t byte_size);
BitStream* bits_load_file(char* path);

void bits_skip(size_t bit_num, BitStream* bits);
void bits_setpos(size_t bit_pos, BitStream* bits);
void bits_fetch(BitStream* bits);

int bits_read_one_bit(BitStream* bits);
uint8_t bits_read_le_u8(BitStream* bits);
uint16_t bits_read_le_u16(BitStream* bits);
uint32_t bits_read_le_u32(BitStream* bits);
float bits_read_le_f32(BitStream* bits);
uint32_t bits_read_bits(size_t bit_size, BitStream* bits);

uint32_t bits_read_varuint32(BitStream* bits);
VectorCoord bits_read_vcoord(BitStream* bits);
float bits_read_bit_angle(size_t bit_size, BitStream* bits);

uint8_t* bits_read_bits_arr(size_t bit_size, BitStream* bits);
size_t bits_read_bytes(char* buf, size_t len, BitStream* bits);
char* bits_read_str(BitStream* bits);

void bits_free(BitStream* bits);

#endif