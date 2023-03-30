#include <iostream>
#include "../include/lexicalAnalysis/lexer.hpp"
#include "../include/tokens/token.hpp"
#include "../include/tokens/tokenType.hpp"
#include "../include/tokens/fileReader.hpp"
#include "../include/syntaxAnalysis/parser.hpp"

using namespace std;

int main() {
    FileReader fileReader("../input/test9.cp");
    
    string filename = "../output/error.txt";
    fstream errorFile;
    errorFile.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    errorFile.close();
    
    Lexer lex = Lexer(fileReader.getFileContents());
//    while(lex.getNextToken().getType().toString() != TokenType::END.toString())
//        lex.getNextToken();
//    lex.transitionTableStates.printTransitionTables();
//    lex.transitionTableStates2.printTransitionTablesWithoutHeaders();
    
    Parser parser(lex);
    parser.parse();
    
    parser.getLexer().transitionTableStates.printTransitionTables();
    parser.getLexer().transitionTableStates2.printTransitionTablesWithoutHeaders();
        
    if (parser.getParsed()) {
        for (auto i : parser.getLines()) {
            for (Token t : i) {
//                cout << t.toString() << ", ";
                cout << t.getType().toString() << ", ";
            }
            cout << endl;
        }
    }
    return 0;
}
