#include <stdio.h>

#include "cell.h"
#include "lookup.h"

static void print_error_loc(FILE *file, const cell_key_t *loc) {
    if (loc) {
        fprintf(file, "\tat %s%ld\n", loc->column_name, loc->row_number);
    } else {
        fprintf(file, "\tat ?\n");
    }
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
    const cell_key_t *key = &val->data.cell_ref;
    cell_t *cell = lookup_get(lookup, key);
    if (cell_resolve(cell, lookup, recursion_limit, key)) {
        return true;
    }
    cell_key_free(&val->data.cell_ref);
    val->type = CV_VALUE;
    val->data.value = cell->left.data.value;
    return false;
}

bool cell_resolve(cell_t *cell, const lookup_t *lookup, uint32_t depth_limit,
                  const cell_key_t *loc) {
    if (depth_limit == 0) {
        fprintf(stderr, "error: depth limit exceeded\n");
        print_error_loc(stderr, loc);
        return true;
    }
    depth_limit -= 1;

    if (cell_val_resolve(&cell->left, lookup, depth_limit)) {
        print_error_loc(stderr, loc);
        return true;
    }
    if (cell_val_resolve(&cell->right, lookup, depth_limit)) {
        print_error_loc(stderr, loc);
        return true;
    }

    value_t left = cell->left.data.value;
    value_t right = cell->right.data.value;

    switch (cell->op) {
    case OP_NONE:
        return false;
    case OP_SUB: {
        if (sub_overflows(left, right)) {
            fprintf(stderr, "error: subtration overflow (%ld - %ld)\n", left,
                    right);
            print_error_loc(stderr, loc);
            return true;
        }
        left -= right;
    } break;
    case OP_ADD: {
        if (add_overflows(left, right)) {
            fprintf(stderr, "error: addition overflow (%ld + %ld)\n", left,
                    right);
            print_error_loc(stderr, loc);
            return true;
        }
        left += right;
    } break;
    case OP_MUL: {
        if (mul_overflows(left, right)) {
            fprintf(stderr, "error: multiplication overflow (%ld * %ld)\n",
                    left, right);
            print_error_loc(stderr, loc);
            return true;
        }
        left *= right;
    } break;
    case OP_DIV: {
        if (right == 0) {
            fprintf(stderr, "error: division by zero\n");
            print_error_loc(stderr, loc);
            return true;
        }
        left /= right;
    } break;
    }
    return false;
}

static void cell_val_free(cell_val_t *val) {
    switch (val->type) {
    case CV_CELL_REF:
        cell_key_free(&val->data.cell_ref);
        break;
    default:
        break;
    }
}

void cell_free(cell_t *cell) {
    cell_val_free(&cell->left);
    cell_val_free(&cell->right);
}
