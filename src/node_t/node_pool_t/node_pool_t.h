//
// Created by atbev on 6/1/2025.
//

#ifndef NODE_POOL_T_H
#define NODE_POOL_T_H

#include "../ast_node_t.h"

#define MAX_AST_NODES 128

struct node_pool_t {
    ast_node_t nodes[MAX_AST_NODES];    // pool of nodes
    bool used[MAX_AST_NODES];           // tracker for used nodes
    int free_count;                     // number of free nodes
};

void node_pool_init(node_pool_t*);
ast_node_t* node_pool_alloc(node_pool_t*);
bool node_pool_free_node(node_pool_t*, const ast_node_t*);
void node_pool_free_subtree(node_pool_t*, ast_node_t*);
void node_pool_status(const node_pool_t*);

#endif //NODE_POOL_T_H

