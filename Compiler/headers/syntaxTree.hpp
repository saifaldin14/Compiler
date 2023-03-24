/*
 File: syntaxTree.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "syntaxTreeNode.hpp"
#include "leaf.hpp"
#include "syntaxProp.hpp"

using namespace std;

template<typename ValueType>
class SyntaxTree
{
public:
    SyntaxTree() {}
    typename SyntaxTreeNode<ValueType>::SyntaxProp makeProp(string op);
    typename SyntaxTreeNode<ValueType>::SyntaxProp makeProp(string op, vector<SyntaxTreeNode<ValueType>> children);
    typename SyntaxTreeNode<ValueType>::Leaf makeLeaf(string op, ValueType lexValue);
    vector<SyntaxTreeNode<ValueType>> getTraversalList();
    string toString();
    void setEmpty();
    

private:
    vector<SyntaxTreeNode<ValueType>> traversalList;
    SyntaxTreeNode<ValueType> root;
    void buildListPostorder(SyntaxTreeNode<ValueType> startPoint);
};

#endif /* syntaxTree_hpp */
