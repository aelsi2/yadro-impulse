#ifndef __LOOKUP_H
#define __LOOKUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t row_number_t;
typedef uint32_t hash_t;

struct cell;

typedef struct {
    char *column_name;
    row_number_t row_number;
} cell_key_t;

bool cell_key_equals(const cell_key_t *a, const cell_key_t *b);
void cell_key_copy(cell_key_t *to, const cell_key_t *from);
void cell_key_free(cell_key_t *key);

typedef struct {
    cell_key_t key;
    struct cell *value;
} lookup_slot_t;

typedef struct {
    lookup_slot_t *slots;
    size_t count;
    size_t capacity;
} lookup_t;

void lookup_set(lookup_t *lookup, const cell_key_t *cell_key,
                struct cell *cell);
struct cell *lookup_get(const lookup_t *lookup, const cell_key_t *cell_key);

void lookup_init(lookup_t *lookup);
void lookup_free(lookup_t *lookup);

#endif // __LOOKUP_H
