//
// Created by atbev on 5/18/2025.
//

#ifndef TOKEN_T_H
#define TOKEN_T_H

#include "function_t/function_t.h"
#include "operator_t/operator_t.h"
#include "symbol_t/symbol_t.h"
#include "number_t/number_t.h"
#include "unary_t/unary_t.h"

#define MAX_LEXEME_LEN 8
typedef struct token_pool_t token_pool_t;

typedef enum {
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_UNARY,
    TOKEN_OPERATOR,
    TOKEN_FUNCTION,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_UNKNOWN
} token_type_t;

typedef struct token_t {
    token_type_t type;
    union {
        number_t number;
        symbol_t* symbol;
        unary_t unary;
        const operator_t* operator;
        function_t* function;
    };
} token_t;

bool is_const(token_t);
token_t* num_token_init(token_pool_t*, const char*);
token_t* num_token_init_double(token_pool_t*, double);
token_t* sym_token_init(token_pool_t*, symbol_table_t*, const char*);
token_t* unary_token_init(token_pool_t*, unary_op_t);
token_t* op_token_init(token_pool_t*, char);
token_t* paren_token_init(token_pool_t*, char);
token_t* token_copy(token_pool_t*, const token_t*);
const char* token_type_to_string(const token_t*);

#endif //TOKEN_T_H