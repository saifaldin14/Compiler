/*
 File: lexer.cpp
 Project: Compiler
 Author: Saif Al-Din Ali
 Created: 2023-02-15
*/

#include "../headers/lexer.hpp"
#include "../headers/transitionTableEntry.hpp"
#include "../headers/transitionTableStates.hpp"

/*
 Description:
    Constructor for the lexer without a source file.
 
 Parameters:
    - None
 */
Lexer::Lexer() { Lexer::reserve(); }

/*
 Description:
    Constructor for the lexer the initializes the source file and index.
 
 Parameters:
    - fileContents: The content of the file saved as text (string)
 */
Lexer::Lexer(string fileContents) {
    source = fileContents;
    sourceFileIndex = 0;
    reserve();
}

/*
 Description:
    Copy constructor for the lexer to be initialized with another lexer.
 
 Parameters:
    - other: The address of another Lexer object (Lexer)
 */
Lexer::Lexer(const Lexer& other)
    : transitionTableStates(other.transitionTableStates)
    , transitionTableStates2(other.transitionTableStates2)
    , lineNum(other.lineNum)
    , next(other.next)
    , sourceFileIndex(other.sourceFileIndex)
    , source(other.source)
    , symbols(other.symbols)
{
}

/*
 Description:
    Assignment operation for the Lexer. To allow for lex = otherLex;
 
 Parameters:
    - other: The address of another Lexer object (Lexer)
 */
Lexer& Lexer::operator=(const Lexer& other) {
    if (this != &other) {
        transitionTableStates = other.transitionTableStates;
        transitionTableStates2 = other.transitionTableStates2;
        lineNum = other.lineNum;
        next = other.next;
        sourceFileIndex = other.sourceFileIndex;
        source = other.source;
        symbols = other.symbols;
    }
    return *this;
}

/*
 Description:
    Saves reserved keywords to the symbols vector.
 
 Parameters:
    - None
 
 Returns:
    - None
 */
void Lexer::reserve() {
    for (const auto &reservedWord : RESERVED)
        symbols.push_back(reservedWord);
}

vector<string> Lexer::getSymbols() { return symbols; }

/*
 Description:
    Save the error text to a seperate file. Creates a new file for error or appends to it if it already exisits. Writes the invalid token to it.
 
 Parameters:
    - text: The error text to be written (string)
 
 Returns:
    - None
 */
void Lexer::saveErrorText(char c) {
    fstream appendFileToWorkWith;
    string filename = "../output/error.txt";
    appendFileToWorkWith.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    
    // Print errors to file
    if (!appendFileToWorkWith) {
        appendFileToWorkWith.open(filename,  fstream::in | fstream::out | fstream::trunc);
        appendFileToWorkWith << "LEXICAL ERROR" << endl;
        appendFileToWorkWith << c << endl;
        appendFileToWorkWith.close();
    } else {    // use existing file
        appendFileToWorkWith << "LEXICAL ERROR" << endl;
        cout << "ERROR: " << c << endl;
        appendFileToWorkWith << "Lexical error with token: <" << c << "> on line " << getLineNum() << endl;
        appendFileToWorkWith.close();
    }
    
}

/*
 Description:
    Read a specific character and save it to next (to be checked if it's a token).
    Goes to the current source file index position to get the char.
 
 Parameters:
    - None
 
 Returns:
    - None
 */
void Lexer::readChar() {
    try {
        next = source[sourceFileIndex];
        sourceFileIndex++;
    } catch (const exception& e) {
        cerr << "Error occurred: " << e.what() << endl;
    }
}

/*
 Description:
    Read a specific character and save it to next (to be checked if it's a token)
 
 Parameters:
    - c: The current character token (char)
 
 Returns:
    - Whether or not the character is read successfully (bool)
 */
bool Lexer::readChar(char c) {
    try {
        readChar();
        if (next != c) return false;
        else {
            readChar();
            return true;
        }
    } catch (exception const& e) {
        cerr << "Error: " << e.what() << endl;
        return false;
    }
}

/*
 Description:
    Determine whether or not a token is a letter.
 
 Parameters:
    - c: The current character token (char)
 
 Returns:
    - Whether or not the character is a letter (bool)
 */
bool Lexer::isLetter(char c) { return (isalpha(c)); }

/*
 Description:
    Determine whether or not a token is a number.
 
 Parameters:
    - c: The current character token (char)
 
 Returns:
    - Whether or not the character is a number (bool)
 */
bool Lexer::isDigit(char c) { return (isdigit(c)); }

/*
 Description:
    Retrieve the corresponding line number of the token.
 
 Parameters:
    - None

 Returns:
    - lineNum: The current line number (int)
 */
int Lexer::getLineNum() { return lineNum; }

/*
 Description:
    Used to recover the token and not crash the program upon an invalid token.
 
 Parameters:
    - token: The invalid token needed to be recovered (string)
 
 Returns:
    - The recovered token (string)
 */
