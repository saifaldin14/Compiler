/*
 File: repl.hpp
 Project: Épée Database Query Language
 Description: REPL (Read-Eval-Print Loop) interface for interactive queries
*/

#ifndef EPEE_REPL_H
#define EPEE_REPL_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "executor.hpp"
#include "dbLexer.hpp"
#include "dbParser.hpp"

namespace epee {

class Repl {
public:
    Repl();

    void run();
    void executeFile(const std::string& filename);
    void executeString(const std::string& source);

private:
    Database db_;
    Executor executor_;
    DbLexer lexer_;

    void printBanner();
    void printHelp();
    std::string readMultiline(std::istream& in);
};

} // namespace epee

#endif /* EPEE_REPL_H */
