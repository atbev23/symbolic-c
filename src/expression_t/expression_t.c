//
// Created by atbev on 5/24/2025.
//

#include <stdlib.h>
#include <string.h>
#include "expression_t.h"

#include <ctype.h>
#include <math.h>
#include <pthread_time.h>
#include <stdio.h>
#include "../token_t/token_pool_t/token_pool_t.h"
#include "../node_t/node_pool_t/node_pool_t.h"
#include "../stack_t/stack_t.h"

//#define DEBUG_EXPRESSION_INIT
//#define DEBUG_COMMUTATIVE_PROPERTY
//#define DEBUG_DISTRIBUTIVE_PROPERTY

int expression_init(expression_t* expr, const char* input_expr, symbol_table_t* table) {
    static token_pool_t token_pool;
    token_pool_init(&token_pool);

    static node_pool_t node_pool;
    node_pool_init(&node_pool);

    expr->token_pool = &token_pool;
    expr->node_pool = &node_pool;

    strncpy(expr->expression, input_expr, sizeof(expr->expression));
    expr->expression[sizeof(expr->expression)-1] = '\0';

#ifdef DEBUG_EXPRESSION_INIT
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    expr->token_count = tokenize(expr, table);
    if (expr->token_count == 0) {
        return 0;
    }

    expr->node_count = build_ast(expr);
    if (!expr->root) {
        return 0; // ast build error
    }

    const ast_node_t* old_root = NULL;
    ast_node_t* current_root = expr->root;

    while (current_root != old_root) {
        old_root = current_root;
        current_root = simplify(expr, current_root);
    }
    expr->root = current_root;
    if (!expr->root) {
        return 0; // Simplify error
    }

#ifdef DEBUG_EXPRESSION_INIT
    clock_gettime(CLOCK_MONOTONIC, &end);
    const double seconds = (double)(end.tv_sec - start.tv_sec);
    const double nanoseconds = end.tv_nsec - start.tv_nsec;
    const double elapsed = seconds + nanoseconds * 1e-9;
    printf("\nExpression simplified in %f seconds.\n\n", elapsed);
#endif

    return 1;
}

// straightforward string to tokens function
int tokenize(expression_t* expr, symbol_table_t* table) {
    int token_count = 0;
    char buf[MAX_LEXEME_LEN] = {0};
    const char *p = expr->expression;
    int last_token_type = TOKEN_UNKNOWN;

    // for (int i = 0; expr->expression[i] != '\0'; i++) {
    //     printf("expr.expression[%d] = '%c' (0x%02X)\n", i, expr->expression[i], (unsigned char)expr->expression[i]);
    // }

    while (*p != '\0' && token_count < MAX_EXPR_LEN) {    // parse through inputted expression
        if (*p == ' ') { // skip spaces in input
            p++;
            continue;
        }

        // implicit multiplication detection and correction
        if ((last_token_type == TOKEN_NUMBER || last_token_type == TOKEN_SYMBOL || last_token_type == TOKEN_RPAREN)
            && (*p == '(' || isdigit(*p) || isalpha(*p))  // starts something new
            && !is_operator(*p) && *p != ')') {           // make sure it's not already an operator
            expr->tokens[token_count++] = op_token_init(expr->token_pool, '*');
            last_token_type = TOKEN_OPERATOR;
            continue;
        }

        // handle number
        if (isdigit(*p)) {
            // if the current character is a number
            int j = 0;
            while (isdigit(*p) || *p == '.') {
                if (j < MAX_LEXEME_LEN - 1) {
                    buf[j++] = *p;
                }
                p++;
            }
            buf[j] = '\0';
            expr->tokens[token_count++] = num_token_init(expr->token_pool, buf);
            last_token_type = TOKEN_NUMBER;
            memset(buf, 0, MAX_LEXEME_LEN);
            continue;
        }
    // Handle symbol
        if (isalpha(*p)) {
            int j = 0;
            while (isalpha(*p) || *p == '_') {
                if (j < MAX_LEXEME_LEN - 1) {
                    buf[j++] = *p;
                }
                p++;
            }
            buf[j] = '\0';
            if (find_symbol(table, buf)) {
                expr->tokens[token_count++] = sym_token_init(expr->token_pool, table, buf);
                last_token_type = TOKEN_SYMBOL;
                continue;
            }
            memset(buf, 0, MAX_LEXEME_LEN);
        }

            // Handle operator
        if (is_operator(*p)) {
            expr->tokens[token_count++] = op_token_init(expr->token_pool, *p);  //  .type = TOKEN_OPERATOR,
            last_token_type = TOKEN_OPERATOR;
            p++;
            continue;
        }

        if (*p == '(') {
            expr->tokens[token_count++] = paren_token_init(expr->token_pool, *p);
            last_token_type = TOKEN_LPAREN;
            p++;
            continue;
        }

        if (*p == ')') {
            expr->tokens[token_count++] = paren_token_init(expr->token_pool, *p);
            last_token_type = TOKEN_RPAREN;
            p++;
            continue;
        }
        // Unknown character
        p++;
    }
    return token_count;
}

