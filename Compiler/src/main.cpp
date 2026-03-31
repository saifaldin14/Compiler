/*
 File: main.cpp
 Project: Épée Database Query Language & Compiler
 Author: Saif Al-Din Ali (original), Enhanced with Database Query Engine
 Description: Entry point supporting REPL mode, file execution, and legacy compiler mode

 Usage:
   ./epee                  - Start interactive REPL
   ./epee <file.ep>        - Execute database query file
   ./epee --compile <file> - Use legacy compiler pipeline (lexer/parser/semantic/TAC)
   ./epee --help           - Show usage information
*/

#include <iostream>
#include <string>
#include <cstring>

// Database engine
#include "../include/database/repl.hpp"

// Legacy compiler pipeline
#include "../include/lexicalAnalysis/lexer.hpp"
#include "../include/tokens/token.hpp"
#include "../include/tokens/tokenType.hpp"
#include "../include/tokens/fileReader.hpp"
#include "../include/syntaxAnalysis/parser.hpp"
#include "../include/semanticAnalysis/analyzer.hpp"
#include "../include/intermediateCode/threeAddressCode.hpp"

using namespace std;

void printUsage(const char* programName) {
    cout << "Épée - The Pipeline-First Database Query Language" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  " << programName << "                    Start interactive REPL" << endl;
    cout << "  " << programName << " <file.ep>          Execute a query file" << endl;
    cout << "  " << programName << " --compile <file>   Legacy compiler mode" << endl;
    cout << "  " << programName << " --help             Show this help" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  " << programName << " queries.ep" << endl;
    cout << "  " << programName << " --compile program.ep" << endl;
    cout << endl;
}

void runLegacyCompiler(const string& filename) {
    FileReader fileReader(filename);

    string errorFilename = "error.txt";
    fstream errorFile;
    errorFile.open(errorFilename, fstream::in | fstream::out | fstream::app);
    errorFile.close();

    Lexer lex = Lexer(fileReader.getFileContents());

    Parser parser(lex);
    parser.parse();

    if (parser.getParsed()) {
        Analyzer analyzer(parser.getLines());
        analyzer.analyzeSemantics();

        if (analyzer.getCorrect()) {
            ThreeAddressCode threeAddressCode(parser.getLines());
            threeAddressCode.createCode();
            threeAddressCode.printThreeAddressCode();
            threeAddressCode.printSymbolTable();
            cout << "Compilation successful." << endl;
        } else {
            cout << "Semantic analysis failed. See error.txt for details." << endl;
        }
    } else {
        cout << "Parsing failed. See error.txt for details." << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments - start REPL
        epee::Repl repl;
        repl.run();
    } else if (argc == 2) {
        string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
        } else {
            // Execute file in database mode
            epee::Repl repl;
            repl.executeFile(arg);
        }
    } else if (argc == 3) {
        string flag = argv[1];
        string filename = argv[2];
        if (flag == "--compile" || flag == "-c") {
            runLegacyCompiler(filename);
        } else {
            printUsage(argv[0]);
            return 1;
        }
    } else {
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
