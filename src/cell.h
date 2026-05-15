#ifndef __CELL_H
#define __CELL_H

#include <stdbool.h>
#include <stdint.h>

#include "lookup.h"

typedef int64_t value_t;

typedef enum {
    CV_CELLREF,
    CV_VALUE,
} cell_val_type_t;

typedef struct {
    union {
        cell_key_t cell_ref;
        value_t value;
    } data;
    cell_val_type_t type;
} cell_val_t;

typedef enum {
    OP_NONE = 0, // No operation (cell value is treated as a literal)
    OP_ADD = 1,
    OP_SUB = 2,
    OP_MUL = 3,
    OP_DIV = 4,
} op_type_t;

typedef struct cell {
    cell_val_t left;
    cell_val_t right; // Ignored when op is OP_NONE
    op_type_t op;
    bool resolving; // This flag is set to true to indicate that a cell value
                    // is currently being resolved to detect dependency cycles.
} cell_t;

// Attempts to resolve the value of the cell.
// On success: sets cell->op to OP_NONE, writes the value to cell->left and
// returns false. On failure: prints the error to stderr and returns true.
bool cell_resolve(cell_t *cell, const lookup_t *lookup);

// Frees the data owned by the cell.
void cell_free(cell_t *cell);

#endif // __CELL_H
