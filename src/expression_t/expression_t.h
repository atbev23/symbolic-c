//
// Created by atbev on 5/18/2025.
//

#ifndef EXPRESSION_T_H
#define EXPRESSION_T_H

#include "../token_t/token_t.h"
#include "../node_t/ast_node_t.h"

#define MAX_EXPR_LEN 64
#define MAX_TOKEN_LEN 128

typedef struct expression_t expression_t;

struct expression_t {
    char expression[MAX_EXPR_LEN];
    token_t* tokens[MAX_EXPR_LEN];
    ast_node_t* root;
    token_pool_t* token_pool;
    node_pool_t* node_pool;
    int token_count, rpn_count, node_count;
    bool is_symbolic;
};

int expression_init(expression_t*, const char*, symbol_table_t*);   // takes in an expression as a string, creates expression_t object
int tokenize(expression_t*, symbol_table_t*);       // turns expression string to list of tokens
int build_ast(expression_t*);                       // creates an abstract syntax tree using our tokens
ast_node_t* simplify(expression_t*, ast_node_t*);    // performs "math rules" to simplify the ast

#endif //EXPRESSION_T_H