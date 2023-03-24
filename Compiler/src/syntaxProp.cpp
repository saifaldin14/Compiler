/*
 File: syntaxProp.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#include "../headers/syntaxProp.hpp"

/*
 Description:
    Constructor to initialize a Syntax Property object
 
 Parameters:
    - op: Operation to build the property from (string)
 */
template<typename ValueType>
SyntaxTreeNode<ValueType>::SyntaxProp::SyntaxProp(string inputOp): SyntaxTreeNode(inputOp)  {
    children = vector<SyntaxTreeNode>();
}

/*
 Description:
    Constructor to initialize a Syntax Property object from exisiting Syntax Nodes
 
 Parameters:
    - op: Operation to build the property from (string)
    - inputChildren: Existing Syntax Nodes to build the property from (vector<SyntaxTreeNode<ValueType>>)
 */
template<typename ValueType>
SyntaxTreeNode<ValueType>::SyntaxProp::SyntaxProp(string inputOp, vector<SyntaxTreeNode<ValueType>> inputChildren): SyntaxTreeNode(inputOp)  {
    children = vector<SyntaxTreeNode>();
    for (SyntaxTreeNode currentChild : inputChildren) {
        if (!currentChild.toString().empty()) {
            currentChild.parent = this->parent;
            children.push_back(currentChild);
        }
    }
}

/*
 Description:
    Add a node to the children of the property
 
 Parameters:
    - child: Node to add (SyntaxTreeNode<ValueType>)
 
 Returns:
    - None
 */
template<typename ValueType>
void SyntaxTreeNode<ValueType>::SyntaxProp::addChild(SyntaxTreeNode<ValueType> child) {
    children.push_back(child);
}

/*
 Description:
    Get a specific child from the property
 
 Parameters:
    - index: Index to find the child from
 
 Returns:
    - Found child (SyntaxTreeNode<ValueType>)
 */
template<typename ValueType>
SyntaxTreeNode<ValueType> SyntaxTreeNode<ValueType>::SyntaxProp::getChild(int index) {
    return children[index];
}

/*
 Description:
    Get the number of children of the property
 
 Parameters:
    - None
 
 Returns:
    - Number of children (int)
 */
template<typename ValueType>
int SyntaxTreeNode<ValueType>::SyntaxProp::numChildren() {
    return (int)children.size();
}

template class SyntaxTreeNode<double>::SyntaxProp;
template class SyntaxTreeNode<int>::SyntaxProp;
template class SyntaxTreeNode<const char*>::SyntaxProp;
