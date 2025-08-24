//
// Created by atbev on 5/24/2025.
//

// HUUUGE IDEA, MAYBE THE BIGGEST IDEA, THAT'S WHAT THEY'RE SAYING ANYWAY
// this is really advanced, at least in my eyes
// but i think we should split the expression into its terms
// and then we can thread each term, simplify, then recombine
// i don't think we will run into concurrency issues
// because as terms (separated by +/-) they should have no
// effect on each other

#include <stdlib.h>
#include <string.h>
#include "expression_t.h"
#include "../token_t/token_pool_t/token_pool_t.h"
#include "../node_t/node_pool_t/node_pool_t.h"
#include "../stack_t/stack_t.h"
#include "utils/utils.h"

#include <ctype.h>
#include <math.h>
#include <pthread_time.h>
#include <stdio.h>
#include <stdbool.h>

//#define DEBUG_EXPRESSION_INIT
//#define DEBUG_COMMUTATIVE_PROPERTY
//#define DEBUG_DISTRIBUTIVE_PROPERTY

int expression_init(expression_t* expr, const char* input_expr, symbol_table_t* table) {
    static token_pool_t token_pool; // declare a token pool
    token_pool_init(&token_pool);   // initialize the token pool

    static node_pool_t node_pool;   // declare a node pool
    node_pool_init(&node_pool);     // initialize the node pool

    expr->token_pool = &token_pool; // link the declared token pool to the expression
    expr->node_pool = &node_pool;   // link the declared node pool to the expression

    strncpy(expr->expression, input_expr, sizeof(expr->expression));    // copy the inputted string to the expression
    expr->expression[sizeof(expr->expression)-1] = '\0';                      // null terminate the string

#ifdef DEBUG_EXPRESSION_INIT
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    expr->token_count = tokenize(expr, table);  // tokenize the expression
    if (expr->token_count == 0) {               // if no tokens were created,
        return 0;                               // return immediately
    }

    expr->node_count = build_ast(expr); // build an abstract syntax tree using the tokenized expression
    if (!expr->root) {                  // if a tree was not created,
        return 0;                       // return immediately
    }
    // TODO: need to simplify until no more simplification can be done
    expr->root = sub_to_add(expr->root, expr->token_pool, expr->node_pool);
    expr->root = div_to_mul(expr->root, expr->token_pool, expr->node_pool);
    expr->root = neg_to_mul(expr->root, expr->token_pool, expr->node_pool);
    expr->root = simplify(expr->root, expr->token_pool, expr->node_pool);   // simplify the expression using algebraic properties
    expr->root = simplify(expr->root, expr->token_pool, expr->node_pool);   // run the simplification algorithm again
    expr->root = simplify(expr->root, expr->token_pool, expr->node_pool);   // and again

    if (!expr->root) {  // if the expression simplifies to 0 or errored,
        return 0;       // return immediately
    }

#ifdef DEBUG_EXPRESSION_INIT
    clock_gettime(CLOCK_MONOTONIC, &end);
    const double seconds = (double)(end.tv_sec - start.tv_sec);
    const double nanoseconds = end.tv_nsec - start.tv_nsec;
    const double elapsed = seconds + nanoseconds * 1e-9;
    printf("\nExpression simplified in %f seconds.\n\n", elapsed);
#endif

    return 1; // function success
}

