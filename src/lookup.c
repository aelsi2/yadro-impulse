#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "lookup.h"

#define LOAD_FACTOR 0.75
#define GROWTH_FACTOR 2
#define INITIAL_CAPACITY 16

static hash_t fnv_1a(const char *data, size_t length) {
    hash_t result = 2166136261;
    for (size_t i = 0; i < length; i++) {
        result ^= (unsigned char)data[i];
        result *= 16777619;
    }
    return result;
}

static hash_t cell_key_hash(const cell_key_t *key) {
    hash_t hashes[2];
    hashes[0] = fnv_1a(key->column_name, strlen(key->column_name));
    hashes[1] = fnv_1a((const char *)&key->row_number, sizeof(key->row_number));
    return fnv_1a((const char *)hashes, sizeof(hashes));
}

bool cell_key_equals(const cell_key_t *a, const cell_key_t *b) {
    if (a->row_number != b->row_number) {
        return false;
    }
    return strcmp(a->column_name, b->column_name) == 0;
}

void cell_key_copy(cell_key_t *to, const cell_key_t *from) {
    size_t col_name_size = strlen(from->column_name) + 1;
    to->column_name = malloc(col_name_size);
    memcpy(to->column_name, from->column_name, col_name_size);
    to->row_number = from->row_number;
}

void cell_key_free(cell_key_t *key) {
    free((void *)key->column_name);
    key->column_name = NULL;
    key->row_number = 0;
}

static bool lookup_slot_isempty(const lookup_slot_t *slot) {
    return slot->key.column_name == NULL;
}

static lookup_slot_t *lookup_find_slot(const lookup_t *lookup,
                                       const cell_key_t *key) {
    size_t initial_index = cell_key_hash(key) % lookup->capacity;
    size_t index = initial_index;
    while (true) {
        lookup_slot_t *slot = &lookup->slots[index];
        index = (index + 1) % lookup->capacity;
        if (lookup_slot_isempty(slot)) {
            return slot;
        }
        if (cell_key_equals(&slot->key, key)) {
            return slot;
        }
        if (index == initial_index) {
            // We've wrapped around the table.
            // This should never happen, since the hash table should have grown.
            assert(false && "Wrapped around the hash map without finding a "
                            "single free slot.");
        }
    }
}

static void lookup_grow(lookup_t *lookup) {
    size_t old_capacity = lookup->capacity;
    lookup_slot_t *old_slots = lookup->slots;
    if (old_capacity >= INITIAL_CAPACITY) {
        lookup->capacity = old_capacity * GROWTH_FACTOR;
    } else {
        lookup->capacity = INITIAL_CAPACITY * GROWTH_FACTOR;
    }
    lookup->slots = calloc(lookup->capacity, sizeof(lookup_slot_t));
    for (size_t i = 0; i < old_capacity; i++) {
        lookup_slot_t *old_slot = &old_slots[i];
        if (lookup_slot_isempty(old_slot)) {
            continue;
        }
        lookup_slot_t *new_slot = lookup_find_slot(lookup, &old_slot->key);
        *new_slot = *old_slot;
    }
    free(old_slots);
}

void lookup_set(lookup_t *lookup, const cell_key_t *key, struct cell *value) {
    if (++lookup->count >= lookup->capacity * LOAD_FACTOR) {
        lookup_grow(lookup);
    }
    lookup_slot_t *slot = lookup_find_slot(lookup, key);
    cell_key_free(&slot->key);
    cell_key_copy(&slot->key, key);
    slot->value = value;
}

struct cell *lookup_get(const lookup_t *lookup, const cell_key_t *key) {
    lookup_slot_t *slot = lookup_find_slot(lookup, key);
    if (lookup_slot_isempty(slot)) {
        return NULL;
    } else {
        return slot->value;
    }
}

void lookup_init(lookup_t *lookup) {
    lookup->count = 0;
    lookup->capacity = INITIAL_CAPACITY;
    lookup->slots = calloc(lookup->capacity, sizeof(lookup_slot_t));
}

void lookup_free(lookup_t *lookup) {
    for (size_t i = 0; i < lookup->capacity; i++) {
        lookup_slot_t *slot = &lookup->slots[i];
        cell_key_free(&slot->key);
    }
    free(lookup->slots);
    lookup->count = 0;
    lookup->capacity = 0;
}
