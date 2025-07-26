//
// Created by atbev on 6/7/2025.
//
#include <stddef.h>

#include "../ast_node_t.h"
#include "node_queue_t.h"

void node_queue_init(node_queue_t* queue) { // that's a queue, innit?
    queue->front = 0;
    queue->size = 0;
}

bool node_queue_is_empty(const node_queue_t* queue) {
    return queue->size == 0;
}

bool node_queue_is_full(const node_queue_t* queue) {
    return queue->size == MAX_QUEUE_SIZE;
}

const ast_node_t* node_queue_peek(const node_queue_t* queue) {
    if (node_queue_is_empty(queue)) return NULL;    // guard against stack underflow
    return queue->nodes[queue->front];              // currently serving
}

void node_queue_enqueue(node_queue_t* queue, const ast_node_t* node) {
    if (node_queue_is_full(queue)) return;                          // guard against stack overflow
    const int rear = (queue->front + queue->size) % MAX_QUEUE_SIZE; // geeks for geeks wrote this one ngl
    queue->nodes[rear] = node;                                      // back of the line, buster
    queue->size++;                                                  // the queue just got larger, reflect that
}

const ast_node_t* node_queue_dequeue(node_queue_t* queue) {
    if (node_queue_is_empty(queue)) return NULL;            // don't dequeue from empty queue. should be obv but there are some real sickos out there that have never felt the touch of a stack underflow
    const ast_node_t* node = queue->nodes[queue->front];    // we need to modify front so we need to get the return node now
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;     // circular queue type shi
    queue->size--;                                          // the queue just got smaller, reflect that
    return node;
}