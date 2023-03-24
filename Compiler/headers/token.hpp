/*
 File: token.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-24
*/

#ifndef TOKEN_H
#define TOKEN_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "tokenType.hpp"

using namespace std;

class Token
{
public:
    Token() {
        type = TokenType("id");
        name = "";
    }
    
    Token(string n, TokenType t) {
        type = t;
        name = n;
    }
    
    Token(char n, TokenType t) {
        std::string s;
        std::stringstream ss;
        ss << n;
        ss >> s;
        name = s;
        type = t;
    }
    
    virtual string toString() {
        return ("< " + name + " >");
    }
    
    TokenType getType() {
        return type;
    }
    
    string getRepresentation() {
        return name;
    }
    
private:
    TokenType type = TokenType("id");
    string name = "";
};

#endif /* token_hpp */

