//
//  scopeVariable.cpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-03-30.
//

#include "../../include/semanticAnalysis/scopeVariable.hpp"

string ScopeVariable::toString() {
    string ret;
    
    ret += "[ ";
    ret += "Name: " + varName + ", ";
    ret += "Type: " + type + ", ";
    ret += "Scope: " + scope;
    ret += " ]";
    
    return ret;
}

void ScopeVariable::setEmptyValues() {
    varName = "";
    type = "";
}

void ScopeVariable::clearScope() { scope = ""; }
