//
// Created by atbev on 7/6/2025.
//

#include "symbol_t.h"
#include <string.h>

symbol_t* sym_init(symbol_table_t* table, const char* symbol) {
    if (table->count >= MAX_SYMS || strlen(symbol) > MAX_SYM_LEN) {    // if the table is full or input is too long
        return NULL;                                                    // can't create token, return NULL
    }
    symbol_t* sym = &table->symbols[table->count++];
    sym->symbol = symbol;
    return sym;
}

void symbol_table_init(symbol_table_t* table) {
    table->count = 0;
}

// take in a char and return it if it is found in the symbol table
symbol_t* find_symbol(symbol_table_t* table, const char* c) {
    for (int i = 0; i < table->count; i++) {                // loop through the table
        if (strcmp(table->symbols[i].symbol, c) == 0) {  // if the inputted string matches a symbol
            return &table->symbols[i];                       // return a pointer to the matching symbol_t
        }
    }
    return NULL; // str did not match any symbols in the table
}

bool is_symbol(symbol_table_t* table, const char* c) {
    return find_symbol(table, c) != NULL;
}
/*
inline ast_node_t* create_node(const ast_t* tree, const token_t* token, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = node_pool_alloc(tree->pool);
    if (!node) return NULL;

    node->token = token;
    node->left = left;
    node->right = right;
    return node;
}
 */