/* how to make ast? // leaving this here bc it makes me feel smart
 *
 * I am thinking we don't just move around tokens in postfix - we build the tree in it
 * meaning we need to make a node struct that has variables value, left, and right.
 * when we detect a number, add it to a buffer, when we detect a symbol, add it to the
 * buffer, when we detect an operator, the operator becomes value and the left becomes
 * the next operator and right becomes the value of buffer. then we just make a stack
 * of those nodes
*/

// shunting yard algo but builds ast directly instead of returning postfix notation
// i'm not handling functions (sin, cos, ect.) yet

int build_ast(expression_t* expr) {
    token_t* tokens = *expr->tokens;
    const int token_count = expr->token_count;

    stack_t n, t;
    stack_init(&n);
    stack_init(&t);

    int i = 0;
    ast_node_t* node;

    while (i < token_count) {
        switch (tokens[i].type) {
            case TOKEN_NUMBER:
            case TOKEN_SYMBOL:
                node = create_node(expr->node_pool, &tokens[i], NULL, NULL);
                stack_push(&n, node);
                break;

            case TOKEN_OPERATOR:
                while (!stack_is_empty(&t)) {
                    const token_t* top = stack_peek(&t);
                    if (top->type != TOKEN_OPERATOR) break;

                    if (top->operator->precedence > tokens[i].operator->precedence ||
                        (top->operator->precedence == tokens[i].operator->precedence &&
                        tokens[i].operator->associativity == ASSOC_LEFT)) {
                        token_t* tok = stack_pop(&t);
                        ast_node_t* right = stack_pop(&n);
                        ast_node_t* left = stack_pop(&n);
                        node = create_node(expr->node_pool, tok, left, right);
                        stack_push(&n, node);
                    } else {
                        break;
                    }
                }
                stack_push(&t, &tokens[i]);
                break;

            case TOKEN_LPAREN:
                stack_push(&t, &tokens[i]);
                break;

            case TOKEN_RPAREN:
                while (!stack_is_empty(&t) && ((token_t*)stack_peek(&t))->type != TOKEN_LPAREN) {
                    token_t* tok = stack_pop(&t);
                    ast_node_t* right = stack_pop(&n);
                    ast_node_t* left = stack_pop(&n);
                    node = create_node(expr->node_pool, tok, left, right);
                    stack_push(&n, node);
                }
                if (!stack_is_empty(&t)) {

                    const token_t* lparen = stack_pop(&t);
                    token_pool_free_token(expr->token_pool, lparen);
                }

                token_pool_free_token(expr->token_pool, &tokens[i]);
                break;

            default: break;
        }
        i++;
    }
    while (!stack_is_empty(&t)) {
        token_t* tok = stack_pop(&t);
        ast_node_t* right = stack_pop(&n);
        ast_node_t* left = stack_pop(&n);
        node = create_node(expr->node_pool, tok, left, right);
        stack_push(&n, node);
    }

    ast_node_t* root = stack_pop(&n); // I think root would be tree->nodes[0] but not sure so for safety we are here
    expr->root = root;

    //return 0;
}

void free_subtree(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node) {
        return;
    }

    ast_node_t* left = node->left;
    ast_node_t* right = node->right;

    // Recurse first
    free_subtree(token_pool, node_pool, left);
    free_subtree(token_pool, node_pool, right);

    // Then free the token and node itself
    if (node->token) {
        token_pool_free_token(token_pool, node->token);
    }

    node_pool_free_node(node_pool, node);
}


