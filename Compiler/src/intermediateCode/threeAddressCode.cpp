/*
 File: threeAddressCode.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-04-10
*/

#include "../../include/intermediateCode/threeAddressCode.hpp"

ThreeAddressCode::ThreeAddressCode(vector<vector<Token>> inputLines) {
    lines = inputLines;
    scopes.push_back("GLOBAL");
    printString += "GLOBAL SCOPE";
    printString += '\n';
    threeAddressCodeText += BRANCH + " " + labelNames.back();
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
    
    string textToAdd = labelNames.back() + ": ";
    textToAdd += '\n';
    textToAdd += BEGIN + " " + to_string(numberOfBytes) + ": ";
    textToAdd += '\n';
    textToAdd += globalText;
    threeAddressCodeText += textToAdd;
    globalText = "";
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
    }
    // Handle if blocks
    else if (tokenValue == "if") {
        scopeCounter = 1;
        handleIfCode(line);
    }
    // Handle else block
    else if (tokenValue == "else") {
        scopes.pop_back(); // Remove the existing IF scope
        scopeCounter = 1;
        
        string textToAdd = NEW_LABEL + " " + label + to_string(labelCounter);
        textToAdd += '\n';
        addCode(textToAdd, scopes.back());
        textToAdd = "";
        
        textToAdd = labelNames.back() + ":";
        textToAdd += '\n';
        textToAdd += operationText;
        labelNames.pop_back();
        labelNames.push_back(label + to_string(labelCounter));
        
        generatedOperationCode.push_back(textToAdd);
        operationText = "";
        scopes.push_back("ELSE");
    }
    // Handle while block
    else if (tokenValue == "while") {
        scopeCounter = 1;
        handleWhileCode(line);
    }
    // Handle closing scopes
    else if (tokenValue == "od") {
        scopes.pop_back();
        scopeCounter--;
        
        // Move this to dedicated function
        string textToAdd = labelNames.back() + ": ";
        textToAdd += '\n';
        textToAdd += operationText;
        labelNames.pop_back();
//        generatedOperationCode.push_back(textToAdd);
        addCode(textToAdd, scopes.back());
        operationText = "";
    } else if (tokenValue == "fi") {
        scopes.pop_back();
        scopeCounter--;
        
        string textToAdd = labelNames.back() + ":";
        textToAdd += '\n';
        textToAdd += operationText;
        labelNames.pop_back();
        generatedOperationCode.push_back(textToAdd);
        operationText = "";
    }
    // Handle function close
    else if (tokenValue == "fed") {
        scopes.pop_back();
        
        string textToAdd;
        textToAdd += '\n';
        textToAdd += functionName + " " + to_string(numberOfBytes) +":";
        textToAdd += '\n';
        textToAdd += BEGIN + ": ";
        textToAdd += '\n';
        for (auto i : generatedOperationCode) {
            functionText += '\n';
            functionText += i;
            functionText += '\n';
        }
        textToAdd += functionText;
        textToAdd += '\n';
        textToAdd += returnLabel + ":";
        textToAdd += '\n';
        textToAdd += "pop {" + FP + "}";
        textToAdd += '\n';
        textToAdd += "pop {PC}";
        textToAdd += '\n';
        textToAdd += '\n';

        threeAddressCodeText += textToAdd;
        
        functionText = "";
        operationText = "";
        generatedOperationCode.clear();
        labelNames.pop_back();
        numberOfBytes = 0;
        functionName = "";
        functionType = "";
        returnLabel = "";
        temporaryVariableCounter = 1;
        
        scopeCounter--;
    }
    // Handle Return statements
    else if (tokenValue == "return") {
        handleReturnCode(line);
        addCode(NEW_LABEL + " " + returnLabel, scopes.back());
        scopeCounter--;
    }
    // Handle print statements
    else if (tokenValue == "print") {
        handlePrintCode(line);
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
    functionNames[funcName] = "GLOBAL";
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
    string generatedCode;
    string currentScope = scopes.back();
    scopes.push_back("IF");
    vector<string> componentsOfIf = { "if", "(", ")", ",", "then" },
    componentsOfCondition = { "and", "or", ">", "<", "==", ">=", "<=", "<>" };
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
        generatedCode += tempOperation;
        j += 2;
        generatedCode += '\n';
    }
    
    for (string op : operations) {
        string labelName = label + to_string(labelCounter);
        labelNames.push_back(labelName);
        if (op == "<")
            generatedCode += LESS_THAN + " " + labelName;
        else if (op == "<=")
            generatedCode += LESS_THAN_EQUAL + " " + labelName;
        else if (op == ">")
            generatedCode += GREATER_THAN + " " + labelName;
        else if (op == ">=")
            generatedCode += GREATER_THAN_EQUAL + " " + labelName;
        else if (op == "==")
            generatedCode += EQUAL + " " + labelName;
        else if (op == "<>")
            generatedCode += NOT_EQUAL + " " + labelName;
        generatedCode += '\n';
        labelCounter++;
        operationLabels.push_back(labelName);
        addCode(generatedCode, currentScope);
    }
}

