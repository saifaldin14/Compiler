/*
 File: transitionTableNode.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/

#ifndef TRANSITION_TABLE_NODE_H
#define TRANSITION_TABLE_NODE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "transitionTableEntry.hpp"

using namespace std;

template<typename ValueType>
class TransitionTableNode
{
public:
    static string GLOBAL_SCOPE;
    
    vector<TransitionTableNode> children;
    vector<TransitionTableEntry<ValueType>> transitionTable;
    string transitionTableName;
    
    TransitionTableNode();
    TransitionTableNode(TransitionTableNode parentTable, string functionName);
    
    TransitionTableNode clone();
    TransitionTableEntry<ValueType> get(string name);
    
    void printTable();
    void printTableWithoutHeader();
    void add(TransitionTableEntry<ValueType> entryToAdd);
private:
    TransitionTableNode *parent;
    string op;
    ValueType val; // Possible result of computation
    
};

#endif /* transitionTableNode_hpp */