ast_node_t* identity_property(const expression_t* expr, ast_node_t* node) {
    // TODO: expand this function to handle sub, div, and power operators (not 'true' identity property but meh they can be converted to addition)
    if (!(node->token->type == TOKEN_OPERATOR && (node->token->operator->type == ADD || node->token->operator->type == MUL))) {
        return node;
    }

    if (node->token->operator->type == ADD || node->token->operator->type == SUB) {                                                       // check if add or subtract zero
        if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {    // on the right
            ast_node_t* kept = node->left;
            node_pool_free_node(expr->node_pool, node->right);
            node_pool_free_node(expr->node_pool, node);  // free current node
            return kept;                                                                       // return left node
        }
        if (node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {      // for both children of add
            ast_node_t* kept = node->right;
            node_pool_free_node(expr->node_pool, node->left);
            node_pool_free_node(expr->node_pool, node);  // free current node
            return kept;
        }
    }
    if (node->token->operator->type == MUL) {
        if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 1) {    // on the right
            ast_node_t* kept = node->left;
            node_pool_free_node(expr->node_pool, node->right);
            node_pool_free_node(expr->node_pool, node);  // free current node
            return kept;                                                                    // return left node
        }
        if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 1) {      // for both children of add
            ast_node_t* kept = node->right;
            node_pool_free_node(expr->node_pool, node->left);
            node_pool_free_node(expr->node_pool, node);  // free current node
            return kept;
        }
    }
    return node;
}

ast_node_t* zero_property(const expression_t* expr, ast_node_t* node) {
    if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL)) {
        return node;
    }

    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {    // on the right
        ast_node_t* kept = node->right;
        node_pool_free_node(expr->node_pool, node->left);
        node_pool_free_node(expr->node_pool, node);  // free current node
        return kept;                                                                    // return left node
    }
    if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {      // for both children of add
        ast_node_t* kept = node->left;
        node_pool_free_node(expr->node_pool, node->right);
        node_pool_free_node(expr->node_pool, node);  // free current node
        return kept;
    }
    return node;
}


bool is_operator_node(const ast_node_t* node, const operator_type_t type) {
    return node && node->token->type == TOKEN_OPERATOR && node->token->operator->type == type;
}

void collect_terms(/*token_pool_t* token_pool,node_pool_t* node_pool,*/ const ast_node_t* node, stack_t* stack, const operator_type_t type) {
    if (!node) {
        return;
    }

    if (!is_operator_node(node, type)) {
        stack_push(stack, (ast_node_t*)node);
        return;
    }

    collect_terms(/*token_pool, node_pool,*/ node->left, stack, type);
    collect_terms(/*token_pool, node_pool,*/ node->right, stack, type);
}

ast_node_t* rebuild_commutative_subtree(token_pool_t* token_pool, node_pool_t* node_pool, stack_t* stack, const char* op) {
    // if the stack is empty, return immediately a NULL
    if (stack_is_empty(stack)) return NULL;

    // pop the top node from the stack, copy it, then free the original
    ast_node_t* original = stack_pop(stack);
    ast_node_t* left = copy_subtree(token_pool, node_pool, original);
    free_subtree(token_pool, node_pool, original);

    while (!stack_is_empty(stack)) {
        // again pop the top node from the stack, copy it, then free the original
        original = stack_pop(stack);
        ast_node_t* right = copy_subtree(token_pool, node_pool, original);
        free_subtree(token_pool, node_pool, original);

        // create a parent node of left and right
        ast_node_t* new_node = node_pool_alloc(node_pool);
        if (!new_node) return NULL;

        new_node->token = op_token_init(token_pool, *op);
        new_node->left = left;
        new_node->right = right;

        // build tree top down, left becomes future left's left child
        left = new_node;
    }

#ifdef DEBUG_COMMUTATIVE_PROPERTY
    print_tree(left);
#endif

    return left;
}

// associative property returns a left heavy tree, shuffling right associative multiplication and addition to left associative
void associative_property(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node, stack_t* stack) {
    if (!node || node->token->type != TOKEN_OPERATOR) return;

    const operator_type_t op_type = node->token->operator->type; // store current operators type

    if (op_type != ADD && op_type != MUL) return;  // return unedited node if not mul or add
    //ast_node_t* terms[MAX_TERMS];                       // create a list to store all terms
    //size_t count = 0;
    collect_terms(/*token_pool, node_pool,*/ node, stack, op_type);                                // collect all terms
    // return rebuild_commutative_subtree(stack, pool, node->token);    // rebuild the tree
}

/* The distributed property detects and simplifies expressions in the form of a*(b+c) into
 * the form of ab + ac, where a, b, and c are arbitrary nodes. The function must return a node,
 * either the root node of an unedited tree, or, if the tree is a candidate for distribution,
 * the root node of a distributed tree.
 *
 * Structure must be:
 *
 *              *                *
 *             / \              / \
 *            +   a     or     a   +
 *           / \                  / \
 *          b   c                b   c
 *
 *      * note that we only care about the operators and their local precedence
 *
 * And the distributed structure is:
 *
 *              +  <- return node
 *             / \
 *            *   *
 *           /\   /\
 *          a  b a  c
 *
*/

