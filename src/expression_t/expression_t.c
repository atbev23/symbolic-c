//
// Created by atbev on 5/24/2025.
//

#include <stdlib.h>
#include <string.h>
#include "expression_t.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include "../token_t/token_pool_t/token_pool_t.h"
#include "../node_t/node_pool_t/node_pool_t.h"
#include "../stack_t/stack_t.h"

void expression_init(expression_t* expr_obj, const char* expr_str) {
    memset(expr_obj, 0, sizeof(expression_t));               // make sure expressiion
    strncpy(expr_obj->expression, expr_str, MAX_EXPR_LEN);  // set the expression
    //expr_obj->expression[MAX_EXPR_LEN - 1] = '\0';                   // null terminate the expression
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
            if (find_symbol(table, buf)) {
                expr->tokens[token_count++] = sym_token_init(expr->token_pool, table, buf);
                last_token_type = TOKEN_SYMBOL;
                continue;
            }
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

    return 0;
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
        if (node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 1) {      // for both children of add
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
    // if (!a || !b) {
    //     // handle error or return 0 for equality to avoid segfault
    //     return 0;
    // }
    const ast_node_t* node_a = *(const ast_node_t**)a;
    const ast_node_t* node_b = *(const ast_node_t**)b;

    return (int)node_a->token->type - (int)node_b->token->type; // the enum values of token_type_t
}

ast_node_t* commutative_property(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!node || node->token->type != TOKEN_OPERATOR) return node;

    const operator_type_t op_type = node->token->operator->type;
    if (op_type != ADD && op_type != MUL) return node;

    stack_t terms;
    stack_init(&terms);
    // TODO: Switch to queue based data structure

    associative_property(token_pool, node_pool, node, &terms);
    if (!terms.top) {
        return node;
    }

    // sort the nodes by comparing each node's token's type (number->symbol->operator)
    // using a c standard sorting algorithm. i think it, quick sort, is pretty nifty in both the
    // algorithm's simplicity, and the fact that the c standard library provides few other
    // niceties to get excited about
    qsort(&terms.items, terms.top + 1, sizeof(ast_node_t*), compare_types); // c stdio.h function
    // TODO: when i implement functions, unary operators, etc. check to make sure this still works

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
        double new_val = op_type == MUL ? 1 : 0;
        switch (op_type) {
            case ADD:
                for (int j = 0; j < i; j++) {
                    new_val += vals[j];
                }
                break;
            case MUL:
                for (int j = 0; j < i; j++) {
                    new_val *= vals[j];
                }
                break;
            default: break;
        }

        ast_node_t* new_node = node_pool_alloc(node_pool);
        new_node->token = num_token_init_double(token_pool, new_val);
        new_node->left = NULL;
        new_node->right = NULL;

        for (int k = i; k > 0; k--) {
            const ast_node_t* free_node = stack_pop_bottom(&terms);
            //token_pool_free_token(token_pool, free_node->token);
            //node_pool_free_node(node_pool, free_node);
        }
        stack_push(&terms, new_node);
    }
    ast_node_t* new_root = rebuild_commutative_subtree(token_pool, node_pool, &terms, node->token->operator->operator);
    return new_root;
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

#define MAX_BASE_GROUPS 8

// data type to store information related to a singular power node
typedef struct {
    ast_node_t* base;
    stack_t exponents;
} base_group_t;

// data type to store a group of base_group_t
typedef struct {
    base_group_t groups[MAX_BASE_GROUPS];
    int group_count;
} base_groups_t;

