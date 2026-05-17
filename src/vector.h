#ifndef __VECTOR_H
#define __VECTOR_H

#include <stddef.h>

// A generic vector
typedef struct {
    char *data;
    size_t capacity;
    size_t count;
    size_t element_size;
    void (*element_free)(void *, size_t);
} vec_t;

// Initializes a vector.
// - element_size is the size of one vector element.
// - elem_free is the destructor that will be called for each element in
// vec_free (can be NULL, means noop). First parameter of the destructor is the
// address of the element, second parameter is the size of the element.
void vec_init(vec_t *vec, size_t element_size,
              void (*element_free)(void *, size_t));

// Frees the resources owned by the vector.
void vec_free(vec_t *vec);

// Copies the value behind the pointer to the end of the vector.
// If the pointer is NULL, zero-initializes the element.
// Returns the pointer to the new element.
void *vec_append(vec_t *vec, const void *value);

#endif // __VECTOR_H
