/*
 File: transitionTableStates.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/

#include "../../include/lexicalAnalysis/transitionTableStates.hpp"

/*
 Description:
    Default constructor of the transition table states and initlize empty values.
 
 Parameters:
    - None
*/
template<typename ValueType>
TransitionTableStates<ValueType>::TransitionTableStates() {
    transitionTableNode = TransitionTableNode<ValueType>();
    transitionTableEntry = TransitionTableEntry<ValueType>();
};

/*
 Description:
    Get the current instance of the transition table state.
 
 Parameters:
    - None

 Returns:
    - A pointer to the current instance of the transition table state (TransitionTableStates<ValueType>*)
*/
template<typename ValueType>
TransitionTableStates<ValueType>* TransitionTableStates<ValueType>::getInstance() {
    return &instance;
}

/*
 Description:
    Get the current transition table of a child node from a cloned table.
 
 Parameters:
    - functionName: The name of the table to be found (string)
 
 Returns:
    - The transition table node of the found function (TransitionTableNode<ValueType>)
*/
template<typename ValueType>
TransitionTableNode<ValueType> TransitionTableStates<ValueType>::locateFunctionTable(string functionName) {
    TransitionTableNode<ValueType> currentNode = globalTable.clone();
    
    for (int i = 0; i < currentNode.children.size(); i++) {
        if (currentNode.children[i].transitionTableName == functionName) {
            return currentNode.children[i];
        }
    }
            
    throw runtime_error("Function table not found.");
}

/*
 Description:
    Print the transition table of the children nodes (with header).
 
 Parameters:
    - None

 Returns:
    - None
*/
template<typename ValueType>
void TransitionTableStates<ValueType>::printTransitionTables() {
    globalTable.printTable();
    
    for (int i = 0; i < globalTable.children.size(); i++) {
        cout << endl;
        globalTable.children[i].printTable();
    }
}

/*
 Description:
    Print the transition table of the children nodes (without header).
 
 Parameters:
    - None

 Returns:
    - None
*/
template<typename ValueType>
void TransitionTableStates<ValueType>::printTransitionTablesWithoutHeaders() {
    globalTable.printTableWithoutHeader();
    
    for (int i = 0; i < globalTable.children.size(); i++) {
        cout << endl;
        globalTable.children[i].printTableWithoutHeader();
    }
}

/*
 Description:
    Check if a particular entry exists in the global table (used to avoid duplicate entries in the table).
 
 Parameters:
    - entry: The entry to be checked if it exisits (TransitionTableEntry<ValueType>)
 
 Returns:
    - Whether or not the entry already exists (bool)
*/
template<typename ValueType>
bool TransitionTableStates<ValueType>::checkIfValExists(TransitionTableEntry<ValueType> entry) {
    for (auto e : globalTable.transitionTable) {
        if (e.getValue() == entry.getValue()) {
            return true;
        }
    }
    
    return false;
}

/*
 Description:
    Add an entry to the default global table.
 
 Parameters:
    - entry: The entry to be added (TransitionTableEntry<ValueType>)
 
 Returns:
    - None
*/
template<typename ValueType>
void TransitionTableStates<ValueType>::addEntry(TransitionTableEntry<ValueType> entry) {
    try {
        if (entry.getIdType() == TransitionTableEntry<ValueType>::FUNCTION) {
            globalTable.add(entry);
            
            TransitionTableNode<ValueType> functionChildTable = TransitionTableNode<ValueType>(globalTable, entry.getName());
            
            globalTable.children.push_back(functionChildTable);
        } else if (entry.getIdType() == TransitionTableEntry<ValueType>::VARIABLE || entry.getIdType() == TransitionTableEntry<ValueType>::CONSTANT || entry.getIdType() == TransitionTableEntry<ValueType>::KEYWORD || entry.getIdType() == TransitionTableEntry<ValueType>::OPERATION || entry.getIdType() == TransitionTableEntry<ValueType>::TERMINAL) {
            
            globalTable.add(entry);
        }
    }
    catch (exception& e) {
        cout << "Error occurred: " << e.what() << endl;
    }
}

/*
 Description:
    Add an entry to the transition table at a specific function name.
 
 Parameters:
    - functionName: The table name to find (string)
 
 Returns:
    - None
*/
template<typename ValueType>
void TransitionTableStates<ValueType>::addEntry(TransitionTableEntry<ValueType> entry, string functionName) {
    TransitionTableNode<ValueType> nodeToModify = locateFunctionTable(functionName);
    
    if (entry.getIdType() == TransitionTableEntry<ValueType>::FUNCTION) {
        nodeToModify.add(entry);
        
        TransitionTableNode<ValueType> functionChildTable = TransitionTableNode<ValueType>(nodeToModify, entry.getName());
        
        nodeToModify.children.push_back(functionChildTable);
    } else if (entry.getIdType() == TransitionTableEntry<ValueType>::VARIABLE || entry.getIdType() == TransitionTableEntry<ValueType>::CONSTANT || entry.getIdType() == TransitionTableEntry<ValueType>::OPERATION || entry.getIdType() == TransitionTableEntry<ValueType>::TERMINAL) {
        nodeToModify.add(entry);
    } else {
        throw runtime_error("Invalid symbol table entry type.");
    }
}

/*
 Description:
    Get the current entry.
 
 Parameters:
    - None

 Returns:
    - The retrieved entry (TransitionTableEntry<ValueType>)
*/
template<typename ValueType>
TransitionTableEntry<ValueType> TransitionTableStates<ValueType>::getEntry() {
    return transitionTableEntry;
}

/*
 Description:
    Get a specific entry from the transition table based on a specific function name.
 
 Parameters:
    - name: The table name to find (string)
 
 Returns:
    - The retrieved entry from the global table (TransitionTableEntry<ValueType>)
*/
template<typename ValueType>
TransitionTableEntry<ValueType> TransitionTableStates<ValueType>::getEntry(string name) {
    return globalTable.get(name);
}

/*
 Description:
    Update a specific entry from the transition table with a specific value.
 
 Parameters:
    - name: The table name to find (string)
    - value: The value to update the entry with (ValueType)
 
 Returns:
    - None
*/
template<typename ValueType>
void TransitionTableStates<ValueType>::updateValue(string name, ValueType value) {
    globalTable.get(name).setValue(value);
}

// Define allowable template types
template class TransitionTableStates<double>;
template class TransitionTableStates<int>;
template class TransitionTableStates<const char*>;
template class TransitionTableStates<char>;
