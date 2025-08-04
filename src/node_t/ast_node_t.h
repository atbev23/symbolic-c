//
// Created by atbev on 5/18/2025.
//

#ifndef AST_NODE_T_H
#define AST_NODE_T_H

#include "../token_t/token_t.h"
#include "../stack_t/stack_t.h"

typedef struct ast_node_t ast_node_t;
typedef struct node_pool_t node_pool_t;

struct ast_node_t {
    token_t* token;
    ast_node_t* left;
    ast_node_t* right;
};

ast_node_t* create_node(node_pool_t*, token_t*, ast_node_t*, ast_node_t*);
ast_node_t* copy_subtree(token_pool_t*, node_pool_t*, const ast_node_t*);
int get_tree_depth(const ast_node_t*);
void ast_to_string(ast_node_t* node, char* buffer, int buf_size, int* offset);
void print_tree(const ast_node_t*);

#endif //AST_NODE_T_H