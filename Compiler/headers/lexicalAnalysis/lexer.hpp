/*
 File: lexer.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-15
*/

#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "../tokens/token.hpp"
#include "../tokens/tokenType.hpp"
#include "../tokens/id.hpp"
#include "../tokens/reserved.hpp"
#include "transitionTableStates.hpp"
#include "transitionTableEntry.hpp"

using namespace std;

class Lexer
{
public:
    Lexer();
    Lexer(string fileContents);
    Lexer(const Lexer& other);
    Lexer& operator=(const Lexer& other);
    
    Token getNextToken();
    
    TransitionTableStates<double> transitionTableStates{TransitionTableStates<double>()};
    TransitionTableStates<const char*> transitionTableStates2{TransitionTableStates<const char*>()};
    int getLineNum();
    vector<string> getSymbols();
    void setLexer(TransitionTableStates<double> t1, TransitionTableStates<const char*> t2, vector<string> inputSymbols);

private:
    inline static const vector<string> RESERVED = {
        "if","then","else","fi","while","do","od","def","fed","int","double","print","return","or","and","not"
    };
    
    inline static const vector<char> TERMINALS = {
        ';',',','(',')','[',']','+','-','*','/','%','.','=','>','<'
    };
    
    inline static const vector<char> WHITESPACE = { ' ','\t','\n' };
    
    static bool isLetter (char c);
    static bool isDigit (char c);
    
    const char end = '.';
    int lineNum = 1;
    char next = ' ';
    int sourceFileIndex;
    string source;
    vector<string> symbols;
    
    string recover(string token);
    void saveErrorText(char c);
    void addEntryToTable(string n, string i, string d, double v);
    void addEntryToTable(string n, string i, string d, int v);
    void addEntryToTable(string n, string i, string d, const char* v);
    void reserve();
    void readChar();
    bool readChar(char c);
};

#endif /* lexer_hpp */