// straightforward string to tokens function
int tokenize(expression_t* expr, symbol_table_t* table) {
    int token_count = 0;                    // create a variable to store the number of tokens created
    char buf[MAX_LEXEME_LEN] = {0};         // buffer to store numbers or characters as we are looping through a chain
    const char *p = expr->expression;       // pointer to the current character of the expression (char array)
    int last_token_type = TOKEN_UNKNOWN;    // variable to track the last token type

    // for (int i = 0; expr->expression[i] != '\0'; i++) {
    //     printf("expr.expression[%d] = '%c' (0x%02X)\n", i, expr->expression[i], (unsigned char)expr->expression[i]);
    // }

    while (*p != '\0' && token_count < MAX_EXPR_LEN) {  // parse through inputted expression,
        if (*p == ' ') {                                // if the current character is a space
            p++;                                        // ignore the character
            continue;
        }

        if (*p == '-') {
            // detect if this is a unary minus, not binary minus
            const bool is_unary =
                token_count == 0 ||
                last_token_type == TOKEN_OPERATOR ||
                last_token_type == TOKEN_LPAREN ||
                last_token_type == TOKEN_UNARY;

            if (is_unary) {
                expr->tokens[token_count++] = unary_token_init(expr->token_pool, UNARY_NEG);
                last_token_type = TOKEN_UNARY;
                p++;
                continue;
            }
        }

        // implicit multiplication detection and correction
        if ((last_token_type == TOKEN_NUMBER ||
            last_token_type == TOKEN_SYMBOL ||
            last_token_type == TOKEN_RPAREN) &&
            (*p == '(' || isdigit(*p) || isalpha(*p))  // starts something new
            && !is_operator(*p) && *p != ')') {        // make sure it's not already an operator

            expr->tokens[token_count++] = op_token_init(expr->token_pool, '*');
            last_token_type = TOKEN_OPERATOR;
            continue;
        }

        if (isdigit(*p)) {
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

            // handle operator
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
        // unknown character
        p++;
    }
    return token_count;
}

/* how to build ast from tokens?
 *
 * I am thinking we don't just move around tokens in postfix - we build the tree in it
 * meaning we need to make a node struct that has variables value, left, and right.
 * when we detect a number or symbol, add it to a buffer, when we detect an operator,
 * the operator becomes the node's value and the left becomes the next operator and right
 * becomes the value of buffer. then we just make a stack of those nodes.
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

    while (i < token_count) {       // loop through the tokens
        switch (tokens[i].type) {
            case TOKEN_NUMBER:      // if the token is a number or a symbol,
            case TOKEN_SYMBOL:      // create a node and push to node stack
                node = create_node(expr->node_pool, &tokens[i], NULL, NULL);
                stack_push(&n, node);
                break;              // and break out of the switch statement so the next token can be evaluated

            case TOKEN_UNARY:

                i++;                            // move to the operand

                if (i >= token_count) {         // if it is a floating unary
                    break;                      // break out of case structure
                }                               // if errors were being handled, maybe return an error

                ast_node_t* operand = NULL;     // otherwise, try to collect the operand
                switch (tokens[i].type) {
                    case TOKEN_SYMBOL:          // if the operand is a symbol, number, or left parenthesis,
                    case TOKEN_NUMBER:          //
                    case TOKEN_LPAREN:
                        operand = create_node(expr->node_pool, &tokens[i], NULL, NULL);
                        break;

                    case TOKEN_UNARY: {
                        i--;
                        continue;
                    }

                    default:
                        // invalid
                        break;
                }

                node = create_node(expr->node_pool, &tokens[i - 1], operand, NULL);
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

    free_subtree(token_pool, node_pool, node->left);    // recurse to the bottom left of the subtree
    free_subtree(token_pool, node_pool, node->right);   // recurse to the bottom right of the subtree

    if (node->token) {                                  // if the current node's token is not NULL
        token_pool_free_token(token_pool, node->token); // free the token
    }
    node_pool_free_node(node_pool, node);               // then free the node
}

ast_node_t* sub_to_add(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!node) {
        return NULL;
    }

    // if the node is a subtraction operator
    if (node->token->type == TOKEN_OPERATOR && node->token->operator->type == SUB) {
        // free the node's subtraction operator and replace it with addition
        token_pool_free_token(token_pool, node->token);
        node->token = op_token_init(token_pool, '+');

        // if node's right child's token is a number, negate its value and overwrite the old token
        if (node->right->token->type == TOKEN_NUMBER) {
            const double val = node->right->token->number.value;
            token_pool_free_token(token_pool, node->right->token);
            node->right->token = num_token_init_double(token_pool, -val);
        }

        // otherwise, negate the right child with a unary minus
        else {
            ast_node_t* neg = node_pool_alloc(node_pool);
            neg->token = unary_token_init(token_pool, UNARY_NEG);
            neg->left = node->right;
            neg->right = NULL;

            node->right = neg;
        }
    }

    // recurse through the tree top down
    if (node->left) {
        node->left = sub_to_add(node->left, token_pool, node_pool);
    }

    if (node->right) {
        node->right = sub_to_add(node->right, token_pool, node_pool);
    }

    return node;
}

// a/b = a*(b^(-1)) or a/(b^c) = a*(b^(-c))
ast_node_t* div_to_mul(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!node) {
        return NULL;
    }

    if (node->left) {
        node->left = div_to_mul(node->left, token_pool, node_pool);
    }

    if (node->right) {
        node->right = div_to_mul(node->right, token_pool, node_pool);
    }

    if (node->token->type == TOKEN_UNARY) {
        if (node->left) {
            node->left = div_to_mul(node->left, token_pool, node_pool);
        }
        return node;
    }

    if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == DIV)) {
        return node;
    }

    ast_node_t* numerator = node->left;
    ast_node_t* denominator = node->right;

    // break the link from the div node
    node->left = NULL;
    node->right = NULL;

    ast_node_t* reciprocal = NULL;

    // if the denominator is a power operator
    if (denominator->token->type == TOKEN_OPERATOR && denominator->token->operator->type == POW) {
        reciprocal = denominator;
        ast_node_t* exponent = reciprocal->right;          // collect the exponent

        reciprocal->right = NULL;                          // break the link between power operator and its

        if (exponent->token->type == TOKEN_NUMBER) {        // if the exponent is a number
            const double val = exponent->token->number.value;   // collect it's value
            reciprocal->right = node_pool_alloc(node_pool);
            reciprocal->right->token = num_token_init_double(token_pool, -val); // negate the value
            reciprocal->right->left = NULL;
            reciprocal->right->right = NULL;
        } else {
            reciprocal->right = node_pool_alloc(node_pool);
            reciprocal->right->token = unary_token_init(token_pool, UNARY_NEG);
            reciprocal->right->right = exponent;
        }

    } else {                                                                // if the denominator is first order
        reciprocal = node_pool_alloc(node_pool);                            // allocate a new node for the reciprocal
        reciprocal->token = op_token_init(token_pool, '^');                 // reciprocal becomes a power operator
        reciprocal->left = denominator;                                     // set the base to the denominator

        reciprocal->right = node_pool_alloc(node_pool);                     // allocate another node to hold
        reciprocal->right->token = num_token_init_double(token_pool, -1);   // an exponent with a value of -1
        reciprocal->right->left = NULL;
        reciprocal->right->right = NULL;
    }

    ast_node_t* new_root = node_pool_alloc(node_pool);
    new_root->token = op_token_init(token_pool, '*');
    new_root->left = numerator;
    new_root->right = reciprocal;

    return new_root;
}

ast_node_t* neg_to_mul(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!node) {
        return NULL;
    }

    if (node->left) {
        node->left = neg_to_mul(node->left, token_pool, node_pool);
    }
    if (node->right) {
        node->right = neg_to_mul(node->right, token_pool, node_pool);
    }

    if (node->token->type == TOKEN_UNARY && node->token->unary.op == UNARY_NEG) {
        ast_node_t* operand = node->left;

        token_pool_free_token(token_pool, node->token);   // free the unary token

        // reuse the current node to become the '*' node
        node->token = op_token_init(token_pool, '*');

        // allocate new -1 node
        ast_node_t* neg_one = node_pool_alloc(node_pool);
        neg_one->token = num_token_init_double(token_pool, -1);
        neg_one->left = NULL;
        neg_one->right = NULL;

        node->left = neg_one;
        node->right = operand;

        return node;
    }
    return node;
}

ast_node_t* identity_property_add(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    // if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == ADD )) {
    //     return node;
    // }
    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {
        ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node);
        return left;
    }
    if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {
        ast_node_t* right = copy_subtree(token_pool, node_pool, node->right);
        free_subtree(token_pool, node_pool, node);
        return right;
    }
    return node;
}

// ast_node_t* identity_property_sub(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
//     // if (!(node && node->token->type == TOKEN_OPERATOR && node->token->operator->type == SUB)) {
//     //     return node;
//     // }
//     if (!node->left || !node->right) {
//         return node;
//     }
//     // x-x = 0
//     if (branch_is_equal(node->left, node->right)) {
//         ast_node_t* zero_node = node_pool_alloc(node_pool);
//         zero_node->token = num_token_init(token_pool, "0");
//         zero_node->left = NULL;
//         zero_node->right = NULL;
//
//         // free original tree and return 0
//         free_subtree(token_pool, node_pool, node);
//         return zero_node;
//     }
//
//     return node;
// }

ast_node_t* identity_property_mul(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    // if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL)) {
    //     return node;
    // }
    if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 1) {
        ast_node_t* right = copy_subtree(token_pool, node_pool, node->right);
        free_subtree(token_pool, node_pool, node);
        return right;
    }
    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 1) {
        ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node);
        return left;
    }
    return node;
}

// ast_node_t* identity_property_div(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
//     // if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == DIV)) {
//     //     return node;
//     // }
//     // x/1 = x
//     if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 1) {
//         ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
//         free_subtree(token_pool, node_pool, node->left);
//         return left;
//     }
//     return node;
// }

ast_node_t* identity_property_pow(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == POW)) {
        return node;
    }
    // x^1 = x
    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 1) {
        ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node->left);
        return left;
    }
    return node;
}

// ast_node_t* zero_property_sub(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
//     if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == SUB)) {
//         return node;
//     }
//     // x-0 = x
//     if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {
//         ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
//         free_subtree(token_pool, node_pool, node);
//         return left;
//     }
//     // 0-x = -x
//     // TODO: cannot handle unary operators yet
//     // if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {
//     //     ast_node_t* right = copy_subtree(token_pool, node_pool, node->right);
//     //     free_subtree(token_pool, node_pool, node);
//     //     return right;
//     // }
//     return node;
// }

ast_node_t* zero_property_mul(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL)) {
        return node;
    }

    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {    // on the right
        ast_node_t* right = copy_subtree(token_pool, node_pool, node->right);
        free_subtree(token_pool, node_pool, node);
        return right;                                                                    // return left node
    }
    if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {      // for both children of add
        ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node);
        return left;
    }
    return node;
}

// ast_node_t* zero_property_div(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
//     if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type != DIV)) {
//         return node;
//     }
//     // 0/x = 0
//     if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {
//         ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
//         free_subtree(token_pool, node_pool, node);
//         return left;
//     }
//     return node;
// }

ast_node_t* zero_property_pow(token_pool_t* token_pool, node_pool_t* node_pool, ast_node_t* node) {
    if (!(node->token->type == TOKEN_OPERATOR && node->token->operator->type == POW)) {
        return node;
    }

    if (node->left && node->left->token->type == TOKEN_NUMBER && node->left->token->number.value == 0) {    // on the right
        ast_node_t* left = copy_subtree(token_pool, node_pool, node->left);
        free_subtree(token_pool, node_pool, node);
        return left;                                                                    // return left node
    }

    if (node->right && node->right->token->type == TOKEN_NUMBER && node->right->token->number.value == 0) {      // for both children of add
        ast_node_t* one_node = node_pool_alloc(node_pool);
        one_node->token = num_token_init(token_pool, "1");
        one_node->left = NULL;
        one_node->right = NULL;
        free_subtree(token_pool, node_pool, node);
        return one_node;
    }
    return node;
}



bool is_operator_node(const ast_node_t* node, const operator_type_t type) {
    return node && node->token->type == TOKEN_OPERATOR && node->token->operator->type == type;
}

void collect_terms(const ast_node_t* node, stack_t* stack, const operator_type_t type) {
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

#define MAX_TERMS 16 // we can store up to 16 terms
#define MAX_TERM_LEN 16 // each term can store up to 16 nodes, that's 256 total nodes that can be stored

typedef struct {
    ast_node_t* root;
    // bool term_is_negative;
} term_t;

typedef struct {
    term_t terms[MAX_TERMS];
    int term_count;
} terms_t;

static void collect_terms2(ast_node_t* node, terms_t* terms) {
    if (!node) {
        return;
    }

    if (node->token->type == TOKEN_OPERATOR) {
        if (node->token->operator->type == ADD) {
            collect_terms2(node->left, terms);
            collect_terms2(node->right, terms);
            return;
        }

        // if (node->token->operator->type == SUB) {
        //     collect_terms2(node->left, terms, negate);
        //     collect_terms2(node->right, terms, !negate);
        //     return;
        // }
    }

    if (terms->term_count < MAX_TERMS) {
        term_t* term = &terms->terms[terms->term_count++];
        term->root = node;
    }
}

ast_node_t* rebuild_dist_mul_tree(const terms_t* terms, const ast_node_t* multiplier,
                                  token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!(terms && multiplier)) {
        return NULL;
    }

    ast_node_t* new_tree = NULL;

    for (int i = 0; i < terms->term_count; i++) {
        ast_node_t* mul_node = node_pool_alloc(node_pool);
        mul_node->token = op_token_init(token_pool, '*');
        mul_node->left = copy_subtree(token_pool, node_pool, terms->terms[i].root);
        mul_node->right = copy_subtree(token_pool, node_pool, multiplier);

        if (!new_tree) {
            new_tree = mul_node;
        } else {
            ast_node_t* op_node = node_pool_alloc(node_pool);
            op_node->token = op_token_init(token_pool, '+');
            op_node->left = new_tree;
            op_node->right = mul_node;
            new_tree = op_node;
        }
    }
    return new_tree;
}
// TODO: both sides can be sums, my function only handles one or the other
ast_node_t* distributive_property(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!(node && node->token->type == TOKEN_OPERATOR && node->token->operator->type == MUL)) {
        return node;
    }
    // what we want to do

    // check if the operator is * and one or both children are sums
    // if so, flatten the expression
    // collect all terms (split on +/-, so a term could be something like '2*x^2')
    // flatten anything that can be flattened,
    // rebuild the tree, appending each term's root to a mul node and the multiplier

    const bool left_is_sum = node->left &&
         node->left->token->type == TOKEN_OPERATOR &&
         node->left->token->operator->type == ADD;

    const bool right_is_sum = node->right &&
         node->right->token->type == TOKEN_OPERATOR &&
         node->right->token->operator->type == ADD;

    if (!left_is_sum && !right_is_sum) {
        return node;
    }

    terms_t terms = {0};
    ast_node_t* multiplicand;
    ast_node_t* multiplier;

    if (left_is_sum) {
        multiplicand = node->left;
        multiplier = node->right;
    } else {
        multiplicand = node->right;
        multiplier = node->left;
    }

    collect_terms2(multiplicand, &terms);
    ast_node_t* new_tree = rebuild_dist_mul_tree(&terms, multiplier, token_pool, node_pool);
    free_subtree(token_pool, node_pool, node);
    return new_tree;
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
    ast_node_t* base;           // the base subtree (e.g. 'x', '2', '(a+b)')
    double exponent_sum;        // sum of numeric exponents
    stack_t symbolic_exponents; // stack of symbolic exponent nodes (non-numeric)
    bool exponent_is_one;       // flag meaning we need to append ^1
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
static ast_node_t* reconstruct_product(base_group_list_t* groups, token_pool_t* token_pool, node_pool_t* node_pool) {
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

        // If exponent is NULL (meaning sum=0 and no symbolic exponents),
        // or if exponent is 1, skip pow node, set flag
        const bool exp_one_flag = !exp_chain ||
                                  (exp_chain->token->type == TOKEN_NUMBER && exp_chain->token->number.value == 1.0);

        ast_node_t* term = NULL;
        if (exp_one_flag) {
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
ast_node_t* simplify(ast_node_t* node, token_pool_t* token_pool, node_pool_t* node_pool) {
    if (!node) return NULL;

    // recurse to the bottom of the tree
    node->left = simplify(node->left, token_pool, node_pool);
    node->right = simplify(node->right, token_pool, node_pool);

    // if the current node is an operator
    if (node->token->type == TOKEN_OPERATOR) {
        // apply simplification algorithms based on operator type
        switch (node->token->operator->type) {
            case ADD:
                node = identity_property_add(node, token_pool, node_pool);
                node = commutative_property_add(token_pool, node_pool, node);
                break;

            case MUL:
                node = identity_property_mul(node, token_pool, node_pool);
                node = zero_property_mul(token_pool, node_pool, node);
                node = distributive_property(node, token_pool, node_pool);
                node = product_of_powers(token_pool, node_pool, node);
                break;

            case POW:
                node = identity_property_pow(node, token_pool, node_pool);
                node = zero_property_pow(token_pool, node_pool, node);
                node = power_of_powers(token_pool, node_pool, node);
                break;

            default:
                break;
        }

        // now try to fold constants
        if (node &&
            node->token->type == TOKEN_OPERATOR &&
            node->left &&
            node->right &&
            node->left->token->type == TOKEN_NUMBER &&
            node->right->token->type == TOKEN_NUMBER) {
            node = solve(token_pool, node_pool, node);
        }
    }
    return node;
}