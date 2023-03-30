/*
 File: reserved.hpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-24
*/

#ifndef RESERVED_H
#define RESERVED_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "id.hpp"

using namespace std;

class Reserved : public Id
{
public:
    Reserved(string n, TokenType t, int o) : Id(n, t, o) {}
    string toString() {
        return "< " + getRepresentation() + " >";
    }
};

#endif /* reserved_hpp */
