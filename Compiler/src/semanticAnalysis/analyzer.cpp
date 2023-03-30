/*
 File: analyzer.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-30
*/

#include "../../include/semanticAnalysis/analyzer.hpp"

Analyzer::Analyzer(vector<vector<Token>> inputLines) {
    lines = inputLines;
    scopes.push_back("GLOBAL");
}

void Analyzer::analyzeSemantics() {
    for (auto line : lines) {
        for (Token token : line) {
            handleScopes(token);
        }
    }
}

void Analyzer::handleScopes(Token token) {
    // Handle functions
    if (token.getRepresentation() == "def") {
        scopes.push_back("FUNCTION");
    }
    // Handle if blocks
    else if (token.getRepresentation() == "if") {
        scopes.push_back("IF");
    }
    // Handle else block
    else if (token.getRepresentation() == "else") {
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (token.getRepresentation() == "while") {
        scopes.push_back("WHILE");
    }
}
