/*
 File: leaf.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#ifndef LEAF_H
#define LEAF_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "syntaxTreeNode.hpp"

using namespace std;

template<typename ValueType>
class SyntaxTreeNode<ValueType>::Leaf : public SyntaxTreeNode<ValueType>
{
public:
    Leaf(string inputOp, ValueType inputVal);
};

#endif /* leaf_hpp */
