#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "lookup.h"

#define CAPACITY_FACTOR 2

typedef uint32_t hash_t;

static hash_t fnv_1a(const char *data, size_t length) {
    hash_t result = 2166136261;
    for (size_t i = 0; i < length; i++) {
        result ^= (unsigned char)data[i];
        result *= 16777619;
    }
    return result;
}

static bool col_slot_isempty(const col_slot_t *slot) {
    return slot->name == NULL;
}

static bool row_slot_isempty(const row_slot_t *slot) {
    return !slot->is_taken;
}

static row_slot_t *lookup_find_row_slot(const lookup_t *lookup,
                                        row_number_t row_number) {
    size_t index = fnv_1a((const char *)&row_number, sizeof(row_number_t)) %
                   lookup->row_capacity;
    while (true) {
        row_slot_t *slot = &lookup->row_slots[index];
        index = (index + 1) % lookup->row_capacity;
        if (row_slot_isempty(slot)) {
            return slot;
        }
        if (slot->number == row_number) {
            return slot;
        }
    }
}

static col_slot_t *lookup_find_col_slot(const lookup_t *lookup,
                                        const char *column_name) {
    size_t index =
        fnv_1a(column_name, strlen(column_name)) % lookup->col_capacity;
    while (true) {
        col_slot_t *slot = &lookup->col_slots[index];
        index = (index + 1) % lookup->col_capacity;
        if (col_slot_isempty(slot)) {
            return slot;
        }
        if (strcmp(slot->name, column_name) == 0) {
            return slot;
        }
    }
}

struct cell *lookup_get(const lookup_t *lookup, const cell_ref_t *key) {
    col_slot_t *col = lookup_find_col_slot(lookup, key->column_name);
    if (col_slot_isempty(col)) {
        return NULL;
    }
    row_slot_t *row = lookup_find_row_slot(lookup, key->row_number);
    if (row_slot_isempty(row)) {
        return NULL;
    }

    size_t index = col->index + row->index * lookup->sheet->width;
    return &lookup->sheet->cells[index];
}

bool lookup_init(lookup_t *lookup, sheet_t *sheet) {
    if (sheet->height > SIZE_MAX / CAPACITY_FACTOR) {
        fprintf(stderr, "error: too many rows to build a lookup\n");
        return true;
    }
    if (sheet->width > SIZE_MAX / CAPACITY_FACTOR) {
        fprintf(stderr, "error: too many columns to build a lookup\n");
        return true;
    }

    lookup->sheet = sheet;
    lookup->row_capacity = sheet->height * CAPACITY_FACTOR;
    lookup->col_capacity = sheet->width * CAPACITY_FACTOR;
    lookup->row_slots = calloc(sizeof(row_slot_t), lookup->row_capacity);
    if (lookup->row_slots == NULL) {
        fprintf(stderr, "error: could not allocate memory for lookup\n");
        return true;
    }
    lookup->col_slots = calloc(sizeof(col_slot_t), lookup->col_capacity);
    if (lookup->col_slots == NULL) {
        free(lookup->row_slots);
        fprintf(stderr, "error: could not allocate memory for lookup\n");
        return true;
    }

    for (size_t i = 0; i < sheet->width; i++) {
        const char *name = sheet->column_names[i];
        col_slot_t *slot = lookup_find_col_slot(lookup, name);
        if (!col_slot_isempty(slot)) {
            free(lookup->row_slots);
            free(lookup->col_slots);
            fprintf(stderr, "error: duplicate column name: %s\n", name);
            return true;
        }
        slot->name = name;
        slot->index = i;
    }
    for (size_t i = 0; i < sheet->height; i++) {
        row_number_t number = sheet->row_numbers[i];
        row_slot_t *slot = lookup_find_row_slot(lookup, number);
        if (!row_slot_isempty(slot)) {
            free(lookup->row_slots);
            free(lookup->col_slots);
            fprintf(stderr, "error: duplicate row number: %" PRI_ROW "\n",
                    number);
            return true;
        }
        slot->number = number;
        slot->index = i;
        slot->is_taken = true;
    }
    return false;
}

void lookup_free(lookup_t *lookup) {
    free((void *)lookup->row_slots);
    free((void *)lookup->col_slots);
    lookup->sheet = NULL;
    lookup->row_slots = NULL;
    lookup->col_slots = NULL;
    lookup->row_capacity = 0;
    lookup->col_capacity = 0;
}