ast_node_t* distributive_property(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != MUL) {
        return node;
    }

    // Case: (b + c) * a
    if (node->left && node->left->token->type == TOKEN_OPERATOR &&
        node->left->token->operator->type == ADD) {

        const ast_node_t* a = node->right;
        const ast_node_t* b = node->left->left;
        const ast_node_t* c = node->left->right;

        ast_node_t* ab = node_pool_alloc(node_pool);
        ab->token = op_token_init(token_pool, '*');
        ab->left = copy_subtree(token_pool, node_pool, a);
        ab->right = copy_subtree(token_pool, node_pool, b);

        ast_node_t* ac = node_pool_alloc(node_pool);
        ac->token = op_token_init(token_pool, '*');
        ac->left = copy_subtree(token_pool, node_pool, a);
        ac->right = copy_subtree(token_pool, node_pool, c);

        ast_node_t* new_root = node_pool_alloc(node_pool);
        new_root->token = op_token_init(token_pool, '+');
        new_root->left = ab;
        new_root->right = ac;

        //free_subtree(token_pool, node_pool, node);
        return new_root;
        }

    // Optional: Case a * (b + c)
    if (node->right && node->right->token->type == TOKEN_OPERATOR &&
        node->right->token->operator->type == ADD) {

        const ast_node_t* a = node->left;
        const ast_node_t* b = node->right->left;
        const ast_node_t* c = node->right->right;

        ast_node_t* ab = node_pool_alloc(node_pool);
        ab->token = op_token_init(token_pool, '*');
        ab->left = copy_subtree(token_pool, node_pool, a);
        ab->right = copy_subtree(token_pool, node_pool, b);

        ast_node_t* ac = node_pool_alloc(node_pool);
        ac->token = op_token_init(token_pool, '*');
        ac->left = copy_subtree(token_pool, node_pool, a);
        ac->right = copy_subtree(token_pool, node_pool, c);

        ast_node_t* new_node = node_pool_alloc(node_pool);
        new_node->token = op_token_init(token_pool, '+');
        new_node->left = ab;
        new_node->right = ac;

        free_subtree(token_pool, node_pool, node);
        return new_node;
        }

    return node;
}

int compare_types(const void* a, const void* b) {
    const ast_node_t* node_a = *(const ast_node_t**)a;
    const ast_node_t* node_b = *(const ast_node_t**)b;

    if (!node_a || !node_b) {
        // Null goes last in sort order
        return node_a ? -1 : (node_b ? 1 : 0);
    }
    if (!node_a->token || !node_b->token) {
        return node_a->token ? -1 : (node_b->token ? 1 : 0);
    }

    return (int)node_a->token->type - (int)node_b->token->type;
}

void print_ast_stack(const stack_t* stack) {
    printf("Stack contents (top = %d):\n", stack->top);
    for (int t = 0; t <= stack->top; ++t) {
        ast_node_t* node = (ast_node_t*)stack->items[t];
        if (!node) {
            printf("  [%d]: NULL\n", t);
            continue;
        }
        if (!node->token) {
            printf("  [%d]: NODE with NULL token\n", t);
            continue;
        }
        switch (node->token->type) {
            case TOKEN_NUMBER:
                printf("  [%d]: NUMBER: %f\n", t, node->token->number.value);
                break;
            case TOKEN_SYMBOL:
                printf("  [%d]: SYMBOL: %s\n", t, node->token->symbol->symbol);
                break;
            case TOKEN_OPERATOR:
                printf("  [%d]: OPERATOR: %c\n", t, node->token->operator->operator);
                break;
            default:
                printf("  [%d]: UNKNOWN or OTHER type: %d\n", t, node->token->type);
                break;
        }
    }
    printf("\n");
}

ast_node_t* commutative_property_add(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR) return node;

    const operator_type_t op_type = node->token->operator->type;
    if (op_type != ADD) return node;

    stack_t terms;
    stack_init(&terms);
    // TODO: Switch to queue based data structure

    associative_property(token_pool, node_pool, node, &terms);
    if (terms.top < 0) {
        return node;
    }

#ifdef DEBUG_COMMUTATIVE_PROPERTY
    printf("Before sorting:/n");
    print_ast_stack(&terms);
#endif

    // sort the nodes by comparing each node's token's type (number->symbol->operator)
    // using a c standard sorting algorithm. i think it, quick sort, is pretty nifty in both the
    // algorithm's simplicity, and the fact that the c standard library provides few other
    // niceties to get excited about (that I have come across anyway)
    qsort(terms.items, terms.top + 1, sizeof(ast_node_t*), compare_types); // c stdio.h function
    // TODO: when i implement functions, unary operators, etc. check to make sure this still works

