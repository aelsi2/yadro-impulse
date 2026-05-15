#include "lookup.h"

hash_t cell_key_hash(const cell_key_t *key) {
}

void cell_key_free(cell_key_t *key) {
}

void lookup_set(lookup_t *lookup, const cell_key_t *cell_key, const struct cell *cell) {
}

const struct cell *lookup_get(const lookup_t *lookup,
                              const cell_key_t *cell_key) {
}

void lookup_init(lookup_t *lookup) {
}

void lookup_free(lookup_t *lookup) {
}