void ThreeAddressCode::handleWhileCode(vector<Token> line) {
    vector<string> componentsOfIf = { "while", "(", ")", ",", "do" },
    componentsOfCondition = { "and", "or", ">", "<", "==", ">=", "<=", "<>" };
    
    string currentScope = scopes.back();
    scopes.push_back("WHILE");
    
    vector<ScopeVariable> variables;
    string generatedCode;
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
    
    string loopLabel = label + to_string(labelCounter);
    labelCounter++;
    
    generatedCode += loopLabel;
    generatedCode += '\n';
    string tempOperation = COMPARE + " " + variables[j].getVarName() + ", " + variables[j + 1].getVarName();
    
    if (tempOperation != compareOperation) {
        compareOperation = tempOperation;
        generatedCode += tempOperation;
        j += 2;
        generatedCode += '\n';
    }
    
    // Do the opposite reaction to have the break condition
    for (string op : operations) {
        string labelName = label + to_string(labelCounter);
        labelNames.push_back(labelName);
        if (op == "<")
            generatedCode += GREATER_THAN_EQUAL + " " + labelName;
        else if (op == "<=")
            generatedCode += GREATER_THAN + " " + labelName;
        else if (op == ">")
            generatedCode += LESS_THAN_EQUAL + " " + labelName;
        else if (op == ">=")
            generatedCode += LESS_THAN + " " + labelName;
        else if (op == "==")
            generatedCode += NOT_EQUAL + " " + labelName;
        else if (op == "<>")
            generatedCode += EQUAL + " " + labelName;
        generatedCode += '\n';
        labelCounter++;
        operationLabels.push_back(labelName);
        addCode(generatedCode, currentScope);
    }
    
    string endLoopLabel = label + to_string(labelCounter - 1);
    
    generatedCode += endLoopLabel;
    generatedCode += '\n';
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
    
    variableNames[variable.getVarName()] = scopes.back();
    
    addCode(generatedCode, scopes.back());
}

