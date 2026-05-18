#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define INITIAL_CAPACITY 4
#define GROWTH_FACTOR 2

static void print_bad_alloc(FILE *file) {
    fprintf(file, "error: could not allocate memory for spreadsheet data\n");
}

void vec_init(vec_t *vec, size_t element_size,
              void (*elem_free)(void *, size_t)) {
    vec->element_free = elem_free;
    vec->element_size = element_size;
    vec->count = 0;
    if (element_size == 0) {
        vec->capacity = 0;
        vec->data = NULL;
        return;
    }
    vec->capacity = INITIAL_CAPACITY;
    if (vec->element_size > SIZE_MAX / vec->capacity) {
        print_bad_alloc(stderr);
        exit(2);
    }
    vec->data = malloc(vec->capacity * vec->element_size);
    if (vec->data == NULL) {
        print_bad_alloc(stderr);
        exit(2);
    }
}

void vec_free(vec_t *vec) {
    if (vec->element_free != NULL) {
        for (size_t i = 0; i < vec->count; i++) {
            char *ptr = vec->data + vec->element_size * i;
            vec->element_free((void *)ptr, vec->element_size);
        }
    }
    free((void *)vec->data);
    vec->data = NULL;
    vec->capacity = 0;
    vec->count = 0;
    vec->element_size = 0;
}

static void vec_ensure_capacity(vec_t *vec) {
    if (vec->count < vec->capacity) {
        return;
    }
    if (vec->capacity > SIZE_MAX / vec->element_size / GROWTH_FACTOR) {
        print_bad_alloc(stderr);
        exit(2);
    }
    vec->capacity *= GROWTH_FACTOR;
    vec->data = realloc(vec->data, vec->capacity * vec->element_size);
    if (vec->data == NULL) {
        print_bad_alloc(stderr);
        exit(2);
    }
}

void *vec_append(vec_t *vec, const void *data) {
    if (vec->element_size == 0) {
        vec->count += 1;
        return (void *)vec->data;
    }
    vec_ensure_capacity(vec);
    char *data_ptr = vec->data + (vec->element_size * vec->count);
    if (data != NULL) {
        memcpy((void *)data_ptr, data, vec->element_size);
    } else {
        memset((void *)data_ptr, 0, vec->element_size);
    }
    vec->count += 1;
    return (void *)data_ptr;
}
