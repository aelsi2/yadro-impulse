#include "parse.h"
#include "cell.h"
#include "vector.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define PCH_NONE (-1)
#define PCH_EOF (-2)
#define PCH_ERROR (-3)

void parser_init(parser_t *parser, FILE *file, char col_sep) {
    parser->file = file;
    parser->line = 1;
    parser->column = 1;
    parser->peek_value = PCH_NONE;
    parser->col_sep = col_sep;
}

static void print_error_loc(FILE *file, parser_t *parser) {
    fprintf(file, "\tat file position %" PRIu32 ":%" PRIu32 "\n", parser->line,
            parser->column);
}

static bool char_is_valid(parser_t *parser, char ch) {
    if (isalpha(ch) || isdigit(ch)) {
        return true;
    }
    if (strchr("+-*/=_\n", ch) != NULL) {
        return true;
    }
    if (ch == parser->col_sep) {
        return true;
    }
    return false;
}

static pch_t parser_peek(parser_t *parser) {
    if (parser->peek_value != PCH_NONE) {
        return parser->peek_value;
    }
    int result = fgetc(parser->file);
    if (result == EOF && ferror(parser->file)) {
        perror("file read error");
        parser->peek_value = PCH_ERROR;
    } else if (result == EOF) {
        parser->peek_value = PCH_EOF;
    } else if (!char_is_valid(parser, result)) {
        fprintf(stderr, "error: invalid character\n");
        print_error_loc(stderr, parser);
        parser->peek_value = PCH_ERROR;
    } else {
        parser->peek_value = result;
    }
    return parser->peek_value;
}

static pch_t parser_consume(parser_t *parser) {
    pch_t result = parser_peek(parser);
    if (result != PCH_EOF && result != PCH_ERROR) {
        parser->peek_value = PCH_NONE;
    }
    if (result == '\n') {
        parser->line += 1;
        parser->column = 1;
    } else {
        parser->column += 1;
    }
    return result;
}

static bool parser_consume_line(parser_t *parser) {
    while (true) {
        pch_t ch = parser_peek(parser);
        switch (ch) {
        case '\n':
            parser_consume(parser);
            return false;
        case PCH_EOF:
            return false;
        case PCH_ERROR:
            return true;
        default:
            break;
        }
        parser_consume(parser);
    }
}

static bool parser_read_name(parser_t *parser, char **name) {
    pch_t pch = parser_peek(parser);
    if (pch == PCH_EOF || pch == '\n' || pch == parser->col_sep) {
        fprintf(stderr, "error: column name cannot be empty\n");
        print_error_loc(stderr, parser);
        return true;
    }
    if (pch > 0 && !isalpha(pch) && pch != '_') {
        fprintf(stderr, "error: column name must contain only letters and '_'\n");
        print_error_loc(stderr, parser);
        return true;
    }
    
    vec_t vec;
    vec_init(&vec, sizeof(char), NULL);

    while (true) {
        pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            vec_free(&vec);
            return true;
        }
        if (pch == PCH_EOF) {
            break;
        }
        if (!isalpha(pch) && pch != '_') {
            break;
        }
        char ch = pch;
        vec_append(&vec, &ch);
        parser_consume(parser);
    }

    static const char zero = '\0';
    vec_append(&vec, &zero);
    *name = (char *)vec.data;

    return false;
}

static bool parser_read_unsigned(parser_t *parser, uint64_t *number) {
    uint64_t result = 0;
    bool has_digits = false;
    while (true) {
        pch_t pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            return true;
        }
        if (pch == PCH_EOF || !isdigit(pch)) {
            break;
        }
        parser_consume(parser);
        int value = (char)pch - '0';
        has_digits = true;

        if (result > (UINT64_MAX - value) / 10) {
            fprintf(stderr, "error: number has too many digits\n");
            print_error_loc(stderr, parser);
            return true;
        }
        result = result * 10 + value;
    }
    if (!has_digits) {
        fprintf(stderr, "error: unexpected character, number expected\n");
        print_error_loc(stderr, parser);
        return true;
    }
    *number = result;
    return false;
}

