/*
 File: wal.cpp
 Project: Épée Database Query Language
 Description: Implementation of Write-Ahead Log for crash recovery
*/

#include "../../include/database/wal.hpp"

#include <sstream>
#include <stdexcept>

namespace epee {

WriteAheadLog::WriteAheadLog(const std::string& filepath)
    : filepath_(filepath) {
    logFile_.open(filepath_, std::ios::app);
    if (!logFile_.is_open()) {
        throw std::runtime_error("Cannot open WAL file: " + filepath_);
    }
}

WriteAheadLog::~WriteAheadLog() {
    close();
}

void WriteAheadLog::logStatement(const std::string& sql) {
    if (!logFile_.is_open()) return;
    // Write each statement on its own line, replacing newlines with spaces
    std::string sanitized = sql;
    for (auto& ch : sanitized) {
        if (ch == '\n' || ch == '\r') ch = ' ';
    }
    // Trim trailing whitespace/semicolons for consistency
    size_t end = sanitized.find_last_not_of(" \t;");
    if (end != std::string::npos) {
        sanitized = sanitized.substr(0, end + 1);
    }
    logFile_ << sanitized << ";" << std::endl;
}

void WriteAheadLog::checkpoint() {
    if (logFile_.is_open()) {
        logFile_.flush();
        logFile_.close();
    }
    // Truncate the WAL file
    logFile_.open(filepath_, std::ios::trunc);
    if (!logFile_.is_open()) {
        throw std::runtime_error("Cannot reopen WAL file: " + filepath_);
    }
}

std::vector<std::string> WriteAheadLog::recover() {
    std::vector<std::string> statements;
    std::ifstream in(filepath_);
    if (!in.is_open()) return statements;

    std::string line;
    while (std::getline(in, line)) {
        // Skip empty lines
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        if (!line.empty()) {
            statements.push_back(line);
        }
    }
    in.close();
    return statements;
}

bool WriteAheadLog::hasEntries() const {
    std::ifstream in(filepath_);
    if (!in.is_open()) return false;
    std::string line;
    while (std::getline(in, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) return true;
    }
    return false;
}

void WriteAheadLog::close() {
    if (logFile_.is_open()) {
        logFile_.flush();
        logFile_.close();
    }
}

} // namespace epee
