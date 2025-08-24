cmake_minimum_required(VERSION 3.31)
project(symbolic-c C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")

# Platform-specific flags
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        message(STATUS "Detected ARM Linux — adding ARMv7 flags")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a")

    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        message(STATUS "Detected x86_64 Linux — adding native arch flags")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
    endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Windows build — no ARM flags")
endif()

# Source files
set(SOURCES
        main.c
        src/token_t/symbol_t/symbol_t.c
        src/node_t/ast_node_t.c
        src/expression_t/expression_t.c
        src/token_t/operator_t/operator_t.c
        src/token_t/token_t.c
        src/node_t/node_pool_t/node_pool_t.c
        src/node_t/node_queue_t/node_queue_t.c
        src/stack_t/stack_t.c
        src/token_t/number_t/number_t.c
        src/queue_t/queue_t.c
        src/token_t/token_pool_t/token_pool_t.c
        src/token_t/function_t/function_t.c
        src/token_t/unary_t/unary_t.c
)

if (WIN32)
    # Include the resource file so the icon is compiled into the .exe
    list(APPEND SOURCES resources/icon/variants/ruby/icon-ruby.rc)
endif()

add_executable(symbolic-c ${SOURCES})
