//
// Created by atbev on 5/26/2025.
//

#include "token_t.h"

#include <stddef.h>

#include "token_pool_t/token_pool_t.h"

bool is_number(const token_t token) {
    return token.type == TOKEN_NUMBER || token.type == TOKEN_SYMBOL;
}

token_t* num_token_init(token_pool_t* pool, const char* num_str) {
    token_t* token = token_pool_alloc(pool);
    if (!token) return NULL;

    token->type = TOKEN_NUMBER;
    token->number = num_init_str(num_str);
    return token;
}

token_t* num_token_init_double(token_pool_t* pool, const double num_val) {
    token_t* token = token_pool_alloc(pool);
    if (!token) return NULL;

    token->type = TOKEN_NUMBER;
    token->number = num_init_double(num_val);
    return token;
}

token_t* sym_token_init(token_pool_t* pool, symbol_table_t* table, const char* sym) {
    token_t* token = token_pool_alloc(pool);
    if (!token) return NULL;

    token->type = TOKEN_SYMBOL;
    token->symbol = find_symbol(table, sym);
    return token;
}

token_t* sym_token_copy(token_pool_t* token_pool, const token_t* sym_token) {
    token_t* copy = token_pool_alloc(token_pool);
    if (!copy) {
        return NULL;
    }
    copy->symbol = sym_token->symbol;
    copy->type = TOKEN_SYMBOL;
    return copy;
}

token_t* op_token_init(token_pool_t* pool, const char op) {
    token_t* token = token_pool_alloc(pool);
    if (!token) return NULL;
    token->type = TOKEN_OPERATOR;
    token->operator = get_operator(op);
    return token;
}

token_t* paren_token_init(token_pool_t* pool, const char paren) {
    token_t* token = token_pool_alloc(pool);
    if (!token) return NULL;

    if (paren == '(') {
        token->type = TOKEN_LPAREN;
    } else if (paren == ')') {
        token->type = TOKEN_RPAREN;
    } else {
        token->type = TOKEN_UNKNOWN;
    }
    return token;
}

token_t* token_copy(token_pool_t* token_pool, const token_t* token) {
    if (!token) {
        return NULL;
    }

    switch (token->type) {
        case TOKEN_NUMBER:
            return num_token_init_double(token_pool, token->number.value);

        case TOKEN_SYMBOL:
            return sym_token_copy(token_pool, token);

        case TOKEN_OPERATOR:
            return op_token_init(token_pool, token->operator->operator[0]);

        case TOKEN_LPAREN:
            return paren_token_init(token_pool, '(');

        case TOKEN_RPAREN:
            return paren_token_init(token_pool, ')');

        case TOKEN_UNKNOWN:
        default:
            return NULL;
    }
}

const char* token_type_to_string(const token_t* token) {
    switch (token->type) {
        case TOKEN_NUMBER: return token->number.disp;
        case TOKEN_SYMBOL: return token->symbol->symbol;
        case TOKEN_OPERATOR: return token->operator->operator;
        case TOKEN_FUNCTION: return "FUNC";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_UNKNOWN:
        default: return "?";
    }
}