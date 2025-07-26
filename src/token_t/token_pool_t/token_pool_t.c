//
// Created by atbev on 7/13/2025.
//

#include "token_pool_t.h"

#include <stdio.h>
#include <string.h>

void token_pool_init(token_pool_t* pool) {
    memset(pool->used, 0, sizeof(pool->used)); // set all used variables to false for all nodes
    pool->free_count = MAX_TOKENS;
}

token_t* token_pool_alloc(token_pool_t* pool) { // function to reserve a space in the token pool and return it
    if (pool->free_count == 0) return NULL;

    for (int i = 0; i < MAX_TOKENS; i++) {
        if (!pool->used[i]) {           // if the token isn't used
            pool->used[i] = true;       // turn it to used
            pool->free_count--;         // 1 fewer nodes are free
            //memset(&pool->tokens[i], 0, sizeof(token_t));
            return &pool->tokens[i];     // return the allocated space
        }
    }
    return NULL;  // should not reach here
}

bool token_pool_free_token(token_pool_t* pool, const token_t* token) { // function to release a token when it is no longer used
    const ptrdiff_t index = token - pool->tokens;             // get address to node
    if (index < 0 || index >= MAX_TOKENS) return false;  // check if freeing memory not in the pool
    if (!pool->used[index]) return false;                   // make sure the token isn't still in use
    // if all those checks pass:
    pool->used[index] = false;                                      // set used to false
    pool->free_count++;                                             // add one to the free nodes count
    memset(pool->tokens + index, 0, sizeof(token_t)); // clear the memory
    return true;
}

void token_pool_status(const token_pool_t* pool) {
    printf("Token pool usage: %d/%d tokens used\n", MAX_TOKENS - pool->free_count, MAX_TOKENS);
}