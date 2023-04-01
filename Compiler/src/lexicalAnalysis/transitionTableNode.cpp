/*
 File: transitionTableNode.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
*/

#include "../../include/lexicalAnalysis/transitionTableNode.hpp"

template<typename ValueType>
string TransitionTableNode<ValueType>::GLOBAL_SCOPE = "GLOBAL";

/*
 Description:
    Default constructor for the TransitionTableNode to define global values with initial values
 
 Parameters:
    - None
 */
template<typename ValueType>
TransitionTableNode<ValueType>::TransitionTableNode() {
    try {
        transitionTable = vector<TransitionTableEntry<ValueType>>();
        children = vector<TransitionTableNode>();
        parent = nullptr;
        transitionTableName = GLOBAL_SCOPE;
    } catch (...) {
        throw runtime_error("Error creating TransitionTableNode object.");
    }
}

/*
 Description:
    Constructor for the TransitionTableNode to define global values with based on exisiting values
 
 Parameters:
    - parentTable: The parent table where this node will be a child of (TransitionTableNode)
    - functionName: Existing function name to set the table name to (string)
 */
template<typename ValueType>
TransitionTableNode<ValueType>::TransitionTableNode(TransitionTableNode parentTable, string functionName) {
    try {
        transitionTable = vector<TransitionTableEntry<ValueType>>();
        children = vector<TransitionTableNode>();
        parent = &parentTable;
        transitionTableName = functionName;
    } catch (...) {
        throw runtime_error("Error creating TransitionTableNode object.");
    }
}

/*
 Description:
    Clone the current node into a seperate variable (to keep values seperate from memory addresses)
 
 Parameters:
    - None

 Returns:
    - A clone of the current node (TransitionTableNode<ValueType>)
 */
template<typename ValueType>
TransitionTableNode<ValueType> TransitionTableNode<ValueType>::clone() {
    TransitionTableNode<ValueType> nodeToReturn = TransitionTableNode();
    
    try {
        nodeToReturn.children = children;
        nodeToReturn.transitionTable = transitionTable;
        nodeToReturn.parent = parent;
        nodeToReturn.transitionTableName = transitionTableName;
    } catch (...) {
        cout << "Exception occurred while cloning transition table node" << endl;
    }
    
    return nodeToReturn;
}

/*
 Description:
    Save the contents of the transition table to a file. If there's no exisitng file create a new one, otherwise append to it instead. Writes to the file without the table header (basically to add values to the file without reprinting the table name)
 
 Parameters:
    - None

 Returns:
    - None
 */
template<typename ValueType>
void TransitionTableNode<ValueType>::printTableWithoutHeader() {
    try {
        vector<string> itemsToPrint;
        for (int i = 0; i < transitionTable.size(); i++) {
            if (find(itemsToPrint.begin(), itemsToPrint.end(), transitionTable[i].toString()) == itemsToPrint.end()) {
                itemsToPrint.push_back(transitionTable[i].toString());
            }
        }
        
        sort(itemsToPrint.begin(), itemsToPrint.end());
        
        fstream appendFileToWorkWith;
        string filename = "../output/symbolTable.txt";
        appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
        
        // If file does not exist, Create new file
        if (!appendFileToWorkWith) {
            appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
            appendFileToWorkWith <<"\n";
            appendFileToWorkWith.close();
        } else {    // use existing file
            for (auto i : itemsToPrint)
                appendFileToWorkWith << i << endl;
            appendFileToWorkWith.close();
        }
    } catch (const std::out_of_range& e) {
        cerr << "Out of Range error: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception caught" << endl;
    }
}

/*
 Description:
    Save the contents of the transition table to a file. If there's no exisitng file create a new one, otherwise append to it instead. Writes to the file with the table header (used as the initial entry point to writing to a file)
 
 Parameters:
    - None

 Returns:
    - None
 */
template<typename ValueType>
void TransitionTableNode<ValueType>::printTable() {
    try {
        fstream appendFileToWorkWith;
        string filename = "../output/symbolTable.txt";
        appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);

      // If file does not exist, Create new file
      if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith <<"\n";
        appendFileToWorkWith.close();
       } else {    // use existing file
           appendFileToWorkWith << transitionTableName << endl << "--------------------------"<<endl;
           appendFileToWorkWith.close();
        }
        printTableWithoutHeader();
    } catch (const std::out_of_range& e) {
        cerr << "Out of Range error: " << e.what() << endl;
    } catch (...) {
        cerr << "Unknown exception caught" << endl;
    }
}

/*
 Description:
    Add a new entry to the transition table.
 
 Parameters:
    - entryToAdd: The entry that's to be added (TransitionTableEntry<ValueType>)
 
 Returns:
    - None
 */
template<typename ValueType>
void TransitionTableNode<ValueType>::add(TransitionTableEntry<ValueType> entryToAdd) {
    try {
        transitionTable.push_back(entryToAdd);
    } catch (exception& e) {
        cerr << "Exception occurred: " << e.what() << endl;
    }
}

/*
 Description:
    Retrieves a specific entry from the transition table based on a name.
 
 Parameters:
    - name: The name of the entry to be found (string)
 
 Returns:
    - The returned entry, empty entry if not found (TransitionTableEntry<ValueType>)
 */
template<typename ValueType>
TransitionTableEntry<ValueType> TransitionTableNode<ValueType>::get(string name) {
    TransitionTableEntry<ValueType> locatedEntry = TransitionTableEntry<ValueType>();
    // Search local symbol table first
    for (int i = 0; i < transitionTable.size(); i++) {
        if (transitionTable[i].getName() == name)
            return transitionTable[i];
    }
    // ID wasn't found in local symbol table, must search child tables
    for (int i = 0; i < children.size(); i++) {
        for (int j = 0; j < children[i].transitionTable.size(); j++) {
            if (children[i].transitionTable[j].getName() == name) {
                return children[i].transitionTable[j];
            }
        }
    }
    
    // ID not found in child symbol tables, must search parents
    TransitionTableNode* currentTable = parent;
    
    while (currentTable != nullptr) {
        for (int i = 0; i < currentTable->transitionTable.size(); i++) {
            if (currentTable->transitionTable[i].getName() == name) {
                return currentTable->transitionTable[i];
            }
        }
            
        // ID still not found, must go another level higher.
        currentTable = currentTable->parent;
    }
        
    return locatedEntry;
}

// Define allowable template types
template class TransitionTableNode<int>;
template class TransitionTableNode<double>;
template class TransitionTableNode<const char*>;
