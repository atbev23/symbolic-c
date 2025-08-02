//
// Created by atbev on 7/12/2025.
//

#include "queue_t.h"

#include <stddef.h>

void queue_init(queue_t* queue) { // that's a queue, innit?
    queue->front = 0;
    queue->size = 0;
}

bool queue_is_empty(const queue_t* queue) {
    return queue->size == 0;
}

bool queue_is_full(const queue_t* queue) {
    return queue->size == MAX_QUEUE_SIZE;
}

const void* queue_peek(const queue_t* queue) {
    if (queue_is_empty(queue)) return NULL;    // guard against stack underflow
    return queue->items[queue->front];              // currently serving
}

void queue_enqueue(queue_t* queue, const void* item) {
    if (queue_is_full(queue)) return;                               // guard against stack overflow
    const int rear = (queue->front + queue->size) % MAX_QUEUE_SIZE; // geeks for geeks wrote this one ngl
    queue->items[rear] = item;                                      // back of the line, buster
    queue->size++;                                                  // the queue just got larger, reflect that
}

const void* queue_dequeue(queue_t* queue) {
    if (queue_is_empty(queue)) return NULL;                 // don't dequeue from empty queue
    const void* item = queue->items[queue->front];          // we need to modify front so we need to get the return node now
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;     // again, geeks for geeks
    queue->size--;                                          // the queue just got smaller, reflect that
    return item;
}