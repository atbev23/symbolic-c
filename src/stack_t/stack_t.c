//
// Created by atbev on 7/7/2025.
//
#include <stddef.h>
#include "stack_t.h"

void stack_init(stack_t* s) {
    s->top = -1;
}

bool stack_is_empty(const stack_t* s) {
    return s->top == -1;
}

bool stack_is_full(const stack_t* s) {
    return s->top == MAX_STACK_SIZE - 1;
}

bool stack_push(stack_t* s, void* item) {
    if (stack_is_full(s)) return false;
    s->items[++s->top] = item;
    return true;
}

void* stack_pop(stack_t* s) {
    if (stack_is_empty(s)) return NULL;
    return s->items[s->top--];
}

const void* stack_pop_bottom(stack_t* s) {
    if (stack_is_empty(s)) return NULL;

    const void* item = s->items[0];

    // Shift all items one slot to the left
    for (int i = 0; i < s->top; ++i) {
        s->items[i] = s->items[i + 1];
    }

    s->top--;  // Reduce stack size
    return item;
}

void* stack_peek(const stack_t* s) {
    if (stack_is_empty(s)) return NULL;
    return s->items[s->top];
}