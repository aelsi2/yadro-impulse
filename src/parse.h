#ifndef __PARSE_H
#define __PARSE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "sheet.h"

// Parser character
typedef int16_t pch_t;

typedef struct {
    FILE *file;
    uint32_t line;
    uint32_t column;
    pch_t peek_value;
    char col_sep;
} parser_t;

// Initializes a parser with the given file.
// The parser does NOT take ownership of the file.
void parser_init(parser_t *parser, FILE *file, char col_sep);

// Reads a spreadsheet with the parser.
// Returns false and writes the parsed spreadsheet to sheet on success.
bool parser_read_sheet(parser_t *parser, sheet_t *sheet);

#endif // __PARSE_H
