//
//  threeAddressCode.cpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-10.
//

#include "../../include/intermediateCode/threeAddressCode.hpp"

ThreeAddressCode::ThreeAddressCode(vector<vector<Token>> inputLines) {
    lines = inputLines;
    scopes.push_back("GLOBAL");
    printString += "GLOBAL SCOPE";
    printString += '\n';
    threeAddressCodeText += BRANCH + " " + labelNames.back();
    threeAddressCodeText += '\n';
    threeAddressCodeText += "Begin: ";
    threeAddressCodeText += '\n';
    threeAddressCodeText += PUSH + "{" + LR + "}";
    threeAddressCodeText += '\n';
    threeAddressCodeText += PUSH + "{" + FP + "}";
    threeAddressCodeText += '\n';
    sp += 8;
}

void ThreeAddressCode::createCode() {
    vector<vector<Token>> brokenUpLines = breakUpLines(lines);
    
    for (auto line : brokenUpLines) {
        if (line.size() > 0)
            handleCodeLine(line);
    }
}

vector<vector<Token>> ThreeAddressCode::breakUpLines(vector<vector<Token>> lines) {
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
    return brokenUpLines;
}

void ThreeAddressCode::handleCodeLine(vector<Token> line) {
    string tokenValue = line[0].getRepresentation();
    // Handle functions
    if (tokenValue == "def") {
        scopeCounter = 1;
        
        scopes.push_back(FUNCTION);
        handleFunctionCode(line);

//        handleFunctionScope(token, line);
    }
    // Handle if blocks
    else if (tokenValue == "if") {
        scopeCounter = 1;
        
        scopes.push_back("IF");
        handleIfCode(line);
//        handleCondition(token, line);
    }
    // Handle else block
    else if (tokenValue == "else") {
        scopes.pop_back(); // Remove the existing IF scope
        scopeCounter = 1;
        
        functionText += ELSE_LABEL + " " + label + to_string(labelCounter);
        functionText += '\n';
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        scopeCounter = 1;
        
        scopes.push_back("WHILE");
//        handleCondition(token, line);
    }
    // Handle closing scopes
    else if (tokenValue == "od" or tokenValue == "fi") {
        scopes.pop_back();
        scopeCounter--;
    }
    // Handle function close
    else if (tokenValue == "fed") {
        scopes.pop_back();
        scopeCounter--;
    }
    // Handle Return statements
    else if (tokenValue == "return") {
//        if(find(scopes.begin(), scopes.end(), FUNCTION) != scopes.end()) {
//            // We are currently in a function
//            validReturn = validReturn and checkValidReturnType(token, line);
//        }
        scopeCounter--;
    }
    // Handle print statements
    else if (tokenValue == "print") {
//        handlePrint(token, line);
    }
    // Handle variable decleration
    else if (isVariableType(line[0])) {
        if (scopeCounter == 1) {
            printValue(scopes.back() + " SCOPE", 1);
        }
        scopeCounter++;
        handleVariableDeclerationCode(line);
    }
    // Assignment operations
    else {
        handleOperationCode(line);
    }
}

void ThreeAddressCode::handleFunctionCode(vector<Token> line) {
    string functionType = determineType(line[1]);
    string functionName = line[2].getRepresentation();
    labelNames.push_back(functionName + ": "); // Add the function name
        
    if (scopeCounter == 1) {
        printValue(scopes.back() + " SCOPE", 1);
    }
    scopeCounter++;
    printValue(functionName + ", " + functionType + ", " + scopes.back(), 0);
    
    ScopeVariable variable;
    variable.setScope(FUNCTION);
    vector<ScopeVariable> variables;
    
    // Save input parameters as scope variables
    for (int i = 4; i < line.size() - 1; i++) {
        if (line[i].getType().toString() == "id") {
            if (isVariableType(line[i]))
                variable.setType(determineType(line[i]));
            else {
                variable.setVarName(line[i].getRepresentation());
                printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
                variables.push_back(variable);
            }
        } else if (line[i].getRepresentation() == ",") {
            variable.setEmptyValues();
        }
    }
        
    for (unsigned i = (int)variables.size(); i-- > 0; ) {
        functionText += variables[i].getVarName() + " = fp + " + to_string(sp);
        sp += variables[i].getType() == "integer" ? 4 : 8;
        functionText += '\n';
        numberOfBytes += 4;
    }
}

