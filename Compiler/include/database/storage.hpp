/*
 File: storage.hpp
 Project: Épée Database Query Language
 Description: Binary file persistence for database serialization/deserialization
*/

#ifndef EPEE_STORAGE_H
#define EPEE_STORAGE_H

#include <string>
#include <fstream>
#include <cstdint>
#include "table.hpp"
#include "value.hpp"

namespace epee {

class Storage {
public:
    static bool saveDatabase(const Database& db, const std::string& filepath);
    static bool loadDatabase(Database& db, const std::string& filepath);

private:
    static constexpr const char* MAGIC = "EPED";  // Épée PErsistence Data
    static constexpr uint32_t VERSION = 1;
    static constexpr uint32_t MAX_STRING_LENGTH = 10 * 1024 * 1024;  // 10MB

    static void writeString(std::ofstream& out, const std::string& s);
    static std::string readString(std::ifstream& in);
    static void writeValue(std::ofstream& out, const Value& v);
    static Value readValue(std::ifstream& in);

    static bool validatePath(const std::string& filepath);
};

} // namespace epee

#endif /* EPEE_STORAGE_H */
