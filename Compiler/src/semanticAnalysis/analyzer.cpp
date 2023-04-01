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
    printString += "GLOBAL SCOPE";
    printString += '\n';
}

void Analyzer::analyzeSemantics() {
    vector<vector<Token>> brokenUpLines;
    for (auto line : lines) {
        vector<Token> tLine;
        for (Token token : line) {
            string tokenValue = token.getRepresentation();
            tLine.push_back(token);
            if (tokenValue == "do" or tokenValue == "then" or tokenValue == ";") {
                brokenUpLines.push_back(tLine);
                tLine.clear();
            } else if (tokenValue == "od" or tokenValue == "fi" or tokenValue == "else") {
                tLine.pop_back();
                brokenUpLines.push_back(tLine);
                tLine.clear();
                tLine.push_back(token);
            }
        }
        brokenUpLines.push_back(tLine);
    }
    
    for (auto line : brokenUpLines) {
        if (line.size() > 0)
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
        for (int i = 0; i < scopes.size(); i++)
            printString += '\t';
        printString += "FUNCTION SCOPE";
        printString += '\n';
        
        scopes.push_back(FUNCTION);
        handleFunctionScope(token, line);
    }
    // Handle if blocks
    else if (tokenValue == "if") {
        for (int i = 0; i < scopes.size(); i++)
            printString += '\t';
        printString += "IF SCOPE";
        printString += '\n';
        
        scopes.push_back("IF");
        handleCondition(token, line);
    }
    // Handle else block
    else if (tokenValue == "else") {
        scopes.pop_back(); // Remove the existing IF scope
        
        for (int i = 0; i < scopes.size(); i++)
            printString += '\t';
        printString += "ELSE SCOPE";
        printString += '\n';
        
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        for (int i = 0; i < scopes.size(); i++)
            printString += '\t';
        printString += "WHILE SCOPE";
        printString += '\n';
        
        scopes.push_back("WHILE");
        handleCondition(token, line);
    }
    // Handle closing scopes
    else if (tokenValue == "od" or tokenValue == "fi") {
        scopes.pop_back();
    }
    // Handle function close
    else if (tokenValue == "fed") {
        if (!validReturn)
            saveErrorText("Invalid! Function has mismatching return types");
        
        validReturn = true; // Reset it for the next function
        returnTypes.pop_back();
        scopes.pop_back();
    }
    // Handle Return statements
    else if (tokenValue == "return") {
        if(find(scopes.begin(), scopes.end(), FUNCTION) != scopes.end()) {
            // We are currently in a function
            validReturn = validReturn and checkValidReturnType(token, line);
        } else {
            // We are not in a function
            saveErrorText("Invalid! Return keyword added outside of function");
        }
    }
    // Handle print statements
    else if (tokenValue == "print") {
        handlePrint(token, line);
    }
    // Handle variable decleration
    else if (isVariableType(token)) {
        handleVariableDeclaration(token, line);
    }
    // Assignment operations
    else {
        handleOperations(token, line);
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
    
    for (int i = 0; i < scopes.size(); i++)
        printString += '\t';
    printString += functionName + ", " + functionType + ", " + scopes.back();
    printString += '\n';
    
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
                saveErrorText("Invalid! " + variable.getVarName() + " variable already exists within this scope!");
            } else {
                setVariableInScope(variable);
            }
        }
    }
    
    for (int i = 0; i < scopes.size(); i++)
        printString += '\t';
    printString += variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope();
    printString += '\n';
}

void Analyzer::handleOperations(Token token, vector<Token> line) {
    vector<int> equalOperations; // If we put multiple statement on the same line
    vector<vector<int>> arthmeticIncrement; // When we have an arithmetic operation we want to handle it as one token
    
    for (int i = 0; i < line.size(); i++) {
        string t = line[i].getRepresentation();
        if (t == "=") {
            equalOperations.push_back(i);
            arthmeticIncrement.push_back({ i + 1, 0 });
        } else if (t == "+" or t == "-" or t == "/" or t == "*") {
            if (arthmeticIncrement.size() == 2)
                arthmeticIncrement.back().back() = i + 1;
        }
    }
    
    for (int i = 0; i < equalOperations.size(); i++) {
        bool assignment = checkValidAssignment(line, equalOperations[i], arthmeticIncrement[i]);
        if (!assignment)
            saveErrorText("Invalid! Assignment operation has mismatching types!");
    }
}

