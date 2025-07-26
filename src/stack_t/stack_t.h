//
// Created by atbev on 5/24/2025.
//

#ifndef STACK_T_H
#define STACK_T_H

#define MAX_STACK_SIZE 32

typedef struct stack_t stack_t;

struct stack_t {
        void* items[MAX_STACK_SIZE];
        int top;
};

void stack_init(stack_t*);
bool stack_is_empty(const stack_t*);
bool stack_is_full(const stack_t*);
bool stack_push(stack_t*, void*);
void* stack_pop(stack_t*);
const void* stack_pop_bottom(stack_t*);
void* stack_peek(const stack_t*);

#endif //STACK_T_H