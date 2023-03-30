/*
 File: parser.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-19
*/

#include "../../include/syntaxAnalysis/parser.hpp"

/*
 Description:
    Set the lexer instance for Parser with a Lexer object.
 
 Parameters:
    - lexCopy: Another Lexer object (Lexer)
*/
void Parser::setLexer(Lexer lexCopy) {
    lex = lexCopy;
}

/*
 Description:
    Initialize the FIRST set for the Parser
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::initializeFIRST() {
    FIRST["program"] = {"def", "int", "double", "if", "while", "print", "return", "ID"};
    FIRST["funcDecl"] = {"def", "EPSILON"};
    FIRST["funcDef"] = {"def"};
    FIRST["funcDefRight"] = {"def", "EPSILON"};
    FIRST["params"] = {"int", "double", "EPSILON"};
    FIRST["paramsRight"] = {",", "EPSILON"};
    FIRST["funcName"] = {"ID"};
    FIRST["declarations"] = {"int", "double", "EPSILON"};
    FIRST["decl"] = {"int", "double"};
    FIRST["declRight"] = {"int", "double", "EPSILON"};
    FIRST["type"] = {"int", "double"};
    FIRST["varlist"] = {"ID"};
    FIRST["varlistRight"] = {",", "EPSILON"};
    FIRST["statementSequence"] = {"if", "while", "print", "return", "ID", "EPSILON"};
    FIRST["statement"] = {"if", "while", "print", "return", "ID", "EPSILON"};
    FIRST["statementSequenceRight"] = {";", "EPSILON"};
    FIRST["optionElse"] = {"else", "EPSILON"};
    FIRST["expr"] = {"ID", "NUMBER", "("};
    FIRST["term"] = {"ID", "NUMBER", "("};
    FIRST["termRight"] = {"+", "-", "EPSILON"};
    FIRST["varRight"] = {"[","EPSILON"};
    FIRST["var"] = {"ID"};
    FIRST["comp"] = {"<", ">", "==", "<=", ">=", "<>"};
    FIRST["branchFactorParen"] = {"(", "not", "ID", "NUMBER", "EPSILON"};
    FIRST["branchFactor"] = {"(", "not"};
    FIRST["branchFactorRight"] = {"and", "EPSILON"};
    FIRST["branchTerm"] = {"(", "not"};
    FIRST["branchTermRight"] = {"or", "EPSILON"};
    FIRST["branchExpression"] = {"(", "not"};
    FIRST["expressionSequenceRight"] = {",", "EPSILON"};
    FIRST["expressionSequence"] = {"(", "ID", "NUMBER"};
    FIRST["factor"] = {"(", "ID", "NUMBER"};
    FIRST["factorRight"] = {"*", "/", "%", "EPSILON"};
    FIRST["factorParen"] = {"(","EPSILON"};
}

/*
 Description:
    Initialize the FOLLOW set for the Parser
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::initializeFOLLOW() {
    FOLLOW["program"] = {"$"};
    FOLLOW["funcDecl"] = {"int", "double", "if", "while", "print", "return", "ID"};
    FOLLOW["funcDef"] = {";"};
    FOLLOW["funcDefRight"] = {";"};
    FOLLOW["params"] = {")"};
    FOLLOW["paramsRight"] = {")"};
    FOLLOW["funcName"] = {"("};
    FOLLOW["declarations"] = {"if","while","print","return","ID"};
    FOLLOW["decl"] = {";"};
    FOLLOW["declRight"] = {";"};
    FOLLOW["type"] = {"ID"};
    FOLLOW["varlist"] = {";", ",", ".", "(", ")", "]", "[", "then", "+", "-", "", "/", "%", "==", "<>", "<", ">"};
    FOLLOW["varlistRight"] = {";", ",", ".", "(", ")", "]", "[", "then", "+", "-", "", "/", "%", "==", "<>", "<", ">"};
    FOLLOW["statementSequence"] = {".", "fed", "fi", "od", "else"};
    FOLLOW["statement"] = {".", ";", "fed", "fi", "od", "else"};
    FOLLOW["statementSequenceRight"] = {".", ";", "fed", "fi", "od", "else"};
    FOLLOW["optionElse"] = {"fi"};
    FOLLOW["expr"] = {".", ";", "fed", "fi", "od", "else", ")", "=", ">", "<", "]",};
    FOLLOW["term"] = {".", ";", "fed", "fi", "od", "else", ")", "=", ">", "<", "]", "+", "-", "", "/",};
    FOLLOW["termRight"] = {".", ";", "fed", "fi", "od", "else", ")", "=", ">", "<", "]", "+", "-", "", "/",};
    FOLLOW["varRight"] = {";", ",", ".", "(", ")", "]", "[", "then", "+", "-", "", "/", "%", "==", "<>", "<", ">"};
    FOLLOW["var"] = {";", ",", ".", "(", ")", "]", "[", "then", "+", "-", "", "/", "%", "==", "<>", "<", ">"};
    FOLLOW["comp"] = {""};
    FOLLOW["branchFactorParen"] = {"then", "do", ")", "or", "and"};
    FOLLOW["branchFactor"] = {"then", "do", ")", "or", "and"};
    FOLLOW["branchFactorRight"] = {"then", "do", ")", "or", "and"};
    FOLLOW["branchTerm"] = {"then", "do", ")", "or", "and"};
    FOLLOW["branchTermRight"] = {"then", "do", ")", "or", "and"};
    FOLLOW["branchExpression"] = {"then", "do", ")", "or"};
    FOLLOW["expressionSequenceRight"] = {")"};
    FOLLOW["expressionSequence"] = {")"};
    FOLLOW["factor"] = {".", ";", "fed", "fi", "od", "else", ")", "=", ">", "<", "]", "+", "-", "", "/",};
    FOLLOW["factorRight"] = {".", ";", "fed", "fi", "od", "else", ")", "=", ">", "<", "]", "+", "-", "", "/",};
    FOLLOW["factorParen"] = {".",";","fed","fi","od","else",")","=",">","<","]","+","-","*","/"};
}

/*
 Description:
    Constructor to initialize the Parser with all necessary instances
 
 Parameters:
    - copy: Instance of Lexer object (Lexer)
 
 Returns:
    - None
*/
Parser::Parser(Lexer copy) {
    setLexer(copy);
    initializeFIRST();
    initializeFOLLOW();
    consumeToken();
    consumeToken(); // Twice to initialize token & lookahead
}

