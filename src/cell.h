#ifndef __CELL_H
#define __CELL_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PRI_VALUE PRIi64
#define PRI_ROW PRIu64

#define VALUE_MIN INT64_MIN
#define VALUE_MAX INT64_MAX
#define ROW_MAX UINT64_MAX

// Row index (used in cell references).
typedef uint64_t row_number_t;

// Cell value (parsed from the input file and used in calculations).
typedef int64_t value_t;

typedef struct {
    const char *column_name;
    row_number_t row_number;
} cell_ref_t;

// Frees the resources owned by the cell reference.
void cell_ref_free(cell_ref_t *ref);

typedef enum {
    CV_VALUE = 0,    // Literal value
    CV_CELL_REF = 1, // Cell reference
} cell_val_type_t;

typedef struct {
    union {
        cell_ref_t cell_ref;
        value_t value;
    } data;
    cell_val_type_t type;
} cell_val_t;

// Frees the resources owned by the cell value.
void cell_val_free(cell_val_t *val);

typedef enum {
    OP_NONE = 0, // No operation (cell value is treated as a literal)
    OP_ADD = 1,
    OP_SUB = 2,
    OP_MUL = 3,
    OP_DIV = 4,
} op_type_t;

// Spreadsheet cell.
// When operator is equal to OP_NONE, the second operand is ignored,
// and the cell is assumed to have the value of the first operand.
typedef struct cell {
    cell_val_t left;
    cell_val_t right;
    op_type_t op;
} cell_t;

struct lookup;

// Attempts to resolve the value of the cell.
//
// depth_limit is decremented for every recursive cell_resolve
// call. The function fails if it is equal to zero.
//
// - On success: sets cell->op to OP_NONE, writes the value to cell->left
// and returns false.
// - On failure: prints the error and the location specified by loc to stderr
// and returns true.
bool cell_resolve(cell_t *cell, const struct lookup *lookup,
                  uint32_t depth_limit, const cell_ref_t *loc);

// Frees the data owned by the cell.
void cell_free(cell_t *cell);

// Prints the cell to the specified file.
void cell_print(cell_t *cell, FILE *file);

#endif // __CELL_H
