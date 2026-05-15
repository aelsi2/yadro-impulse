#ifndef __LOOKUP_H
#define __LOOKUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t row_number_t;
typedef uint32_t hash_t;

struct cell;

typedef struct {
    const char *column_name;
    row_number_t row_number;
} cell_key_t;

hash_t cell_key_hash(const cell_key_t *key);
void cell_key_free(cell_key_t *key);

typedef struct {
    cell_key_t key;
    const struct cell *cell;
    bool occupied;
} lookup_slot_t;

typedef struct {
    lookup_slot_t *slots;
    size_t capacity;
} lookup_t;

void lookup_set(lookup_t *lookup, const cell_key_t *cell_key, const struct cell *cell);
const struct cell *lookup_get(const lookup_t *lookup,
                              const cell_key_t *cell_key);

void lookup_init(lookup_t *lookup);
void lookup_free(lookup_t *lookup);

#endif // __LOOKUP_H