static bool parser_read_row_number(parser_t *parser, row_number_t *number) {
    uint64_t parsed_num;
    if (parser_read_unsigned(parser, &parsed_num)) {
        return true;
    }
    if (parsed_num > ROW_MAX) {
        fprintf(stderr, "error: row number too large\n");
        print_error_loc(stderr, parser);
        return true;
    }
    *number = (row_number_t)parsed_num;
    return false;
}

static bool parser_read_value(parser_t *parser, value_t *value) {
    pch_t pch = parser_peek(parser);
    if (pch == PCH_ERROR) {
        return true;
    }
    bool negative = false;
    if (pch == '-') {
        negative = true;
        parser_consume(parser);
    }

    uint64_t parsed_num;
    if (parser_read_unsigned(parser, &parsed_num)) {
        return true;
    }
    if (!negative && parsed_num > (uint64_t)VALUE_MAX) {
        fprintf(stderr, "error: number too large\n");
        print_error_loc(stderr, parser);
        return true;
    }
    if (negative && parsed_num && parsed_num - 1 > (uint64_t)VALUE_MAX) {
        fprintf(stderr, "error: number too small\n");
        print_error_loc(stderr, parser);
        return true;
    }
    *value = negative ? (value_t)-parsed_num : (value_t)parsed_num;
    return false;
}

static bool parser_read_header(parser_t *parser, vec_t *vec) {
    while (true) {
        pch_t pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            return true;
        }
        if (pch == parser->col_sep || pch == PCH_EOF || pch == '\n') {
            break;
        }
        parser_consume(parser);
    }

    while (true) {
        pch_t pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            return true;
        }
        if (pch == PCH_EOF || pch == '\n') {
            parser_consume(parser);
            break;
        }
        if (pch != parser->col_sep) {
            fprintf(stderr,
                    "error: unexpected character, separator expected\n");
            print_error_loc(stderr, parser);
            return true;
        }
        parser_consume(parser);

        char *name;
        if (parser_read_name(parser, &name)) {
            return true;
        }
        vec_append(vec, &name);
    }

    return false;
}

static bool parser_read_operand(parser_t *parser, cell_val_t *val) {
    pch_t pch = parser_peek(parser);
    if (pch == PCH_ERROR) {
        return true;
    }

    if ((pch > 0 && isalpha(pch)) || pch == '_') {
        char *name;
        row_number_t number;
        if (parser_read_name(parser, &name)) {
            return true;
        }
        if (parser_read_row_number(parser, &number)) {
            free(name);
            return true;
        }

        val->type = CV_CELL_REF;
        val->data.cell_ref.column_name = name;
        val->data.cell_ref.row_number = number;
        return false;
    }

    if ((pch > 0 && isdigit(pch)) || pch == '-') {
        value_t value;
        if (parser_read_value(parser, &value)) {
            return true;
        }

        val->type = CV_VALUE;
        val->data.value = value;
        return false;
    }

    if (pch == PCH_EOF || pch == '\n') {
        fprintf(stderr, "error: unexpected end of line, operand expected\n");
    } else {
        fprintf(stderr, "error: unexpected character, operand expected\n");
    }
    print_error_loc(stderr, parser);
    return true;
}

static bool parser_read_operator(parser_t *parser, op_type_t *op) {
    pch_t pch = parser_peek(parser);
    switch (pch) {
    case '+':
        *op = OP_ADD;
        parser_consume(parser);
        return false;
    case '-':
        *op = OP_SUB;
        parser_consume(parser);
        return false;
    case '*':
        *op = OP_MUL;
        parser_consume(parser);
        return false;
    case '/':
        *op = OP_DIV;
        parser_consume(parser);
        return false;
    case PCH_ERROR:
        return true;
    case '\n':
    case PCH_EOF:
        fprintf(stderr, "error: unexpected end of line, operator expected\n");
        print_error_loc(stderr, parser);
        return true;
    default:
        fprintf(stderr, "error: unexpected character, operator expected\n");
        print_error_loc(stderr, parser);
        return true;
    }
}

