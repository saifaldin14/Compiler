/*
 File: scopeVariable.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-03-30
*/

#include "../../include/semanticAnalysis/scopeVariable.hpp"

/*
 Description:
    Turn the scope variable to a readable string
 
 Parameters:
    - None
 
 Returns:
    - ret: The values of the scope variable as text (string)
*/
string ScopeVariable::toString() {
    string ret;
    
    ret += "[ ";
    ret += "Name: " + varName + ", ";
    ret += "Type: " + type + ", ";
    ret += "Scope: " + scope;
    ret += " ]";
    
    return ret;
}

/*
 Description:
    Clears the values of the scope variable (but not the scope)
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void ScopeVariable::setEmptyValues() {
    varName = "";
    type = "";
}

/*
 Description:
    Clears the scope of the scope varibale
 
 Parameters:
    - None
 
 Returns:
    - None
*/
void ScopeVariable::clearScope() { scope = ""; }
