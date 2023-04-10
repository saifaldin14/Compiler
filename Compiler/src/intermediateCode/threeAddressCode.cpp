//
//  threeAddressCode.cpp
//  Compiler
//
//  Created by Saif Al-Din Ali on 2023-04-10.
//

#include "../../include/intermediateCode/threeAddressCode.hpp"

ThreeAddressCode::ThreeAddressCode(vector<vector<Token>> inputLines) {
    lines = inputLines;
    scopes.push_back("GLOBAL");
    printString += "GLOBAL SCOPE";
    printString += '\n';
}
