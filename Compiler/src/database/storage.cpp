/*
 File: storage.cpp
 Project: Épée Database Query Language
 Description: Implementation of binary file persistence for database
*/

#include "../../include/database/storage.hpp"

#include <stdexcept>
#include <cstring>

namespace epee {

bool Storage::validatePath(const std::string& filepath) {
    if (filepath.empty()) return false;
    // Reject directory traversal
    if (filepath.find("..") != std::string::npos) return false;
    // Reject null bytes
    if (filepath.find('\0') != std::string::npos) return false;
    return true;
}

void Storage::writeString(std::ofstream& out, const std::string& s) {
    if (s.size() > UINT32_MAX) {
        throw std::runtime_error("String too large for serialization");
    }
    uint32_t len = static_cast<uint32_t>(s.size());
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    if (len > 0) {
        out.write(s.data(), len);
    }
}

std::string Storage::readString(std::ifstream& in) {
    uint32_t len = 0;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!in.good()) throw std::runtime_error("Unexpected end of file reading string length");
    if (len > MAX_STRING_LENGTH) throw std::runtime_error("String length exceeds safety limit");
    std::string s(len, '\0');
    if (len > 0) {
        in.read(&s[0], len);
        if (!in.good()) throw std::runtime_error("Unexpected end of file reading string data");
    }
    return s;
}

void Storage::writeValue(std::ofstream& out, const Value& v) {
    if (v.isNull()) {
        uint8_t tag = 0;
        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
    } else if (v.isInt()) {
        uint8_t tag = 1;
        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
        int32_t val = static_cast<int32_t>(v.asInt());
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    } else if (v.isDouble()) {
        uint8_t tag = 2;
        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
        double val = v.asDouble();
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    } else if (v.isString()) {
        uint8_t tag = 3;
        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
        writeString(out, v.asString());
    } else if (v.isBool()) {
        uint8_t tag = 4;
        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
        uint8_t val = v.asBool() ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&val), sizeof(val));
    }
}

Value Storage::readValue(std::ifstream& in) {
    uint8_t tag = 0;
    in.read(reinterpret_cast<char*>(&tag), sizeof(tag));
    if (!in.good()) throw std::runtime_error("Unexpected end of file reading value tag");

    switch (tag) {
        case 0: return Value::null();
        case 1: {
            int32_t val = 0;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            if (!in.good()) throw std::runtime_error("Unexpected end of file reading int value");
            return Value(static_cast<int>(val));
        }
        case 2: {
            double val = 0.0;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            if (!in.good()) throw std::runtime_error("Unexpected end of file reading double value");
            return Value(val);
        }
        case 3: {
            return Value(readString(in));
        }
        case 4: {
            uint8_t val = 0;
            in.read(reinterpret_cast<char*>(&val), sizeof(val));
            if (!in.good()) throw std::runtime_error("Unexpected end of file reading bool value");
            return Value(val != 0);
        }
        default:
            throw std::runtime_error("Unknown value type tag: " + std::to_string(tag));
    }
}

bool Storage::saveDatabase(const Database& db, const std::string& filepath) {
    if (!validatePath(filepath)) {
        throw std::runtime_error("Invalid file path: " + filepath);
    }

    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filepath);
    }

    // Magic bytes
    out.write(MAGIC, 4);

    // Version
    uint32_t ver = VERSION;
    out.write(reinterpret_cast<const char*>(&ver), sizeof(ver));

    // Table count
    const auto& tables = db.getAllTables();
    uint32_t tableCount = static_cast<uint32_t>(tables.size());
    out.write(reinterpret_cast<const char*>(&tableCount), sizeof(tableCount));

    for (const auto& [name, table] : tables) {
        // Table name
        writeString(out, table.getName());

        // Columns
        const auto& cols = table.getColumns();
        uint32_t colCount = static_cast<uint32_t>(cols.size());
        out.write(reinterpret_cast<const char*>(&colCount), sizeof(colCount));

        for (const auto& col : cols) {
            writeString(out, col.name);
            uint8_t type = static_cast<uint8_t>(col.type);
            out.write(reinterpret_cast<const char*>(&type), sizeof(type));
            uint8_t nullable = col.nullable ? 1 : 0;
            out.write(reinterpret_cast<const char*>(&nullable), sizeof(nullable));
            uint8_t unique = col.unique ? 1 : 0;
            out.write(reinterpret_cast<const char*>(&unique), sizeof(unique));
            uint8_t pk = col.primaryKey ? 1 : 0;
            out.write(reinterpret_cast<const char*>(&pk), sizeof(pk));
        }

        // Rows
        const auto& rows = table.getRows();
        uint32_t rowCount = static_cast<uint32_t>(rows.size());
        out.write(reinterpret_cast<const char*>(&rowCount), sizeof(rowCount));

        for (const auto& row : rows) {
            for (const auto& val : row) {
                writeValue(out, val);
            }
        }
    }

    out.flush();
    if (!out.good()) {
        throw std::runtime_error("Error writing to file: " + filepath);
    }
    out.close();
    return true;
}