/*
 Description:
    Parse the incoming tokens and build a syntax tree
 
 Parameters:
    - None
 
 Returns:
    - statements: The parsed statements (or epsilon if nothing is parsed) (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::parse() {
    string first = checkFIRST("program");
    SyntaxTreeNode<const char*> epsilon;
    if (!first.empty()) {
        typename SyntaxTreeNode<const char*>::SyntaxProp statements = syntaxTree.makeProp("statementSequence");
        funcDecl();
        declarations();
        statementSequence(statements);
        match(".");
        parsed = true;
        return move(statements);
    } else {
        cout << "ERROR CAUSED 1: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
    return epsilon;
}

/*
 Description:
    Parse the grammar for function declaration
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::funcDecl() {
    string first = checkFIRST("funcDecl");
    if(!first.empty()) {
        funcDef();
        match(";");
        funcDefRight();
    }
}

/*
 Description:
    Parse the grammar for function definition
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::funcDef() {
    string first = checkFIRST("funcDef");
    if(!first.empty()) {
        currentFuncBody = SyntaxTree<const char*>();
        typename SyntaxTreeNode<const char*>::SyntaxProp currentFuncRoot = currentFuncBody.makeProp("statementSequence");
            
        match("def");
        type();
        funcName();
        match("(");
        params();
        match(")");
        declarations();
        statementSequence(currentFuncRoot);
        match("fed");
    }
}

/*
 Description:
    Parse the right-hand side grammar for function declaration
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::funcDefRight() {
    string first = checkFIRST("funcDefRight");
    if(!first.empty()) {
        funcDef();
        match(";");
        funcDefRight();
    }
}

/*
 Description:
    Parse the grammar for the parameters for a function
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::params() {
    string first = checkFIRST("params");
    if (!first.empty()) {
        type();
        var();
        paramsRight();
    }
}

/*
 Description:
    Parse the right-hand side grammar for the parameters of a function
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::paramsRight() {
        string first = checkFIRST("paramsRight");
    if (!first.empty()) {
        match(",");
        params();
    }
}

/*
 Description:
    Parse the grammar for a function name (treat it like a variable)
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::funcName() {
    string first = checkFIRST("funcName");
    if (!first.empty()) {
        currentName = lookahead.getRepresentation();
        currentFuncName = currentName;
        TransitionTableEntry<const char*> entry(currentName, TransitionTableEntry<const char*>::FUNCTION, currentType, NULL);
        lex.transitionTableStates2.addEntry(entry);
        match(TokenType::ID);
    } else {
        cout << "ERROR CAUSED 2: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
}

/*
 Description:
    Parse the grammar for complete statement declarations
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::declarations() {
    string first = checkFIRST("declarations");
    if(!first.empty()) {
        decl();
        match(";");
        declRight();
    }
}

/*
 Description:
    Parse the left-hand side grammar for statement declaration
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::decl() {
    string first = checkFIRST("decl");
    if(!first.empty()) {
        type();
        varlist();
    } else {
        cout << "ERROR CAUSED 3: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
}

/*
 Description:
    Parse the right-hand side grammar for statement declaration
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::declRight() {
    string first = checkFIRST("declRight");
    if(!first.empty()) {
        decl();
        match(";");
        declRight();
    }
}

/*
 Description:
    Parse the grammar for the type of a variable
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::type() {
    string first = checkFIRST("type");
    
    if (first == "int") {
        currentType = TransitionTableEntry<const char*>::INT;
        match("int");
    } else if (first == "double") {
        currentType = TransitionTableEntry<const char*>::DOUBLE;
        match("double");
    } else {
        cout << "ERROR CAUSED 4: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
}

/*
 Description:
    Parse the grammar for the sequence of statements within the code
 
 Parameters:
    - seqList: The sequence list of statements (typename SyntaxTreeNode<const char*>::Interior)
 
 Returns:
    - None
*/
void Parser::statementSequence(typename SyntaxTreeNode<const char*>::SyntaxProp seqList) {
    string first = checkFIRST("statementSequence");
    if(!first.empty()) {
        seqList.addChild(statement());
        statementSequenceRight(seqList);
    }
}

