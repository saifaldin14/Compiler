/*
 File: wal.hpp
 Project: Épée Database Query Language
 Description: Write-Ahead Log for crash recovery via SQL statement replay
*/

#ifndef EPEE_WAL_H
#define EPEE_WAL_H

#include <string>
#include <vector>
#include <fstream>

namespace epee {

class WriteAheadLog {
public:
    explicit WriteAheadLog(const std::string& filepath);
    ~WriteAheadLog();

    void logStatement(const std::string& sql);
    void checkpoint();  // Flush and clear the WAL
    std::vector<std::string> recover();  // Read pending statements
    bool hasEntries() const;
    void close();

private:
    std::string filepath_;
    std::ofstream logFile_;
};

} // namespace epee

#endif /* EPEE_WAL_H */
