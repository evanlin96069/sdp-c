#include "vector.h"
#include "alloc.h"

void _vector_make_room(_Vector* _vec, size_t item_size) {
    if (!_vec->capacity) {
        _vec->data = malloc_s(item_size * VECTOR_MIN_CAPACITY);
        _vec->capacity = VECTOR_MIN_CAPACITY;
    }
    if (_vec->size >= _vec->capacity) {
        _vec->capacity *= VECTOR_EXTEND_RATE;
        _vec->data = realloc_s(_vec->data, _vec->capacity * item_size);
    }
}