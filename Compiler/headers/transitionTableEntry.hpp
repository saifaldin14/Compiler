/*
 File: transitionTableEntry.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
 */

#ifndef TRANSITION_TABLE_ENTRY_H
#define TRANSITION_TABLE_ENTRY_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "tokenType.hpp"

using namespace std;

template<typename ValueType>
class TransitionTableEntry
{
public:
    inline static string INT = "int";
    inline static string DOUBLE = "double";
    inline static string VARIABLE = "variable";
    inline static string FUNCTION = "function";
    inline static string CONSTANT = "constant";
    inline static string KEYWORD = "keyword";
    inline static string OPERATION = "operation";
    inline static string TERMINAL = "terminal";

    TransitionTableEntry();
    TransitionTableEntry(string n, string i, string d, ValueType v);
    
    string getName();
    string getIdType();
    string toString();
    
    void setValue(ValueType inputVal);
    void setParams(string n, string i, string d, ValueType v);
    
    ValueType getValue();
    bool equals(TransitionTableEntry otherEntry);
    
    
private:
    string name, idType, dataType;
    string printValue;
    ValueType value; // Possible result of computation
};

#endif /* transitionTableEntry_hpp */
