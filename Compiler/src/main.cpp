#include <iostream>
#include "../include/lexicalAnalysis/lexer.hpp"
#include "../include/tokens/token.hpp"
#include "../include/tokens/tokenType.hpp"
#include "../include/tokens/fileReader.hpp"
#include "../include/syntaxAnalysis/parser.hpp"
#include "../include/semanticAnalysis/analyzer.hpp"
#include "../include/intermediateCode/threeAddressCode.hpp"

using namespace std;

int main() {
    FileReader fileReader("../input/test10.cp");
    
    string filename = "../output/error.txt";
    fstream errorFile;
    errorFile.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
    errorFile.close();
    
    Lexer lex = Lexer(fileReader.getFileContents());
    
    Parser parser(lex);
    parser.parse();
    
    if (parser.getParsed()) {
        Analyzer analyzer(parser.getLines());
        analyzer.analyzeSemantics();
//        analyzer.printVariables();
        
        ThreeAddressCode threeAddressCode(parser.getLines());
        threeAddressCode.createCode();
        threeAddressCode.printThreeAddressCode();
    }
    return 0;
}
