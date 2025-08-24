//
// Created by atbev on 8/8/2025.
//
#include "function_t.h"

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

static const function_t functions[] = {
    {"sqrt", SQRT},
    {"log", LOG},
    {"ln", LN},
    {"sin", SIN},
    {"cos", COS},
    {"tan", TAN}
};

bool is_function(const char* c) {
    if (strcmp(c, "sqrt") == 0 ||
        strcmp(c, "log") == 0 ||
        strcmp(c, "ln") == 0 ||
        strcmp(c, "sin") == 0 ||
        strcmp(c, "cos") == 0 ||
        strcmp(c, "tan") == 0) {
        return true;
    }
    return false;
}

const function_t* get_function(const char* c) {
    if (strcmp(c, "sqrt") == 0) return &functions[SQRT];
    if (strcmp(c, "log") == 0) return &functions[LOG];
    if (strcmp(c, "ln") == 0) return &functions[LN];
    if (strcmp(c, "sin") == 0) return &functions[SIN];
    if (strcmp(c, "cos") == 0) return &functions[COS];
    if (strcmp(c, "tan") == 0) return &functions[TAN];
}