void collect_assoc_base_groups(token_pool_t* token_pool, node_pool_t* node_pool, base_groups_t* groups, ast_node_t* node) {
    // return immediately if the node is null
    if (!node)
        return;

    // call the function recursively until the node is no longer a part of an associative chain
    if (node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL) {
        collect_assoc_base_groups(token_pool, node_pool, groups, node->left);
        collect_assoc_base_groups(token_pool, node_pool, groups, node->right);
        return;
    }

    // if the node is a power operator
    if (node->token->type == TOKEN_OPERATOR && node->token->operator->type == POW) {
        ast_node_t* base = node->left;
        ast_node_t* exp = node->right;

        // look for a matching base group
        for (int i = 0; i < groups->group_count; i++) {
            // if found, add the right child to its stack of exponents and return
            if (base->token->type == TOKEN_SYMBOL &&
                base->token->symbol->symbol == groups->groups[i].base->token->symbol->symbol) {
                stack_push(&groups->groups[i].exponents, exp);
                return;
            }
        }

        if (groups->group_count >= MAX_BASE_GROUPS) return; // safeguard

        // if a matching base is not found, create a new base group with the nodes base and exponent and return
        const int i = groups->group_count++;
        groups->groups[i].base = base;
        stack_init(&groups->groups[i].exponents);
        stack_push(&groups->groups[i].exponents, exp);
        return;
    }

    // if the node is not a power operator but a part of the associative chain, store it as a power of 1
    ast_node_t* base = node;
    ast_node_t* exp = node_pool_alloc(node_pool);
    exp->token = num_token_init_double(token_pool, 1);
    exp->left = NULL;
    exp->right = NULL;

    // look for a matching base group
    for (int i = 0; i < groups->group_count; ++i) {
        // if found, add the right child to its stack of exponents and return
        if (groups->groups[i].base == base) {
            stack_push(&groups->groups[i].exponents, exp);
            return;
        }
    }

    if (groups->group_count >= MAX_BASE_GROUPS) return;

    // if a matching base is not found, create a new base group with the nodes base and exponent and return
    const int i = groups->group_count++;
    groups->groups[i].base = base;
    stack_init(&groups->groups[i].exponents);
    stack_push(&groups->groups[i].exponents, exp);
}


