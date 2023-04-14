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
        
        functionText += NEW_LABEL + " " + label + to_string(labelCounter);
        functionText += '\n';
        
        // Move this to dedicated function
        generatedOperationCode.push_back(operationText);
        operationText = "";
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        scopeCounter = 1;
        
        scopes.push_back("WHILE");
    }
    // Handle closing scopes
    else if (tokenValue == "od" or tokenValue == "fi") {
        scopes.pop_back();
        scopeCounter--;
        
        // Move this to dedicated function
        generatedOperationCode.push_back(operationText);
        operationText = "";
    }
    // Handle function close
    else if (tokenValue == "fed") {
        scopes.pop_back();
        scopeCounter--;
    }
    // Handle Return statements
    else if (tokenValue == "return") {
        handleReturnCode(line);
        addCode(NEW_LABEL + " " + returnLabel);
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
    string funcType = determineType(line[1]);
    string funcName = line[2].getRepresentation();
    labelNames.push_back(funcName + ": "); // Add the function name
    
    returnLabel = "exit" + funcName;
    functionName = funcName;
    functionType = funcType;
    
    if (scopeCounter == 1) {
        printValue(scopes.back() + " SCOPE", 1);
    }
    scopeCounter++;
    printValue(funcName + ", " + funcType + ", " + scopes.back(), 0);
    
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
        variableTypes[variables[i].getVarName()] = variables[i].getType();
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
            variableTypes[line[i].getRepresentation()] = variableType;
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
        operationText += generatedCode;
}

void ThreeAddressCode::handleOperationCode(vector<Token> line) {
    string generatedCode;
    
    string finalVariable = line[0].getRepresentation(); // The variable we will assign everything to in the end
    
    // Recursive call
    if (finalVariable == functionName) {
        handleRecursiveCallCode(line);
        return;
    }
    
    string variableType = variableTypes[finalVariable];
    vector<vector<Token>> bracketsOperations; // Brackets
    vector<Token> beginOp = line, endOp;
        
    // We have a math operation in the assignment
    vector<Token> temp;
    for (int i = 2; i < line.size() - 1; i++) {
        if (line[i].getRepresentation() == "(" or line[i].getRepresentation() == ")") {
            bracketsOperations.push_back(temp);
            temp.clear();
        } else {
            temp.push_back(line[i]);
        }
    }
    
    if (bracketsOperations.size() > 0) {
        for (auto i : bracketsOperations) {
            if (i.size() > 0)
                simplifyMultiplicationOperation(i, variableType);
        }
    } else {
        vector<Token> expression;
        for (int i = 2; i < line.size() - 1; i++) {
            expression.push_back(line[i]);
        }
        string expVal = simplifyMultiplicationOperation(expression, variableType);
        generatedCode += line[0].getRepresentation() + " = " + expVal;
        sp += variableTypes[line[0].getRepresentation()] == "integer" ? 4 : 8;
        generatedCode += '\n';
        numberOfBytes += 4;
    }
    
    addCode(generatedCode);
}

string ThreeAddressCode::simplifyMultiplicationOperation(vector<Token> exp, string variableType) {
    string generatedCode;
    int numberOfMultiplcation = 0, completeScan = 0;
    
    for (auto ct : exp) {
        string c = ct.getRepresentation();
        if (c == "*" or c == "/")
            numberOfMultiplcation++;
    }
    
    // Incomplete expression
    string lastChar = exp.back().getRepresentation();
    if (lastChar == "-" or lastChar == "+" or lastChar == "/" or lastChar == "*")
        completeScan++;
    
    // Deal with multiplcation and division first
    vector<Token> tempExp = exp;
    if (numberOfMultiplcation > 0) {
        vector<Token> newTempExp;
        for (int j = 0; j < tempExp.size() - completeScan; j ++) {
            string curr = tempExp[j].getRepresentation();
            if (curr == "*" or curr == "/") {
                numberOfMultiplcation--;
                
                // Create temporary variable
                ScopeVariable variable;
                variable.setScope(scopes.back());
                variable.setType(variableType);
                variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
                temporaryVariableCounter++;
                                
                // Print temporary variable to symbol table
                printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
                
                // Create Three Adress Code of temporary variable
                string front = tempExp[j - 1].getRepresentation(), back = tempExp[j + 1].getRepresentation();
                generatedCode += variable.getVarName() + " = " + front + " " + curr + " " + back;
                sp += variable.getType() == "integer" ? 4 : 8;
                generatedCode += '\n';
                numberOfBytes += 4;
                
                // Add temporary variable in place of the multiplication operation
                Token varToken(variable.getVarName(), TokenType(variable.getType()));
                newTempExp.push_back(varToken);
            } else if (curr == "+" or curr == "-") {
                if (newTempExp.size() > 0) {
                    // Already added a variable
                    newTempExp.push_back(tempExp[j]);
                    if (numberOfMultiplcation <= 0)
                        newTempExp.push_back(tempExp[j + 1]);
                    else if (j + 2 < tempExp.size() - completeScan) {
                        // This means that there will be a multiplication or division coming up, but right now we're adding or subtracting something
                        if (tempExp[j + 2].getRepresentation() == "+" or tempExp[j + 2].getRepresentation() == "-")
                            newTempExp.push_back(tempExp[j + 1]);
                    }
                } else {
                    newTempExp.push_back(tempExp[j - 1]);
                    newTempExp.push_back(tempExp[j]);
                }
            }
        }
        tempExp = newTempExp;
    }
    
    // Deal with addition and subtraction
    Token finalAssignment;
    while (tempExp.size() > 3) {
        vector<Token> newTempExp;
        for (int j = 0; j < tempExp.size() - completeScan; j ++) {
            string curr = tempExp[j].getRepresentation();
            if (curr == "+" or curr == "-") {
                finalAssignment = tempExp[j];
                // Create temporary variable
                ScopeVariable variable;
                variable.setScope(scopes.back());
                variable.setType(variableType);
                variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
                temporaryVariableCounter++;
                                
                // Print temporary variable to symbol table
                printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
                
                // Create Three Adress Code of temporary variable
                string front = tempExp[j - 1].getRepresentation(), back = tempExp[j + 1].getRepresentation();
                generatedCode += variable.getVarName() + " = " + front + " " + curr + " " + back;
                sp += variable.getType() == "integer" ? 4 : 8;
                generatedCode += '\n';
                numberOfBytes += 4;
                
                // Add temporary variable in place of the multiplication operation
                Token varToken(variable.getVarName(), TokenType(variable.getType()));
                newTempExp.push_back(varToken);
                for (int k = j + 2; k < tempExp.size(); k++)
                    newTempExp.push_back(tempExp[k]);
                tempExp = newTempExp;
                break;
            }
        }
        
        tempExp = newTempExp;
    }
    string ret = "";
    for (auto token : tempExp)
        ret += token.getRepresentation() + " ";
    
    addCode(generatedCode);
    
    return ret;
}

