//
//  saveToken.hpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-14.
//
#ifndef SAVE_TOKEN_H
#define SAVE_TOKEN_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "../tokens/token.hpp"

using namespace std;

class SaveToken
{
public:
    SaveToken(vector<Token> inputValue, int inputPosition);
    void setValue(vector<Token> inputValue);
    void setPosition(int inputPosition);
    
    vector<Token> getValues() { return values; }
    int getPosition() { return position; }
    
private:
    vector<Token> values;
    int position;
};

#endif /* saveToken_hpp */
