/*
 File: tokenType.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-24
*/

#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

class TokenType
{
public:
    string type;
    TokenType(string text);
    string toString();
    static TokenType RESERVED;
    static TokenType ID;
    static TokenType INT;
    static TokenType DOUBLE;
    static TokenType COMP;
    static TokenType TERM;
    static TokenType END;
    static TokenType ERR;
};

#endif /* lexer_hpp */

