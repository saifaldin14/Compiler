//
//  saveToken.cpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-14.
//

#include "../../include/intermediateCode/saveToken.hpp"

SaveToken::SaveToken(vector<Token> inputValue, int inputPosition) {
    values = inputValue;
    position = inputPosition;
}

void SaveToken::setValue(vector<Token> inputValue) {
    values = inputValue;
}

void SaveToken::setPosition(int inputPosition) {
    position = inputPosition;
}
