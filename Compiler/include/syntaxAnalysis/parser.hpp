/*
 File: parser.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-19
*/

#ifndef PARSER_H
#define PARSER_H

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

class Parser
{
public:
    Parser(Lexer copy);
    SyntaxTreeNode<const char*> parse();
    
    bool getParsed() { return parsed; }
    vector<vector<Token>> getLines();

    void setLexer(Lexer lexCopy);
    void error();
    void printSyntaxTree();
    void funcDecl();
    void funcDef();
    void funcDefRight();
    void params();
    void paramsRight();
    void funcName();
    void declarations();
    void decl();
    void declRight();
    void type();
    void varlist();
    void varlistRight();
    void varRight();
    
    // Overloaded Match Functions
    void match();
    void match(char c);
    void match(string s);
    SyntaxTreeNode<const char*> match(TokenType type);
    
    void consumeToken();
    
    void statementSequence(typename SyntaxTreeNode<const char*>::SyntaxProp seqList);
    void statementSequenceRight(typename SyntaxTreeNode<const char*>::SyntaxProp seqList);
    void expressionSequence(typename SyntaxTreeNode<const char*>::SyntaxProp params);
    void expressionSequenceRight(typename SyntaxTreeNode<const char*>::SyntaxProp params);

    SyntaxTreeNode<const char*> statement();
    SyntaxTreeNode<const char*> optionElse();
    SyntaxTreeNode<const char*> expr();
    SyntaxTreeNode<const char*> term();
    SyntaxTreeNode<const char*> termRight();
    SyntaxTreeNode<const char*> factor();
    SyntaxTreeNode<const char*> factorRight();
    SyntaxTreeNode<const char*> factorParen();
    SyntaxTreeNode<const char*> branchExpression();
    SyntaxTreeNode<const char*> branchTerm();
    SyntaxTreeNode<const char*> branchTermRight();
    SyntaxTreeNode<const char*> branchFactor();
    SyntaxTreeNode<const char*> branchFactorRight();
    SyntaxTreeNode<const char*> branchFactorParen();
    SyntaxTreeNode<const char*> comp();
    SyntaxTreeNode<const char*> var();
    
    string checkFIRST(string nonTerminal);
    
    Lexer getLexer() { return lex; }

private:
    Lexer lex;
    Token lookahead;
    Token token;
    
    SyntaxTree<const char*> syntaxTree;

    unordered_map<string, vector<string>> FIRST;
    unordered_map<string, vector<string>> FOLLOW;
    string currentName, currentFuncName, currentType, currentValue;
    SyntaxTree<const char*> currentFuncBody;
    
    bool parsed = true;
    
    void initializeFIRST();
    void initializeFOLLOW();
    
    void writeStackTrace(fstream& appendFileToWorkWith);
};
#endif /* parser_hpp */