void ThreeAddressCode::handleOperationCode(vector<Token> line) {
    string generatedCode;
    
    string finalVariable = line[0].getRepresentation(); // The variable we will assign everything to in the end
    
    // Recursive call
    if (finalVariable == functionName) {
        handleFunctionCallCode(line);
        return;
    }
    
    string variableType = variableTypes[finalVariable];
    vector<vector<Token>> bracketsOperations; // Brackets
    vector<Token> beginOp = line, endOp;
        
    // We have a math operation in the assignment
    vector<Token> temp;
    for (int i = 2; i < line.size(); i++) {
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
        
        int skipLast = 0;
        if (line.back().getRepresentation() == ";")
            skipLast++;
        
        for (int i = 2; i < line.size() - skipLast; i++) {
            expression.push_back(line[i]);
        }
        string expVal = simplifyMultiplicationOperation(expression, variableType);
        generatedCode += line[0].getRepresentation() + " = " + expVal;
        sp += variableTypes[line[0].getRepresentation()] == "integer" ? 4 : 8;
        generatedCode += '\n';
        numberOfBytes += 4;
    }
    
    addCode(generatedCode, scopes.back());
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
    if (exp.size() > 0) {
        string lastChar = exp.back().getRepresentation();
        if (lastChar == "-" or lastChar == "+" or lastChar == "/" or lastChar == "*")
            completeScan++;
    }
    
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
    
    addCode(generatedCode, scopes.back());
    
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
    
    if (valuesInsideParens.size() > 0) {
        valuesInsideParens.pop_back();

        // This means that we popped a ";"
        if (valuesInsideParens.size() > 0 and valuesInsideParens.back().getRepresentation() == ")")
            valuesInsideParens.pop_back();
    }
    
    if (valuesInsideParens.size() > 1) {
        for (Token token : valuesInsideParens) {
            if (token.getRepresentation() == functionName)
                finalReturn = handleFunctionCallCode(valuesInsideParens);
        }
    } else {
        if (valuesInsideParens.size() > 0)
            finalReturn = valuesInsideParens[0].getRepresentation();
    }
    
    int decrement = functionType == "integer" ? 4 : 8;
    fp -= decrement;
    
    generatedCode += FP + " - " + to_string(decrement) + " = " + finalReturn;
    generatedCode += '\n';
    addCode(generatedCode, scopes.back());
}

string ThreeAddressCode::handleFunctionCallCode(vector<Token> line) {
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
        generatedCode += PUSH + " {" + value + "}";
        generatedCode += '\n';
    }
    
    // Create the branch link
    ScopeVariable variable;
    variable.setScope(scopes.back());
    variable.setType(functionType);
    variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
    temporaryVariableCounter++;
    numberOfBytes += 4;
                    
    // Print temporary variable to symbol table
    printValue(variable.getVarName() + ", function, " + variable.getScope(), 0);

    generatedCode += variable.getVarName() + " = " + BRANCH_LINK + " " + functionName;
    generatedCode += '\n';
    
    // Print the pop stack
    for (auto it = variableStack.rbegin(); it != variableStack.rend(); ++it) {
        generatedCode += POP + " {" + *it + "}";
        generatedCode += '\n';
    }
    
    addCode(generatedCode, scopes.back());
    
    return variable.getVarName();
}

string ThreeAddressCode::handleInplaceDeclerationCode(vector<Token> line, string variableType, string generatedCode) {
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
    
    addCode(generatedCode, scopes.back());
    
    return variable.getVarName();
}

