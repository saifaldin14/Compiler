/*
 File: saveToken.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-14
*/

#include "../../include/intermediateCode/saveToken.hpp"

/*
 Description:
    Construtor for the Save Token
 
 Parameters:
    - inputValue: The default values (vector<Token>)
    - inputPosition: The default position of the token relative to others (int)
*/
SaveToken::SaveToken(vector<Token> inputValue, int inputPosition) {
    values = inputValue;
    position = inputPosition;
}

/*
 Description:
    Sets the token value
 
 Parameters:
    - inputValue: The values to set (vector<Token>)
 
 Returns:
    - None
*/
void SaveToken::setValue(vector<Token> inputValue) {
    values = inputValue;
}

/*
 Description:
    Sets the token position
 
 Parameters:
    - inputPosition: The position of the token relative to others to set (int)
 
 Returns:
    - None
*/
void SaveToken::setPosition(int inputPosition) {
    position = inputPosition;
}
