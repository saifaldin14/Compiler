/*
 File: btree.hpp
 Project: Épée Database Query Language
 Description: In-memory balanced tree index for fast value lookups
*/

#ifndef EPEE_BTREE_H
#define EPEE_BTREE_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include "value.hpp"

namespace epee {

// Index built on std::map (red-black tree) giving O(log n) lookups.
// Maps column values to sets of row indices.
class BTreeIndex {
public:
    BTreeIndex() = default;
    BTreeIndex(const std::string& name, const std::string& tableName,
               const std::string& columnName, int columnIndex, bool isUnique = false);

    const std::string& getName() const { return name_; }
    const std::string& getTableName() const { return tableName_; }
    const std::string& getColumnName() const { return columnName_; }
    int getColumnIndex() const { return columnIndex_; }
    bool isUnique() const { return unique_; }
    size_t size() const { return index_.size(); }

    // Mutators
    void insert(const Value& key, size_t rowIndex);
    void remove(const Value& key, size_t rowIndex);
    void clear();

    // Queries
    std::set<size_t> find(const Value& key) const;
    std::set<size_t> findRange(const Value& low, const Value& high) const;
    std::set<size_t> findGreaterThan(const Value& key) const;
    std::set<size_t> findLessThan(const Value& key) const;

    // Rebuild from scratch
    void rebuild(const std::vector<std::vector<Value>>& rows);

private:
    std::string name_;
    std::string tableName_;
    std::string columnName_;
    int columnIndex_ = -1;
    bool unique_ = false;

    struct ValueCompare {
        bool operator()(const Value& a, const Value& b) const {
            if (a.isNull() && b.isNull()) return false;
            if (a.isNull()) return true;
            if (b.isNull()) return false;
            try { return a < b; } catch (...) { return false; }
        }
    };
    std::map<Value, std::set<size_t>, ValueCompare> index_;
};

} // namespace epee

#endif /* EPEE_BTREE_H */