/*
 Description:
    Parse the right-hand side grammar for the sequence of statements within the code
 
 Parameters:
    - seqList: The sequence list of statements (typename SyntaxTreeNode<const char*>::SyntaxProp)
 
 Returns:
    - None
*/
void Parser::statementSequenceRight(typename SyntaxTreeNode<const char*>::SyntaxProp seqList) {
    string first = checkFIRST("statementSequenceRight");
    if(!first.empty()) {
        match(";");
        statementSequence(seqList);
    }
}

/*
 Description:
    Parse the grammar for the complete set of statements for a given snippet
 
 Parameters:
    - None
 
 Returns:
    - None
*/
SyntaxTreeNode<const char*> Parser::statement() {
    string first = checkFIRST("statement");
    SyntaxTreeNode<const char*> varNode, exprNode, branchExpressionNode, elseNode, epsilon;
    typename SyntaxTreeNode<const char*>::SyntaxProp statements;
    
    if (currentFuncBody.toString().empty())
        statements = syntaxTree.makeProp("statementSequence");
    else
        statements = currentFuncBody.makeProp("statementSequence");
    
     if (first == "if" or lookahead.getRepresentation() == "if" or lookahead.getRepresentation() == "fi") {
         if (lookahead.getRepresentation() != "fi") {
             match("if");
             branchExpressionNode = branchExpression();
             match("then");
             statementSequence(statements);
             elseNode = optionElse();
             match("fi");
                     
             if (!elseNode.toString().empty()) {
                 vector<SyntaxTreeNode<const char*>> nodesToReturn = {branchExpressionNode, statements, elseNode};

                 if (currentFuncBody.toString().empty())
                     return syntaxTree.makeProp("if", nodesToReturn);
                 else
                     return currentFuncBody.makeProp("if", nodesToReturn);
             } else {
                 vector<SyntaxTreeNode<const char*>> nodesToReturn = {branchExpressionNode, statements};

                 if (currentFuncBody.toString().empty())
                     return syntaxTree.makeProp("if", nodesToReturn);
                 else
                     return currentFuncBody.makeProp("if", nodesToReturn);
             }
         } else {
             if (!elseNode.toString().empty()) {
                 if (currentFuncBody.toString().empty())
                     return syntaxTree.makeProp("if", {branchExpressionNode, statements, elseNode});
                 else
                     return currentFuncBody.makeProp("if", {branchExpressionNode, statements, elseNode});
             } else {
                 if (currentFuncBody.toString().empty())
                     return syntaxTree.makeProp("if", {branchExpressionNode, statements});
                 else
                     return currentFuncBody.makeProp("if", {branchExpressionNode, statements});
             }
         }
        
    } else if (first == "while" or lookahead.getRepresentation() == "while" or lookahead.getRepresentation() == "od") {
        if (lookahead.getRepresentation() != "od") {
            match("while");
            branchExpressionNode = branchExpression();
            match("do");
            statementSequence(statements);
            match("od");
            vector<SyntaxTreeNode<const char*>> nodesToReturn = {branchExpressionNode, statements};
                            
            if (currentFuncBody.toString().empty())
                return syntaxTree.makeProp("while", nodesToReturn);
            else
                return currentFuncBody.makeProp("while", nodesToReturn);
        } else {
            vector<SyntaxTreeNode<const char*>> nodesToReturn = {branchExpressionNode, statements};
                            
            if (currentFuncBody.toString().empty())
                return syntaxTree.makeProp("while", nodesToReturn);
            else
                return currentFuncBody.makeProp("while", nodesToReturn);
        }
    } else if (first == "print" or lookahead.getRepresentation() == "print") {
        match("print");
        exprNode = expr();
        
        vector<SyntaxTreeNode<const char*>> nodesToReturn = {exprNode};
                        
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("print", nodesToReturn);
        else
            return currentFuncBody.makeProp("print", nodesToReturn);
    } else if (first == "return" or lookahead.getRepresentation() == "return") {
        match("return");
        exprNode = expr();
        vector<SyntaxTreeNode<const char*>> nodesToReturn = {exprNode};
                        
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("return", nodesToReturn);
        else
            return currentFuncBody.makeProp("return", nodesToReturn);
    } else if (lookahead.getRepresentation() == "fed") {
        vector<SyntaxTreeNode<const char*>> nodesToReturn = {exprNode};
                        
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("fed", nodesToReturn);
        else
            return currentFuncBody.makeProp("fed", nodesToReturn);
    } else if (first == "ID") {
        varNode = var();
        match("=");
        exprNode = expr();
        
        vector<SyntaxTreeNode<const char*>> nodesToReturn = {varNode, exprNode};
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("=", nodesToReturn);
        else
            return currentFuncBody.makeProp("=", nodesToReturn);
    } else {
        // Epsilon
        return epsilon;
    }
    // Epsilon
    return epsilon;
}

