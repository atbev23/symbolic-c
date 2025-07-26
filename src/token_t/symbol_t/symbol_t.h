//
// Created by atbev on 5/17/2025.
//

#ifndef SYMBOL_T_H
#define SYMBOL_T_H

#define MAX_SYM_LEN 1 // room for 1 char and changing this
#define MAX_SYMS 8

typedef struct {
    const char* symbol;
    //bool has_value;
    //double value;
} symbol_t;

typedef struct {
    symbol_t symbols[MAX_SYMS];
    int count;
} symbol_table_t;

symbol_t* sym_init(symbol_table_t*, const char*);
void symbol_table_init(symbol_table_t*);
symbol_t* find_symbol(symbol_table_t*, const char*);
bool is_symbol(symbol_table_t*, const char*);

#endif //SYMBOL_T_H