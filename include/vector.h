#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include "alloc.h"

#define VECTOR_MIN_CAPACITY 16
#define VECTOR_EXTEND_RATE 1.5

#define VECTOR(type)        \
struct {                    \
    size_t size;            \
    size_t capacity;        \
    type *data;             \
}

typedef struct {
    size_t size;
    size_t capacity;
    void* data;
} _Vector;

#ifdef __GNUC__
__attribute__((unused))
#endif
static void _vector_make_room(_Vector* _vec, size_t item_size) {
    if (!_vec->capacity) {
        _vec->data = malloc_s(item_size * VECTOR_MIN_CAPACITY);
        _vec->capacity = VECTOR_MIN_CAPACITY;
    }
    if (_vec->size >= _vec->capacity) {
        _vec->capacity *= VECTOR_EXTEND_RATE;
        _vec->data = realloc_s(_vec->data, _vec->capacity * item_size);
    }
}

#define vector_push(vec, val) do {                      \
    _vector_make_room((_Vector*)&(vec), sizeof(val));   \
    (vec).data[(vec).size++] = (val);                   \
} while(0)

#define vector_pop(vec) ((vec).data[--(vec).size])

#define vector_shrink(vec) do {                                     \
    ((_Vector*)&(vec))->data = realloc_s(vec.data, sizeof(*vec.data) * vec.size);   \
} while(0)

#endif