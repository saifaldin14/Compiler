/*
 File: tokenType.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-24
*/

#include "../../include/tokens/tokenType.hpp"

/*
 Description:
    Constructor to define a token's type
 
 Parameters:
    - text: The token's type (string)
*/
TokenType::TokenType(string text) {
    type = text;
}

/*
 Description:
    Gets the type of the token and returns it as a human-readable string
 
 Parameters:
    - None

 Returns:
    - The token's type (string)
*/
string TokenType::toString() {
    return type;
}

// Define Token Types
TokenType TokenType::RESERVED = TokenType("reserved");
TokenType TokenType::ID = TokenType("id");
TokenType TokenType::INT = TokenType("integer");
TokenType TokenType::DOUBLE = TokenType("double");
TokenType TokenType::COMP = TokenType("comparison");
TokenType TokenType::TERM = TokenType("terminal");
TokenType TokenType::END = TokenType("end");
TokenType TokenType::ERR = TokenType("error");

