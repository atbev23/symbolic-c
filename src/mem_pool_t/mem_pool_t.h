//
// Created by atbev on 7/12/2025.
//

#ifndef MEM_POOL_T_H
#define MEM_POOL_T_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

// include this in the .c file for whatever you want to make a memory pool for and to use it elsewhere,
// declare an external variable, ie. extern TYPE_pool_t GLOBAL_TYPE_POOL, and redeclare that in every .c file
// you want to use it. (But i cant egt it to work)

#define DECLARE_MEM_POOL(TYPE, NAME, MAX_COUNT)                         \
    typedef struct NAME##_pool_t {                                      \
        TYPE items[MAX_COUNT];                                          \
        bool used[MAX_COUNT];                                           \
        int free_count;                                                 \
    } NAME##_pool_t;                                                    \
                                                                        \
    static void NAME##_pool_init(NAME##_pool_t* pool) {                 \
        memset(pool->used, 0, sizeof(pool->used));                      \
        pool->free_count = MAX_COUNT;                                   \
    }                                                                   \
                                                                        \
    static TYPE* NAME##_alloc(NAME##_pool_t* pool) {                    \
        if (pool->free_count == 0) return NULL;                         \
        for (int i = 0; i < MAX_COUNT; ++i) {                           \
            if (!pool->used[i]) {                                       \
                pool->used[i] = true;                                   \
                pool->free_count--;                                     \
                return &pool->items[i];                                 \
            }                                                           \
        }                                                               \
        return NULL;                                                    \
    }                                                                   \
                                                                        \
    static bool NAME##_free(NAME##_pool_t* pool, const TYPE* item) {    \
        ptrdiff_t index = item - pool->items;                           \
        if (index < 0 || index >= MAX_COUNT) return false;              \
        if (!pool->used[index]) return false;                           \
        pool->used[index] = false;                                      \
        pool->free_count++;                                             \
        memset(&pool->items[index], 0, sizeof(TYPE));                   \
        return true;                                                    \
    }                                                                   \
                                                                        \
    static void NAME##_pool_status(const NAME##_pool_t* pool) {         \
        printf("Pool usage: %d/%d nodes used\n",                        \
               MAX_COUNT - pool->free_count, MAX_COUNT);                \
    }

#endif // MEM_POOL_T_H
/*

#define MAX_POOL 128

typedef struct {
    void* items[MAX_POOL];   // pool of nodes
    bool used[MAX_POOL];    // tracker for used nodes
    int free_count;         // number of free nodes
    int size;               // number of bits in single iteration
} mem_pool_t;

void mem_pool_init(mem_pool_t*);
void* mem_pool_alloc(mem_pool_t*);
bool mem_pool_free_item(mem_pool_t*, const void*);
void mem_pool_free_items(mem_pool_t*, void*);
void mem_pool_status(const mem_pool_t*);

#endif //MEM_POOL_T_H
*/