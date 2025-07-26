//
// Created by atbev on 5/16/2025.
//

#ifndef OPERATION_T_H
#define OPERATION_T_H

typedef enum {
    ADD = 0,
    SUB,
    MUL,
    DIV,
    POW
} operator_type_t;

typedef enum {
    ASSOC_LEFT = 0,
    ASSOC_RIGHT
} operator_assoc_t;

typedef struct {
    const char* operator; // just need a c string. its only 1 character
    operator_type_t type;
    int precedence;
    operator_assoc_t associativity;
} operator_t;

bool is_operator(char);
const operator_t* get_operator(char);
int get_precedence(char);
bool is_assoc_comm_operator(operator_t);

#endif //OPERATION_T_H