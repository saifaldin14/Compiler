/*
 File: compareToken.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-14
*/

#include "../../include/intermediateCode/compareToken.hpp"

/*
 Description:
    Construtor for the Compare Token
 
 Parameters:
    - inputToken: The default token (Token)
    - inputPosition: The default position of the token relative to others (int)
*/
CompareToken::CompareToken(Token inputToken, int inputPosition) {
    token = inputToken;
    position = inputPosition;
}

/*
 Description:
    Sets the token value
 
 Parameters:
    - inputToken: The token to set (Token)
 
 Returns:
    - None
*/
void CompareToken::setToken(Token inputToken) {
    token = inputToken;
}

/*
 Description:
    Sets the token position
 
 Parameters:
    - inputPosition: The position of the token relative to others to set (int)
 
 Returns:
    - None
*/
void CompareToken::setPosition(int inputPosition) {
    position = inputPosition;
}
