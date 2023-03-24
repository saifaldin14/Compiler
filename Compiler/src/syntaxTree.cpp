/*
 File: syntaxTree.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-01
 */

#include "../headers/syntaxTree.hpp"

/*
 Description:
    Make a SyntaxProp instance from an operation
 
 Parameters:
    - op: Operation to build node from (string)
 
 Returns:
    - node: The built node (typename SyntaxTreeNode<ValueType>::SyntaxProp)
 */
template<typename ValueType>
typename SyntaxTreeNode<ValueType>::SyntaxProp SyntaxTree<ValueType>::makeProp(string op) {
    typename SyntaxTreeNode<ValueType>::SyntaxProp node(op);
    if (root.toString().empty()) root = node;
    traversalList.push_back(node);
    return node;
}

/*
 Description:
    Make a SyntaxProp instance from an operation and children
 
 Parameters:
    - op: Operation to build node from (string)
    - children: SyntaxTreeNode children to add (vector<SyntaxTreeNode<ValueType>>)
 
 Returns:
    - node: The built node (typename SyntaxTreeNode<ValueType>::SyntaxProp)
 */
template<typename ValueType>
typename SyntaxTreeNode<ValueType>::SyntaxProp SyntaxTree<ValueType>::makeProp(string op, vector<SyntaxTreeNode<ValueType>> children) {
    typename SyntaxTreeNode<ValueType>::SyntaxProp node(op, children);
    if (root.toString().empty()) root = node;
    traversalList.push_back(node);
    return node;
}

/*
 Description:
    Make a Leaf node instance from an operation and a lexer value
 
 Parameters:
    - op: Operation to build node from (string)
    - lexValue: Lexer value to build from (ValueType)
 
 Returns:
    - node: The built node (typename SyntaxTreeNode<ValueType>::Leaf)
 */
template<typename ValueType>
typename SyntaxTreeNode<ValueType>::Leaf SyntaxTree<ValueType>::makeLeaf(string op, ValueType lexValue) {
    typename SyntaxTreeNode<ValueType>::Leaf node(op, lexValue);
    traversalList.push_back(node);
    return node;
}

/*
 Description:
    Make a Post Order traversal list from props
 
 Parameters:
    - startPoint: Begining point to build the traversal list from (SyntaxTreeNode<ValueType>)
 
 Returns:
    - None
 */
template<typename ValueType>
void SyntaxTree<ValueType>::buildListPostorder(SyntaxTreeNode<ValueType> startPoint) {
    if (!startPoint.toString().empty()) return;
    auto prop = makeProp(startPoint.toString());
    
    if (prop.numChildren() != 0) {
        for (int i = 0; i < prop.numChildren(); i++) {
            buildListPostorder(prop.getChild(i));
        }
    }
    
    traversalList.push_back(startPoint);
}

/*
 Description:
    Getter function for the traversal list
 
 Parameters:
    - None
 
 Returns:
    - Copy of traversal list (vector<SyntaxTreeNode<ValueType>>)
 */
template<typename ValueType>
vector<SyntaxTreeNode<ValueType>> SyntaxTree<ValueType>::getTraversalList() {
    if (traversalList.empty()) buildListPostorder(root);
    return traversalList;
}

/*
 Description:
    Save value to string
 
 Parameters:
    - v: Any value (ValueType)
 
 Returns:
    - String of content
 */
template <typename ValueType>
string to_string_value2(const ValueType& v){
    stringstream ss;
    ss << &v;
    return ss.str();
}

/*
 Description:
    Save the contents of the syntax tree to a human-readable text
 
 Parameters:
    - None
 
 Returns:
    - The converted text of the entry values (string)
 */
template<typename ValueType>
string SyntaxTree<ValueType>::toString() {
    string ret = "";
    for (SyntaxTreeNode<ValueType> child : traversalList) {
        ret += to_string_value2(child) + " ";
    }
    return ret;
}

/*
 Description:
    Set the contents of the syntax tree to NULL
 
 Parameters:
    - None
 
 Returns:
    - None
 */
template<typename ValueType>
void SyntaxTree<ValueType>::setEmpty() {
    traversalList.clear();
    root = SyntaxTreeNode<ValueType>();
}

template class SyntaxTree<double>;
template class SyntaxTree<int>;
template class SyntaxTree<const char*>;
