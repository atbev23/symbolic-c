//
// Created by atbev on 7/13/2025.
//

#ifndef TOKEN_POOL_T_H
#define TOKEN_POOL_T_H

#include "../token_t.h"

#define MAX_TOKENS 128

typedef struct token_pool_t {
    token_t tokens[MAX_TOKENS];    // pool of tokens
    bool used[MAX_TOKENS];        // tracker for used tokens
    int free_count;                  // number of free tokens
} token_pool_t;

void token_pool_init(token_pool_t*);
token_t* token_pool_alloc(token_pool_t*);
bool token_pool_free_token(token_pool_t*, const token_t*);
void token_pool_status(const token_pool_t*);

#endif //TOKEN_POOL_T_H
