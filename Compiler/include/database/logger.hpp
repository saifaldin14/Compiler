/*
 File: logger.hpp
 Project: Épée Database Query Language
 Description: Query logger with timestamps
*/

#ifndef EPEE_LOGGER_H
#define EPEE_LOGGER_H

#include <string>
#include <fstream>
#include <chrono>

namespace epee {

class Logger {
public:
    Logger();
    explicit Logger(const std::string& filepath);
    ~Logger();

    void open(const std::string& filepath);
    void close();
    bool isOpen() const { return logFile_.is_open(); }
    const std::string& getFilepath() const { return filepath_; }

    void logQuery(const std::string& query, bool success, const std::string& message = "");
    void logEvent(const std::string& event);

    static std::string timestamp();

private:
    std::ofstream logFile_;
    std::string filepath_;
};

} // namespace epee

#endif /* EPEE_LOGGER_H */
