/*
 File: transitionTableEntry.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-25
 */

#include "../../headers/lexicalAnalysis/transitionTableEntry.hpp"
#include <sstream>

/*
 Description:
    Convert a template value to a string to be saved and printed.
 
 Parameters:
    - v: The value to be converted (const ValueType&)
 
 Returns:
    - The textual contents of the value (string)
 */
template <typename ValueType>
string to_string_value(const ValueType& v){
    stringstream ss;
    ss << v;
    return ss.str();
}

/*
 Description:
    Constructor to define a TransitionTableEntry based on specified inputs.
 
 Parameters:
    - n: The name of the entry (string)
    - i: The ID Type of the entry (string)
    - d: The Data Type of the entry (string)
    - v: The value to be converted (ValueType)
 */
template<typename ValueType>
TransitionTableEntry<ValueType>::TransitionTableEntry(string n, string i, string d, ValueType v) {
    name = n;
    idType = i;
    dataType = d;
    value = v;
}

/*
 Description:
    Default constructor to define a TransitionTableEntry without specified inputs.
 
 Parameters:
    - None
 */
template<typename ValueType>
TransitionTableEntry<ValueType>::TransitionTableEntry() {
    name = "";
    idType = "";
    dataType = "";
    value = (ValueType)0;
}

/*
 Description:
    Getter function for the name of the entry.
 
 Parameters:
    - None

 Returns:
    - The name of the entry (string)
 */
template<typename ValueType>
string TransitionTableEntry<ValueType>::getName() { return name; }

/*
 Description:
    Getter function for the value of the entry.
 
 Parameters:
    - None

 Returns:
    - The value of the entry (ValueType)
 */
template<typename ValueType>
ValueType TransitionTableEntry<ValueType>::getValue() { return value; }

/*
 Description:
    Getter function for the ID Type of the entry.
 
 Parameters:
    - None

 Returns:
    - The ID Type of the entry (string)
 */
template<typename ValueType>
string TransitionTableEntry<ValueType>::getIdType() { return idType; }

/*
 Description:
    Setter function for the value of the entry.
 
 Parameters:
    - inputVal: The value to save (ValueType)
 
 Returns:
    - None
 */
template<typename ValueType>
void TransitionTableEntry<ValueType>::setValue(ValueType inputVal) {
    value = inputVal;
    printValue = to_string_value(inputVal);
}

/*
 Description:
    Update the paramers of an entry. Instead of creating a new entry, update an old one if it's redundant
 
 Parameters:
    - n: The name of the entry (string)
    - i: The ID Type of the entry (string)
    - d: The Data Type of the entry (string)
    - v: The value to be saved (ValueType)
 
 Returns:
    - None
 */
template<typename ValueType>
void TransitionTableEntry<ValueType>::setParams(string n, string i, string d, ValueType v) {
    name = n;
    idType = i;
    dataType = d;
    value = v;
    printValue = to_string_value(v);
}

/*
 Description:
    Compare the values of 2 entries and check if they're equal.
 
 Parameters:
    - otherEntry: The entry to be compared against (TransitionTableEntry<ValueType>)
 
 Returns:
    - Whether or not the 2 entries are equal (bool)
 */
template<typename ValueType>
bool TransitionTableEntry<ValueType>::equals(TransitionTableEntry<ValueType> otherEntry) {
    return ((name == otherEntry.name) && (value == otherEntry.value) && (idType == otherEntry.idType) && (dataType == otherEntry.dataType));
}

/*
 Description:
    Save the contents of the entry to a human-readable text
 
 Parameters:
    - None

 Returns:
    - The converted text of the entry values (string)
 */
template<typename ValueType>
string TransitionTableEntry<ValueType>::toString() {
    string output;
    
    if (name != "")
        output += name + " ";
    
    if (idType != "")
        output += idType + " ";
    
    if (dataType != "")
        output += dataType + " ";
    
    if (!printValue.empty()) {
        output += "-> ";
        output += printValue;
    }
    
    return output;
}

// Define allowable template types
template class TransitionTableEntry<int>;
template class TransitionTableEntry<double>;
template class TransitionTableEntry<const char*>;