/*
 Description:
    Parse the varaible list grammar like int a,b,c,d
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::varlist() {
    string first = checkFIRST("varlist");
    
    if (!first.empty()) {
        var();
        varlistRight();
    } else {
        cout << "ERROR CAUSED 5: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
}

/*
 Description:
    Parse the right-hand side grammar for variable list (ensuring a list of varaibles)
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::varlistRight() {
    string first = checkFIRST("varlistRight");
    if (!first.empty()) {
        match(",");
        varlist();
    }
}

/*
 Description:
    Parse the else option grammar for if-conditions
 
 Parameters:
    - None
 
 Returns:
    - statements: The statement sequence for the else block (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::optionElse() {
    string first = checkFIRST("optionElse");
    SyntaxTreeNode<const char*> epsilon;
    
    if (!first.empty()) {
        typename SyntaxTreeNode<const char*>::SyntaxProp statements;
        
        if (currentFuncBody.toString().empty())
            statements = syntaxTree.makeProp("statementSequence");
        else
            statements = currentFuncBody.makeProp("statementSequence");
        
        match("else");
        statementSequence(statements);
        return move(statements);
    } else return epsilon;
}

/*
 Description:
    Parse the expression grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - term: The terminal nodes (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::expr() {
    string first = checkFIRST("expr");
    SyntaxTreeNode<const char*> termNode, termRightNode, epsilon;
    
    if (!first.empty()) {
        termNode = term();
        termRightNode = termRight();
        
        if (!termNode.toString().empty()) {
            typename SyntaxTreeNode<const char*>::SyntaxProp prop = syntaxTree.makeProp(first, {termRightNode});
            if (!prop.toString().empty()) {
                prop.addChild(termNode);
                return termRightNode;
            } else {
                return termNode;
            }
        } else {
            return epsilon;
        }
    } else {
        cout << "ERROR CAUSED 6: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
        return epsilon;
    }

}

/*
 Description:
    Parse the terminal statement grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - factor: The factor nodes (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::term() {
    string first = checkFIRST("term");
    SyntaxTreeNode<const char*> factorNode, factorRightNode, epsilon;
    if (!first.empty()) {
        factorNode = factor();
        factorRightNode = factorRight();
        
        if (!factorNode.toString().empty()) {
            typename SyntaxTreeNode<const char*>::SyntaxProp prop = syntaxTree.makeProp(first, {factorNode});
            if (!prop.toString().empty()) {
                prop.addChild(factorRightNode);
                return factorNode;
            } else if (!prop.toString().empty() && !factorRightNode.toString().empty()) {
                prop.addChild(factorNode);
                return factorRightNode;
            } else if (!factorNode.toString().empty() && !factorRightNode.toString().empty()) {
                return factorRightNode;
            } else {
                return factorNode;
            }
        } else {
            return epsilon;
        }
    } else {
        cout << "ERROR CAUSED 7: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
        return epsilon;
    }

}

/*
 Description:
    Parse the right-hand side terminal statement grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - factor: The factor nodes (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::termRight() {
    string first = checkFIRST("termRight");
    SyntaxTreeNode<const char*> termNode, termRightNode, epsilon;
    if (!first.empty()) {
        if (first == "+") {
            match("+");
            termNode = term();
            termRightNode = termRight();

            if (currentFuncBody.toString().empty()) {
                return syntaxTree.makeProp("+", {termNode, termRightNode});
            } else {
                return currentFuncBody.makeProp("+", {termNode, termRightNode});
            }
        } else if (first == "-") {
            match("-");
            termNode = term();
            termRightNode = termRight();

            if (currentFuncBody.toString().empty()) {
                return syntaxTree.makeProp("-", {termNode, termRightNode});
            } else {
                return currentFuncBody.makeProp("-", {termNode, termRightNode});
            }
        } else {
            cout << "ERROR CAUSED 8: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
            error();
            return epsilon;
        }
    } else {
        return epsilon;
    }
}

/*
 Description:
    Parse the factor grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - node: The resultant factor node (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::factor() {
    string first = checkFIRST("factor");
    SyntaxTreeNode<const char*> exprNode, idNode, funcParams, epsilon;
    if (!first.empty()) {
        if (first == "ID") { // Either a function call or a variable usage
            idNode = match(TokenType::ID);
            funcParams = factorParen();

            if (!funcParams.toString().empty()) {
                if (currentFuncBody.toString().empty())
                    return syntaxTree.makeProp(idNode.toString(), {funcParams});
                else
                    return currentFuncBody.makeProp(idNode.toString(), {funcParams});
            } else {
                return idNode;
            }
        } else if (first == "NUMBER") {
            return match(TokenType::INT); // Technically INT too
        } else if (first == "(") {
            match("(");
            exprNode = expr();
            match(")");
            return exprNode;
        } else if (first == "ID") {
            return var();
        } else {
            cout << "ERROR CAUSED 9: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
            error();
            return epsilon;
        }
    } else {
        cout << "ERROR CAUSED 10: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
        return epsilon;
    }
}

/*
 Description:
    Parse the right-hand side factor grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - node: The resultant factor node (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::factorRight() {
    string first = checkFIRST("factorRight");
    SyntaxTreeNode<const char*> factorNode, factorRightNode, epsilon;
    if (!first.empty()) {
        if (first == "*") {
            match("*");
            factorNode = factor();
            factorRightNode = factorRight();

            if (currentFuncBody.toString().empty()) {
                return syntaxTree.makeProp("*", {factorNode, factorRightNode});
            } else {
                return currentFuncBody.makeProp("*", {factorNode, factorRightNode});
            }
        } else if (first == "/") {
            match("/");
            factorNode = factor();
            factorRightNode = factorRight();

            if (currentFuncBody.toString().empty()) {
                return syntaxTree.makeProp("/", {factorNode, factorRightNode});
            } else {
                return currentFuncBody.makeProp("/", {factorNode, factorRightNode});
            }
        } else if (first == "%") {
            match("%");
            factorNode = factor();
            factorRightNode = factorRight();

            if (currentFuncBody.toString().empty()) {
                return syntaxTree.makeProp("%", {factorNode, factorRightNode});
            } else {
                return currentFuncBody.makeProp("%", {factorNode, factorRightNode});
            }
        } else {
            cout << "ERROR CAUSED 11: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
            error();
            return epsilon;
        }
    } else {
        return epsilon;
    }
}

/*
 Description:
    Parse the factor within parenthesis grammar for statement sequences
 
 Parameters:
    - None
 
 Returns:
    - node: The resultant factor node (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::factorParen() {
    string first = checkFIRST("factorParen");
    SyntaxTreeNode<const char*> epsilon;

    if (!first.empty()) {
        if (first == "(") {
            typename SyntaxTreeNode<const char*>::SyntaxProp params;
            
            if (currentFuncBody.toString().empty())
                params = syntaxTree.makeProp("params");
            else
                params = currentFuncBody.makeProp("params");
            match("(");
            expressionSequence(params);
            match(")");
            return std::move(params);
        } else {
            return epsilon;
        }
    } else {
        return epsilon;
    }
}

/*
 Description:
    Parse the expression sequence grammar
 
 Parameters:
    - params: The parameters within the expression (typename SyntaxTreeNode<const char*>::SyntaxProp)
 
 Returns:
    - None
*/
void Parser::expressionSequence(typename SyntaxTreeNode<const char*>::SyntaxProp params) {
    string first = checkFIRST("expressionSequence");
    SyntaxTreeNode<const char*> exprNode;
    if (!first.empty()) {
        exprNode = expr();
        params.addChild(exprNode);
        expressionSequenceRight(params);
    }
}

