//
// Created by atbev on 7/12/2025.
//

#ifndef QUEUE_T_H
#define QUEUE_T_H

#include "stdbool.h"

#define MAX_QUEUE_SIZE 128

typedef struct {
    const void* items[MAX_QUEUE_SIZE];
    int front;
    int size;
} queue_t;

void queue_init(queue_t*);
bool queue_is_empty(const queue_t*);
bool queue_is_full(const queue_t*);
const void* queue_peek(const queue_t*);
void queue_enqueue(queue_t*, const void*);
const void* queue_dequeue(queue_t*);
#endif //QUEUE_T_H