#ifdef DEBUG_COMMUTATIVE_PROPERTY
    printf("After sorting:/n");
    print_ast_stack(&terms);
#endif

    double vals[terms.top + 1];
    int i = 0;

    // count how many numbers are at the front of the stack
    while (i <= terms.top && ((ast_node_t*)terms.items[i])->token->type == TOKEN_NUMBER) {
        vals[i] = ((ast_node_t*)terms.items[i])->token->number.value;
        i++;
    }
    // if two or more numbers are collected
    if (i >= 2) { // if we collect only one number, there is nothing to do
        // if the operator is MUL, we need to set new_val to 1 (avoid mul by 0) or if ADD, set it to 0
        double new_val = 0;
        for (int j = 0; j < i; j++) {
            new_val += vals[j];
        }

        ast_node_t* new_node = node_pool_alloc(node_pool);
        new_node->token = num_token_init_double(token_pool, new_val);
        new_node->left = NULL;
        new_node->right = NULL;

        for (int k = 0; k < i; k++) {
            const ast_node_t* free_node = stack_pop_bottom(&terms);
            token_pool_free_token(token_pool, free_node->token);
            node_pool_free_node(node_pool, free_node);
        }
        stack_push(&terms, new_node);
    }
    //ast_node_t* new_root = rebuild_commutative_subtree(token_pool, node_pool, &terms, node->token->operator->operator);
    return rebuild_commutative_subtree(token_pool, node_pool, &terms, node->token->operator->operator);
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

ast_node_t* properties_of_subtraction(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != SUB) {
        return node;
    }

    // Case: x - 0 => x
    if (node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {
        ast_node_t* new_node = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node);
        return new_node;
    }

    // Case: x - x => 0
    // NOTE: this compares pointer equality. If you want structural equality, write node_equals().
    if (branch_is_equal(node->left, node->right)) {
        ast_node_t* zero_node = node_pool_alloc(node_pool);
        zero_node->token = num_token_init_double(token_pool, 0);
        zero_node->left = NULL;
        zero_node->right = NULL;

        free_subtree(token_pool, node_pool, node);
        return zero_node;
    }

    return node;
}

ast_node_t* properties_of_division(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != DIV) {
        return node;
    }
    if (!node->left || !node->right) {
        return node;
        // TODO: Should return error, can't have floating div op, but should also never happen unless someone (me)
        //     : purposely inputs it
    }

    // TODO: should i be checking if this is a const? ie. number, symbol, or special char (pi, e, ...; not implemented)
    if (node->left->token->type == TOKEN_SYMBOL || node->right->token->type == TOKEN_SYMBOL) {
        // if 0 div x (ie. 0/1)
        if (node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {
            ast_node_t* kept = node->left;

            token_pool_free_token(token_pool, node->right->token);
            node_pool_free_node(node_pool, node->right);

            token_pool_free_token(token_pool, node->token);
            node_pool_free_node(node_pool, node);  // free current node
            return kept;
        }
        // div by 0
        if (node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {
            // this function doesn't work
            // TODO: should return error
            //     : idea: return an error node
            free_subtree(token_pool, node_pool, node);
            return NULL;
        }

        if (node->right->token->number.value == 1) {
            ast_node_t* kept = node->right;

            token_pool_free_token(token_pool, node->left->token);
            node_pool_free_node(node_pool, node->left);

            token_pool_free_token(token_pool, node->token);
            node_pool_free_node(node_pool, node);  // free current node
            return kept;
        }
    }
    return node;
}

/* The product of powers function detects and simplifies expressions in the form of a^m * a^n into
 * the form of a^(m + n), where a, m, and n are arbitrary nodes. The function must return a node,
 * either the root node of an unedited tree, or, if the tree is a candidate for simplification,
 * the root node of the simplified tree.
 *
 * Structure must be:
 *
 *              *
 *          ^       ^
 *        a   m   a   n
 *
 *      * note that we only care about the operators and their local precedence
 *
 * And the simplified structure is:
 *
 *              ^
 *          a       +
 *                m   n
 *
 *
*/

#define MAX_BASE_GROUPS 32

// Base group to hold a unique base with accumulated exponents
typedef struct {
    ast_node_t* base;           // The base subtree (e.g. 'x', '2', '(a+b)')
    double exponent_sum;        // Sum of numeric exponents
    stack_t symbolic_exponents; // stack of symbolic exponent nodes (non-numeric)
} base_group_t;

// list of base groups in mul chain
typedef struct {
    base_group_t groups[MAX_BASE_GROUPS];   // list of base groups
    int count;                              // number of base groups in chain
    double numeric_product;                 // product of all pure numeric bases^exponents in chain
} base_group_list_t;

// if the node is a^ (x + y) and a is numeric, rewrite as (a^x) * (a^y)
ast_node_t* expand_exponent_addition(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != POW)
        return node;

    const ast_node_t* base = node->left;
    const ast_node_t* exp = node->right;

    // only transform if base is numeric and exponent is a sum
    if (base->token->type != TOKEN_NUMBER)
        return node;

    if (exp->token->type != TOKEN_OPERATOR || exp->token->operator->type != ADD)
        return node;

    // rewrite: a^(x + y) → a^x * a^y
    ast_node_t* left_pow = node_pool_alloc(node_pool);
    left_pow->token = op_token_init(token_pool, '^');
    left_pow->left = copy_subtree(token_pool, node_pool, base);
    left_pow->right = copy_subtree(token_pool, node_pool, exp->left);

    ast_node_t* right_pow = node_pool_alloc(node_pool);
    right_pow->token = op_token_init(token_pool, '^');
    right_pow->left = copy_subtree(token_pool, node_pool, base);
    right_pow->right = copy_subtree(token_pool, node_pool, exp->right);

    ast_node_t* mul = node_pool_alloc(node_pool);
    mul->token = op_token_init(token_pool, '*');
    mul->left = left_pow;
    mul->right = right_pow;

    free_subtree(token_pool, node_pool, node); // optionally free the original
    return mul;
}