bool Storage::loadDatabase(Database& db, const std::string& filepath) {
    if (!validatePath(filepath)) {
        throw std::runtime_error("Invalid file path: " + filepath);
    }

    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + filepath);
    }

    // Validate magic bytes
    char magic[4] = {};
    in.read(magic, 4);
    if (!in.good() || std::memcmp(magic, MAGIC, 4) != 0) {
        throw std::runtime_error("Invalid file format: bad magic bytes");
    }

    // Validate version
    uint32_t ver = 0;
    in.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    if (!in.good() || ver != VERSION) {
        throw std::runtime_error("Unsupported file version: " + std::to_string(ver));
    }

    // Table count
    uint32_t tableCount = 0;
    in.read(reinterpret_cast<char*>(&tableCount), sizeof(tableCount));
    if (!in.good()) throw std::runtime_error("Error reading table count");
    if (tableCount > 100000) throw std::runtime_error("Table count exceeds safety limit");

    for (uint32_t t = 0; t < tableCount; t++) {
        // Table name
        std::string tableName = readString(in);

        // Columns
        uint32_t colCount = 0;
        in.read(reinterpret_cast<char*>(&colCount), sizeof(colCount));
        if (!in.good()) throw std::runtime_error("Error reading column count");
        if (colCount > 10000) throw std::runtime_error("Column count exceeds safety limit");

        std::vector<Column> columns;
        columns.reserve(colCount);
        for (uint32_t c = 0; c < colCount; c++) {
            Column col;
            col.name = readString(in);
            uint8_t typeVal = 0;
            in.read(reinterpret_cast<char*>(&typeVal), sizeof(typeVal));
            col.type = static_cast<ValueType>(typeVal);
            uint8_t nullableVal = 0;
            in.read(reinterpret_cast<char*>(&nullableVal), sizeof(nullableVal));
            col.nullable = (nullableVal != 0);
            uint8_t uniqueVal = 0;
            in.read(reinterpret_cast<char*>(&uniqueVal), sizeof(uniqueVal));
            col.unique = (uniqueVal != 0);
            uint8_t pkVal = 0;
            in.read(reinterpret_cast<char*>(&pkVal), sizeof(pkVal));
            col.primaryKey = (pkVal != 0);
            if (!in.good()) throw std::runtime_error("Error reading column definition");
            columns.push_back(col);
        }

        // Create table in database
        if (db.hasTable(tableName)) {
            db.dropTable(tableName);
        }
        db.createTable(tableName, columns);
        Table& tbl = db.getTable(tableName);

        // Rows
        uint32_t rowCount = 0;
        in.read(reinterpret_cast<char*>(&rowCount), sizeof(rowCount));
        if (!in.good()) throw std::runtime_error("Error reading row count");
        if (rowCount > 10000000) throw std::runtime_error("Row count exceeds safety limit");

        for (uint32_t r = 0; r < rowCount; r++) {
            Row row;
            row.reserve(colCount);
            for (uint32_t c = 0; c < colCount; c++) {
                row.push_back(readValue(in));
            }
            tbl.insertRow(row);
        }
    }

    in.close();
    return true;
}

} // namespace epee
