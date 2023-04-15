/*
 File: analyzer.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-30
*/

#include "../../include/semanticAnalysis/analyzer.hpp"

/*
 Description:
    Construtor for the semantic analyzer
 
 Parameters:
    - inputLines: The code lines split into Token arrays (vector<vector<Token>> inputLines)
*/
Analyzer::Analyzer(vector<vector<Token>> inputLines) {
    lines = inputLines;
    scopes.push_back("GLOBAL");
    printString += "GLOBAL SCOPE";
    printString += '\n';
}

/*
 Description:
    Performs semantic analysis on the code lines
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Analyzer::analyzeSemantics() {
    vector<vector<Token>> brokenUpLines;
    vector<int> lineNumbers;
    int currentLine = 1;
    
    for (auto line : lines) {
        vector<Token> tLine;
        for (Token token : line) {
            string tokenValue = token.getRepresentation();
            tLine.push_back(token);
            if (tokenValue == "do" or tokenValue == "then" or tokenValue == ";") {
                brokenUpLines.push_back(tLine);
                lineNumbers.push_back(currentLine);
                tLine.clear();
            } else if (tokenValue == "od" or tokenValue == "fi" or tokenValue == "else") {
                tLine.pop_back();
                brokenUpLines.push_back(tLine);
                lineNumbers.push_back(currentLine);
                tLine.clear();
                tLine.push_back(token);
            }
        }
        brokenUpLines.push_back(tLine);
        lineNumbers.push_back(currentLine);
        currentLine++;
    }
    
    for (int i = 0; i < brokenUpLines.size(); i++) {
        if (brokenUpLines[i].size() > 0) {
            handleScopes(brokenUpLines[i][0], brokenUpLines[i]);
            currentLineNumber = lineNumbers[i];
        }
    }
}

/*
 Description:
    Gets the type of the variable (ints are cast to integers)
 
 Parameters:
    - token: The token that will be analyzed (Token)
 
 Returns:
    - The token type (string)
*/
string Analyzer::determineType(Token token) {
    if (token.getRepresentation() == "int") return "integer";
    return token.getRepresentation();
}

/*
 Description:
    Checks to see is an ID token is an int or double declaration
 
 Parameters:
    - token: The token that will be analyzed (Token)
 
 Returns:
    - Whether or not the token is an variable declaration
*/
bool Analyzer::isVariableType(Token token) {
    if (token.getRepresentation() == "int" or token.getRepresentation() == "double")
        return true;
    return false;
}

/*
 Description:
    Determines and sets the scope for the current code block
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
void Analyzer::handleScopes(Token token, vector<Token> line) {
    string tokenValue = token.getRepresentation();
    // Handle functions
    if (tokenValue == "def") {
        scopeCounter = 1;
        
        scopes.push_back(FUNCTION);
        handleFunctionScope(token, line);
    }
    // Handle if blocks
    else if (tokenValue == "if") {
        scopeCounter = 1;
        
        scopes.push_back("IF");
        handleCondition(token, line);
    }
    // Handle else block
    else if (tokenValue == "else") {
        scopes.pop_back(); // Remove the existing IF scope
        scopeCounter = 1;
        
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        scopeCounter = 1;
        
        scopes.push_back("WHILE");
        handleCondition(token, line);
    }
    // Handle closing scopes
    else if (tokenValue == "od" or tokenValue == "fi") {
        scopes.pop_back();
        scopeCounter--;
    }
    // Handle function close
    else if (tokenValue == "fed") {
        if (!validReturn)
            saveErrorText("Invalid! Function has mismatching return types");
        
        validReturn = true; // Reset it for the next function
        returnTypes.pop_back();
        scopes.pop_back();
        scopeCounter--;
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
        scopeCounter--;
    }
    // Handle print statements
    else if (tokenValue == "print") {
        handlePrint(token, line);
    }
    // Handle variable decleration
    else if (isVariableType(token)) {
        if (scopeCounter == 1) {
            printValue(scopes.back() + " SCOPE", 1);
        }
        scopeCounter++;
        handleVariableDeclaration(token, line);
    }
    // Assignment operations
    else {
        handleOperations(token, line);
    }
}

/*
 Description:
    Sets the function scope and populates the symbol table
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
void Analyzer::handleFunctionScope(Token token, vector<Token> line) {
    // Line structure is:
    // <def> <returnType> <name> <(> <arguments> <)>
    
    // Since the code is parsed correctly we know after def there will be a return type argument
    string functionType = determineType(line[1]), functionName = line[2].getRepresentation();
    returnTypes.push_back(functionType); // Add the return type of the function to the returnType stack
    functionDefinition[functionName] = functionType;
    variableDefinition[functionName] = { functionName, scopes.back(), functionType };
    
    if (scopeCounter == 1) {
        printValue(scopes.back() + " SCOPE", 1);
    }
    scopeCounter++;
    printValue(functionName + ", " + functionType + ", " + scopes.back(), 0);
    
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
                printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
            }
        } else if (line[i].getRepresentation() == ",") {
            variable.setEmptyValues();
        }
    }
}

/*
 Description:
    Sets the variable scope and handles the declaration of a new variable
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
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
            printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
        }
    }
}

/*
 Description:
    Handles the assignment operation and checks to make sure that the type of both tokens is the same
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
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

/*
 Description:
    Handles the comparison operations (==, <, >, <=, >=, <>)
    and makes sure the types of both tokens is the same
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
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

/*
 Description:
    Handles the print operation and ensures that no new variables are created inside it
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - None
*/
void Analyzer::handlePrint(Token token, vector<Token> line) {
    for (auto t : line) {
        if (t.getRepresentation() != "print" and t.getType().toString() == "id" and !checkVariableExists(t)) {
            saveErrorText("Invalid! " + t.getRepresentation() + " variable does not exist!");
            break;
        }
    }
}