// flatten multiplicative tree into an array, updating count
static void flatten_mul_chain(ast_node_t* node, ast_node_t* flat[], int* count) {
    if (!node) return;

    if (node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL) {
        flatten_mul_chain(node->left, flat, count);
        flatten_mul_chain(node->right, flat, count);
    } else {
        flat[(*count)++] = node;
    }
}

// Normalize node into base^exp form.
// If node is POW, base=node->left, exp=node->right
// else base=node, exp=1 (allocated new number node)
static void normalize_node(ast_node_t* node, ast_node_t** base_out, ast_node_t** exp_out,
                           token_pool_t* token_pool, node_pool_t* node_pool) {
    if (node->token->type == TOKEN_OPERATOR && node->token->operator->type == POW) {
        *base_out = node->left;
        *exp_out = node->right;
    } else {
        *base_out = node;

        // Allocate exponent node = 1
        ast_node_t* exp_node = node_pool_alloc(node_pool);
        exp_node->token = num_token_init_double(token_pool, 1);
        exp_node->left = NULL;
        exp_node->right = NULL;

        *exp_out = exp_node;
    }
}

// Insert base^exp into groups, folding numeric exponents and accumulating symbolic exponents
static void base_group_match(base_group_list_t* groups, ast_node_t* base, ast_node_t* exp) {

    // fold pure numeric bases and exponents immediately
    if (base->token->type == TOKEN_NUMBER && exp->token->type == TOKEN_NUMBER) {
        const double pow_val = pow(base->token->number.value, exp->token->number.value);
        groups->numeric_product *= pow_val;
        return;
    }

    // Search existing groups for base equality
    for (int i = 0; i < groups->count; i++) {
        // if a matching base is found
        if (branch_is_equal(groups->groups[i].base, base)) {
            // if exp is a number
            if (exp->token->type == TOKEN_NUMBER) {
                // add it to the exponent sum
                groups->groups[i].exponent_sum += exp->token->number.value;
            }
            // otherwise
            else {
                // push symbolic exponent on stack
                stack_push(&groups->groups[i].symbolic_exponents, exp);
            }
            return;
        }
    }

    // no matching base group found; create a new base group
    // protect from base_group overflow
    if (groups->count >= MAX_BASE_GROUPS) {
        return; // should probably return error
    }

    base_group_t* group = &groups->groups[groups->count++]; // create a new base group
    group->base = base;                                     // set the group's base to the input base
    group->exponent_sum = 0;                                // set the group's exponent sum to zero
    stack_init(&group->symbolic_exponents);                 // init the group's stack of symbolic exponents

    if (exp->token->type == TOKEN_NUMBER) {
        group->exponent_sum = exp->token->number.value;
    } else {
        stack_push(&group->symbolic_exponents, exp);
    }
}