ast_node_t* product_of_powers(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    // return immediately if node is not a part of an associative chain
    if (!node || node->token->type != TOKEN_OPERATOR || node->token->operator->type != MUL) {
        return node;
    }
    // TODO: i think we will infrequently need more than one base group so this function should decide whether to define
    //      one or multiple base groups
    base_groups_t groups = {0};
    // gather stacks of all bases and stacks of associated exponents
    // TODO: this function does not work on an ambiguous input, such as 3^2m tokenized to (3^2)*m instead of 3^(2*m).
    //       i need to find a way to make inputs more rigid or tokenized to such. And, if the former, it can be
    //       handled in an embedded wrapper
    collect_assoc_base_groups(token_pool, node_pool, &groups, node);

    ast_node_t* result = NULL;
    // loop through all fo the collected base groups
    for (int i = 0; i < groups.group_count; i++) {
        stack_t* exponents = &groups.groups[i].exponents;
        const int num_exp = exponents->top + 1;

        // sort the base group's exponents, numbers first
        qsort(&exponents->items, num_exp, sizeof(void*), compare_types);

        // Collect the numbers
        double vals[num_exp];
        int j = 0;
        while (j <= exponents->top && ((ast_node_t*)exponents->items[j])->token->type == TOKEN_NUMBER) {
            vals[j] = ((ast_node_t*)exponents->items[j])->token->number.value;
            j++;
        }

        // If two or more numbers, fold them
        if (j >= 2) {
            double sum = 0;
            for (int k = 0; k < j; k++) {
                sum += vals[k];
            }

            // create a number node with the sum
            ast_node_t* sum_node = node_pool_alloc(node_pool);
            sum_node->token = num_token_init_double(token_pool, sum);
            sum_node->left = NULL;
            sum_node->right = NULL;
            // Replace first number exponent with the sum
            //((ast_node_t*)exponents->items[0])->token->number.value = sum;

            for (int l = j; l > 0; l--) {
                const ast_node_t* free_node = stack_pop_bottom(exponents);
                token_pool_free_token(token_pool, free_node->token);
                node_pool_free_node(node_pool, free_node);
            }

            stack_push(exponents, sum_node);
        }

        // Now build the exponent chain from what's left in exponents
        if (exponents->top < 0) {
            // No exponents, so power is base ^ 1 (just base)
            ast_node_t* base_copy = copy_subtree(token_pool, node_pool, groups.groups[i].base);

            if (!result) {
                result = base_copy;
            } else {
                ast_node_t* mul_node = node_pool_alloc(node_pool);
                mul_node->token = op_token_init(token_pool, '*');
                mul_node->left = result;
                mul_node->right = base_copy;
                result = mul_node;
            }

            continue;
        }

        // Pop the first exponent and copy it
        ast_node_t* exp_chain = copy_subtree(token_pool, node_pool, stack_pop(exponents));

        // Pop remaining exponents and build right-associative exponent chain: e1 ^ (e2 ^ (e3 ...))
        // TODO: this function creates nodes for exponent values or one (a^1) whereas it should skip creating nodes
        //       for OPERATOR_TOKEN->POW and '1'
        while (!stack_is_empty(exponents)) {
            ast_node_t* next_exp = copy_subtree(token_pool, node_pool, stack_pop(exponents));

            ast_node_t* pow_node = node_pool_alloc(node_pool);
            pow_node->token = op_token_init(token_pool, '+');
            pow_node->left = exp_chain;
            pow_node->right = next_exp;

            exp_chain = pow_node;
        }

        // Now build base ^ exponent_chain, copying base to avoid aliasing
        ast_node_t* base_copy = copy_subtree(token_pool, node_pool, groups.groups[i].base);

        ast_node_t* base_pow = node_pool_alloc(node_pool);
        base_pow->token = op_token_init(token_pool, '^');
        base_pow->left = base_copy;
        base_pow->right = exp_chain;

        // Multiply into result
        if (!result) {
            result = base_pow;
        } else {
            ast_node_t* mul_node = node_pool_alloc(node_pool);
            mul_node->token = op_token_init(token_pool, '*');
            mul_node->left = result;
            mul_node->right = base_pow;
            result = mul_node;
        }
    }

    return result;
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

        token_pool_free_token(token_pool, node->token);
        token_pool_free_token(token_pool, node->left->token);
        node_pool_free_node(node_pool, node->left);
        token_pool_free_token(token_pool, node->right->token);
        node_pool_free_node(node_pool, node->right);

        // Allocate a new token and store the number value
        //ast_node_t* new_node = node_pool_alloc(node_pool);
        node->token = num_token_init_double(token_pool, new_val);
        node->left = NULL;
        node->right = NULL;

        return node;
        }
    return node;
}
// TODO: this should be the only place in these functions that we expose expr. in all function inside simplify, use expr's pools,
ast_node_t* simplify(expression_t* expr, ast_node_t* node) {
    if (!node) return NULL;

    node->left = simplify(expr, node->left);    // recurse to bottom left of tree
    node->right = simplify(expr, node->right);  // then bottom right

    if (node->token->type != TOKEN_OPERATOR) {
        node = solve(expr->token_pool, expr->node_pool, node);
        return node;
    }

    switch (node->token->operator->type) {
        case ADD:
            node = zero_property(expr, node); // try zero property
            node = identity_property(expr, node); // try zero property
            node = commutative_property(expr->token_pool, expr->node_pool, node);
            return node;

        case SUB:
            node = properties_of_subtraction(expr->token_pool, expr->node_pool, node);
            return node;

        case MUL:
            node = zero_property(expr, node); // try zero property
            node = identity_property(expr, node); // try zero property
            node = distributive_property(expr->token_pool, expr->node_pool, node);
            node = commutative_property(expr->token_pool, expr->node_pool, node);
            //node = product_of_powers(expr->token_pool, expr->node_pool, node);
            return node;

        case DIV:
            node = properties_of_division(expr->token_pool, expr->node_pool, node);
            node = quotient_of_powers(expr->token_pool, expr->node_pool, node);
            return node;

        case POW:
            node = power_of_powers(expr->token_pool, expr->node_pool, node);
            return node;

        default: return node;
    }
    //ast_node_t* simplified = NULL;
    //return node;
}