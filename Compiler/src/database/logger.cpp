/*
 File: logger.cpp
 Project: Épée Database Query Language
 Description: Logger implementation
*/

#include "../../include/database/logger.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace epee {

Logger::Logger() = default;

Logger::Logger(const std::string& filepath) {
    open(filepath);
}

Logger::~Logger() {
    close();
}

void Logger::open(const std::string& filepath) {
    filepath_ = filepath;
    logFile_.open(filepath, std::ios::app);
    if (logFile_.is_open())
        logEvent("Logger started");
}

void Logger::close() {
    if (logFile_.is_open()) {
        logEvent("Logger stopped");
        logFile_.close();
    }
}

void Logger::logQuery(const std::string& query, bool success, const std::string& message) {
    if (!logFile_.is_open()) return;
    // Trim the query for logging (single line)
    std::string trimmed = query;
    // Replace newlines with spaces
    std::replace(trimmed.begin(), trimmed.end(), '\n', ' ');
    // Remove leading/trailing whitespace
    size_t start = trimmed.find_first_not_of(" \t\r");
    size_t end = trimmed.find_last_not_of(" \t\r");
    if (start != std::string::npos && end != std::string::npos)
        trimmed = trimmed.substr(start, end - start + 1);
    // Truncate very long queries
    if (trimmed.size() > 500)
        trimmed = trimmed.substr(0, 497) + "...";

    logFile_ << "[" << timestamp() << "] QUERY "
             << (success ? "OK" : "ERROR") << ": " << trimmed;
    if (!success && !message.empty())
        logFile_ << " -- " << message;
    logFile_ << std::endl;
}

void Logger::logEvent(const std::string& event) {
    if (!logFile_.is_open()) return;
    logFile_ << "[" << timestamp() << "] EVENT: " << event << std::endl;
}

std::string Logger::timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm buf;
#ifdef _WIN32
    localtime_s(&buf, &time);
#else
    localtime_r(&time, &buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace epee
