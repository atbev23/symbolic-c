//
// Created by atbev on 6/7/2025.
//

#ifndef NODE_QUEUE_T_H
#define NODE_QUEUE_T_H

#define MAX_QUEUE_SIZE 128

typedef struct {
    const ast_node_t* nodes[MAX_QUEUE_SIZE];
    int front;
    int size;
} node_queue_t;

void node_queue_init(node_queue_t*);
bool node_queue_is_empty(const node_queue_t*);
bool node_queue_is_full(const node_queue_t*);
const ast_node_t* node_queue_peek(const node_queue_t*);
void node_queue_enqueue(node_queue_t*, const ast_node_t*);
const ast_node_t* node_queue_dequeue(node_queue_t*);

#endif //NODE_QUEUE_T_H