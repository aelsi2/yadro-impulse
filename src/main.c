#include <stdbool.h>
#include <stdio.h>

#include "sheet.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        char *program_name = argc > 0 ? argv[0] : "csvreader";
        fprintf(stderr, "Usage: %s <FILE>\n", program_name);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("could not open file");
        return 1;
    }

    sheet_t sheet;
    bool parse_result = sheet_parse(&sheet, file);
    fclose(file);
    if (parse_result) {
        return 1;
    }

    if (sheet_resolve(&sheet)) {
        sheet_free(&sheet);
        return 1;
    }
    sheet_print(&sheet, stdout);
    sheet_free(&sheet);
    return 0;
}
