/*
 File: security.cpp
 Project: Épée Database Query Language
 Description: SecurityManager implementation
*/

#include "../../include/database/security.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace epee {

// ---------------------------------------------------------------------------
// Permission helpers
// ---------------------------------------------------------------------------

std::string permissionToString(Permission p) {
    switch (p) {
        case Permission::SELECT:      return "SELECT";
        case Permission::INSERT:      return "INSERT";
        case Permission::UPDATE:      return "UPDATE";
        case Permission::DELETE_PERM: return "DELETE";
        case Permission::CREATE:      return "CREATE";
        case Permission::DROP:        return "DROP";
        case Permission::ALL:         return "ALL";
    }
    return "UNKNOWN";
}

Permission stringToPermission(const std::string& s) {
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    if (upper == "SELECT") return Permission::SELECT;
    if (upper == "INSERT") return Permission::INSERT;
    if (upper == "UPDATE") return Permission::UPDATE;
    if (upper == "DELETE") return Permission::DELETE_PERM;
    if (upper == "CREATE") return Permission::CREATE;
    if (upper == "DROP")   return Permission::DROP;
    if (upper == "ALL")    return Permission::ALL;
    throw std::runtime_error("Unknown permission: " + s);
}

// FNV-1a 64-bit with a fixed salt prefix
std::string hashPassword(const std::string& password) {
    const std::string salted = "epee_salt_" + password;
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char c : salted) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return oss.str();
}

// ---------------------------------------------------------------------------
// SecurityManager
// ---------------------------------------------------------------------------

SecurityManager::SecurityManager() = default;

void SecurityManager::createUser(const std::string& name, const std::string& password, bool isAdmin) {
    if (users_.find(name) != users_.end())
        throw std::runtime_error("User '" + name + "' already exists");
    User u;
    u.name = name;
    u.passwordHash = hashPassword(password);
    u.isAdmin = isAdmin;
    users_[name] = std::move(u);
}

void SecurityManager::dropUser(const std::string& name) {
    auto it = users_.find(name);
    if (it == users_.end())
        throw std::runtime_error("User '" + name + "' does not exist");
    users_.erase(it);
    if (currentUser_ == name)
        currentUser_.clear();
}

bool SecurityManager::authenticate(const std::string& name, const std::string& password) const {
    auto it = users_.find(name);
    if (it == users_.end()) return false;
    return it->second.passwordHash == hashPassword(password);
}

bool SecurityManager::userExists(const std::string& name) const {
    return users_.find(name) != users_.end();
}

void SecurityManager::grant(const std::string& userName, Permission perm, const std::string& tableName) {
    auto it = users_.find(userName);
    if (it == users_.end())
        throw std::runtime_error("User '" + userName + "' does not exist");
    if (tableName.empty())
        it->second.globalPermissions.insert(perm);
    else
        it->second.tablePermissions[tableName].insert(perm);
}

void SecurityManager::revoke(const std::string& userName, Permission perm, const std::string& tableName) {
    auto it = users_.find(userName);
    if (it == users_.end())
        throw std::runtime_error("User '" + userName + "' does not exist");
    if (tableName.empty())
        it->second.globalPermissions.erase(perm);
    else {
        auto tit = it->second.tablePermissions.find(tableName);
        if (tit != it->second.tablePermissions.end())
            tit->second.erase(perm);
    }
}

bool SecurityManager::hasPermission(const std::string& userName, Permission perm, const std::string& tableName) const {
    auto it = users_.find(userName);
    if (it == users_.end()) return false;
    const User& u = it->second;
    if (u.isAdmin) return true;

    // Check global ALL
    if (u.globalPermissions.count(Permission::ALL)) return true;
    // Check global specific
    if (u.globalPermissions.count(perm)) return true;
    // Check table ALL
    auto tit = u.tablePermissions.find(tableName);
    if (tit != u.tablePermissions.end()) {
        if (tit->second.count(Permission::ALL)) return true;
        if (tit->second.count(perm)) return true;
    }
    return false;
}

void SecurityManager::login(const std::string& userName) {
    if (users_.find(userName) == users_.end())
        throw std::runtime_error("User '" + userName + "' does not exist");
    currentUser_ = userName;
}

void SecurityManager::logout() {
    currentUser_.clear();
}

QueryResult SecurityManager::showUsers() const {
    QueryResult result;
    result.columnNames = {"Username", "Admin"};
    for (const auto& [name, user] : users_) {
        Row row;
        row.push_back(Value(name));
        row.push_back(Value(user.isAdmin ? std::string("YES") : std::string("NO")));
        result.rows.push_back(row);
    }
    return result;
}

QueryResult SecurityManager::showGrants(const std::string& userName) const {
    auto it = users_.find(userName);
    if (it == users_.end())
        throw std::runtime_error("User '" + userName + "' does not exist");
    const User& u = it->second;

    QueryResult result;
    result.columnNames = {"Permission", "Scope"};
    for (const auto& p : u.globalPermissions) {
        Row row;
        row.push_back(Value(permissionToString(p)));
        row.push_back(Value(std::string("GLOBAL")));
        result.rows.push_back(row);
    }
    for (const auto& [table, perms] : u.tablePermissions) {
        for (const auto& p : perms) {
            Row row;
            row.push_back(Value(permissionToString(p)));
            row.push_back(Value(table));
            result.rows.push_back(row);
        }
    }
    return result;
}

} // namespace epee