void ThreeAddressCode::handleIfCode(vector<Token> line) {
    vector<string> componentsOfIf = {"if", "(", ")", ",", "then"},
    componentsOfCondition = {"and", "or", ">", "<", "==", ">=", "<="};
    vector<ScopeVariable> variables;
    vector<string> operations;
    int j = 0;
    ScopeVariable variable;
    
    for (Token token : line) {
        string tokenValue = token.getRepresentation();
        if(std::find(componentsOfIf.begin(), componentsOfIf.end(), tokenValue) == componentsOfIf.end()) {
            if(std::find(componentsOfCondition.begin(), componentsOfCondition.end(), tokenValue) != componentsOfCondition.end()) {
                // Is a boolean operation
                operations.push_back(tokenValue);
            } else {
                // Is a variable
                variable.setType(token.getType().toString());
                variable.setScope(scopes.back());
                variable.setVarName(tokenValue);
                variables.push_back(variable);
                variable.setEmptyValues();
            }
        }
    }
    
    string tempOperation = COMPARE + " " + variables[j].getVarName() + ", " + variables[j + 1].getVarName();
    
    if (tempOperation != compareOperation) {
        compareOperation = tempOperation;
        functionText += tempOperation;
        j += 2;
        functionText += '\n';
    }
    
    for (string op : operations) {
        string labelName = label + to_string(labelCounter);
        if (op == "<")
            functionText += LESS_THAN + " " + labelName;
        else if (op == "<=")
            functionText += LESS_THAN_EQUAL + " " + labelName;
        else if (op == ">")
            functionText += GREATER_THAN + " " + labelName;
        else if (op == ">=")
            functionText += GREATER_THAN_EQUAL + " " + labelName;
        else if (op == "==")
            functionText += EQUAL + " " + labelName;
        functionText += '\n';
        labelCounter++;
        operationLabels.push_back(labelName);
    }
}

void ThreeAddressCode::handleVariableDeclerationCode(vector<Token> line) {
    string variableType = determineType(line[0]);
    string generatedCode;
    
    ScopeVariable variable;
    variable.setScope(scopes.back());
    vector<ScopeVariable> variables;
    
    // Save input parameters as scope variables
    for (int i = 1; i < line.size() - 1; i++) {
        if (line[i].getType().toString() == "id") {
            variable.setVarName(line[i].getRepresentation());
            variable.setType(variableType);
            printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
            variables.push_back(variable);
        } else if (line[i].getRepresentation() == ",") {
            variable.setEmptyValues();
        }
    }
        
    for (unsigned i = (int)variables.size(); i-- > 0; ) {
        generatedCode += variables[i].getVarName() + " = fp + " + to_string(sp);
        sp += variables[i].getType() == "integer" ? 4 : 8;
        generatedCode += '\n';
        numberOfBytes += 4;
    }
    
    if (scopes.back() == FUNCTION)
        functionText += generatedCode;
    else if (scopes.back() == GLOBAL)
        threeAddressCodeText += generatedCode;
    else
        generatedOperationCode.push_back(generatedCode);
}

void ThreeAddressCode::handleOperationCode(vector<Token> line) {
    if (scopes.back() == "IF") {
        cout << "We're in an if statement: " + line[0].getRepresentation() << endl;
    }
}

void ThreeAddressCode::printValue(string text, int incr) {
    for (int i = 0; i < scopes.size() - incr; i++)
        printString += '\t';
    printString += text;
    printString += '\n';
}

void ThreeAddressCode::printThreeAddressCode() {
    cout << threeAddressCodeText << endl;
    cout << "--------" << endl;
    cout << functionText << endl;
    cout << "--------" << endl;
    cout << printString << endl;
}

string ThreeAddressCode::determineType(Token token) {
    if (token.getRepresentation() == "int") return "integer";
    return token.getRepresentation();
}

bool ThreeAddressCode::isVariableType(Token token) {
    if (token.getRepresentation() == "int" or token.getRepresentation() == "double")
        return true;
    return false;
}