// Build AST for addition chain of symbolic exponents and numeric sum
static ast_node_t* build_exponent_chain(base_group_t* group, token_pool_t* token_pool, node_pool_t* node_pool) {
    ast_node_t* exp_chain = NULL;

    // Add numeric exponent if nonzero
    if (group->exponent_sum != 0.0) {
        exp_chain = node_pool_alloc(node_pool);
        exp_chain->token = num_token_init_double(token_pool, group->exponent_sum);
        exp_chain->left = NULL;
        exp_chain->right = NULL;
    }

    // Fold symbolic exponents as additions: e1 + e2 + ...
    while (!stack_is_empty(&group->symbolic_exponents)) {
        const ast_node_t* sym_exp = stack_pop(&group->symbolic_exponents);
        ast_node_t* sym_copy = copy_subtree(token_pool, node_pool, sym_exp);

        if (!exp_chain) {
            exp_chain = sym_copy;
        } else {
            ast_node_t* add_node = node_pool_alloc(node_pool);
            add_node->token = op_token_init(token_pool, '+');
            add_node->left = exp_chain;
            add_node->right = sym_copy;
            exp_chain = add_node;
        }
    }

    return exp_chain;
}

// Reconstruct the simplified product AST from groups
static ast_node_t* reconstruct_product(base_group_list_t* groups,
                                      token_pool_t* token_pool, node_pool_t* node_pool) {
    ast_node_t* result = NULL;

    // Add folded numeric product if not 1
    if (groups->numeric_product != 1.0) {
        ast_node_t* num_node = node_pool_alloc(node_pool);
        num_node->token = num_token_init_double(token_pool, groups->numeric_product);
        num_node->left = NULL;
        num_node->right = NULL;
        result = num_node;
    }

    for (int i = 0; i < groups->count; i++) {
        base_group_t* group = &groups->groups[i];
        ast_node_t* base_copy = copy_subtree(token_pool, node_pool, group->base);
        ast_node_t* exp_chain = build_exponent_chain(group, token_pool, node_pool);

        // If exponent is NULL (meaning sum=0 and no symbolic exponents), exponent = 1
        if (!exp_chain) {
            exp_chain = node_pool_alloc(node_pool);
            exp_chain->token = num_token_init_double(token_pool, 1);
            exp_chain->left = NULL;
            exp_chain->right = NULL;
        }

        // If exponent == 1, skip pow node, just use base
        const bool is_exp_one = (exp_chain->token->type == TOKEN_NUMBER && exp_chain->token->number.value == 1.0);

        ast_node_t* term = NULL;
        if (is_exp_one) {
            term = base_copy;
            // free exp_chain node since unused? (depends on your mem management)
        } else {
            term = node_pool_alloc(node_pool);
            term->token = op_token_init(token_pool, '^');
            term->left = base_copy;
            term->right = exp_chain;
        }

        if (!result) {
            result = term;
        } else {
            ast_node_t* mul_node = node_pool_alloc(node_pool);
            mul_node->token = op_token_init(token_pool, '*');
            mul_node->left = result;
            mul_node->right = term;
            result = mul_node;
        }
    }

    // If no groups and numeric product is 1, result is 1
    if (!result) {
        ast_node_t* one_node = node_pool_alloc(node_pool);
        one_node->token = num_token_init_double(token_pool, 1);
        one_node->left = NULL;
        one_node->right = NULL;
        result = one_node;
    }

    return result;
}

// The full function integrating everything
ast_node_t* product_of_powers(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != MUL)
        return node;

    node = expand_exponent_addition(node, token_pool, node_pool);

    ast_node_t* flat[MAX_BASE_GROUPS];
    int count = 0;

    // collect the associative mul chain in an array
    flatten_mul_chain(node, flat, &count);

    base_group_list_t groups = { .count = 0, .numeric_product = 1.0 };

    // loop through the array
    for (int i = 0; i < count; i++) {
        // create base and exp nodes
        ast_node_t* base = NULL;
        ast_node_t* exp = NULL;
        // populate the base and exp nodes
        normalize_node(flat[i], &base, &exp, token_pool, node_pool);
        // insert exp into the matching base group, creating a new one with base if necessary
        base_group_match(&groups, base, exp);
    }
    ast_node_t* subroot = reconstruct_product(&groups, token_pool, node_pool);
    free_subtree(token_pool, node_pool, node);
    return subroot;
}

ast_node_t* quotient_of_powers(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != DIV) {
        return node;
    }

    ast_node_t* left = node->left;
    ast_node_t* right = node->right;

    if (!left || !right ||
        left->token->type != TOKEN_OPERATOR || left->token->operator->type != POW ||
        right->token->type != TOKEN_OPERATOR || right->token->operator->type != POW) {
        return node;
    }

    ast_node_t* a1 = left->left;
    ast_node_t* a2 = right->left;

    if (!branch_is_equal(a1, a2)) {
        return node;
    }

    // Create m + n
    ast_node_t* diff = node_pool_alloc(node_pool);
    diff->token = op_token_init(token_pool, '-');
    diff->left = copy_subtree(token_pool, node_pool, left->right);
    diff->right = copy_subtree(token_pool, node_pool, right->right);

    // Create a^(m + n)
    ast_node_t* result = node_pool_alloc(node_pool);
    result->token = op_token_init(token_pool, '^');
    result->left = copy_subtree(token_pool, node_pool, a1);
    result->right = diff;

    //free_subtree(token_pool, node_pool, node); // uncomment when you're ready to manage memory
    return result;
}

