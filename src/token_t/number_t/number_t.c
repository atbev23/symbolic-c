//
// Created by atbev on 7/8/2025.
//

#include "number_t.h"
#include <stdio.h>
#include <stdlib.h>

// this function takes in a double and returns an equivalent number token
inline number_t num_init_double(const double num_d) {
    number_t number;                                                    // create number token
    snprintf(number.disp, sizeof(number.disp), "%g", num_d);    // convert double to string and copy to number.disp
    number.value = num_d;                                               // convert the string to a double and store it
    return number;                                                      // return the created number token
}

// this function takes in a string and returns an equivalent number token
inline number_t num_init_str(const char* num_str) {
    number_t number;                                                    // create number token
    snprintf(number.disp, sizeof(number.disp), "%s", num_str);  // set the input string to number.disp
    number.value = strtod(num_str, NULL);                               // convert the string to a double and store it
    return number;                                                      // return the created number token
}