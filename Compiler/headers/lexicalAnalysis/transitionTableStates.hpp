/*
 File: transitionTableStates.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/

#ifndef TRANSITION_TABLE_STATES_H
#define TRANSITION_TABLE_STATES_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "transitionTableNode.hpp"
#include "syntaxTree.hpp"

using namespace std;

template<typename ValueType>
class TransitionTableStates
{
public:
    TransitionTableStates* getInstance();
    
    TransitionTableStates();
    
    void addEntry(TransitionTableEntry<ValueType> entry);
    void addEntry(TransitionTableEntry<ValueType> entry, string atFunction);
    void updateValue(string variableName, ValueType value);
    void printTransitionTables();
    void printTransitionTablesWithoutHeaders();
    
    TransitionTableEntry<ValueType> getEntry();
    TransitionTableEntry<ValueType> getEntry(string nameToFind);
    
    bool checkIfValExists(TransitionTableEntry<ValueType> entry);
    
private:
    inline static TransitionTableNode<ValueType> globalTable = TransitionTableNode<ValueType>();
    inline static TransitionTableStates instance = TransitionTableStates();

    TransitionTableNode<ValueType> locateFunctionTable(string functionName);
    TransitionTableNode<ValueType> transitionTableNode;
    TransitionTableEntry<ValueType> transitionTableEntry;
};

#include "../../src/lexicalAnalysis/transitionTableNode.cpp"
#include "../../src/lexicalAnalysis/transitionTableEntry.cpp"

#endif /* transitionTableStates_hpp */

