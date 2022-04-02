#include "bits.h"
#include <stdlib.h>
#include <assert.h>

BitStream* bits_init(size_t byte_size, FILE* fp) {
    BitStream* bits = malloc(sizeof(BitStream));
    if(!bits) return NULL;
    bits->bits = malloc(byte_size);
    if(!bits->bits) {
        free(bits);
        return NULL;
    }
    bits->bit_size = fread(bits->bits, 1, byte_size, fp) * 8;
    bits->current = 0;
    return bits;
}

int bits_read_one_bit(BitStream* bits) {
    if(bits->current >= bits->bit_size) {
        bits->current -= 8;
    }
    int bit = ((bits->bits[bits->current >> 3] >> (bits->current % 8)) & 1);
    bits->current++;
    return bit;
}

uint8_t bits_read_le_u8(BitStream* bits) {
    return bits_read_bits(8, bits);
}

uint16_t bits_read_le_u16(BitStream* bits) {
    return bits_read_bits(16, bits);
}

uint32_t bits_read_le_u32(BitStream* bits) {
    return bits_read_bits(32, bits);
}

float bits_read_le_f32(BitStream* bits) {
    uint32_t n = bits_read_le_u32(bits);
    return *(float*)&n;
}

uint32_t bits_read_bits(size_t bit_size, BitStream* bits) {
    uint32_t n = 0;
    for(size_t i = 0; i < bit_size; i++) {
        n |= bits_read_one_bit(bits) << i;
    }
    return n;
}

size_t bits_read_bytes(char* buf, size_t len, BitStream* bits) {
    uint8_t* ptr = (uint8_t*)buf;
    size_t i;
    for(i = 0; i < len; i++) {
        uint8_t n = 0;
        for(size_t j = 0; j < 8; j++) {
            n = (n << 1) | ((bits->bits[bits->current >> 3] >> (7 - (bits->current % 8))) & 1);
            bits->current++;
        }
        *ptr = n;
        ptr++;
    }
    return i;
}

char* bits_read_str(size_t* len, BitStream* bits) {
    uint8_t* buf = NULL;
    uint8_t n;
    *len = 0;
    do {
        if(bits->current + 8 > bits->bit_size) break;
        buf = realloc(buf, *len + 1);
        n = 0;
        for(size_t i = 0; i < 8; i++) {
            n = (n << 1) | ((bits->bits[bits->current >> 3] >> (7 - (bits->current % 8))) & 1);
            bits->current++;
        }
        buf[*len] = n;
        (*len)++;
    } while(n);
    return (char*)buf;
}

void bits_free(BitStream* bits) {
    free(bits->bits);
    free(bits);
}