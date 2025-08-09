cmake_minimum_required(VERSION 3.31)
project(sym_calc C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

add_executable(sym_calc main.c
        src/token_t/operator_t/operator_t.h
        src/token_t/function_t/function_t.h
        src/token_t/symbol_t/symbol_t.h
        src/token_t/symbol_t/symbol_t.c
        src/token_t/token_t.h
        src/node_t/ast_node_t.h
        src/expression_t/expression_t.h
        src/node_t/ast_node_t.c
        src/stack_t/stack_t.h
        src/expression_t/expression_t.c
        src/token_t/operator_t/operator_t.c
        src/token_t/token_t.c
        src/node_t/node_pool_t/node_pool_t.h
        src/node_t/node_pool_t/node_pool_t.c
        src/node_t/node_queue_t/node_queue_t.h
        src/node_t/node_queue_t/node_queue_t.c
        src/token_t/number_t/number_t.h
        src/stack_t/stack_t.c
        src/token_t/number_t/number_t.c
        src/queue_t/queue_t.h
        src/queue_t/queue_t.c
        src/mem_pool_t/mem_pool_t.h
        src/mem_pool_t/mem_pool_t.txt
        src/token_t/token_pool_t/token_pool_t.h
        src/token_t/token_pool_t/token_pool_t.c
        src/token_t/function_t/function_t.c
)