void ThreeAddressCode::handleReturnCode(vector<Token> line) {
    vector<Token> valuesInsideParens;
    string finalReturn;
    string generatedCode;
    
    bool startOfParen = false;
    for (Token token : line) {
        if (startOfParen)
            valuesInsideParens.push_back(token);
        if (token.getRepresentation() == "(")
            startOfParen = true;
    }
    
    
    valuesInsideParens.pop_back();
    
    // This means that we popped a ";"
    if (valuesInsideParens.back().getRepresentation() == ")")
        valuesInsideParens.pop_back();
    
    if (valuesInsideParens.size() > 1) {
        for (Token token : valuesInsideParens) {
            if (token.getRepresentation() == functionName)
                finalReturn = handleRecursiveCallCode(valuesInsideParens);
        }
    } else {
        finalReturn = valuesInsideParens[0].getRepresentation();
    }
    
    int decrement = functionType == "integer" ? 4 : 8;
    fp -= decrement;
    
    generatedCode += FP + " - " + to_string(decrement) + " = " + finalReturn;
    generatedCode += '\n';
    addCode(generatedCode);
}

string ThreeAddressCode::handleRecursiveCallCode(vector<Token> line) {
    string generatedCode;
    vector<vector<Token>> definedVariables;
    vector<string> variableStack;
    vector<Token> temp;
    
    bool startOfParen = false;
    for (Token token : line) {
        if (token.getRepresentation() == "(")
            startOfParen = true;
        else if (startOfParen and token.getRepresentation() != "," and token.getRepresentation() != ")" and token.getRepresentation() != "(")
            temp.push_back(token);
        else if (startOfParen and (token.getRepresentation() == "," or token.getRepresentation() == ")")) {
            definedVariables.push_back(temp);
            temp.clear();
        }
    }
    definedVariables.push_back(temp);
    
    cout << "YOYOY: ";
    for (vector<Token> parameter : definedVariables) {
        for (Token t : parameter)
            cout << t.getRepresentation() << ", ";
        cout << endl;
    }
    cout << endl;
    
    for (vector<Token> parameter : definedVariables) {
        if (parameter.size() > 1) {
            // This means that we have a mathematication operation
            string varName = handleInplaceDeclerationCode(parameter, parameter[0].getType().toString());
            variableStack.push_back(varName);
        } else {
            variableStack.push_back(parameter[0].getRepresentation());
        }
    }
    
    for (string value : variableStack) {
        generatedCode += "push {" + value + "}";
        generatedCode += '\n';
    }
    
    // Create the branch link
    ScopeVariable variable;
    variable.setScope(scopes.back());
    variable.setType(functionType);
    variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
    temporaryVariableCounter++;
                    
    // Print temporary variable to symbol table
    printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
    
    generatedCode += variable.getVarName() + " = " + BRANCH_LINK + " " + functionName;
    generatedCode += '\n';
    
    // Print the pop stack
    for (auto it = variableStack.rbegin(); it != variableStack.rend(); ++it) {
        generatedCode += "pop {" + *it + "}";
        generatedCode += '\n';
    }
    
    addCode(generatedCode);
    
    return variable.getVarName();
}

string ThreeAddressCode::handleInplaceDeclerationCode(vector<Token> line, string variableType) {
    string generatedCode;
    // Create temporary variable
    ScopeVariable variable;
    variable.setScope(scopes.back());
    variable.setType(variableType);
    variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
    temporaryVariableCounter++;
                    
    // Print temporary variable to symbol table
    printValue(variable.getVarName() + ", " + variable.getType() + ", " + variable.getScope(), 0);
    
    // Create Three Adress Code of temporary variable
    string expression = simplifyMultiplicationOperation(line, variable.getType());
    generatedCode += variable.getVarName() + " = " + expression;
    sp += variable.getType() == "integer" ? 4 : 8;
    generatedCode += '\n';
    numberOfBytes += 4;
    
    addCode(generatedCode);
    
    return variable.getVarName();
}

void ThreeAddressCode::addCode(string generatedCode) {
    if (scopes.back() == FUNCTION)
        functionText += generatedCode;
    else if (scopes.back() == GLOBAL)
        threeAddressCodeText += generatedCode;
    else
        operationText += generatedCode;
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
    for (auto i : generatedOperationCode) {
        cout << i << endl;
        cout << "--------" << endl;
    }
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
