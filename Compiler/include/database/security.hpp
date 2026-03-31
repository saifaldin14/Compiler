/*
 File: security.hpp
 Project: Épée Database Query Language
 Description: Authentication and access control
*/

#ifndef EPEE_SECURITY_H
#define EPEE_SECURITY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <cstdint>
#include "table.hpp"

namespace epee {

enum class Permission {
    SELECT, INSERT, UPDATE, DELETE_PERM, CREATE, DROP, ALL
};

std::string permissionToString(Permission p);
Permission stringToPermission(const std::string& s);

// FNV-1a 64-bit hash (no external deps)
std::string hashPassword(const std::string& password);

struct User {
    std::string name;
    std::string passwordHash;
    bool isAdmin = false;
    std::unordered_map<std::string, std::set<Permission>> tablePermissions;
    std::set<Permission> globalPermissions;
};

class SecurityManager {
public:
    SecurityManager();

    void createUser(const std::string& name, const std::string& password, bool isAdmin = false);
    void dropUser(const std::string& name);
    bool authenticate(const std::string& name, const std::string& password) const;
    bool userExists(const std::string& name) const;

    void grant(const std::string& userName, Permission perm, const std::string& tableName = "");
    void revoke(const std::string& userName, Permission perm, const std::string& tableName = "");
    bool hasPermission(const std::string& userName, Permission perm, const std::string& tableName) const;

    void login(const std::string& userName);
    void logout();
    bool isLoggedIn() const { return !currentUser_.empty(); }
    const std::string& currentUser() const { return currentUser_; }

    bool isEnabled() const { return !users_.empty(); }

    QueryResult showUsers() const;
    QueryResult showGrants(const std::string& userName) const;

private:
    std::unordered_map<std::string, User> users_;
    std::string currentUser_;
};

} // namespace epee

#endif /* EPEE_SECURITY_H */