string Lexer::recover(string token) {
    try {
        while(isLetter(next) || isDigit(next)) {
            token += next;
            readChar();
        }
    } catch (exception const& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return token;
}

/*
 Description:
    Add Double values to transition table
    Saves to the first transition table (used for numerical data)
 
 Parameters:
    - n: Name of entry (string)
    - i: ID Type of entry (string)
    - d: Data Type of entry (string)
    - v: Value of entry (double)
 */
void Lexer::addEntryToTable(string n, string i, string d, double v) {
    auto entry = transitionTableStates.getEntry();
    entry.setParams(n, i, d, v);
    transitionTableStates.getInstance()->addEntry(entry);
}

/*
 Description:
    Add Int values to transition table
    Saves to the first transition table (used for numerical data)
 
 Parameters:
    - n: Name of entry (string)
    - i: ID Type of entry (string)
    - d: Data Type of entry (string)
    - v: Value of entry (int)
 
 Returns:
    - None
*/
void Lexer::addEntryToTable(string n, string i, string d, int v) {
    auto entry = transitionTableStates.getEntry();
    entry.setParams(n, i, d, v);
    transitionTableStates.getInstance()->addEntry(entry);
}

/*
 Description:
    Add string values to transition table
    Saves to the second transition table (used for character data)
 
 Parameters:
    - n: Name of entry (string)
    - i: ID Type of entry (string)
    - d: Data Type of entry (string)
    - v: Value of entry (const char*)
 
 Returns:
    - None
 */
void Lexer::addEntryToTable(string n, string i, string d, const char* v) {
    auto entry = transitionTableStates2.getEntry();
    entry.setParams(n, i, d, v);
    transitionTableStates2.getInstance()->addEntry(entry);
}

/*
 Description:
    Extracts token values from character.
    Reads code as a string and generates the corresponding tokens.
 
 Parameters:
    - None

 Returns:
    - The extracted token (Token)
 */
Token Lexer::getNextToken() {
    // Skip Whitespace
    while (find(WHITESPACE.begin(), WHITESPACE.end(), next) != WHITESPACE.end()) {
        if (next == '\n') lineNum += 1;
        readChar();
    }
    
    auto entry = transitionTableStates.getEntry(); // Used to easily get the ID Type of the token
    
    // Check for operation symbols
    // Return the corresponding operation tokens and save it to the transition table
    switch (next) {
        case '=':
            if (readChar('=')) {
                addEntryToTable("", entry.OPERATION, "equals", "==");
                return Token("==", TokenType::COMP);
            } else {
                addEntryToTable("", entry.OPERATION, "assignment", "=");
                return Token('=', TokenType::TERM);
            }
        case '<':
            if (readChar('=')) {
                addEntryToTable("", entry.OPERATION, "less-than equal", "<=");
                return Token("<=", TokenType::COMP);
            } else if (next == '>') {
                readChar();
                addEntryToTable("", entry.OPERATION, "not equal", "<>");
                return Token("<>", TokenType::COMP);
            } else {
                addEntryToTable("", entry.OPERATION, "less than", "<");
                return Token('<', TokenType::COMP);
            }
        case '>':
            if (readChar('=')) {
                addEntryToTable("", entry.OPERATION, "greater-than equal", ">=");
                return Token(">=", TokenType::COMP);
            } else {
                addEntryToTable("", entry.OPERATION, "greater than", ">");
                return Token('>', TokenType::COMP);
            }
    }
        
    // Start Processing Number Token
    if (isDigit(next)) {
        // Integer Part
        string num = "";
        do {
            num += next;
            readChar();
        } while (isDigit(next));
        
        // Invalid Integer
        if (isLetter(next) && next != 'e' && next != 'E')  {
            saveErrorText(next);
            return Token(recover(num), TokenType::ERR);
        // Valid Integer
        } else if (next != '.') {
            addEntryToTable("", entry.CONSTANT, entry.INT, stoi(num));
            return Token(num, TokenType::INT);
        // Double Part
        } else {
            num += next;
            readChar();
            while (isDigit(next)) {
                num += next;
                readChar();
            }
            // Scientific Notation
            if (next == 'e' || next == 'E') {
                num += next;
                readChar();
                if (next == '-' || next == '+') {
                    num += next;
                    readChar();
                }
                
                if (isDigit(next)) {
                    while (isDigit(next)) {
                        num += next;
                        readChar();
                    }
                // Invalid Double
                } else {
                    saveErrorText(next);
                    return Token(recover(num), TokenType::ERR);
                }
            // Invalid Double
            } else if (isLetter(next)) {
                saveErrorText(next);
                return Token(recover(num), TokenType::ERR);
            }
            // Valid Double
            addEntryToTable("", entry.CONSTANT, entry.DOUBLE, stod(num));
            return Token(num, TokenType::DOUBLE);
        }
    // Start Processing ID (or Reserved) Token
    } else if (isLetter(next)) {
        // Build ID token
        string buffer;
        do {
            buffer += next;
            readChar();
        } while(isLetter(next) || isDigit(next));
        string id = buffer;
        
        // Add to Transition Table if ID not already defined
        if (find(symbols.begin(), symbols.end(), id) != symbols.end()) {
            symbols.push_back(id);
            
            addEntryToTable("", entry.KEYWORD, "", id.c_str());

            return Id(id, TokenType::ID, (int)symbols.size() - 1);
        } else {
            int index = (int)distance(symbols.begin(), find(symbols.begin(), symbols.end(), id));
            
            // Return depending on if within Reserved word range
            if (index < RESERVED.size()) {
                addEntryToTable("", entry.KEYWORD, "", id.c_str());
                return Reserved(id, TokenType::RESERVED, index);
            } else {
                addEntryToTable("", entry.VARIABLE, "", id.c_str());
                return Id(id, TokenType::ID, index);
            }
        }
    } else if (find(TERMINALS.begin(), TERMINALS.end(), next) != TERMINALS.end()) {
        if (next == end) {
            return Token(next, TokenType::END);
        } else {
            addEntryToTable("", entry.TERMINAL, "", new char(next));
            Token token = Token(next, TokenType::TERM);
            readChar();
            return token;
        }
    } else {
        saveErrorText(next);
        Token token = Token(next, TokenType::ERR);
        readChar();
        return token;
    }
}