void ThreeAddressCode::handlePrintCode(vector<Token> line) {
    string generatedCode, finalVariable;
    bool functionExpression = false, seenFunction = false;
    int mathOp = 0;
    
    for (Token token : line) {
        string value = token.getRepresentation();
        
        if (value == "+" or value == "-" or value == "*" or value == "/")
            mathOp++;
        
        if (functionNames.find(value) != functionNames.end() and functionNames[value] == scopes.back()) {
            seenFunction = true;
            if (mathOp > 0)
                functionExpression = true;
        }
    }
        
    if (functionExpression) {
        vector<SaveToken> variables, functionVariables;
        deque<CompareToken> values, operations;
        vector<Token> temp;
        int position = 0;
        vector<string> variableStack;
        bool seenFunction = false;
        
        for (Token token : line) {
            string value = token.getRepresentation();
            if (value != "print" and value != ";") {
                if (functionNames.find(value) != functionNames.end() and functionNames[value] == scopes.back()) {
                    if (temp.size() > 0) {
                        SaveToken saveTemp(temp, position);
                        variables.push_back(saveTemp);
                    }
                    temp.clear();
                    position++;
                    seenFunction = true;
                    temp.push_back(token);
                } else if (seenFunction and value == ")") {
                    seenFunction = false;
                    SaveToken saveTemp(temp, position);
                    functionVariables.push_back(saveTemp);
                    position++;
                    temp.clear();
                } else if (seenFunction) {
                    if (value == ")" or value == "," or value == "(") {
                        SaveToken saveTemp(temp, position);
                        functionVariables.push_back(saveTemp);
                        position++;
                        temp.clear();
                    } else if (value != "(")
                        temp.push_back(token);
                } else if (!seenFunction and value != "(" and value != ")")
                    temp.push_back(token);
                else if (!seenFunction and value == "(")
                    position = -1; // We're inside a bracket and make it the top priority
            }
        }
        
        SaveToken saveTemp(temp, position);
        variables.push_back(saveTemp);
        
        for (SaveToken parameter : functionVariables) {
            vector<Token> p = parameter.getValues();
            if (p.size() > 1) {
                // This means that we have a mathematication operation
                string varName = handleInplaceDeclerationCode(p, p[0].getType().toString());
                variableStack.push_back(varName);
            } else if (p.size() == 1) {
                string value = p[0].getRepresentation();
                if (variableNames.find(value) != variableNames.end() and variableNames[value] == scopes.back()) {
                    // We have encountered a variable
                    variableStack.push_back(value);
                } else if (functionNames.find(value) == functionNames.end()) {
                    // Not a function token
                    string varName = handleInplaceDeclerationCode(p, p[0].getType().toString());
                    variableStack.push_back(varName);
                }
            }
        }
        
        finalVariable = variableStack.back();
                
        for (string value : variableStack) {
            generatedCode += PUSH + "{" + value + "}";
            generatedCode += '\n';
        }
        // Create the branch link
        ScopeVariable variable;
        variable.setScope(scopes.back());
        variable.setType(functionType);
        variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
        temporaryVariableCounter++;
        
        // Add temporary variable to final vector
        Token token(variable.getVarName(), variable.getType());
        CompareToken ct(token, functionVariables[0].getPosition());
        values.push_back(ct);
                        
        // Print temporary variable to symbol table
        printValue(variable.getVarName() + ", function, " + variable.getScope(), 0);

        finalVariable = variable.getVarName();
        
        string funcName = functionVariables[0].getValues()[0].getRepresentation();
        generatedCode += variable.getVarName() + " = " + BRANCH_LINK + " " + funcName;
        generatedCode += '\n';
        
        // Print the pop stack
        for (auto it = variableStack.rbegin(); it != variableStack.rend(); ++it) {
            generatedCode += POP + "{" + *it + "}";
            generatedCode += '\n';
        }
        
        // Now that we've created the 3TAC for the function call, we have to piece the expression together
        for (SaveToken v : variables) {
            for (Token token : v.getValues()) {
                CompareToken ct(token, v.getPosition());
                string tokenValue = token.getRepresentation();
                if (tokenValue == "+" or tokenValue == "-" or tokenValue == "/" or tokenValue == "*")
                    operations.push_back(ct);
                else {
                    string varName = handleInplaceDeclerationCode({ token }, token.getType().toString(), generatedCode);
                    generatedCode = "";
                    // Add temporary variable to final vector
                    Token t(varName, token.getType());
                    CompareToken ct(t, v.getPosition());
                    values.push_back(ct);
                }
            }
        }
        
        // Sort the variables and operations by position
        sort(values.begin(), values.end(), [](auto &left, auto &right) {
            return left.getPosition() > right.getPosition();
        });
        
        sort(operations.begin(), operations.end(), [](auto &left, auto &right) {
            return left.getPosition() > right.getPosition();
        });
        
        while (operations.size() > 0) {
            string type = values.front().getToken().getType().toString();
            vector<Token> vec = { values.front().getToken(), operations.front().getToken(), values[1].getToken() };
            string varName = handleInplaceDeclerationCode(vec, type);
            CompareToken ct(Token(varName, type), -1);
            operations.pop_front();
            values.pop_front();
            values.pop_front();
            values.push_front(ct);
        }
    } else {
        vector<string> variableStack;
        vector<vector<Token>> valuesInsideParens;
        vector<Token> temp;
        for (Token token : line) {
            string tokenValue = token.getRepresentation();
            if (tokenValue != "print" and tokenValue != "(" and tokenValue != ")" and tokenValue != "," and tokenValue != ";")
                temp.push_back(token);
            else if (tokenValue == ")" or tokenValue == "," or tokenValue == "(") {
                valuesInsideParens.push_back(temp);
                temp.clear();
            }
        }
        valuesInsideParens.push_back(temp);
        
        bool insideFunction = false;
        string funcName = "";
        
        for (vector<Token> parameter : valuesInsideParens) {
            if (parameter.size() > 1) {
                // This means that we have a mathematication operation
                string varName = handleInplaceDeclerationCode(parameter, parameter[0].getType().toString());
                variableStack.push_back(varName);
            } else if (parameter.size() == 1) {
                string value = parameter[0].getRepresentation();
                if (functionNames.find(value) != functionNames.end() and functionNames[value] == scopes.back()) {
                    // We have encountered a function
                    funcName = value;
                    insideFunction = true;
                } else {
                    if (variableNames.find(value) != variableNames.end() and variableNames[value] == scopes.back()) {
                        // We have encountered a variable
                        variableStack.push_back(value);
                    } else {
                        string varName = handleInplaceDeclerationCode(parameter, parameter[0].getType().toString());
                        variableStack.push_back(varName);
                    }
                }
            }
        }
        
        finalVariable = variableStack.back();
        
        
        if (insideFunction) {
            for (string value : variableStack) {
                generatedCode += PUSH + "{" + value + "}";
                generatedCode += '\n';
            }
            // Create the branch link
            ScopeVariable variable;
            variable.setScope(scopes.back());
            variable.setType(functionType);
            variable.setVarName(temporaryVariable + to_string(temporaryVariableCounter));
            temporaryVariableCounter++;
            numberOfBytes += 4;
                            
            // Print temporary variable to symbol table
            printValue(variable.getVarName() + ", function, " + variable.getScope(), 0);
            
            finalVariable = variable.getVarName();
            
            generatedCode += variable.getVarName() + " = " + BRANCH_LINK + " " + funcName;
            generatedCode += '\n';
            
            // Print the pop stack
            for (auto it = variableStack.rbegin(); it != variableStack.rend(); ++it) {
                generatedCode += POP + "{" + *it + "}";
                generatedCode += '\n';
            }
        }
        
        generatedCode += PRINT + "(" + finalVariable + ")";
        generatedCode += '\n';
        generatedCode += '\n';
    }
    
    addCode(generatedCode, scopes.back());
}

void ThreeAddressCode::addCode(string generatedCode, string scope) {
    if (scope == FUNCTION)
        functionText += generatedCode;
    else if (scope == GLOBAL)
        globalText += generatedCode;
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
    try {
        fstream appendFileToWorkWith;
        string filename = "../output/intermediateCode.txt";
        appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

      // If file does not exist, Create new file
      if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith <<"\n";
        appendFileToWorkWith.close();
       } else {    // use existing file
           appendFileToWorkWith << threeAddressCodeText <<endl;
           appendFileToWorkWith.close();
        }
    } catch (const std::out_of_range& e) {
        cerr << "Out of Range error: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception caught" << endl;
    }
}

void ThreeAddressCode::printSymbolTable() {
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

string ThreeAddressCode::determineType(Token token) {
    if (token.getRepresentation() == "int") return "integer";
    return token.getRepresentation();
}

bool ThreeAddressCode::isVariableType(Token token) {
    if (token.getRepresentation() == "int" or token.getRepresentation() == "double")
        return true;
    return false;
}
