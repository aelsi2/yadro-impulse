#include <stdio.h>
#include <stdlib.h>

#include "cell.h"
#include "lookup.h"

static void print_error_loc(FILE *file, const cell_ref_t *loc) {
    fprintf(file, "\tat cell %s%" PRI_ROW "\n", loc->column_name,
            loc->row_number);
}

static bool add_overflows(value_t left, value_t right) {
    if (right > 0 && left > VALUE_MAX - right) {
        return true;
    }
    if (right < 0 && left < VALUE_MIN - right) {
        return true;
    }
    return false;
}

static bool sub_overflows(value_t left, value_t right) {
    if (right < 0 && left > VALUE_MAX + right) {
        return true;
    }
    if (right > 0 && left < VALUE_MIN + right) {
        return true;
    }
    return false;
}

static bool mul_overflows(value_t left, value_t right) {
    if (left > 0) {
        if (right > 0) {
            if (left > VALUE_MAX / right) {
                return true;
            }
        } else if (right < 0) {
            if (right < VALUE_MIN / left) {
                return true;
            }
        }
    } else if (left < 0) {
        if (right > 0) {
            if (left < VALUE_MIN / right) {
                return true;
            }
        } else if (right < 0) {
            if (right < VALUE_MAX / left) {
                return true;
            }
        }
    }
    return false;
}

static bool cell_val_resolve(cell_val_t *val, const lookup_t *lookup,
                             uint32_t recursion_limit) {
    if (val->type == CV_VALUE) {
        return false;
    }
    const cell_ref_t *key = &val->data.cell_ref;
    cell_t *cell = lookup_get(lookup, key);
    if (cell == NULL) {
        fprintf(stderr, "error: cell %s%" PRI_ROW " not found\n",
                key->column_name, key->row_number);
        return true;
    }
    if (cell_resolve(cell, lookup, recursion_limit, key)) {
        return true;
    }
    cell_ref_free(&val->data.cell_ref);
    val->type = CV_VALUE;
    val->data.value = cell->left.data.value;
    return false;
}

bool cell_resolve(cell_t *cell, const lookup_t *lookup, uint32_t depth_limit,
                  const cell_ref_t *loc) {
    if (depth_limit == 0) {
        fprintf(stderr, "error: depth limit exceeded\n");
        print_error_loc(stderr, loc);
        return true;
    }
    depth_limit -= 1;

    if (cell_val_resolve(&cell->left, lookup, depth_limit)) {
        return true;
    }
    if (cell_val_resolve(&cell->right, lookup, depth_limit)) {
        return true;
    }

    value_t result = 0;
    value_t left = cell->left.data.value;
    value_t right = cell->right.data.value;

    switch (cell->op) {
    case OP_NONE:
        result = left;
        break;
    case OP_ADD: {
        if (add_overflows(left, right)) {
            fprintf(stderr,
                    "error: addition overflow (%" PRI_VALUE " + %" PRI_VALUE
                    ")\n",
                    left, right);
            print_error_loc(stderr, loc);
            return true;
        }
        result = left + right;
    } break;
    case OP_SUB: {
        if (sub_overflows(left, right)) {
            fprintf(stderr,
                    "error: subtraction overflow (%" PRI_VALUE " - %" PRI_VALUE
                    ")\n",
                    left, right);
            print_error_loc(stderr, loc);
            return true;
        }
        result = left - right;
    } break;
    case OP_MUL: {
        if (mul_overflows(left, right)) {
            fprintf(stderr,
                    "error: multiplication overflow (%" PRI_VALUE
                    " * %" PRI_VALUE ")\n",
                    left, right);
            print_error_loc(stderr, loc);
            return true;
        }
        result = left * right;
    } break;
    case OP_DIV: {
        if (right == 0) {
            fprintf(stderr, "error: division by zero\n");
            print_error_loc(stderr, loc);
            return true;
        }
        if (left == VALUE_MIN && right == -1) {
            fprintf(stderr, "error: division overflow\n");
            print_error_loc(stderr, loc);
            return true;
        }
        result = left / right;
    } break;
    }

    cell->op = OP_NONE;
    cell->left.type = CV_VALUE;
    cell->left.data.value = result;

    return false;
}

void cell_ref_free(cell_ref_t *ref) {
    free((void *)ref->column_name);
    ref->column_name = NULL;
    ref->row_number = 0;
}

void cell_val_free(cell_val_t *val) {
    switch (val->type) {
    case CV_CELL_REF:
        cell_ref_free(&val->data.cell_ref);
        break;
    default:
        break;
    }
    val->type = CV_VALUE;
    val->data.value = 0;
}

void cell_free(cell_t *cell) {
    cell_val_free(&cell->left);
    cell_val_free(&cell->right);
}

static void cell_val_print(cell_val_t *val, FILE *file) {
    if (val->type == CV_VALUE) {
        fprintf(file, "%" PRI_VALUE, val->data.value);
        return;
    }
    const cell_ref_t *key = &val->data.cell_ref;
    fprintf(file, "%s%" PRI_ROW, key->column_name, key->row_number);
}

void cell_print(cell_t *cell, FILE *file) {
    if (cell->op != OP_NONE) {
        fprintf(file, "=");
    }
    cell_val_print(&cell->left, file);
    switch (cell->op) {
    case OP_NONE:
        return;
    case OP_ADD:
        fprintf(file, "+");
        break;
    case OP_SUB:
        fprintf(file, "-");
        break;
    case OP_MUL:
        fprintf(file, "*");
        break;
    case OP_DIV:
        fprintf(file, "/");
        break;
    }
    cell_val_print(&cell->right, file);
}
