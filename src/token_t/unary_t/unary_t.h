//
// Created by atbev on 8/15/2025.
//

#ifndef UNARY_T_H
#define UNARY_T_H

typedef enum {
    UNARY_NEG,
    UNARY_ABS,
} unary_op_t;

typedef struct {
    unary_op_t op;
} unary_t;

#endif //UNARY_T_H
