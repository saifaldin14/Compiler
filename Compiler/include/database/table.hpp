/*
 File: table.hpp
 Project: Épée Database Query Language
 Description: In-memory table storage with schema definition and row operations
*/

#ifndef EPEE_TABLE_H
#define EPEE_TABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <functional>
#include <set>
#include "value.hpp"
#include "btree.hpp"

namespace epee {

struct Column {
    std::string name;
    ValueType type;
    bool nullable = true;
    bool unique = false;
    bool primaryKey = false;
    Value defaultValue;

    Column() = default;
    Column(const std::string& n, ValueType t, bool pk = false)
        : name(n), type(t), nullable(!pk), unique(pk), primaryKey(pk) {}
};

using Row = std::vector<Value>;

struct QueryResult {
    std::vector<std::string> columnNames;
    std::vector<Row> rows;
    std::string message;
    bool success = true;
    int affectedRows = 0;

    QueryResult() = default;
    QueryResult(const std::string& msg, bool ok = true)
        : message(msg), success(ok) {}

    std::string toPrettyTable() const {
        if (rows.empty() && columnNames.empty()) {
            return message.empty() ? "(empty result)" : message;
        }
        if (columnNames.empty()) return message;

        // Calculate column widths
        std::vector<size_t> widths(columnNames.size());
        for (size_t i = 0; i < columnNames.size(); i++)
            widths[i] = columnNames[i].size();

        for (const auto& row : rows) {
            for (size_t i = 0; i < row.size() && i < widths.size(); i++) {
                size_t len = row[i].asString().size();
                if (len > widths[i]) widths[i] = len;
            }
        }

        // Build the table
        std::ostringstream oss;
        auto printSeparator = [&]() {
            oss << "+";
            for (size_t w : widths)
                oss << std::string(w + 2, '-') << "+";
            oss << "\n";
        };

        // Header
        printSeparator();
        oss << "|";
        for (size_t i = 0; i < columnNames.size(); i++)
            oss << " " << std::left << std::setw(static_cast<int>(widths[i]))
                << columnNames[i] << " |";
        oss << "\n";
        printSeparator();

        // Data rows
        for (const auto& row : rows) {
            oss << "|";
            for (size_t i = 0; i < columnNames.size(); i++) {
                std::string val = (i < row.size()) ? row[i].asString() : "NULL";
                bool isNum = (i < row.size()) && row[i].isNumeric();
                if (isNum)
                    oss << " " << std::right << std::setw(static_cast<int>(widths[i]))
                        << val << " |";
                else
                    oss << " " << std::left << std::setw(static_cast<int>(widths[i]))
                        << val << " |";
            }
            oss << "\n";
        }
        printSeparator();
        oss << rows.size() << " row(s)\n";

        if (!message.empty())
            oss << message << "\n";

        return oss.str();
    }
};

class Table {
public:
    Table() = default;
    Table(const std::string& name, const std::vector<Column>& cols)
        : name_(name), columns_(cols) {
        for (size_t i = 0; i < cols.size(); i++)
            columnIndex_[cols[i].name] = i;
    }

    const std::string& getName() const { return name_; }
    const std::vector<Column>& getColumns() const { return columns_; }
    const std::vector<Row>& getRows() const { return rows_; }
    size_t rowCount() const { return rows_.size(); }
    size_t colCount() const { return columns_.size(); }

    int getColumnIndex(const std::string& colName) const {
        // Try exact match first
        auto it = columnIndex_.find(colName);
        if (it != columnIndex_.end()) return static_cast<int>(it->second);

        // Try table.column format
        std::string qualifiedName = name_ + "." + colName;
        it = columnIndex_.find(qualifiedName);
        if (it != columnIndex_.end()) return static_cast<int>(it->second);

        // Try stripping table prefix from colName
        size_t dotPos = colName.find('.');
        if (dotPos != std::string::npos) {
            std::string bareCol = colName.substr(dotPos + 1);
            it = columnIndex_.find(bareCol);
            if (it != columnIndex_.end()) return static_cast<int>(it->second);
        }

        return -1;
    }

