#ifndef __LOOKUP_H
#define __LOOKUP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cell.h"
#include "sheet.h"

typedef struct {
    row_number_t number;
    size_t index;
    bool is_taken;
} row_slot_t;

typedef struct {
    const char *name;
    size_t index;
} col_slot_t;

typedef struct lookup {
    sheet_t *sheet;
    row_slot_t *row_slots;
    col_slot_t *col_slots;
    size_t row_capacity;
    size_t col_capacity;
} lookup_t;

bool lookup_init(lookup_t *lookup, sheet_t *sheet);
void lookup_free(lookup_t *lookup);

cell_t *lookup_get(const lookup_t *lookup, const cell_ref_t *cell_key);


#endif // __LOOKUP_H
