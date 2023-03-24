/*
 File: syntaxTreeNode.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#ifndef SYNTAX_TREE_NODE_H
#define SYNTAX_TREE_NODE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

template<typename ValueType>
class SyntaxTreeNode
{
public:
    SyntaxTreeNode() {}
    SyntaxTreeNode(string inputOp);
    string toString();
    void setValue(ValueType inputVal);
    ValueType getValue();
    class SyntaxProp;
    class Leaf;

private:
    string op;
    SyntaxTreeNode *parent;
    ValueType val;

};

#endif /* syntaxTreeNode_hpp */

