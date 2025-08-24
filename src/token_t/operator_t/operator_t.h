//
// Created by atbev on 5/16/2025.
//

#ifndef OPERATION_T_H
#define OPERATION_T_H
#include <stdbool.h>

typedef enum { // enum to hold named operator types
    ADD = 0,
    SUB,
    MUL,
    DIV,
    POW
} operator_type_t;

// enum to hold named operator type associativity
typedef enum {
    ASSOC_LEFT = 0,
    ASSOC_RIGHT
} operator_assoc_t;

// struct to hold all pertinent information relating to an operator
typedef struct {
    const char* operator;           // operator's equivalent character
    operator_type_t type;           // operator's type
    int precedence;                 // operator's precedence
    operator_assoc_t associativity; // operator's associativity
} operator_t;

bool is_operator(char);
const operator_t* get_operator(char);
int get_precedence(char);
bool is_assoc_comm_operator(operator_t);

#endif //OPERATION_T_H