#include <stdlib.h>
#include <string.h>
#include "bits.h"
#include "vector.h"
#include "utils/alloc.h"
#include "utils/print.h"

BitStream* bits_init(uint8_t* data, size_t byte_size) {
    BitStream* bits = malloc_s(sizeof(BitStream));
    // make sure it's multiple of 8 bytes
    size_t real_byte_size = byte_size + 8 - (byte_size & 0x7);
    bits->bits = malloc_s(real_byte_size);
    bits->byte_size = real_byte_size;
    bits->bit_size = byte_size << 3;
    memcpy(bits->bits, data, byte_size);
    bits->current = 0;
    bits_fetch(bits);
    return bits;
}

BitStream* bits_load_file(char* path) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }

    size_t byte_size;
    fseek(fp, 0L, SEEK_END);
    byte_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    BitStream* bits = malloc_s(sizeof(BitStream));
    // make sure it's multiple of 8 bytes
    size_t real_byte_size = byte_size + 8 - (byte_size & 0x7);
    bits->bits = malloc_s(real_byte_size);
    bits->byte_size = real_byte_size;
    bits->bit_size = fread(bits->bits, 1, byte_size, fp) << 3;
    bits->current = 0;
    bits_fetch(bits);

    fclose(fp);
    return bits;
}

void bits_skip(size_t bit_num, BitStream* bits) {
    bits->current += bit_num;
    if (bits->current > bits->bit_size) {
        error("BitStream overflow when skipping.\n");
        abort();
    }
    bits_fetch(bits);
}

void bits_setpos(size_t bit_pos, BitStream* bits) {
    if (bit_pos > bits->bit_size) {
        error("BitStream overflow when setpos.\n");
        abort();
    }
    bits->current = bit_pos;
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
        error("BitsStream overflow when reading.\n");
        abort();
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

uint32_t bits_read_varuint32(BitStream* bits) {
    uint32_t result = 0;
    for (int i = 0; i < 5;i++) {
        uint32_t b = bits_read_le_u8(bits);
        result |= (b & 0x7f) << (7 * i);
        if ((b & 0x80) == 0)
            break;
    }
    return result;
}

static BitCoord _bits_read_bitcoord(BitStream* bits) {
    BitCoord result = { 0 };
    result.exists = true;
    result.has_int = bits_read_one_bit(bits);
    result.has_frac = bits_read_one_bit(bits);

    if (result.has_int || result.has_frac) {
        result.sign = bits_read_one_bit(bits);
        if (result.has_int)
            result.int_value = bits_read_bits(COORD_INTEGER_BITS, bits);
        if (result.has_frac)
            result.frac_value = bits_read_bits(COORD_FRACTIONAL_BITS, bits);
    }
    return result;
}

VectorCoord bits_read_vcoord(BitStream* bits) {
    VectorCoord result = { 0 };
    result.x.exists = bits_read_one_bit(bits);
    result.y.exists = bits_read_one_bit(bits);
    result.z.exists = bits_read_one_bit(bits);
    if (result.x.exists)
        result.x = _bits_read_bitcoord(bits);
    if (result.y.exists)
        result.y = _bits_read_bitcoord(bits);
    if (result.z.exists)
        result.z = _bits_read_bitcoord(bits);
    return result;
}

float bits_read_bit_angle(size_t bit_size, BitStream* bits) {
    float result;
    int i;
    float shift;

    shift = (float)(1 << bit_size);

    i = bits_read_bits(bit_size, bits);
    result = (float)i * (360.0 / shift);

    return result;
}

uint8_t* bits_read_bits_arr(size_t bit_size, BitStream* bits) {
    if (bit_size <= 0)
        return NULL;
    VECTOR(uint8_t) arr = { 0 };
    uint8_t n;
    while (bit_size > 8) {
        n = bits_read_le_u8(bits);
        bit_size -= 8;
        vector_push(arr, n);
    }
    if (bit_size) {
        n = bits_read_bits(bit_size, bits);
        vector_push(arr, n);
    }
    vector_shrink(arr);
    return arr.data;
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

char* bits_read_str(BitStream* bits) {
    VECTOR(char) str = { 0 };
    uint8_t n;
    do {
        n = bits_read_le_u8(bits);
        vector_push(str, n);
    } while (n);
    vector_shrink(str);
    return str.data;
}

void bits_free(BitStream* bits) {
    free(bits->bits);
    free(bits);
}
