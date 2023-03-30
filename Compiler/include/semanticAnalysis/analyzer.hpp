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

private:
    void handleScopes(Token token, vector<Token> line);
    void handleFunctionScope(Token token, vector<Token> line);
    string determineType(Token token);
    bool isVariableType(Token token);
    
    vector<vector<Token>> lines;
    vector<string> scopes; // Stack to keep track of the current scope we are in
    vector<string> returnTypes; // Stack to keep track of the return types of functions (if we want to have nested functions)
    unordered_map<string, string> functionDefinition; // Used to define recursive method return types
    unordered_map<string, string> variableDefinition;
};
#endif /* analyzer_hpp */
