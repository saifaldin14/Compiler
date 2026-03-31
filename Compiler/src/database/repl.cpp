/*
 File: repl.cpp
 Project: Épée Database Query Language
 Description: REPL implementation for interactive database queries
*/

#include "../../include/database/repl.hpp"

namespace epee {

Repl::Repl() : executor_(db_) {}

Repl::Repl(const std::string& dbPath) : executor_(db_) {
    initPersistence(dbPath);
}

void Repl::initPersistence(const std::string& dbPath) {
    dbPath_ = dbPath;

    // Create WAL
    std::string walPath = dbPath_ + ".wal";
    wal_ = std::make_unique<WriteAheadLog>(walPath);

    // If the .epd file exists, load it
    std::ifstream check(dbPath_);
    if (check.good()) {
        check.close();
        try {
            Storage::loadDatabase(db_, dbPath_);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Could not load database from '"
                      << dbPath_ << "': " << e.what() << std::endl;
        }
    }

    // Replay any WAL entries for crash recovery
    if (wal_->hasEntries()) {
        auto entries = wal_->recover();
        if (!entries.empty()) {
            std::cerr << "Recovering " << entries.size()
                      << " statement(s) from WAL..." << std::endl;
            for (const auto& sql : entries) {
                try {
                    lexer_.setSource(sql);
                    auto tokens = lexer_.tokenize();
                    DbParser parser(tokens);
                    auto stmts = parser.parse();
                    for (const auto& stmt : stmts) {
                        executor_.execute(stmt);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "WAL replay error: " << e.what() << std::endl;
                }
            }
        }
        // After recovery, save and checkpoint
        try {
            Storage::saveDatabase(db_, dbPath_);
        } catch (...) {
            // Best-effort save during recovery; failure is non-fatal
        }
        wal_->checkpoint();
    }
}

bool Repl::isMutatingStatement(const std::string& input) const {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    size_t start = lower.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return false;
    lower = lower.substr(start);
    return lower.rfind("create", 0) == 0 ||
           lower.rfind("drop", 0) == 0 ||
           lower.rfind("insert", 0) == 0 ||
           lower.rfind("update", 0) == 0 ||
           lower.rfind("delete", 0) == 0;
}

void Repl::printBanner() {
    std::cout << R"(
  ╔═══════════════════════════════════════════════════════════╗
  ║                                                           ║
  ║   ███████╗██████╗ ███████╗███████╗                        ║
  ║   ██╔════╝██╔══██╗██╔════╝██╔════╝                        ║
  ║   █████╗  ██████╔╝█████╗  █████╗                          ║
  ║   ██╔══╝  ██╔═══╝ ██╔══╝  ██╔══╝                          ║
  ║   ███████╗██║     ███████╗███████╗                        ║
  ║   ╚══════╝╚═╝     ╚══════╝╚══════╝                        ║
  ║                                                           ║
  ║   The Épée Database Query Language                        ║
  ║   Version 2.0 - Pipeline-First Query Engine               ║
  ║                                                           ║
  ║   Type 'help' for commands, 'exit' to quit                ║
  ╚═══════════════════════════════════════════════════════════╝
)" << std::endl;
}

void Repl::printHelp() {
    std::cout << R"(
  Épée Database Query Language - Command Reference
  ═══════════════════════════════════════════════════

  DDL (Data Definition):
    create table <name> (<col> <type>, ...);
    drop table <name>;
    show tables;
    describe <table>;

  DML (Data Manipulation):
    insert into <table> values (<val>, ...);
    select <cols> from <table> [where <cond>] [orderby <col>] [limit <n>];
    update <table> set <col>=<val> [where <cond>];
    delete from <table> [where <cond>];

  Pipeline Queries (Épée's signature feature!):
    <table> |> where(<condition>)
            |> select(<columns>)
            |> orderby(<col> asc|desc)
            |> groupby(<columns>)
            |> having(<condition>)
            |> join(<table> on <condition>)
            |> distinct
            |> limit(<n>)
            |> offset(<n>)
            |> count
            |> print;

  Types: int, double, string, bool
  Operators: +, -, *, /, %, ==, <>, <, >, <=, >=, and, or, not
  Aggregates: count(*), sum(col), avg(col), min(col), max(col)
  String Funcs: upper(s), lower(s), length(s), substr(s,i,n),
                concat(a,b), trim(s), replace(s,old,new)
  Predicates: between, in, like, is null, is not null
  Transactions: begin; ... commit; / rollback;

  Computed Columns:
    select(name, salary * 12 as annual_pay)

  Control Flow:
    if (<cond>) then ... [else ...] fi;
    while (<cond>) do ... od;

  Functions:
    def <type> <name>(<type> <param>, ...) ... fed;

  Commands:
    help     - Show this help
    exit     - Exit the REPL
    quit     - Exit the REPL

  Examples:
    create table users (id int, name string, age int);
    insert into users values (1, "Alice", 30);
    insert into users values (2, "Bob", 25);
    users |> where(age > 20) |> select(name, age) |> orderby(age desc) |> print;
    users |> groupby(age) |> select(age, count(*)) |> print;

)" << std::endl;
}

std::string Repl::readMultiline(std::istream& in) {
    std::string line, result;
    const size_t MAX_INPUT_SIZE = 1024 * 1024; // 1MB limit
    while (std::getline(in, line)) {
        result += line + "\n";
        if (result.size() > MAX_INPUT_SIZE) {
            std::cerr << "Error: Input exceeds maximum size (1MB)" << std::endl;
            return "";
        }
        // Check if we have a complete statement (ends with ;)
        // Trim trailing whitespace
        size_t end = result.find_last_not_of(" \t\n\r");
        if (end != std::string::npos && result[end] == ';')
            break;
        if (!in.good()) break;
        if (&in == &std::cin)
            std::cout << "  ... > ";
    }
    return result;
}

void Repl::run() {
    printBanner();

    std::string line;
    while (true) {
        std::cout << "épée> ";
        std::cout.flush();

        if (!std::getline(std::cin, line))
            break;

        // Trim
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Commands
        std::string lower = line;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "exit" || lower == "quit" || lower == "\\q") {
            // Auto-save on exit if persistence is configured
            if (!dbPath_.empty()) {
                try {
                    Storage::saveDatabase(db_, dbPath_);
                    if (wal_) wal_->checkpoint();
                    std::cout << "Database saved to '" << dbPath_ << "'" << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Auto-save failed: " << e.what() << std::endl;
                }
            }
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        if (lower == "help" || lower == "\\h" || lower == "?") {
            printHelp();
            continue;
        }

        // Check if statement is complete (ends with ;)
        std::string input = line;
        size_t end = input.find_last_not_of(" \t\n\r");
        if (end != std::string::npos && input[end] != ';') {
            std::cout << "  ... > ";
            std::string rest = readMultiline(std::cin);
            input += "\n" + rest;
        }

        executeString(input);
    }
}

void Repl::executeFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    executeString(source);
}

void Repl::executeString(const std::string& source) {
    try {
        lexer_.setSource(source);
        auto tokens = lexer_.tokenize();

        DbParser parser(tokens);
        auto statements = parser.parse();

        if (parser.hasErrors()) {
            for (const auto& err : parser.getErrors())
                std::cerr << "Parse Error: " << err << std::endl;
            return;
        }

        // Log mutating statements to WAL before executing
        if (wal_ && isMutatingStatement(source)) {
            wal_->logStatement(source);
        }

        for (const auto& stmt : statements) {
            QueryResult result = executor_.execute(stmt);
            if (!result.success) {
                std::cerr << "Error: " << result.message << std::endl;
            } else if (!result.columnNames.empty() || !result.rows.empty()) {
                std::cout << result.toPrettyTable();
            } else if (!result.message.empty()) {
                std::cout << result.message << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace epee
