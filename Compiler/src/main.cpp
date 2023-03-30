#include <iostream>
#include "../headers/lexicalAnalysis/lexer.hpp"
#include "../headers/tokens/token.hpp"
#include "../headers/tokens/tokenType.hpp"
#include "../headers/tokens/fileReader.hpp"
#include "../headers/syntaxAnalysis/parser.hpp"

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