/*
 Description:
    Parse the right-hand side expression sequence grammar
 
 Parameters:
    - params: The parameters within the expression (typename SyntaxTreeNode<const char*>::SyntaxProp)
 
 Returns:
    - None
*/
void Parser::expressionSequenceRight(typename SyntaxTreeNode<const char*>::SyntaxProp params) {
    string first = checkFIRST("expressionSequenceRight");
    if (!first.empty()) {
        match(",");
        expressionSequence(params);
    }
}

/*
 Description:
    Parse the branch expression sequence grammar
 
 Parameters:
    - None
 
 Returns:
    - branchTermNode: The terminal node for the branch expression (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchExpression() {
    string first = checkFIRST("branchExpression");
    SyntaxTreeNode<const char*> branchTermNode, branchTermRightNode, epsilon;
    if (!first.empty()) {
        branchTermNode = branchTerm();
        branchTermRightNode = branchTermRight();
        syntaxTree.makeProp(first, {branchTermRightNode});
        return branchTermNode;
    } else {
        return epsilon;
    }
}

/*
 Description:
    Parse the terminal branch expression sequence node grammar
 
 Parameters:
    - None
 
 Returns:
    - branchFactorNode: The factor node for the terminal branch expression (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchTerm() {
    string first = checkFIRST("branchTerm");
    SyntaxTreeNode<const char*> branchFactorNode, branchFactorRightNode, epsilon;
    if (!first.empty()) {
        branchFactorNode = branchFactor();
        branchFactorRightNode = branchFactorRight();
        syntaxTree.makeProp(first, {branchFactorRightNode});
        return branchFactorNode;
    } else {
        cout << "ERROR CAUSED 12: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
        return epsilon;
    }
}

/*
 Description:
    Parse the right-hand side terminal branch expression sequence node grammar
 
 Parameters:
    - None
 
 Returns:
    - branchFactorNode: The factor node for the terminal branch expression (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchTermRight() {
    string first = checkFIRST("branchTermRight");
    SyntaxTreeNode<const char*> branchTermNode, branchTermRightNode, epsilon;
    if (!first.empty()) {
        match("or");
        branchTermNode = branchTerm();
        branchTermRightNode = branchTermRight();
        
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("or", {branchTermNode, branchTermRightNode});
        else
            return currentFuncBody.makeProp("or", {branchTermNode, branchTermRightNode});
    }
    else{
        return epsilon;
    }
}

/*
 Description:
    Parse the factor branch expression sequence node grammar
 
 Parameters:
    - None
 
 Returns:
    - branchFactorParenNode: The branch factor node either terminal or parenthesis (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchFactor() {
    string first = checkFIRST("branchFactor");
    SyntaxTreeNode<const char*> branchFactorParenNode, branchFactorNode, epsilon;
    
    if (first == "(") {
        match("(");
        branchFactorParenNode = branchFactorParen();
        match(")");
        return branchFactorParenNode;
    } else if (first == "not") {
        match("not");
        branchFactorNode = branchFactor();
                        
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("not", {branchFactorNode});
        else
            return currentFuncBody.makeProp("not", {branchFactorNode});
    } else {
        cout << "ERROR CAUSED 13: " << lex.getLineNum() << " " << token.getRepresentation() << endl;

        error();
        return epsilon;
    }
}

/*
 Description:
    Parse the right-hand side factor branch expression sequence node grammar
 
 Parameters:
    - None
 
 Returns:
    - branchFactorParenNode: The branch factor node either terminal or parenthesis (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchFactorRight() {
    string first = checkFIRST("branchFactorRight");
    SyntaxTreeNode<const char*> branchFactorNode, branchFactorRightNode, epsilon;
    if (!first.empty()) {
        match("and"); branchFactorNode = branchFactor();
        branchFactorRightNode = branchFactorRight();
                    
        if (currentFuncBody.toString().empty())
            return syntaxTree.makeProp("and", {branchFactorNode, branchFactorRightNode});
        else
            return currentFuncBody.makeProp("and", {branchFactorNode, branchFactorRightNode});
    } else {
        return epsilon;
    }
}

/*
 Description:
    Parse the factor branch expression sequence within parenthesis node grammar
 
 Parameters:
    - None
 
 Returns:
    - branchFactorParenNode: The branch factor node either terminal or parenthesis (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::branchFactorParen() {
    string first = checkFIRST("branchFactorParen");
    SyntaxTreeNode<const char*> e1Node, e2Node, compNode, epsilon;
    if (find(FIRST["branchFactorParen"].begin(), FIRST["branchFactorParen"].end(), first) != FIRST["branchFactorParen"].end() && token.getType().toString() == TokenType::COMP.toString()) {
        e1Node = expr();
        compNode = comp();
        e2Node = expr();
        syntaxTree.makeProp(first, {e1Node, e2Node});
        return compNode;
    } else if (find(FIRST["branchFactorParen"].begin(), FIRST["branchFactorParen"].end(), first) != FIRST["branchFactorParen"].end()) {
        return branchExpression();
    } else {
        cout << "ERROR CAUSED 14: " << lex.getLineNum() << " " << token.getRepresentation() << endl;

        error();
        return epsilon;
    }
}

/*
 Description:
    Parse the comparison grammar
 
 Parameters:
    - None
 
 Returns:
    - The match node for the comparison operator (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::comp() {
    string first = checkFIRST("comp");
    SyntaxTreeNode<const char*> epsilon;
    if (!first.empty()) {
        return match(TokenType::COMP);
    } else {
        cout << "ERROR CAUSED 15: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
        return epsilon;
    }
}

/*
 Description:
    Parse the variable grammar
 
 Parameters:
    - None
 
 Returns:
    - The match node for the variable token assignment (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::var() {
    string first = checkFIRST("var");
    SyntaxTreeNode<const char*> nodeToReturn, epsilon;
    
    if (!first.empty()) {
        currentName = lookahead.getRepresentation();
        
        if (!currentFuncName.empty()) {
            TransitionTableEntry<const char*> entry(currentName, TransitionTableEntry<const char*>::VARIABLE, currentType, NULL);
            lex.transitionTableStates2.addEntry(entry, currentFuncName);
        } else {
            TransitionTableEntry<const char*> entry(currentName, TransitionTableEntry<const char*>::VARIABLE, currentType, NULL);
            lex.transitionTableStates2.addEntry(entry);
        }
        nodeToReturn = match(TokenType::ID);
        varRight();
        return nodeToReturn;
    }
    else {
        cout << "ERROR CAUSED 16: " << lex.getLineNum() << " " << token.getRepresentation() << endl;

        error();
        return nodeToReturn;
    }
}

/*
 Description:
    Parse the right-hand side variable grammar
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::varRight() {
    string first = checkFIRST("varRight");
    if (!first.empty()) {
        match("[");
        expr();
        match("]");
    }
}

/*
 Description:
    Parse the next token from the lexer
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::consumeToken() {
    lookahead = token;
    try {
        if (!token.toString().empty() || (token.toString().empty() && token.getType().toString() != TokenType::END.toString())) {
            lookahead = token;
            token = lex.getNextToken();
        }
    } catch (const exception& e) {
        cerr << e.what() << '\n';
    }
}

/*
 Description:
    Check the FIRST set to define the grammar operation
 
 Parameters:
    - nonTerminal: The non-terminal lexeme to check (string)
 
 Returns:
    - first: The type of lexeme (string)
*/
string Parser::checkFIRST(string nonTerminal) {
    vector<string> first = FIRST[nonTerminal];
    if (!first.empty()) {
        if (lookahead.getType().toString() == TokenType::ID.toString() && find(first.begin(), first.end(), "ID") != first.end()) {
            return "ID";
        } else if ((lookahead.getType().toString() == TokenType::INT.toString() || lookahead.getType().toString() == TokenType::DOUBLE.toString()) && find(first.begin(), first.end(), "NUMBER") != first.end()) {
            return "NUMBER";
        } else if (find(first.begin(), first.end(), lookahead.getRepresentation()) != first.end()) {
            return lookahead.getRepresentation();
        } else {
            return "";
        }
    } else {
        return "";
    }
}