void Analyzer::handleCondition(Token token, vector<Token> line) {
    int openParen = 0, closeParen = 0;
    for (int i = 0; i < line.size(); i++) {
        string tokenValue = line[i].getRepresentation();
        if (tokenValue == "(")
            openParen = i;
        else if (tokenValue == ")") {
            closeParen = i;
            bool condition = checkValidCondition(line, openParen, closeParen);
            if (!condition)
                saveErrorText("Invalid! Conditional statement has mismatching types!");
        } else if (tokenValue == "and" or tokenValue == "or") {
            closeParen = i;
            bool condition = checkValidCondition(line, openParen, closeParen);
            if (!condition)
                saveErrorText("Invalid! Conditional statement has mismatching types!");
            openParen = i;
        }
    }
}

void Analyzer::handlePrint(Token token, vector<Token> line) {
    for (auto t : line) {
        if (t.getRepresentation() != "print" and t.getType().toString() == "id" and !checkVariableExists(t)) {
            saveErrorText("Invalid! " + t.getRepresentation() + " variable does not exist!");
            break;
        }
    }
}

Token Analyzer::handleArithmetic(vector<Token> line, vector<int> arthimeticRange) {
    string type = "integer";
    for (int i = arthimeticRange[0]; i < arthimeticRange[1] - arthimeticRange[0]; i++) {
        if (line[i].getType().toString() == "double")
            type = "double";
    }
    
    return Token("Operation", TokenType(type));
}

void Analyzer::setVariableInScope(ScopeVariable variable) {
    string key = variable.getScope() + " " + variable.getVarName();
    variableDefinition[key] = { variable.getVarName(), variable.getScope(), variable.getType() };
}

bool Analyzer::checkValidReturnType(Token token, vector<Token> line) {
    Token returnedValue;
    
    for (Token t : line) {
        if (getTypeFromToken(t) == returnTypes.back()) {
            return true;
        }
    }
    return false;
}

string Analyzer::getTypeFromToken(Token token) {
    string tokenValue = token.getRepresentation();
    
    // Mainly used for functions (recursion)
    if (variableDefinition.find(tokenValue) != variableDefinition.end()) {
        return variableDefinition[tokenValue][2];
    }
    
    for (string scope : scopes) {
        // Make sure that the variable is in the complete scope of the application
        string key = scope + " " + tokenValue;
        if (variableDefinition.find(key) != variableDefinition.end()) {
            return variableDefinition[key][2];
        }
    }
    
    if (token.getType().toString() == TokenType::INT.toString() or token.getType().toString() == TokenType::DOUBLE.toString()) {
        return token.getType().toString();
    }
    
    return "NOT FOUND";
}

bool Analyzer::checkVariableExists(Token token) {
    // Check to see if the left variable is actually defined and in scope
    for (string scope : scopes) {
        string tokenValue = token.getRepresentation();
        string key = scope + " " + token.getRepresentation();
        if (variableDefinition.find(key) != variableDefinition.end() or variableDefinition.find(tokenValue) != variableDefinition.end()) {
            return true;
        }
    }
    
    return false;
}

bool Analyzer::checkValidAssignment(vector<Token> line, int tokenNumber, vector<int> arthimeticRange) {
    Token left = line[tokenNumber - 1];
    bool variableExists = checkVariableExists(left);
    
    if (variableExists) {
        Token right = line[tokenNumber + 1];
        
        if (arthimeticRange[0] < arthimeticRange[1])
            right = handleArithmetic(line, arthimeticRange);
        
        return (getTypeFromToken(left) == getTypeFromToken(right));
    }
    
//    cout << "Invalid! Variable does not exist!" << endl;
    saveErrorText("Invalid! " + left.getRepresentation() + " variable does not exist!");
    return true;
}

bool Analyzer::checkValidCondition(vector<Token> line, int openParen, int closeParen) {
    Token left = line[openParen + 1];
    Token right = line[closeParen - 1];
    
    return (getTypeFromToken(left) == getTypeFromToken(right));
}

void Analyzer::saveErrorText(string text) {
    fstream appendFileToWorkWith;
    string filename = "../output/error.txt";
    appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    
    // Print errors to file
    if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith << "SEMANTIC ERROR" << endl;
        appendFileToWorkWith << text << endl;
        appendFileToWorkWith.close();
    } else {    // use existing file
        appendFileToWorkWith << "SEMANTIC ERROR" << endl;
        appendFileToWorkWith << text << endl;
        appendFileToWorkWith.close();
    }
}

void Analyzer::printVariables() {
//    cout << printString;
    try {
        fstream appendFileToWorkWith;
        string filename = "../output/symbolTable.txt";
        appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

      // If file does not exist, Create new file
      if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith <<"\n";
        appendFileToWorkWith.close();
       } else {    // use existing file
           appendFileToWorkWith << printString <<endl;
           appendFileToWorkWith.close();
        }
    } catch (const std::out_of_range& e) {
        cerr << "Out of Range error: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception caught" << endl;
    }
}