/* The product of powers function detects and simplifies expressions in the form of a^m * a^n into
 * the form of a^(m + n), where a, m, and n are arbitrary nodes. The function must return a node,
 * either the root node of an unedited tree, or, if the tree is a candidate for simplification,
 * the root node of the simplified tree.
 *
 * Structure must be:
 *
 *              ^
 *          ^       n
 *        a   m
 *
 *      * note that we only care about the operators and their local precedence
 *
 * And the simplified structure is:
 *
 *              ^
 *          a       *
 *                m   n
*/
ast_node_t* power_of_powers(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != POW) {
        return node;
    }

    if (!node->left || node->left->token->type != TOKEN_OPERATOR || node->left->token->operator->type != POW) {
        return node;
    }

    const ast_node_t* a = node->left->left;
    const ast_node_t* m = node->left->right;
    const ast_node_t* n = node->right;

    ast_node_t* exponent = node_pool_alloc(node_pool);
    exponent->token = op_token_init(token_pool, '*');
    exponent->left = copy_subtree(token_pool, node_pool, m);
    exponent->right = copy_subtree(token_pool, node_pool, n);

    ast_node_t* base = node_pool_alloc(node_pool);
    base->token = op_token_init(token_pool, '^');
    base->left = copy_subtree(token_pool, node_pool, a);
    base->right = exponent;

    free_subtree(token_pool, node_pool, node);
    return base;
}

ast_node_t* solve(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR) return node;

    if (node->left->token->type == TOKEN_NUMBER &&
        node->right->token->type == TOKEN_NUMBER) {

        const double a = node->left->token->number.value;
        const double b = node->right->token->number.value;
        double new_val;

        switch (node->token->operator->type) {
            case ADD: new_val = a + b; break;
            case SUB: new_val = a - b; break;
            case MUL: new_val = a * b; break;
            case DIV:
                if (b == 0) return node; // avoid divide by zero
                new_val = a / b;
                break;
            case POW: new_val = pow(a, b); break;
            default: return node;
        }

        // Allocate a new token and store the number value
        ast_node_t* new_node = node_pool_alloc(node_pool);
        new_node->token = num_token_init_double(token_pool, new_val);
        new_node->left = NULL;
        new_node->right = NULL;

        free_subtree(token_pool, node_pool, node);
        return new_node;
        }
    return node;
}
// TODO: this should be the only place in these functions that we expose expr. in all function inside simplify, use expr's pools,
ast_node_t* simplify(expression_t* expr, ast_node_t* node) {
    if (!node) return NULL;

    // First simplify children
    node->left = simplify(expr, node->left);
    node->right = simplify(expr, node->right);

    // Try simplifying the current node directly
    if (node->token->type == TOKEN_OPERATOR) {
        switch (node->token->operator->type) {
            case ADD:
                node = zero_property(expr, node);
                node = identity_property(expr, node);
                node = commutative_property_add(expr->token_pool, expr->node_pool, node);
                break;

            case SUB:
                node = properties_of_subtraction(expr->token_pool, expr->node_pool, node);
                break;

            case MUL:
                node = zero_property(expr, node);
                node = identity_property(expr, node);
                node = distributive_property(expr->token_pool, expr->node_pool, node);
                //node = commutative_property_mul(expr->token_pool, expr->node_pool, node);
                node = product_of_powers(expr->token_pool, expr->node_pool, node);
                break;

            case DIV:
                node = properties_of_division(expr->token_pool, expr->node_pool, node);
                node = quotient_of_powers(expr->token_pool, expr->node_pool, node);
                break;

            case POW:
                node = power_of_powers(expr->token_pool, expr->node_pool, node);
                break;

            default:
                break;
        }

        // After all property rewrites, try folding numeric constants again
        if (node &&
            node->token->type == TOKEN_OPERATOR &&
            node->left &&
            node->right &&
            node->left->token->type == TOKEN_NUMBER &&
            node->right->token->type == TOKEN_NUMBER) {
            node = solve(expr->token_pool, expr->node_pool, node);
        }
    }
    return node;
}