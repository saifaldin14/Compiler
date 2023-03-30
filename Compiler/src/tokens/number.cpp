/*
 File: number.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/


#include "../../include/tokens/number.hpp"

/*
 Description:
    Perform a mathematical operation between 2 doubles.
 
 Parameters:
    - op: The operation to perform (string)
    - d1: The first number (double)
    - d2: The second number (double)
 
 Returns:
    - The result of the operation (double)
*/
double Number::doubleOperation(string op, double d1, double d2) {
    double result = 0;
    if (strcmp(op.c_str(), "+") == 0) result = d1 + d2;
    if (strcmp(op.c_str(), "-") == 0) result = d1 - d2;
    if (strcmp(op.c_str(), "* ") == 0) result = d1 * d2;
    if (strcmp(op.c_str(), "/") == 0) result = d1 / d2;
    if (strcmp(op.c_str(), "%") == 0) result = fmod(d1, d2);
    return result;
}

/*
 Description:
    Perform a mathematical operation between 2 ints.
 
 Parameters:
    - op: The operation to perform (string)
    - d1: The first number (int)
    - d2: The second number (int)
 
 Returns:
    - The result of the operation (int)
*/
int Number::intOperation(string op, int d1, int d2) {
    int result = 0;
    if (strcmp(op.c_str(), "+") == 0) result = d1 + d2;
    if (strcmp(op.c_str(), "-") == 0) result = d1 - d2;
    if (strcmp(op.c_str(), "* ") == 0) result = d1 * d2;
    if (strcmp(op.c_str(), "/") == 0) result = d1 / d2;
    if (strcmp(op.c_str(), "%") == 0) result = d1 % d2;
    return result;
}

/*
 Description:
    Perform a comparison operation between 2 doubles.
 
 Parameters:
    - op: The comparison to perform (string)
    - d1: The first number (double)
    - d2: The second number (double)
 
 Returns:
    - The result of the operation (bool)
*/
bool Number::doubleComparison(string op, double d1, double d2) {
    int result = 0;
    if (strcmp(op.c_str(), ">") == 0) result = d1 > d2;
    if (strcmp(op.c_str(), "<") == 0) result = d1 < d2;
    if (strcmp(op.c_str(), ">=") == 0) result = d1 >= d2;
    if (strcmp(op.c_str(), "<=") == 0) result = d1 <= d2;
    if (strcmp(op.c_str(), "==") == 0) result = d1 == d2;
    if (strcmp(op.c_str(), "<>") == 0) result = d1 != d2;
    return result;
}

/*
 Description:
    Perform a comparison operation between 2 ints.
 
 Parameters:
    - op: The comparison to perform (string)
    - d1: The first number (int)
    - d2: The second number (int)
 
 Returns:
    - The result of the operation (bool)
*/
bool Number::intComparison(string op, int d1, int d2) {
    int result = 0;
    if (strcmp(op.c_str(), ">") == 0) result = d1 > d2;
    if (strcmp(op.c_str(), "<") == 0) result = d1 < d2;
    if (strcmp(op.c_str(), ">=") == 0) result = d1 >= d2;
    if (strcmp(op.c_str(), "<=") == 0) result = d1 <= d2;
    if (strcmp(op.c_str(), "==") == 0) result = d1 == d2;
    if (strcmp(op.c_str(), "<>") == 0) result = d1 != d2;
    return result;
}
