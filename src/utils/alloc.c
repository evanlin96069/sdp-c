#include "alloc.h"
#include "print.h"

void* _malloc_s(size_t bytes, char* filename, int line) {
    void* ptr = malloc(bytes);
    if (!ptr) {
        error("malloc failed at %s, line %d.\n", filename, line);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* _realloc_s(void* block, size_t bytes, char* filename, int line) {
    void* ptr = realloc(block, bytes);
    if (!ptr) {
        error("realloc failed at %s, line %d.\n", filename, line);
        exit(EXIT_FAILURE);
    }
    return ptr;
}
