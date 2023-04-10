//
//  threeAddressCode.hpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-10.
//

#ifndef THREE_ADDRESS_CODE_H
#define THREE_ADDRESS_CODE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <execinfo.h>
#include <unordered_map>
#include "../tokens/token.hpp"

using namespace std;

class ThreeAddressCode
{
public:
    ThreeAddressCode(vector<vector<Token>> inputLines);
    
private:
    // Used for the Symbol Table (same as Semantic Analysis)
    inline static const string FUNCTION = "FUNCTION";
    inline static const string IF = "IF";
    inline static const string ELSE = "ELSE";
    inline static const string WHILE = "WHILE";
    
    // Reserved key words for IC
    inline static const string BRANCH = "B";
    inline static const string BRANCH_LINK = "BL";
    inline static const string BEGIN = "Begin";
    inline static const string PUSH = "push";
    inline static const string POP = "pop";
    inline static const string LR = "LR";
    inline static const string FP = "FP";
    inline static const string PRINT = "print";
    
    // Comparator words
    inline static const string COMPARE = "cmp";
    inline static const string GREATER_THAN = "bgt";
    inline static const string GREATER_THAN_EQUAL = "bge";
    inline static const string LESS_THAN = "blt";
    inline static const string LESS_THAN_EQUAL = "ble";
    inline static const string EQUAL = "beq";

    vector<vector<Token>> lines; // Keep track of the code
    vector<string> scopes; // Stack to keep track of the current scope we are in
    vector<string> labelNames = {"main"};
    string printString; // Used to print the symbolTable
    string threeAddressCodeText; // Used to create new Three Address Code
    int fp; // Frame Pointer
    int numberOfBytes;
    
    string label = "label";
    string temporaryVariable = "t";
};
#endif /* threeAddressCode_hpp */
