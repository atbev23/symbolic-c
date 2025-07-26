//
// Created by atbev on 7/6/2025.
//

#ifndef NUMBER_T_H
#define NUMBER_T_H

#define MAX_SIG_FIGS 15             // store numbers between 999,999,999,999,999,999 and 0.00000000000001

typedef struct {
    char disp[MAX_SIG_FIGS + 1];    // Holds up to MAX_SIG_FIGS characters + null terminator
    double value;                   // store the value as a double
} number_t;

number_t num_init_double(double);   // read in a double and return a number token
number_t num_init_str(const char*); // read in a string of digits and return a number token

#endif //NUMBER_T_H