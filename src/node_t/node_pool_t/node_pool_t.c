//
// Created by atbev on 6/1/2025.
//


#include <string.h>
#include <stdio.h>

#include "node_pool_t.h"

void node_pool_init(node_pool_t* pool) {
    memset(pool->used, 0, sizeof(pool->used)); // set all used variables to false for all nodes
    pool->free_count = MAX_AST_NODES;
}

ast_node_t* node_pool_alloc(node_pool_t* pool) { // function to reserve a space in the node pool and return it
    if (pool->free_count == 0) return NULL;

    for (int i = 0; i < MAX_AST_NODES; i++) {
        if (!pool->used[i]) {           // if the node isn't used
            pool->used[i] = true;       // turn it to used
            pool->free_count--;         // 1 fewer nodes are free
            //memset(&pool->nodes[i], 0, sizeof(ast_node_t));
            return &pool->nodes[i];     // return the allocated space
        }
    }
    return NULL;  // should not reach here
}

bool node_pool_free_node(node_pool_t* pool, const ast_node_t* node) { // function to release a node when it is no longer used
    const ptrdiff_t index = node - pool->nodes;             // get address to node
    if (index < 0 || index >= MAX_AST_NODES) return false;  // check if freeing memory not in the pool
    if (!pool->used[index]) return false;                   // make sure the node isn't still in use
    // if all those checks pass:
    pool->used[index] = false;                                      // set used to false
    pool->free_count++;                                             // add one to the free nodes count
    memset(pool->nodes + index, 0, sizeof(ast_node_t)); // clear the memory
    return true;
}

void node_pool_free_subtree(node_pool_t* pool, ast_node_t* node) {
    if (!node) return;
    node_pool_free_subtree(pool, node->left);   // navigate to the bottom of the tree
    node_pool_free_subtree(pool, node->right);  // on both sides
    node_pool_free_node(pool, node);      // then free the noes going up
}

void node_pool_status(const node_pool_t* pool) {
    printf("Node pool usage: %d/%d nodes used\n", MAX_AST_NODES - pool->free_count, MAX_AST_NODES);
}