static bool parser_read_cell(parser_t *parser, cell_t *cell) {
    pch_t pch = parser_peek(parser);
    if (pch == PCH_ERROR) {
        return true;
    }
    if (pch == parser->col_sep || pch == '\n' || pch == PCH_EOF) {
        cell->op = OP_NONE;
        cell->left.type = CV_VALUE;
        cell->left.data.value = 0;
        cell->right.type = CV_VALUE;
        cell->right.data.value = 0;
        return false;
    }

    if (pch != '=') {
        value_t value;
        if (parser_read_value(parser, &value)) {
            return true;
        }
        cell->op = OP_NONE;
        cell->left.type = CV_VALUE;
        cell->left.data.value = value;
        cell->right.type = CV_VALUE;
        cell->right.data.value = 0;
        return false;
    }
    parser_consume(parser);

    cell_val_t left, right;
    op_type_t op;
    if (parser_read_operand(parser, &left)) {
        return true;
    }
    if (parser_read_operator(parser, &op)) {
        cell_val_free(&left);
        return true;
    }
    if (parser_read_operand(parser, &right)) {
        cell_val_free(&left);
        return true;
    }

    cell->left = left;
    cell->right = right;
    cell->op = op;
    return false;
}

static bool parser_read_row(parser_t *parser, row_number_t *number, cell_t *row,
                            size_t width) {
    if (parser_read_row_number(parser, number)) {
        return true;
    }

    for (size_t count = 0; count < width; count++) {
        pch_t pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            return true;
        }
        if (pch == PCH_EOF || pch == '\n') {
            break;
        }
        if (pch != parser->col_sep) {
            fprintf(stderr,
                    "error: unexpected character, separator expected\n");
            print_error_loc(stderr, parser);
            return true;
        }
        parser_consume(parser);

        if (parser_read_cell(parser, &row[count])) {
            return true;
        }
    }

    if (parser_consume_line(parser)) {
        return true;
    }
    return false;
}

static void string_free(void *ptr, size_t size) {
    free(*(void **)ptr);
}

static void row_free(void *ptr, size_t size) {
    cell_t *cells = (cell_t *)ptr;
    size_t count = size / sizeof(cell_t);
    for (size_t i = 0; i < count; i++) {
        cell_free(&cells[i]);
    }
}

bool parser_read_sheet(parser_t *parser, sheet_t *sheet) {
    vec_t name_vec;
    vec_init(&name_vec, sizeof(void *), string_free);

    if (parser_read_header(parser, &name_vec)) {
        vec_free(&name_vec);
        return true;
    }
    size_t width = name_vec.count;
    if (width > SIZE_MAX / sizeof(cell_t)) {
        vec_free(&name_vec);
        fprintf(stderr, "error: too many columns\n");
        return true;
    }

    vec_t num_vec, cell_vec;
    vec_init(&num_vec, sizeof(row_number_t), NULL);
    vec_init(&cell_vec, sizeof(cell_t) * width, row_free);

    while (true) {
        pch_t pch = parser_peek(parser);
        if (pch == PCH_ERROR) {
            vec_free(&name_vec);
            vec_free(&num_vec);
            vec_free(&cell_vec);
            return true;
        }
        if (pch == PCH_EOF) {
            break;
        }

        row_number_t number;
        cell_t *row = (cell_t *)vec_append(&cell_vec, NULL);
        if (parser_read_row(parser, &number, row, width)) {
            vec_free(&name_vec);
            vec_free(&num_vec);
            vec_free(&cell_vec);
            return true;
        }
        vec_append(&num_vec, (void *)&number);
    }

    sheet->column_names = (const char **)name_vec.data;
    sheet->row_numbers = (row_number_t *)num_vec.data;
    sheet->cells = (cell_t *)cell_vec.data;
    sheet->height = cell_vec.count;
    sheet->width = width;

    return false;
}
