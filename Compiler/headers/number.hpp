/*
 File: number.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/

#ifndef NUMBER_H
#define NUMBER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "token.hpp"

using namespace std;

class Number
{
public:
    static double doubleOperation(string op, double d1, double d2);
    static int intOperation(string op, int d1, int d2);
    static constexpr const auto operation = [](string op, auto x, auto y) {
        if (strcmp(typeid(x).name(), "int") == 0 && strcmp(typeid(y).name(), "int") == 0) {
            return intOperation(op, (int)x, (int)y);
        } else if (strcmp(typeid(x).name(), "int") == 0 && strcmp(typeid(y).name(), "double") == 0) {
            return doubleOperation(op, (int)x, (double)y);
        } else if (strcmp(typeid(x).name(), "double") == 0 && strcmp(typeid(y).name(), "int") == 0) {
            return doubleOperation(op, (double)x, (int)y);
        } else if (strcmp(typeid(x).name(), "double") == 0 && strcmp(typeid(y).name(), "double") == 0) {
            return doubleOperation(op, (double)x, (double)y);
        }
    };
    
    static bool doubleComparison(string op, double d1, double d2);
    static bool intComparison(string op, int d1, int d2);
    static constexpr const auto comparison = [](string op, auto x, auto y) {
        if (strcmp(typeid(x).name(), "int") == 0 && strcmp(typeid(y).name(), "int") == 0) {
            return intComparison(op, (int)x, (int)y);
        } else if (strcmp(typeid(x).name(), "int") == 0 && strcmp(typeid(y).name(), "double") == 0) {
            return doubleComparison(op, (int)x, (double)y);
        } else if (strcmp(typeid(x).name(), "double") == 0 && strcmp(typeid(y).name(), "int") == 0) {
            return doubleComparison(op, (double)x, (int)y);
        } else if (strcmp(typeid(x).name(), "double") == 0 && strcmp(typeid(y).name(), "double") == 0) {
            return doubleComparison(op, (double)x, (double)y);
        }
    };
};


#endif /* number_hpp */
