/*
 File: id.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-24
*/

#ifndef ID_H
#define ID_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "token.hpp"

using namespace std;

class Id : public Token
{
public:
    int offset;
    Id(string n, TokenType t, int o) : Token(n, t), offset(o) {};
    virtual string toString() override {
        return "< " + getType().toString() + ", " + to_string(offset) + " >";
    }
};

#endif /* id_hpp */
