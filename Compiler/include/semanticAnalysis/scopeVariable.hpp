//
//  scopeVariable.hpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-03-30.
//

#ifndef SCOPE_VARIABLE_H
#define SCOPE_VARIABLE_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <execinfo.h>
#include <unordered_map>
#include "../tokens/token.hpp"

using namespace std;

class ScopeVariable
{
public:
    ScopeVariable() {}
    ScopeVariable(string inputVarName, string inputScope, string inputType);
    
    string getVarName() { return varName; }
    string getScope() { return scope; }
    string getType() { return type; }
    
    void setVarName(string input) { varName = input; }
    void setScope(string input) { scope = input; }
    void setType(string input) { type = input; }

    
    string toString();

private:
    string varName;
    string scope;
    string type;
};

#endif /* scopeVariable_hpp */
