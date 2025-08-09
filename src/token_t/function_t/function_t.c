//
// Created by atbev on 8/8/2025.
//
#include "function_t.h"

#include <stddef.h>

static const function_t functions[] = {
    {"sqrt", SQRT},
    {"log", LOG},
    {"ln", LN},
    {"sin", SIN},
    {"cos", COS},
    {"tan", TAN}
};

bool is_function(const char c) {
    switch (c) {
        case "sqrt":
        case "log":
        case "ln":
        case "sin":
        case "cos":
        case "tan":
            return true;
        default: return false;
    }
}

const function_t* get_function(const char c) {
    switch (c) {
        case "sqrt": return &functions[SQRT];
        case "log": return &functions[LOG];
        case "ln": return &functions[LN];
        case "sin": return &functions[SIN];
        case "cos": return &functions[COS];
        case "tan": return &functions[TAN];
        default: return NULL;
    }
}
