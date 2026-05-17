#ifndef __SHEET_H
#define __SHEET_H

#include <stdint.h>
#include <stdio.h>

#include "cell.h"

typedef struct {
    const char **column_names;
    const row_number_t *row_numbers;
    cell_t *cells;
    size_t width;
    size_t height;
} sheet_t;

bool sheet_parse(sheet_t *sheet, FILE *file);
void sheet_free(sheet_t *sheet);

bool sheet_resolve(sheet_t *sheet);
void sheet_print(sheet_t *sheet, FILE *file);

#endif // __SHEET_H
