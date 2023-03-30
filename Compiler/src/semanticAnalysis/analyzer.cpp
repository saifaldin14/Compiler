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
            handleScopes(token, line);
        }
    }
}

string Analyzer::determineType(Token token) {
    if (token.getRepresentation() == "int") return "integer";
    return token.getRepresentation();
}

bool Analyzer::isVariableType(Token token) {
    if (token.getRepresentation() == "int" or token.getRepresentation() == "double")
        return true;
    return false;
}

void Analyzer::handleScopes(Token token, vector<Token> line) {
    // Handle functions
    if (token.getRepresentation() == "def") {
        handleFunctionScope(token, line);
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

void Analyzer::handleFunctionScope(Token token, vector<Token> line) {
    // Line structure is:
    // <def> <returnType> <name> <(> <arguments> <)>
    
    scopes.push_back("FUNCTION");
    
    // Since the code is parsed correctly we know after def there will be a return type argument
    string functionType = determineType(line[1]);
    returnTypes.push_back(functionType); // Add the return type of the function to the returnType stack
    functionDefinition[line[2].getRepresentation()] = functionType;
    ScopeVariable scopeVariable;
    
    for (int i = 4; i < line.size() - 1; i++) {
        if (line[i].getType().toString() == "id") {
            if (isVariableType(line[i]))
                scopeVariable.setType(determineType(line[i]));
            else
                scopeVariable.setVarName(line[i].getRepresentation());
        }
            
        if (line[i].getRepresentation() == ",") {
            
        }
    }
}
