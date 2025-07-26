//
// Created by atbev on 5/26/2025.
//

#include "operator_t.h"

#include <stddef.h>

static const operator_t operators[] = {
    {"+", ADD, 1, ASSOC_LEFT},
    {"-", SUB, 1, ASSOC_LEFT},
    {"*", MUL, 2, ASSOC_LEFT},
    {"/", DIV, 2, ASSOC_LEFT},
    {"^", POW, 3, ASSOC_RIGHT}
};

bool is_operator(const char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^': return true;
        default: return false;
    }
}

const operator_t* get_operator(const char c) {
    switch (c) {
        case '+': return &operators[ADD];
        case '-': return &operators[SUB];
        case '*': return &operators[MUL];
        case '/': return &operators[DIV];
        case '^': return &operators[POW];
        default: return NULL;
    }
}

int get_precedence(const char c) {
    const operator_t* op = get_operator(c);
    return op ? op->precedence : -1;
}

bool is_assoc_comm_operator(const operator_t operator) {
    return operator.type == ADD || operator.type == MUL;
}