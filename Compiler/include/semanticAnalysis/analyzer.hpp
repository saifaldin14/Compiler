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
#include "../lexicalAnalysis/lexer.hpp"
#include "syntaxTree.hpp"
#include "../tokens/token.hpp"

using namespace std;

class Analyzer
{
public:
    Analyzer(vector<vector<Token>> inputLines);
    void analyzeSemantics();

private:
    void handleScopes(Token token);
    
    vector<vector<Token>> lines;
    vector<string> scopes; // Stack to keep track of the current scope we are in
};
#endif /* analyzer_hpp */
