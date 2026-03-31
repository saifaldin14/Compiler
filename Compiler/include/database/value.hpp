/*
 File: value.hpp
 Project: Épée Database Query Language
 Description: Variant value type supporting int, double, string, bool, and null
*/

#ifndef EPEE_VALUE_H
#define EPEE_VALUE_H

#include <iostream>
#include <string>
#include <variant>
#include <stdexcept>
#include <cmath>
#include <functional>
#include <regex>
#include <sstream>
#include <iomanip>

namespace epee {

enum class ValueType { INT, DOUBLE, STRING, BOOL, NULL_TYPE };

class Value {
public:
    using VariantType = std::variant<std::monostate, int, double, std::string, bool>;

    Value() : data_(std::monostate{}), type_(ValueType::NULL_TYPE) {}
    explicit Value(int v) : data_(v), type_(ValueType::INT) {}
    explicit Value(double v) : data_(v), type_(ValueType::DOUBLE) {}
    explicit Value(const std::string& v) : data_(v), type_(ValueType::STRING) {}
    explicit Value(const char* v) : data_(std::string(v)), type_(ValueType::STRING) {}
    explicit Value(bool v) : data_(v), type_(ValueType::BOOL) {}

    static Value null() { return Value(); }

    ValueType getType() const { return type_; }
    bool isNull() const { return type_ == ValueType::NULL_TYPE; }
    bool isInt() const { return type_ == ValueType::INT; }
    bool isDouble() const { return type_ == ValueType::DOUBLE; }
    bool isString() const { return type_ == ValueType::STRING; }
    bool isBool() const { return type_ == ValueType::BOOL; }
    bool isNumeric() const { return isInt() || isDouble(); }

    int asInt() const {
        if (isInt()) return std::get<int>(data_);
        if (isDouble()) return static_cast<int>(std::get<double>(data_));
        if (isBool()) return std::get<bool>(data_) ? 1 : 0;
        throw std::runtime_error("Cannot convert to int");
    }

    double asDouble() const {
        if (isDouble()) return std::get<double>(data_);
        if (isInt()) return static_cast<double>(std::get<int>(data_));
        if (isBool()) return std::get<bool>(data_) ? 1.0 : 0.0;
        throw std::runtime_error("Cannot convert to double");
    }

    std::string asString() const {
        if (isString()) return std::get<std::string>(data_);
        if (isNull()) return "NULL";
        if (isInt()) return std::to_string(std::get<int>(data_));
        if (isDouble()) {
            std::ostringstream oss;
            double d = std::get<double>(data_);
            if (d == std::floor(d) && std::abs(d) < 1e15)
                oss << std::fixed << std::setprecision(1) << d;
            else
                oss << d;
            return oss.str();
        }
        if (isBool()) return std::get<bool>(data_) ? "true" : "false";
        return "NULL";
    }

    bool asBool() const {
        if (isBool()) return std::get<bool>(data_);
        if (isInt()) return std::get<int>(data_) != 0;
        if (isDouble()) return std::get<double>(data_) != 0.0;
        if (isString()) return !std::get<std::string>(data_).empty();
        return false;
    }

    std::string typeToString() const {
        switch (type_) {
            case ValueType::INT: return "int";
            case ValueType::DOUBLE: return "double";
            case ValueType::STRING: return "string";
            case ValueType::BOOL: return "bool";
            case ValueType::NULL_TYPE: return "null";
        }
        return "unknown";
    }

    // Arithmetic operations
    Value operator+(const Value& other) const {
        if (isString() || other.isString())
            return Value(asString() + other.asString());
        if (isDouble() || other.isDouble())
            return Value(asDouble() + other.asDouble());
        if (isInt() && other.isInt())
            return Value(asInt() + other.asInt());
        throw std::runtime_error("Invalid + operation");
    }

    Value operator-(const Value& other) const {
        if (isDouble() || other.isDouble())
            return Value(asDouble() - other.asDouble());
        if (isInt() && other.isInt())
            return Value(asInt() - other.asInt());
        throw std::runtime_error("Invalid - operation");
    }

