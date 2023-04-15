/*
 File: compareToken.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-14
*/

#ifndef COMPARE_TOKEN_H
#define COMPARE_TOKEN_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "../tokens/token.hpp"

using namespace std;

class CompareToken
{
public:
    CompareToken(Token inputToken, int inputPosition);
    void setToken(Token inputToken);
    void setPosition(int inputPosition);
    
    Token getToken() { return token; }
    int getPosition() { return position; }
    
private:
    Token token;
    int position;
};

#endif /* compareToken_hpp */