/*
 Description:
    Get the next token
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::match() { consumeToken(); }

/*
 Description:
    Match the character to the lookahead token
 
 Parameters:
    - c: Character to check (char)
 
 Returns:
    - None
*/
void Parser::match(char c) {
    bool isMatch = lookahead.getRepresentation() == string(1, c);
    if (isMatch) consumeToken();
    else {
        cout << "ERROR CAUSED 17: " << lex.getLineNum() << " " << token.getRepresentation() << endl;
        error();
    }
}

/*
 Description:
    Match the string to the lookahead token
 
 Parameters:
    - s: String to check (string)
 
 Returns:
    - None
*/
void Parser::match(string s) {
    bool isMatch = lookahead.getRepresentation() == s;
    if (isMatch) {
        if (s == "fed") {
            lex.transitionTableStates2.updateValue(currentFuncName, currentFuncBody.toString().c_str());
            currentFuncName = "";
            currentFuncBody.setEmpty();
        }
        consumeToken();
    } else {
        cout << "ERROR CAUSED 18: " << lex.getLineNum() << " " << lookahead.getRepresentation() << ", " << s << endl;
        error();
    }
}

/*
 Description:
    Match the token type to the lookahead token
 
 Parameters:
    - type: Token type to check (TokenType)
 
 Returns:
    - node: The SyntaxTree node (SyntaxTreeNode<const char*>)
*/
SyntaxTreeNode<const char*> Parser::match(TokenType type) {
    bool isMatch = false;
    SyntaxTreeNode<const char*> node, epsilon;
    if (type.toString() == TokenType::INT.toString()) {
        isMatch = true;
        
        if (lookahead.getType().toString() == TokenType::INT.toString()) {
            if (currentFuncBody.toString().empty())
                node = syntaxTree.makeLeaf(lookahead.getRepresentation(), lookahead.getRepresentation().c_str());
            else
                node = currentFuncBody.makeLeaf(lookahead.getRepresentation(), lookahead.getRepresentation().c_str());
        } else if (lookahead.getType().toString() == TokenType::DOUBLE.toString()) {
            if (currentFuncBody.toString().empty())
                node = syntaxTree.makeLeaf(lookahead.getRepresentation(), lookahead.getRepresentation().c_str());
            else
                node = currentFuncBody.makeLeaf(lookahead.getRepresentation(), lookahead.getRepresentation().c_str());
        }
    } else if (type.type == TokenType::ID.type){
        isMatch = true;
        
        TransitionTableEntry<const char*> entry = lex.transitionTableStates2.getEntry(lookahead.getRepresentation());
        node = syntaxTree.makeLeaf(lookahead.getRepresentation(), entry.toString().c_str());
    } else if (type.type == TokenType::COMP.type){
        isMatch = true;
        
        if (currentFuncBody.toString().empty())
            node = syntaxTree.makeProp(lookahead.getRepresentation());
        else
            node = currentFuncBody.makeProp(lookahead.getRepresentation());
    } else {
        isMatch = type.toString() == lookahead.toString();
    }

    if (isMatch) {
        consumeToken();
    } else {
        cout << "ERROR CAUSED 19: " << lex.getLineNum() << " " << token.getRepresentation() << endl;

        error();
    }

    return node;
}