    Value operator*(const Value& other) const {
        if (isDouble() || other.isDouble())
            return Value(asDouble() * other.asDouble());
        if (isInt() && other.isInt())
            return Value(asInt() * other.asInt());
        throw std::runtime_error("Invalid * operation");
    }

    Value operator/(const Value& other) const {
        if (other.isNumeric() && other.asDouble() == 0.0)
            throw std::runtime_error("Division by zero");
        if (isDouble() || other.isDouble())
            return Value(asDouble() / other.asDouble());
        if (isInt() && other.isInt())
            return Value(asInt() / other.asInt());
        throw std::runtime_error("Invalid / operation");
    }

    Value operator%(const Value& other) const {
        if (isInt() && other.isInt()) {
            if (other.asInt() == 0) throw std::runtime_error("Modulo by zero");
            return Value(asInt() % other.asInt());
        }
        if (isNumeric() && other.isNumeric())
            return Value(std::fmod(asDouble(), other.asDouble()));
        throw std::runtime_error("Invalid % operation");
    }

    Value operator-() const {
        if (isInt()) return Value(-asInt());
        if (isDouble()) return Value(-asDouble());
        throw std::runtime_error("Invalid unary - operation");
    }

    // Comparison operations
    bool operator==(const Value& other) const {
        if (isNull() && other.isNull()) return true;
        if (isNull() || other.isNull()) return false;
        if (isString() && other.isString())
            return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        if (isBool() && other.isBool())
            return std::get<bool>(data_) == std::get<bool>(other.data_);
        if (isNumeric() && other.isNumeric())
            return asDouble() == other.asDouble();
        return false;
    }

    bool operator!=(const Value& other) const { return !(*this == other); }

    bool operator<(const Value& other) const {
        if (isNull() || other.isNull()) return false;
        if (isString() && other.isString())
            return std::get<std::string>(data_) < std::get<std::string>(other.data_);
        if (isNumeric() && other.isNumeric())
            return asDouble() < other.asDouble();
        throw std::runtime_error("Incompatible types for comparison");
    }

    bool operator>(const Value& other) const { return other < *this; }
    bool operator<=(const Value& other) const { return !(other < *this); }
    bool operator>=(const Value& other) const { return !(*this < other); }

    // LIKE pattern matching (SQL-style with % wildcard)
    bool like(const std::string& pattern) const {
        if (!isString()) return false;
        const std::string& str = std::get<std::string>(data_);
        std::string regexPattern;
        for (char c : pattern) {
            if (c == '%') regexPattern += ".*";
            else if (c == '_') regexPattern += ".";
            else if (c == '.' || c == '[' || c == ']' || c == '(' || c == ')' ||
                     c == '{' || c == '}' || c == '\\' || c == '^' || c == '$' ||
                     c == '|' || c == '?' || c == '+')
                regexPattern += std::string("\\") + c;
            else regexPattern += c;
        }
        try {
            return std::regex_match(str, std::regex(regexPattern, std::regex_constants::icase));
        } catch (...) {
            return false;
        }
    }

    // BETWEEN check
    bool between(const Value& low, const Value& high) const {
        return *this >= low && *this <= high;
    }

    friend std::ostream& operator<<(std::ostream& os, const Value& v) {
        os << v.asString();
        return os;
    }

private:
    VariantType data_;
    ValueType type_;
};

} // namespace epee

namespace std {
    template<> struct hash<epee::Value> {
        size_t operator()(const epee::Value& v) const {
            switch (v.getType()) {
                case epee::ValueType::INT: return hash<int>{}(v.asInt());
                case epee::ValueType::DOUBLE: return hash<double>{}(v.asDouble());
                case epee::ValueType::STRING: return hash<string>{}(v.asString());
                case epee::ValueType::BOOL: return hash<bool>{}(v.asBool());
                case epee::ValueType::NULL_TYPE: return 0;
            }
            return 0;
        }
    };
}

#endif /* EPEE_VALUE_H */