    bool hasColumn(const std::string& colName) const {
        return getColumnIndex(colName) >= 0;
    }

    void insertRow(const Row& row) {
        if (row.size() != columns_.size())
            throw std::runtime_error("Row size (" + std::to_string(row.size()) +
                ") doesn't match column count (" + std::to_string(columns_.size()) + ")");

        // Type validation
        for (size_t i = 0; i < row.size(); i++) {
            if (row[i].isNull()) {
                if (!columns_[i].nullable)
                    throw std::runtime_error("Column '" + columns_[i].name + "' cannot be null");
                continue;
            }
            validateType(row[i], columns_[i]);
        }

        // Unique constraint checking
        for (size_t i = 0; i < columns_.size(); i++) {
            if (columns_[i].unique || columns_[i].primaryKey) {
                for (const auto& existingRow : rows_) {
                    if (existingRow[i] == row[i])
                        throw std::runtime_error("Duplicate value for unique column '" +
                            columns_[i].name + "'");
                }
            }
        }

        rows_.push_back(row);

        // Maintain indexes
        size_t rowIdx = rows_.size() - 1;
        for (auto& [name, idx] : indexes_) {
            int ci = idx.getColumnIndex();
            if (ci >= 0 && ci < static_cast<int>(row.size()))
                idx.insert(row[static_cast<size_t>(ci)], rowIdx);
        }
    }

