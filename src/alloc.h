#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

#define malloc_s(bytes) _malloc_s(bytes, __FILE__, __LINE__)
#define realloc_s(block, bytes) _realloc_s(block, bytes, __FILE__, __LINE__)

void* _malloc_s(size_t bytes, char* filename, int line);
void* _realloc_s(void* block, size_t bytes, char* filename, int line);

#endif