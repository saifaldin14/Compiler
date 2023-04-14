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
#include "../semanticAnalysis/scopeVariable.hpp"
#include "./saveToken.hpp"
#include "./compareToken.hpp"

using namespace std;

class ThreeAddressCode
{
public:
    ThreeAddressCode(vector<vector<Token>> inputLines);
    void createCode();
    void printThreeAddressCode();
    
private:
    // Used for the Symbol Table (same as Semantic Analysis)
    inline static const string GLOBAL = "GLOBAL";
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
    inline static const string NEW_LABEL = "b";
    
    // Define methods
    void handleCodeLine(vector<Token> line);
    void handleFunctionCode(vector<Token> line);
    void handleIfCode(vector<Token> line);
    void handleReturnCode(vector<Token> line);
    void handleVariableDeclerationCode(vector<Token> line);
    void handleOperationCode(vector<Token> line);
    void handlePrintCode(vector<Token> line);
    string handleInplaceDeclerationCode(vector<Token> line, string variableType, string generatedCode="");
    string handleFunctionCallCode(vector<Token> line);
    
    void addCode(string generatedCode);
    string simplifyMultiplicationOperation(vector<Token> exp, string variableType);

    bool isVariableType(Token token);
    vector<vector<Token>> breakUpLines(vector<vector<Token>> lines);
    void printValue(string text, int incr);
    string determineType(Token token);
    
    vector<vector<Token>> lines; // Keep track of the code
    vector<string> scopes; // Stack to keep track of the current scope we are in
    vector<string> labelNames = {"main"};
    vector<string> operationLabels = {}; // Labels for if and while statements within a function
    unordered_map<string, string> variableTypes; // Store the type of each variable
    unordered_map<string, string> variableNames; // Store the name and scope of the variables
    unordered_map<string, string> functionNames; // Store the name and scope of the variables

    string printString; // Used to print the symbolTable
    string threeAddressCodeText; // Used to create new Three Address Code
    string functionText; // Save the code of the function
    string globalText; // Save the code of the function
    string operationText; // Save the code of the function
    string returnLabel;
    string functionName;
    string functionType;
    vector<string> generatedOperationCode; // Save the code of an operation block
    string compareOperation; // Save the current comparison operation so we don't write it twice
    int fp; // Frame Pointer
    int sp = 0; // Stack Pointer
    int numberOfBytes = 0;
    int labelCounter = 1;
    int temporaryVariableCounter = 1;
    int scopeCounter;
    
    string label = "lab";
    string temporaryVariable = "t";
};
#endif /* threeAddressCode_hpp */
