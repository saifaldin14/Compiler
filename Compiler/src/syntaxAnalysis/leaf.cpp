/*
 File: leaf.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#include "../../include/syntaxAnalysis/leaf.hpp"

/*
 Description:
    Constructor to initialize a Syntax Leaf object
 
 Parameters:
    - op: Operation to build the property from (string)
    - inputVal: Value to build the object from (ValueType)
 */
template<typename ValueType>
SyntaxTreeNode<ValueType>::Leaf::Leaf(string inputOp, ValueType inputVal): SyntaxTreeNode(inputOp) {
    this->setValue(inputVal);
}

template class SyntaxTreeNode<double>::Leaf;
template class SyntaxTreeNode<int>::Leaf;
template class SyntaxTreeNode<const char*>::Leaf;
