#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "src/node_t/ast_node_t.h"
#include "src/node_t/node_pool_t/node_pool_t.h"
#include "src/expression_t/expression_t.h"
#include "src/token_t/token_pool_t/token_pool_t.h"

int main(void) {
    static token_pool_t token_pool;
    token_pool_init(&token_pool);

    static node_pool_t node_pool;
    node_pool_init(&node_pool);

    // initialize a symbol table
    symbol_table_t table = {0};
    symbol_table_init(&table);

    // fill it with symbols you plan to use
    sym_init(&table, "a");
    sym_init(&table, "m");
    sym_init(&table, "n");

    // initialize an expression and link it to token and node pools
    expression_t expr = {0};
    expr.token_pool = &token_pool;
    expr.node_pool = &node_pool;

    // ask the user for an expression, entered as a string
    printf("Enter an expression: ");
    fgets(expr.expression, sizeof(expr.expression), stdin);

   // strncpy(expr.expression, "3*a", MAX_EXPR_LEN);

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    expr.token_count = tokenize(&expr, &table); // tokenize the expression
    expr.node_count = build_ast(&expr);         // build an abstract syntax tree
    expr.root = simplify(&expr, expr.root);     // simplify the tree
    //expr.root = simplify(&expr, expr.root);     // simplify the tree
    clock_gettime(CLOCK_MONOTONIC, &end);

    const double seconds = (double)(end.tv_sec - start.tv_sec);
    const double nanoseconds = end.tv_nsec - start.tv_nsec;
    const double elapsed = seconds + nanoseconds * 1e-9;

    printf("\nExpression simplified in %f seconds.\n\n", elapsed);
    //printf("\nExpression simplified in %g seconds.\n\n", (double)(end - start) / CLOCKS_PER_SEC);
    // printf("\n");

    printf("y = %s\n\n", expr.expression);
    print_tree(expr.root);
    printf("\n");
    token_pool_status(expr.token_pool);
    node_pool_status(expr.node_pool);

    // printf("\n");
    // printf("Size of expression_t: %zu bytes \n", sizeof(expression_t));
    // printf("Size of token_t: %zu bytes \n", sizeof(token_t));           // how does it determine memory size when we have unions?
    // printf("Size of token_pool_t: %zu \n", sizeof(token_pool_t));
    // printf("Size of ast_node_t: %zu bytes \n", sizeof(ast_node_t));     // one token_t two pointers
    // printf("Size of node_pool_t: %zu bytes \n", sizeof(node_pool_t));
    return 0;
}