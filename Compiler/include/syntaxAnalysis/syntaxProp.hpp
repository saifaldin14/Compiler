//
//  interior.hpp
/*
 File: syntaxProp.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#ifndef SYNTAX_PROP_H
#define SYNTAX_PROP_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "syntaxTreeNode.hpp"

using namespace std;

template<typename ValueType>
class SyntaxTreeNode<ValueType>::SyntaxProp : public SyntaxTreeNode<ValueType>
{
public:
    SyntaxProp() {}
    SyntaxProp(string inputOp);
    SyntaxProp(string inputOp, vector<SyntaxTreeNode<ValueType>> inputChildren);
    void addChild(SyntaxTreeNode<ValueType> child);
    SyntaxTreeNode<ValueType> getChild(int index);
    int numChildren();
    
private:
    vector<SyntaxTreeNode<ValueType>> children;
    
};

#endif /* syntaxProp_hpp */
