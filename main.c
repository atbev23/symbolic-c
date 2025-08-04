#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "src/node_t/ast_node_t.h"
#include "src/node_t/node_pool_t/node_pool_t.h"
#include "src/expression_t/expression_t.h"
#include "src/token_t/token_pool_t/token_pool_t.h"

int cli_read_expression(char* buffer, const int buf_size) {
    printf("y = ");
    if (fgets(buffer, buf_size, stdin) == NULL) { // Narrowing conversion from 'size_t' (aka 'unsigned long long') to signed type 'int' is implementation-defined
        return 0; // input error
    }
    const size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
    }
    return 1;
}

int main(void) {
    // initialize a symbol table
    symbol_table_t table = {0};
    symbol_table_init(&table);

    // fill it with symbols you plan to use
    sym_init(&table, "x");
    sym_init(&table, "y");
    sym_init(&table, "z");

    char input_buffer[MAX_EXPR_LEN] = {0};
    char output_buffer[MAX_EXPR_LEN] = {0};

    while (cli_read_expression(input_buffer, MAX_EXPR_LEN)) {
        expression_t expr = {0};
        expression_init(&expr, input_buffer, &table);
        //printf("y = %s\n", expr.expression);

        int offset = 0;
        ast_to_string(expr.root, output_buffer, MAX_EXPR_LEN, &offset);
        output_buffer[offset < MAX_EXPR_LEN ? offset : MAX_EXPR_LEN - 1] = '\0';
        printf("y = %s\n\n", output_buffer);

        //print_tree(expr.root);
        //printf("\n");
        // token_pool_status(expr.token_pool);
        // node_pool_status(expr.node_pool);
    }


    // printf("\n");
    // printf("Size of expression_t: %zu bytes \n", sizeof(expression_t));
    // printf("Size of token_t: %zu bytes \n", sizeof(token_t));           // how does it determine memory size when we have unions?
    // printf("Size of token_pool_t: %zu \n", sizeof(token_pool_t));
    // printf("Size of ast_node_t: %zu bytes \n", sizeof(ast_node_t));     // one token_t two pointers
    // printf("Size of node_pool_t: %zu bytes \n", sizeof(node_pool_t));
    return 0;
}