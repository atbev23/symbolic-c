//
// Created by atbev on 5/18/2025.
//
#include <stdio.h>
#include <string.h>

#include "ast_node_t.h"
#include "node_pool_t/node_pool_t.h"
#include "../queue_t/queue_t.h"
#include "../token_t/token_pool_t/token_pool_t.h"
//#include "node_queue_t/node_queue_t.h"

inline ast_node_t* create_node(node_pool_t* pool, token_t* token, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = node_pool_alloc(pool);
    if (!node) return NULL;

    node->token = token;
    node->left = left;
    node->right = right;
    return node;
}

inline ast_node_t* copy_subtree(token_pool_t* token_pool, node_pool_t* node_pool, const ast_node_t* node) {
    if (!node) {
        return NULL;
    }

    ast_node_t* node_copy = node_pool_alloc(node_pool);
    node_copy->token = token_copy(token_pool, node->token);

    node_copy->left = copy_subtree(token_pool, node_pool, node->left);
    node_copy->right = copy_subtree(token_pool, node_pool, node->right);

    return node_copy;
}

bool branch_is_equal(ast_node_t* a, ast_node_t* b) {
    if (!a || !b) return false;
    if (a->token->type != b->token->type) return false;

    switch (a->token->type) {
        case TOKEN_NUMBER:
            return a->token->number.value == b->token->number.value;
        case TOKEN_SYMBOL:
            return strcmp(a->token->symbol->symbol, b->token->symbol->symbol) == 0;
        case TOKEN_OPERATOR:
            return a->token->operator->type == b->token->operator->type &&
                   branch_is_equal(a->left, b->left) &&
                   branch_is_equal(a->right, b->right);
        default:
            return false;
    }
}

int get_tree_depth(const ast_node_t* root) {
    if (root == NULL)
        return 0;

    const int l = get_tree_depth(root->left);
    const int r = get_tree_depth(root->right);

    return (l > r ? l : r) + 1;
}

void print_spaces(const int count) {
    for (int i = 0; i < count; i++) putchar(' ');
}
/*
 * so how do we change this so that the tree is not left aligned
 */
#define MAX_TREE_DEPTH 6
#define MAX_TREE_WIDTH 64
#define NODE_WIDTH 2

const char* levels[MAX_TREE_DEPTH][MAX_TREE_WIDTH];

void level_order_traversal(const ast_node_t* root) {
    memset(levels, 0, sizeof(levels));

    queue_t q;  // assume you already have queue_t and queue functions
    queue_init(&q);
    queue_enqueue(&q, root);

    int curr_level = 0;
    int nodes_in_level = 1;

    while (!queue_is_empty(&q) && curr_level < MAX_TREE_DEPTH) {
        for (int i = 0; i < nodes_in_level; i++) {
            const ast_node_t* node = queue_dequeue(&q);
            if (node) {
                levels[curr_level][i] = token_type_to_string(node->token);
                queue_enqueue(&q, node->left);
                queue_enqueue(&q, node->right);
            } else {
                levels[curr_level][i] = NULL;
                queue_enqueue(&q, NULL);
                queue_enqueue(&q, NULL);
            }
        }
        curr_level++;
        nodes_in_level *= 2;
    }
}

void print_tree(const ast_node_t* root) {
    if (!root) return;

    level_order_traversal(root);
    const int depth = get_tree_depth(root);
    const int canvas_width = (1 << depth) * NODE_WIDTH;

    for (int level = 0; level < depth && level < MAX_TREE_DEPTH; level++) {
        int nodes_this_level = 1 << level;
        int gap = canvas_width / nodes_this_level;
        int prev_end = 0;

        // Find last non-null node index at this level
        int last_non_null = -1;
        for (int i = nodes_this_level - 1; i >= 0; i--) {
            if (levels[level][i] != NULL) {
                last_non_null = i;
                break;
            }
        }
        if (last_non_null == -1) continue; // nothing to print

        for (int i = 0; i <= last_non_null && i < MAX_TREE_WIDTH; i++) {
            const char* label = levels[level][i];
            if (!label) label = "  ";

            int pos = (gap / 2) + i * gap - (NODE_WIDTH / 2);
            int spaces = pos - prev_end;
            if (spaces > 0) print_spaces(spaces);

            printf("%-*s", NODE_WIDTH, label);
            prev_end = pos + NODE_WIDTH;
        }
        printf("\n");
    }
}


void append_token_string(const char* str, char* buffer, int buf_size, int* offset) {
    while (*str && *offset < buf_size - 1) {
        buffer[(*offset)++] = *str++;
    }
}

void ast_to_string(ast_node_t* node, char* buffer, const int buf_size, int* offset) {
    if (!node || !node->token || *offset >= buf_size - 1) return;

    switch (node->token->type) {
        case TOKEN_NUMBER:
            append_token_string(node->token->number.disp, buffer, buf_size, offset);
            break;
        case TOKEN_SYMBOL:
            append_token_string(node->token->symbol->symbol, buffer, buf_size, offset);
            break;

        case TOKEN_OPERATOR:
            append_token_string("(", buffer, buf_size, offset);
            ast_to_string(node->left, buffer, buf_size, offset);
            append_token_string(" ", buffer, buf_size, offset);
            append_token_string(node->token->operator->operator, buffer, buf_size, offset);
            append_token_string(" ", buffer, buf_size, offset);
            ast_to_string(node->right, buffer, buf_size, offset);
            append_token_string(")", buffer, buf_size, offset);
            break;

        default:
            append_token_string("?", buffer, buf_size, offset);
            break;
    }
}