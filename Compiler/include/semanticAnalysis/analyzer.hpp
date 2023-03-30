/*
 File: analyzer.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-30
*/

#ifndef ANALYZER_H
#define ANALYZER_H

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

class Analyzer
{
public:
    Analyzer(vector<vector<Token>> inputLines);
    

private:
    vector<vector<Token>> lines;
};
#endif /* analyzer_hpp */
