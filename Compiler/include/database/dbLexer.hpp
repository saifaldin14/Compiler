/*
 File: dbLexer.hpp
 Project: Épée Database Query Language
 Description: Lexer for the database query language with pipeline support
*/

#ifndef EPEE_DB_LEXER_H
#define EPEE_DB_LEXER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <stdexcept>
#include <algorithm>

namespace epee {

enum class DbTokenType {
    // Literals
    INT_LIT, DOUBLE_LIT, STRING_LIT, BOOL_LIT, NULL_LIT,

    // Identifier
    IDENTIFIER,

    // Keywords - DDL
    CREATE, TABLE, DROP, DESCRIBE, SHOW, TABLES, INDEX, UNIQUE,

    // Keywords - DML
    SELECT, INSERT, INTO, VALUES, UPDATE, SET, DELETE_KW, FROM, WHERE,

    // Keywords - Pipeline stages
    PIPE,  // |>

    // Keywords - Clauses
    JOIN, INNER, LEFT, RIGHT, OUTER, CROSS, ON,
    GROUPBY, ORDERBY, HAVING, LIMIT, OFFSET,
    AS, ASC, DESC, DISTINCT,

    // Keywords - Logical
    AND, OR, NOT,

    // Keywords - Predicates
    BETWEEN, IN, LIKE, EXISTS, IS,

    // Keywords - Set operations
    UNION, INTERSECT, EXCEPT,

    // Keywords - Transaction
    BEGIN_KW, COMMIT, ROLLBACK,

    // Keywords - Types
    INT_TYPE, DOUBLE_TYPE, STRING_TYPE, BOOL_TYPE,

    // Keywords - Aggregate functions
    COUNT, SUM, AVG, MIN_FN, MAX_FN,

    // Keywords - Control flow (from original language)
    IF, THEN, ELSE, FI, WHILE, DO, OD, DEF, FED, RETURN, PRINT,

    // Keywords - String functions
    UPPER, LOWER, LENGTH, SUBSTR, CONCAT, TRIM, REPLACE,

    // Keywords - Utility functions
    COALESCE, NULLIF, TYPEOF, CAST, LEFT_FN, RIGHT_FN,
    LPAD, RPAD, REVERSE, REPEAT_FN,
    POWER, SQRT, LOG_FN, PI_FN, RANDOM, NOW,
    IIF,

    // Keywords - CASE expression
    CASE, WHEN, END_KW,

    // Keywords - Persistence
    SAVE, LOAD, DATABASE,

    // Keywords - Pipeline aliases
    TAKE, SKIP_KW, MAP,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, NEQ, LT, GT, LTE, GTE, EQEQ,
    DOT,

    // Delimiters
    COMMA, SEMICOLON, LPAREN, RPAREN, LBRACKET, RBRACKET,

    // Special
    EOF_TOKEN, ERROR_TOKEN
};

struct DbToken {
    DbTokenType type;
    std::string value;
    int line;
    int col;

    DbToken() : type(DbTokenType::EOF_TOKEN), line(0), col(0) {}
    DbToken(DbTokenType t, const std::string& v, int l, int c)
        : type(t), value(v), line(l), col(c) {}

    bool is(DbTokenType t) const { return type == t; }
    bool isKeyword() const {
        return type >= DbTokenType::CREATE && type <= DbTokenType::REPLACE;
    }

    std::string typeToString() const;
};

class DbLexer {
public:
    DbLexer();
    explicit DbLexer(const std::string& source);

    std::vector<DbToken> tokenize();
    void setSource(const std::string& source);

private:
    std::string source_;
    size_t pos_;
    int line_;
    int col_;

    void initKeywords();
    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();

    DbToken scanToken();
    DbToken scanNumber();
    DbToken scanString();
    DbToken scanIdentifierOrKeyword();

    DbToken makeToken(DbTokenType type, const std::string& value);

    std::unordered_map<std::string, DbTokenType> keywords_;
};

} // namespace epee

#endif /* EPEE_DB_LEXER_H */