    int deleteRows(const std::function<bool(const Row&)>& predicate) {
        int count = 0;
        auto it = rows_.begin();
        while (it != rows_.end()) {
            if (predicate(*it)) {
                it = rows_.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        if (count > 0) rebuildAllIndexes();
        return count;
    }

    int updateRows(const std::function<bool(const Row&)>& predicate,
                   const std::vector<std::pair<int, Value>>& updates) {
        int count = 0;
        for (auto& row : rows_) {
            if (predicate(row)) {
                for (const auto& [colIdx, newVal] : updates) {
                    if (colIdx >= 0 && colIdx < static_cast<int>(row.size()))
                        row[colIdx] = newVal;
                }
                count++;
            }
        }
        if (count > 0) rebuildAllIndexes();
        return count;
    }

    QueryResult selectAll() const {
        QueryResult result;
        for (const auto& col : columns_)
            result.columnNames.push_back(col.name);
        result.rows = rows_;
        return result;
    }

    QueryResult describe() const {
        QueryResult result;
        result.columnNames = {"Column", "Type", "Nullable", "Primary Key", "Unique"};
        for (const auto& col : columns_) {
            Row row;
            row.push_back(Value(col.name));
            row.push_back(Value(typeToString(col.type)));
            row.push_back(Value(col.nullable ? std::string("YES") : std::string("NO")));
            row.push_back(Value(col.primaryKey ? std::string("YES") : std::string("NO")));
            row.push_back(Value(col.unique ? std::string("YES") : std::string("NO")));
            result.rows.push_back(row);
        }
        return result;
    }

    // For transaction support - snapshot and restore
    std::vector<Row> snapshot() const { return rows_; }
    void restore(const std::vector<Row>& snap) {
        rows_ = snap;
        rebuildAllIndexes();
    }

    // Index management
    void createIndex(const std::string& indexName, const std::string& columnName, bool unique = false) {
        if (indexes_.find(indexName) != indexes_.end())
            throw std::runtime_error("Index '" + indexName + "' already exists");
        int colIdx = getColumnIndex(columnName);
        if (colIdx < 0)
            throw std::runtime_error("Column '" + columnName + "' does not exist in table '" + name_ + "'");
        BTreeIndex idx(indexName, name_, columnName, colIdx, unique);
        idx.rebuild(rows_);
        indexes_[indexName] = std::move(idx);
    }

    void dropIndex(const std::string& indexName) {
        auto it = indexes_.find(indexName);
        if (it == indexes_.end())
            throw std::runtime_error("Index '" + indexName + "' does not exist");
        indexes_.erase(it);
    }

    bool hasIndex(const std::string& indexName) const {
        return indexes_.find(indexName) != indexes_.end();
    }

    const BTreeIndex* getIndexForColumn(const std::string& columnName) const {
        for (const auto& [name, idx] : indexes_) {
            if (idx.getColumnName() == columnName) return &idx;
        }
        return nullptr;
    }

    const std::unordered_map<std::string, BTreeIndex>& getIndexes() const { return indexes_; }

    void rebuildAllIndexes() {
        for (auto& [name, idx] : indexes_)
            idx.rebuild(rows_);
    }

private:
    std::string name_;
    std::vector<Column> columns_;
    std::vector<Row> rows_;
    std::unordered_map<std::string, size_t> columnIndex_;
    std::unordered_map<std::string, BTreeIndex> indexes_;

    static std::string typeToString(ValueType t) {
        switch (t) {
            case ValueType::INT: return "int";
            case ValueType::DOUBLE: return "double";
            case ValueType::STRING: return "string";
            case ValueType::BOOL: return "bool";
            case ValueType::NULL_TYPE: return "null";
        }
        return "unknown";
    }

    void validateType(const Value& val, const Column& col) const {
        bool valid = false;
        switch (col.type) {
            case ValueType::INT:
                valid = val.isInt() || val.isDouble();
                break;
            case ValueType::DOUBLE:
                valid = val.isNumeric();
                break;
            case ValueType::STRING:
                valid = val.isString();
                break;
            case ValueType::BOOL:
                valid = val.isBool();
                break;
            case ValueType::NULL_TYPE:
                valid = true;
                break;
        }
        if (!valid)
            throw std::runtime_error("Type mismatch for column '" + col.name +
                "': expected " + typeToString(col.type) + ", got " + val.typeToString());
    }
};

class Database {
public:
    void createTable(const std::string& name, const std::vector<Column>& columns) {
        if (tables_.find(name) != tables_.end())
            throw std::runtime_error("Table '" + name + "' already exists");
        tables_[name] = Table(name, columns);
    }

    void dropTable(const std::string& name) {
        auto it = tables_.find(name);
        if (it == tables_.end())
            throw std::runtime_error("Table '" + name + "' does not exist");
        tables_.erase(it);
    }

    Table& getTable(const std::string& name) {
        auto it = tables_.find(name);
        if (it == tables_.end())
            throw std::runtime_error("Table '" + name + "' does not exist");
        return it->second;
    }

    const Table& getTable(const std::string& name) const {
        auto it = tables_.find(name);
        if (it == tables_.end())
            throw std::runtime_error("Table '" + name + "' does not exist");
        return it->second;
    }

    bool hasTable(const std::string& name) const {
        return tables_.find(name) != tables_.end();
    }

    QueryResult showTables() const {
        QueryResult result;
        result.columnNames = {"Table Name", "Columns", "Rows"};
        for (const auto& [name, table] : tables_) {
            Row row;
            row.push_back(Value(name));
            row.push_back(Value(static_cast<int>(table.colCount())));
            row.push_back(Value(static_cast<int>(table.rowCount())));
            result.rows.push_back(row);
        }
        return result;
    }

    // Transaction support
    void beginTransaction() {
        if (inTransaction_)
            throw std::runtime_error("Nested transactions not supported");
        inTransaction_ = true;
        snapshots_.clear();
        for (const auto& [name, table] : tables_)
            snapshots_[name] = table.snapshot();
    }

    void commitTransaction() {
        if (!inTransaction_)
            throw std::runtime_error("No active transaction");
        inTransaction_ = false;
        snapshots_.clear();
    }

    void rollbackTransaction() {
        if (!inTransaction_)
            throw std::runtime_error("No active transaction");
        for (auto& [name, table] : tables_) {
            auto it = snapshots_.find(name);
            if (it != snapshots_.end())
                table.restore(it->second);
        }
        inTransaction_ = false;
        snapshots_.clear();
    }

    bool inTransaction() const { return inTransaction_; }

    const std::unordered_map<std::string, Table>& getAllTables() const { return tables_; }

private:
    std::unordered_map<std::string, Table> tables_;
    bool inTransaction_ = false;
    std::unordered_map<std::string, std::vector<Row>> snapshots_;
};

} // namespace epee

#endif /* EPEE_TABLE_H */
