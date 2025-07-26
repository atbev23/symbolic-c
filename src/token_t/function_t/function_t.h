//
// Created by atbev on 5/17/2025.
//

#ifndef FUNCTION_T_H
#define FUNCTION_T_H

typedef enum {
    SQRT = 0,
    LOG,
    EXP,
    LN,
    SIN,
    COS,
    TAN,
    CSC
} function_type_t;

typedef struct {
    function_type_t type;
    int precedence;
} function_t;

#endif //FUNCTION_T_H