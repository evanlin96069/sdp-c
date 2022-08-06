#include <stdlib.h>
#include <string.h>
#include "bits.h"
#include "alloc.h"

BitStream* bits_init(size_t byte_size, uint8_t* data) {
    BitStream* bits = malloc_s(sizeof(BitStream));
    bits->bit_size = byte_size << 3;
    // make sure it's multiple of 8 bytes
    size_t real_byte_size = byte_size + 8 - (byte_size & 0x7);
    bits->bits = malloc_s(real_byte_size);
    bits->byte_size = real_byte_size;
    memcpy(bits->bits, data, byte_size);
    bits->current = 0;
    bits_fetch(bits);
    return bits;
}

BitStream* bits_init_file(size_t byte_size, FILE* fp) {
    BitStream* bits = malloc_s(sizeof(BitStream));
    // make sure it's multiple of 8 bytes
    size_t real_byte_size = byte_size + 8 - (byte_size & 0x7);
    bits->bits = malloc_s(real_byte_size);
    bits->byte_size = real_byte_size;
    bits->bit_size = fread(bits->bits, 1, byte_size, fp) << 3;
    bits->current = 0;
    bits_fetch(bits);
    return bits;
}

void bits_skip(size_t bit_num, BitStream* bits) {
    bits->current += bit_num;
    if (bits->current > bits->bit_size) {
        fprintf(stderr, "[ERROR] BitStream overflow when skipping.\n");
        exit(EXIT_FAILURE);
    }
    bits_fetch(bits);
}

void bits_fetch(BitStream* bits) {
    size_t block = bits->current >> 3;
    if (block + 8 > bits->byte_size) {
        block = bits->byte_size - 8;
    }
    bits->fetch = *(uint64_t*)(&bits->bits[block]);
    bits->offset = bits->current - (block << 3);
}

int bits_read_one_bit(BitStream* bits) {
    return bits_read_bits(1, bits);
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
    if (bits->current + bit_size > bits->bit_size) {
        fprintf(stderr, "[ERROR] BitsStream overflow when reading.\n");
        exit(EXIT_FAILURE);
    }
    uint32_t n = 0;
    size_t remain = 64 - bits->offset;
    size_t shift = 0;
    if (bit_size > remain) {
        // need two chunk
        n |= (bits->fetch >> bits->offset) & ((1 << remain) - 1);
        bits_skip(remain, bits);
        bit_size -= remain;
        shift = remain;
    }
    n |= ((bits->fetch >> bits->offset) & (((uint64_t)1 << bit_size) - 1)) << shift;
    bits->offset += bit_size;
    bits->current += bit_size;
    return n;
}

size_t bits_read_bytes(char* buf, size_t len, BitStream* bits) {
    uint8_t* ptr = (uint8_t*)buf;
    size_t i;
    for (i = 0; i < len; i++) {
        *ptr = bits_read_le_u8(bits);
        ptr++;
    }
    return i;
}

size_t bits_read_str(char* dest, size_t max_bytes, BitStream* bits) {
    if (!max_bytes)
        return 0;

    uint8_t n;
    size_t len = 0;
    do {
        n = dest[len] = bits_read_le_u8(bits);
        len++;
    } while (n && len < max_bytes);
    return len;
}

void bits_free(BitStream* bits) {
    free(bits->bits);
    free(bits);
}