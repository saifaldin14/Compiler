/*
 File: syntaxTreeNode.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#include "../../headers/syntaxAnalysis/syntaxTreeNode.hpp"

/*
 Description:
    Constructor to initialize a Syntax Tree Node
 
 Parameters:
    - op: Operation to build node from (string)
 */
template<typename ValueType>
SyntaxTreeNode<ValueType>::SyntaxTreeNode(string inputOp) { op = inputOp; }

/*
 Description:
    Get the operation of the node in a human readable string
 
 Parameters:
    - None
 
 Returns:
    - op: The operation of the node (string)
 */
template<typename ValueType>
string SyntaxTreeNode<ValueType>::toString() { return op; }

/*
 Description:
    Set the value of the node
 
 Parameters:
    - inputVal: Value to add to the node (ValueType)
 
 Returns:
    - None
 */
template<typename ValueType>
void SyntaxTreeNode<ValueType>::setValue(ValueType inputVal) { val = inputVal; }

/*
 Description:
    Retrieve the value of the node
 
 Parameters:
    - None
 
 Returns:
    - val: Value of the node (ValueType)
 */
template<typename ValueType>
ValueType SyntaxTreeNode<ValueType>::getValue() { return val; }

template class SyntaxTreeNode<double>;
template class SyntaxTreeNode<int>;
template class SyntaxTreeNode<const char*>;
