//
//  compareToken.cpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-14.
//

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
