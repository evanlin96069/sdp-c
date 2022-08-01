#include "alloc.h"

#include <stdio.h>

void* _malloc_s(size_t bytes, char* filename, int line) {
    void* ptr = malloc(bytes);
    if (!ptr) {
        fprintf(stderr, "[ERROR] malloc failed at %s, line %d.\n", filename, line);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* _realloc_s(void* block, size_t bytes, char* filename, int line) {
    void* ptr = realloc(block, bytes);
    if (!ptr) {
        fprintf(stderr, "[ERROR] realloc failed at %s, line %d.\n", filename, line);
        exit(EXIT_FAILURE);
    }
    return ptr;
}