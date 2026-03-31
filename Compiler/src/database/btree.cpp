/*
 File: btree.cpp
 Project: Épée Database Query Language
 Description: BTreeIndex implementation
*/

#include "../../include/database/btree.hpp"
#include <stdexcept>

namespace epee {

BTreeIndex::BTreeIndex(const std::string& name, const std::string& tableName,
                       const std::string& columnName, int columnIndex, bool isUnique)
    : name_(name), tableName_(tableName), columnName_(columnName),
      columnIndex_(columnIndex), unique_(isUnique) {}

void BTreeIndex::insert(const Value& key, size_t rowIndex) {
    if (unique_ && !key.isNull()) {
        auto it = index_.find(key);
        if (it != index_.end() && !it->second.empty())
            throw std::runtime_error("Duplicate key in unique index '" + name_ +
                "' for value " + key.asString());
    }
    index_[key].insert(rowIndex);
}

void BTreeIndex::remove(const Value& key, size_t rowIndex) {
    auto it = index_.find(key);
    if (it != index_.end()) {
        it->second.erase(rowIndex);
        if (it->second.empty())
            index_.erase(it);
    }
}

void BTreeIndex::clear() {
    index_.clear();
}

std::set<size_t> BTreeIndex::find(const Value& key) const {
    auto it = index_.find(key);
    if (it != index_.end())
        return it->second;
    return {};
}

std::set<size_t> BTreeIndex::findRange(const Value& low, const Value& high) const {
    std::set<size_t> result;
    ValueCompare cmp;
    auto lo = index_.lower_bound(low);
    for (auto it = lo; it != index_.end(); ++it) {
        if (cmp(high, it->first)) break;
        result.insert(it->second.begin(), it->second.end());
    }
    return result;
}

std::set<size_t> BTreeIndex::findGreaterThan(const Value& key) const {
    std::set<size_t> result;
    auto it = index_.upper_bound(key);
    for (; it != index_.end(); ++it)
        result.insert(it->second.begin(), it->second.end());
    return result;
}

std::set<size_t> BTreeIndex::findLessThan(const Value& key) const {
    std::set<size_t> result;
    for (auto it = index_.begin(); it != index_.end(); ++it) {
        ValueCompare cmp;
        if (!cmp(it->first, key)) break;
        result.insert(it->second.begin(), it->second.end());
    }
    return result;
}

void BTreeIndex::rebuild(const std::vector<std::vector<Value>>& rows) {
    index_.clear();
    for (size_t i = 0; i < rows.size(); i++) {
        if (columnIndex_ >= 0 && columnIndex_ < static_cast<int>(rows[i].size()))
            insert(rows[i][static_cast<size_t>(columnIndex_)], i);
    }
}

} // namespace epee
