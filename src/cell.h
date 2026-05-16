#ifndef __CELL_H
#define __CELL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "lookup.h"

#define VALUE_MIN INT64_MIN
#define VALUE_MAX INT64_MAX

typedef int64_t value_t;

typedef enum {
    CV_VALUE = 0,
    CV_CELL_REF = 1,
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
} cell_t;

// Attempts to resolve the value of the cell.
//
// depth_limit is decremented for every recursive cell_resolve
// call. The function fails if it is equal to zero.
//
// - On success: sets cell->op to OP_NONE, writes the value to cell->left
// and returns false.
// - On failure: prints the error and the location specified by loc to stderr
// and returns true.
bool cell_resolve(cell_t *cell, const lookup_t *lookup, uint32_t depth_limit,
                  const cell_key_t *loc);

// Frees the data owned by the cell.
void cell_free(cell_t *cell);

// Prints the cell to the specified file.
void cell_print(cell_t *cell, FILE *file);

#endif // __CELL_H
