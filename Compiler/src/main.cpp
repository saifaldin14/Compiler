#include <iostream>
#include "../headers/lexer.hpp"
#include "../headers/token.hpp"
#include "../headers/tokenType.hpp"
#include "../headers/fileReader.hpp"
#include "../headers/parser.hpp"

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
    return 0;
}
