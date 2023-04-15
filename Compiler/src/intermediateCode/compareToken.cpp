/*
 File: compareToken.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-14
*/

#include "../../include/intermediateCode/compareToken.hpp"

CompareToken::CompareToken(Token inputToken, int inputPosition) {
    token = inputToken;
    position = inputPosition;
}

void CompareToken::setToken(Token inputToken) {
    token = inputToken;
}

void CompareToken::setPosition(int inputPosition) {
    position = inputPosition;
}
