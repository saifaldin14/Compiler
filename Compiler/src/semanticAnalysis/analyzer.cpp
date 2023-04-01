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
        handleScopes(line[0], line);
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
    string tokenValue = token.getRepresentation();
    // Handle functions
    if (tokenValue == "def") {
        scopes.push_back(FUNCTION);
        handleFunctionScope(token, line);
    }
    // Handle if blocks
    else if (tokenValue == "if") {
        scopes.push_back("IF");
    }
    // Handle else block
    else if (tokenValue == "else") {
        scopes.pop_back(); // Remove the existing IF scope
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        scopes.push_back("WHILE");
    }
    // Handle closing scopes
    else if (tokenValue == "od" or tokenValue == "fed" or tokenValue == "fi") {
        scopes.pop_back();
    }
    // Handle Return statements
    else if (tokenValue == "return") {
        if(find(scopes.begin(), scopes.end(), FUNCTION) != scopes.end()) {
            // We are currently in a function
            checkValidReturnType(token, line);
        } else {
            // We are not in a function
            cout << "Invalid! Return keyword added outside of function" << endl;
        }
    }
    // Handle variable decleration
    else if (isVariableType(token)) {
        handleVariableDeclaration(token, line);
    }
}

void Analyzer::handleFunctionScope(Token token, vector<Token> line) {
    // Line structure is:
    // <def> <returnType> <name> <(> <arguments> <)>
    
    // Since the code is parsed correctly we know after def there will be a return type argument
    string functionType = determineType(line[1]), functionName = line[2].getRepresentation();
    returnTypes.push_back(functionType); // Add the return type of the function to the returnType stack
    functionDefinition[functionName] = functionType;
    variableDefinition[functionName] = { functionName, scopes.back(), functionType };
    ScopeVariable variable;
    variable.setScope(FUNCTION);
    
    // Save input parameters as scope variables
    for (int i = 4; i < line.size() - 1; i++) {
        if (line[i].getType().toString() == "id") {
            if (isVariableType(line[i]))
                variable.setType(determineType(line[i]));
            else {
                variable.setVarName(line[i].getRepresentation());
                setVariableInScope(variable);
            }
        } else if (line[i].getRepresentation() == ",") {
            variable.setEmptyValues();
        }
    }
}

void Analyzer::handleVariableDeclaration(Token token, vector<Token> line) {
    ScopeVariable variable;
    for (int i = 1; i < line.size(); i++) {
        if (line[i].getRepresentation() != "," and line[i].getRepresentation() != ";") {
            variable.setType(determineType(token));
            variable.setScope(scopes.back()); // Set scope to be the top of stack
            variable.setVarName(line[i].getRepresentation());
            
            string key = variable.getScope() + " " + variable.getVarName();
            if (variableDefinition.find(key) != variableDefinition.end()) {
                // Variable name already exists within the same scope!
                cout << "INVALID! VARIABLE ALREADY EXISTS WITHIN THIS SCOPE" << endl;
            } else {
                setVariableInScope(variable);
            }
        }
    }
}

void Analyzer::setVariableInScope(ScopeVariable variable) {
    string key = variable.getScope() + " " + variable.getVarName();
    variableDefinition[key] = { variable.getVarName(), variable.getScope(), variable.getType() };
}

void Analyzer::printVariables() {
    for (const auto & [ key, value ] : variableDefinition) {
        cout << key << ": " << value[0] << ", " << value[1] << ", " << value[2] << endl;
    }
}
