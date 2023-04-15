/*
 File: saveToken.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-14
*/

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