/*
 Description:
    Handles arithmetic operationss (+, -, *, /)
    and sets the type of the final token to be a double
    if some operation with double has occured, otherwise int
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - The operation token (Token)
*/
Token Analyzer::handleArithmetic(vector<Token> line, vector<int> arthimeticRange) {
    string type = "integer";
    for (int i = arthimeticRange[0]; i < arthimeticRange[1] - arthimeticRange[0]; i++) {
        if (line[i].getType().toString() == "double")
            type = "double";
    }
    
    return Token("Operation", TokenType(type));
}

/*
 Description:
    Sets a variable in the scope of the variable definition table
 
 Parameters:
    - variable: The variable that will be saved (ScopeVariabl)
 
 Returns:
    - None
*/
void Analyzer::setVariableInScope(ScopeVariable variable) {
    string key = variable.getScope() + " " + variable.getVarName();
    variableDefinition[key] = { variable.getVarName(), variable.getScope(), variable.getType() };
}

/*
 Description:
    Checks to make sure the return type is the same as the function declaration
 
 Parameters:
    - token: The token that will be analyzed (Token)
    - line: The code line of Tokens (vector<Token>)
 
 Returns:
    - Whether or not the types match (bool)
*/
bool Analyzer::checkValidReturnType(Token token, vector<Token> line) {
    Token returnedValue;
    
    for (Token t : line) {
        if (getTypeFromToken(t) == returnTypes.back()) {
            return true;
        }
    }
    return false;
}

/*
 Description:
    Gets the type of a variable in the symbol table
 
 Parameters:
    - token: The token that will be analyzed (Token)
 
 Returns:
    - The type of the variable (string)
*/
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

/*
 Description:
    Checks whether or not a variable exists in the symbol table
 
 Parameters:
    - token: The token that will be analyzed (Token)
 
 Returns:
    - Does the variable exist (bool)
*/
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

/*
 Description:
    Checks to make sure the assignment operation has matching types
 
 Parameters:
    - line: The code line of Tokens (vector<Token>)
    - tokenNumber: The starting index of the assignment (x = 2, is index: 1) (int)
    - arthimeticRange: The range of operations on the right hand side (x = 2 * t, is {2, 4}) (vector<int>)
 
 Returns:
    - Whether or not the assignment is valid (bool)
*/
bool Analyzer::checkValidAssignment(vector<Token> line, int tokenNumber, vector<int> arthimeticRange) {
    Token left = line[tokenNumber - 1];
    bool variableExists = checkVariableExists(left);
    
    if (variableExists) {
        Token right = line[tokenNumber + 1];
        
        if (arthimeticRange[0] < arthimeticRange[1])
            right = handleArithmetic(line, arthimeticRange);
        
        return (getTypeFromToken(left) == getTypeFromToken(right));
    }
    
    saveErrorText("Invalid! " + left.getRepresentation() + " variable does not exist!");
    return true;
}

/*
 Description:
    Checks to make sure the condition operation has matching types
 
 Parameters:
    - line: The code line of Tokens (vector<Token>)
    - openParen: The starting index of the condition (int)
    - closeParen: The ending index of the condition (int)
 
 Returns:
    - Whether or not the condition is valid (bool)
*/
bool Analyzer::checkValidCondition(vector<Token> line, int openParen, int closeParen) {
    Token left = line[openParen + 1];
    Token right = line[closeParen - 1];
    
    return (getTypeFromToken(left) == getTypeFromToken(right));
}

/*
 Description:
    Saves the error text to a file
 
 Parameters:
    - text: Text to save (string)
 
 Returns:
    - None
*/
void Analyzer::saveErrorText(string text) {
    isCorrect = false;
    fstream appendFileToWorkWith;
    string filename = "../output/error.txt";
    appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    
    // Print errors to file
    if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith << "SEMANTIC ERROR" << endl;
        appendFileToWorkWith << "Error on line number: " << currentLineNumber << endl;
        appendFileToWorkWith << text << endl;
        appendFileToWorkWith.close();
    } else {    // use existing file
        appendFileToWorkWith << "SEMANTIC ERROR" << endl;
        appendFileToWorkWith << "Error on line number: " << currentLineNumber << endl;
        appendFileToWorkWith << text << endl;
        appendFileToWorkWith.close();
    }
}

/*
 Description:
    Updates the print string symbol table with values
 
 Parameters:
    - text: Text to update (string)
    - incr: The increment to have for tabbing (int)
 
 Returns:
    - None
*/
void Analyzer::printValue(string text, int incr) {
    for (int i = 0; i < scopes.size() - incr; i++)
        printString += '\t';
    printString += text;
    printString += '\n';
}

/*
 Description:
    Print the symbol table to a file
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Analyzer::printVariables() {
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
