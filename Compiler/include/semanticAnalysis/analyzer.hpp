/*
 File: analyzer.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-30
*/

#ifndef ANALYZER_H
#define ANALYZER_H

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
#include "scopeVariable.hpp"

using namespace std;

class Analyzer
{
public:
    Analyzer(vector<vector<Token>> inputLines);
    void analyzeSemantics();
    void printVariables();
    bool getCorrect() { return isCorrect; }
    
private:
    inline static const string FUNCTION = "FUNCTION";
    inline static const string IF = "IF";
    inline static const string ELSE = "ELSE";
    inline static const string WHILE = "WHILE";

    // Helper functions
    void handleScopes(Token token, vector<Token> line);
    void handleFunctionScope(Token token, vector<Token> line);
    void handleVariableDeclaration(Token token, vector<Token> line);
    void handleOperations(Token token, vector<Token> line);
    void handleCondition(Token token, vector<Token> line);
    void handlePrint(Token token, vector<Token> line);
    Token handleArithmetic(vector<Token> line, vector<int> arthimeticRange);
    bool checkValidReturnType(Token token, vector<Token> line);
    bool checkValidAssignment(vector<Token> line, int tokenNumber, vector<int> arthimeticRange);
    bool checkValidCondition(vector<Token> line, int openParen, int closeParen);
    bool checkVariableExists(Token token);
    string getTypeFromToken(Token token);
    
    // Utility functions
    void setVariableInScope(ScopeVariable variable);
    void saveErrorText(string text);
    void printValue(string text, int incr);
    string determineType(Token token);
    bool isVariableType(Token token);
    
    vector<vector<Token>> lines;
    vector<string> scopes; // Stack to keep track of the current scope we are in
    vector<string> returnTypes; // Stack to keep track of the return types of functions (if we want to have nested functions)
    unordered_map<string, string> functionDefinition; // Used to define recursive method return types
    unordered_map<string, vector<string>> variableDefinition;
    
    bool validReturn = true;
    bool isCorrect = true;
    string printString;
    int scopeCounter = 0;
};
#endif /* analyzer_hpp */
