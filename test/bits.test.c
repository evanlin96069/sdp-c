#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../include/bits.h"

int tests_run = 0;

static char* test_read_aligned_bits() {
    uint8_t data[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD };
    BitStream bits = { .bit_size = sizeof(data) * 8,.bits = data,.current = 0 };
    mu_assert("[FAILED] Read byte aligned u8.", bits_read_le_u8(&bits) == 0x01);
    mu_assert("[FAILED] Read byte aligned u16.", bits_read_le_u16(&bits) == 0x4523);
    mu_assert("[FAILED] Read byte aligned u32.", bits_read_le_u32(&bits) == 0xCDAB8967);
    mu_assert("[FAILED] Read aligned bytes: bit_size != current", bits.bit_size == bits.current);
    return 0;
}

static char* test_read_non_aligned_bits() {
    uint8_t data[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    BitStream bits = { .bit_size = sizeof(data) * 8,.bits = data,.current = 0 };
    mu_assert("[FAILED] Read 4 bits.", bits_read_bits(4, &bits) == 0x1);
    mu_assert("[FAILED] Read byte non-aligned u8.", bits_read_le_u8(&bits) == 0x30);
    mu_assert("[FAILED] Read byte non-aligned u16.", bits_read_le_u16(&bits) == 0x7452);
    mu_assert("[FAILED] Read byte non-aligned u32.", bits_read_le_u32(&bits) == 0xFCDAB896);
    mu_assert("[FAILED] Read 3 bits.", bits_read_bits(3, &bits) == 6);
    mu_assert("[FAILED] Read one bit.", bits_read_one_bit(&bits) == 1);
    mu_assert("[FAILED] Read non-aligned bytes: bit_size != current", bits.bit_size == bits.current);
    return 0;
}

static char* test_read_f32() {
    // 4.5, 8.125, 500.25, 123456.875
    uint32_t data[] = { 0x40900000 , 0x41020000, 0x43fa2000, 0x47f12070 };
    BitStream bits = { .bit_size = sizeof(data) * 8,.bits = (uint8_t*)data,.current = 0 };
    mu_assert("[FAILED] bits_read_le_f32 4.5", bits_read_le_f32(&bits) == 4.5f);
    mu_assert("[FAILED] bits_read_le_f32 8.125", bits_read_le_f32(&bits) == 8.125f);
    mu_assert("[FAILED] bits_read_le_f32 500.25", bits_read_le_f32(&bits) == 500.25f);
    mu_assert("[FAILED] bits_read_le_f32 123456.875", bits_read_le_f32(&bits) == 123456.875f);
    return 0;
}

static char* test_read_bytes() {
    char data[32] = "Portal";
    char buffer[32];
    BitStream bits = { .bit_size = sizeof(data) * 8,.bits = (uint8_t*)data,.current = 0 };
    bits_read_bytes(buffer, 32, &bits);
    mu_assert("[FAILED] bits_read_bytes", strcmp(buffer, "Portal") == 0);
    return 0;
}

static char* test_read_str() {
    char data[32] = "Portal";
    char buffer[32];
    BitStream bits = { .bit_size = sizeof(data) * 8,.bits = (uint8_t*)data,.current = 0 };
    bits_read_str(buffer, 32, &bits);
    mu_assert("[FAILED] bits_read_bytes", strcmp(buffer, "Portal") == 0);
    return 0;
}

static char* all_tests() {
    mu_run_test(test_read_aligned_bits);
    mu_run_test(test_read_non_aligned_bits);
    mu_run_test(test_read_f32);
    mu_run_test(test_read_bytes);
    mu_run_test(test_read_str);
    return 0;
}

int main(void) {
    printf("Testing bits.test\n");
    char* result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("[SUCCESS] All Test passed.\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}