/*
 Description:
    Write parse error stack to file
 
 Parameters:
    - appendFileToWorkWith: The file to write to (fstream&)
 
 Returns:
    - None
*/
void Parser::writeStackTrace(fstream& appendFileToWorkWith) {
    appendFileToWorkWith << "PARSE ERROR" << endl;

    stringstream ss;
    ss << "Parsing error on Line " << lex.getLineNum() << " at token " << lookahead.getRepresentation();
    appendFileToWorkWith << ss.str() << endl;
    
    vector<string> stackTraceElements;
    const int maxStackTraceDepth = 50;
    void *stackTrace[maxStackTraceDepth];
    int stackTraceDepth = backtrace(stackTrace, maxStackTraceDepth);
    char **stackTraceSymbols = backtrace_symbols(stackTrace, stackTraceDepth);
    for (int i = 0; i < stackTraceDepth; i++) {
        stackTraceElements.push_back(stackTraceSymbols[i]);
    }
    free(stackTraceSymbols);
    appendFileToWorkWith << "StackTrace: " << endl;
    for (const auto& elem : stackTraceElements) {
        appendFileToWorkWith << elem << endl;
    }
}

/*
 Description:
    Write complete parse error file
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::error() {
    fstream appendFileToWorkWith;
    string filename = "../output/error.txt";
    appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    
    // Print errors to file
    if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        writeStackTrace(appendFileToWorkWith);
        
        appendFileToWorkWith.close();
    } else {    // use existing file
        writeStackTrace(appendFileToWorkWith);
        
        appendFileToWorkWith.close();
    }
    cout << "PARSE ERROR" << endl;
    stringstream ss;
    ss << "Error on Line " << lex.getLineNum() << " at token " << lookahead.getRepresentation();
    cout << ss.str() << endl;
    vector<string> stackTraceElements;
    const int maxStackTraceDepth = 50;
    void *stackTrace[maxStackTraceDepth];
    int stackTraceDepth = backtrace(stackTrace, maxStackTraceDepth);
    char **stackTraceSymbols = backtrace_symbols(stackTrace, stackTraceDepth);
    for (int i = 0; i < stackTraceDepth; i++) {
        stackTraceElements.push_back(stackTraceSymbols[i]);
    }
    free(stackTraceSymbols);
    cout << "StackTrace: " << endl;
    for (const auto& elem : stackTraceElements) {
        cout << elem << endl;
    }
        
//    exit(EXIT_FAILURE);
}

/*
 Description:
    Print the resulting SyntaxTree after parsing
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void Parser::printSyntaxTree() {
    cout << "Syntax Tree:" << endl;
    for (auto i : syntaxTree.getTraversalList())
        cout << i.toString() << ", Type: " << endl;
}

/*
 Description:
    Return the lines table.
 
 Parameters:
    - None
 
 Returns:
    - lines: The table of read lines tokens (vector<vector<Token>>)
*/
vector<vector<Token>> Parser::getLines() { return lex.getLines(); }
