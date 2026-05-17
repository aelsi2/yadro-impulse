#include <stdlib.h>

#include "lookup.h"
#include "parse.h"
#include "sheet.h"

#define RESOLVE_DEPTH 20

bool sheet_parse(sheet_t *sheet, FILE *file) {
    parser_t parser;
    parser_init(&parser, file, ',');
    return parser_read_sheet(&parser, sheet);
}

void sheet_free(sheet_t *sheet) {
    if (sheet->column_names) {
        for (size_t col = 0; col < sheet->width; col++) {
            free((void *)sheet->column_names[col]);
        }
        free((void *)sheet->column_names);
        sheet->column_names = NULL;
    }
    free((void *)sheet->row_numbers);
    sheet->row_numbers = NULL;
    if (sheet->cells) {
        for (size_t row = 0; row < sheet->height; row++) {
            for (size_t col = 0; col < sheet->width; col++) {
                cell_t *cell = &sheet->cells[row * sheet->width + col];
                cell_free(cell);
            }
        }
        free((void *)sheet->cells);
        sheet->cells = NULL;
    }
    sheet->width = 0;
    sheet->height = 0;
}

bool sheet_resolve(sheet_t *sheet) {
    cell_ref_t loc;
    lookup_t lookup;
    if (lookup_init(&lookup, sheet)) {
        return true;
    }

    for (size_t row = 0; row < sheet->height; row++) {
        loc.row_number = sheet->row_numbers[row];
        for (size_t col = 0; col < sheet->width; col++) {
            loc.column_name = sheet->column_names[col];
            cell_t *cell = &sheet->cells[row * sheet->width + col];
            if (cell_resolve(cell, &lookup, RESOLVE_DEPTH, &loc)) {
                lookup_free(&lookup);
                return true;
            }
        }
    }
    lookup_free(&lookup);
    return false;
}

void sheet_print(sheet_t *sheet, FILE *file) {
    for (size_t col = 0; col < sheet->width; col++) {
        fprintf(file, ",%s", sheet->column_names[col]);
    }
    fprintf(file, "\n");
    for (size_t row = 0; row < sheet->height; row++) {
        fprintf(file, "%lu", sheet->row_numbers[row]);
        for (size_t col = 0; col < sheet->width; col++) {
            fprintf(file, ",");
            cell_print(&sheet->cells[row * sheet->width + col], file);
        }
        fprintf(file, "\n");
    }
}
