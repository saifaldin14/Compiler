/*
 File: dbLexer.cpp
 Project: Épée Database Query Language
 Description: Implementation of the database query language lexer
*/

#include "../../include/database/dbLexer.hpp"

namespace epee {

std::string DbToken::typeToString() const {
    switch (type) {
        case DbTokenType::INT_LIT: return "INT_LIT";
        case DbTokenType::DOUBLE_LIT: return "DOUBLE_LIT";
        case DbTokenType::STRING_LIT: return "STRING_LIT";
        case DbTokenType::BOOL_LIT: return "BOOL_LIT";
        case DbTokenType::NULL_LIT: return "NULL_LIT";
        case DbTokenType::IDENTIFIER: return "IDENTIFIER";
        case DbTokenType::CREATE: return "CREATE";
        case DbTokenType::TABLE: return "TABLE";
        case DbTokenType::DROP: return "DROP";
        case DbTokenType::DESCRIBE: return "DESCRIBE";
        case DbTokenType::SHOW: return "SHOW";
        case DbTokenType::TABLES: return "TABLES";
        case DbTokenType::INDEX: return "INDEX";
        case DbTokenType::UNIQUE: return "UNIQUE";
        case DbTokenType::SELECT: return "SELECT";
        case DbTokenType::INSERT: return "INSERT";
        case DbTokenType::INTO: return "INTO";
        case DbTokenType::VALUES: return "VALUES";
        case DbTokenType::UPDATE: return "UPDATE";
        case DbTokenType::SET: return "SET";
        case DbTokenType::DELETE_KW: return "DELETE";
        case DbTokenType::FROM: return "FROM";
        case DbTokenType::WHERE: return "WHERE";
        case DbTokenType::PIPE: return "PIPE";
        case DbTokenType::JOIN: return "JOIN";
        case DbTokenType::INNER: return "INNER";
        case DbTokenType::LEFT: return "LEFT";
        case DbTokenType::RIGHT: return "RIGHT";
        case DbTokenType::OUTER: return "OUTER";
        case DbTokenType::CROSS: return "CROSS";
        case DbTokenType::ON: return "ON";
        case DbTokenType::GROUPBY: return "GROUPBY";
        case DbTokenType::ORDERBY: return "ORDERBY";
        case DbTokenType::HAVING: return "HAVING";
        case DbTokenType::LIMIT: return "LIMIT";
        case DbTokenType::OFFSET: return "OFFSET";
        case DbTokenType::AS: return "AS";
        case DbTokenType::ASC: return "ASC";
        case DbTokenType::DESC: return "DESC";
        case DbTokenType::DISTINCT: return "DISTINCT";
        case DbTokenType::AND: return "AND";
        case DbTokenType::OR: return "OR";
        case DbTokenType::NOT: return "NOT";
        case DbTokenType::BETWEEN: return "BETWEEN";
        case DbTokenType::IN: return "IN";
        case DbTokenType::LIKE: return "LIKE";
        case DbTokenType::EXISTS: return "EXISTS";
        case DbTokenType::IS: return "IS";
        case DbTokenType::UNION: return "UNION";
        case DbTokenType::INTERSECT: return "INTERSECT";
        case DbTokenType::EXCEPT: return "EXCEPT";
        case DbTokenType::BEGIN_KW: return "BEGIN";
        case DbTokenType::COMMIT: return "COMMIT";
        case DbTokenType::ROLLBACK: return "ROLLBACK";
        case DbTokenType::INT_TYPE: return "INT_TYPE";
        case DbTokenType::DOUBLE_TYPE: return "DOUBLE_TYPE";
        case DbTokenType::STRING_TYPE: return "STRING_TYPE";
        case DbTokenType::BOOL_TYPE: return "BOOL_TYPE";
        case DbTokenType::COUNT: return "COUNT";
        case DbTokenType::SUM: return "SUM";
        case DbTokenType::AVG: return "AVG";
        case DbTokenType::MIN_FN: return "MIN";
        case DbTokenType::MAX_FN: return "MAX";
        case DbTokenType::IF: return "IF";
        case DbTokenType::THEN: return "THEN";
        case DbTokenType::ELSE: return "ELSE";
        case DbTokenType::FI: return "FI";
        case DbTokenType::WHILE: return "WHILE";
        case DbTokenType::DO: return "DO";
        case DbTokenType::OD: return "OD";
        case DbTokenType::DEF: return "DEF";
        case DbTokenType::FED: return "FED";
        case DbTokenType::RETURN: return "RETURN";
        case DbTokenType::PRINT: return "PRINT";
        case DbTokenType::UPPER: return "UPPER";
        case DbTokenType::LOWER: return "LOWER";
        case DbTokenType::LENGTH: return "LENGTH";
        case DbTokenType::SUBSTR: return "SUBSTR";
        case DbTokenType::CONCAT: return "CONCAT";
        case DbTokenType::TRIM: return "TRIM";
        case DbTokenType::REPLACE: return "REPLACE";
        case DbTokenType::COALESCE: return "COALESCE";
        case DbTokenType::NULLIF: return "NULLIF";
        case DbTokenType::TYPEOF: return "TYPEOF";
        case DbTokenType::CAST: return "CAST";
        case DbTokenType::LEFT_FN: return "LEFT_FN";
        case DbTokenType::RIGHT_FN: return "RIGHT_FN";
        case DbTokenType::LPAD: return "LPAD";
        case DbTokenType::RPAD: return "RPAD";
        case DbTokenType::REVERSE: return "REVERSE";
        case DbTokenType::REPEAT_FN: return "REPEAT_FN";
        case DbTokenType::POWER: return "POWER";
        case DbTokenType::SQRT: return "SQRT";
        case DbTokenType::LOG_FN: return "LOG_FN";
        case DbTokenType::PI_FN: return "PI_FN";
        case DbTokenType::RANDOM: return "RANDOM";
        case DbTokenType::NOW: return "NOW";
        case DbTokenType::IIF: return "IIF";
        case DbTokenType::CASE: return "CASE";
        case DbTokenType::WHEN: return "WHEN";
        case DbTokenType::END_KW: return "END";
        case DbTokenType::TAKE: return "TAKE";
        case DbTokenType::SKIP_KW: return "SKIP";
        case DbTokenType::MAP: return "MAP";
        case DbTokenType::SAVE: return "SAVE";
        case DbTokenType::LOAD: return "LOAD";
        case DbTokenType::DATABASE: return "DATABASE";
        case DbTokenType::USER: return "USER";
        case DbTokenType::PASSWORD: return "PASSWORD";
        case DbTokenType::GRANT_KW: return "GRANT";
        case DbTokenType::REVOKE_KW: return "REVOKE";
        case DbTokenType::LOGIN: return "LOGIN";
        case DbTokenType::LOGOUT: return "LOGOUT";
        case DbTokenType::TO: return "TO";
        case DbTokenType::PRIVILEGES: return "PRIVILEGES";
        case DbTokenType::EXPLAIN: return "EXPLAIN";
        case DbTokenType::PLUS: return "PLUS";
        case DbTokenType::MINUS: return "MINUS";
        case DbTokenType::STAR: return "STAR";
        case DbTokenType::SLASH: return "SLASH";
        case DbTokenType::PERCENT: return "PERCENT";
        case DbTokenType::EQ: return "EQ";
        case DbTokenType::NEQ: return "NEQ";
        case DbTokenType::LT: return "LT";
        case DbTokenType::GT: return "GT";
        case DbTokenType::LTE: return "LTE";
        case DbTokenType::GTE: return "GTE";
        case DbTokenType::EQEQ: return "EQEQ";
        case DbTokenType::DOT: return "DOT";
        case DbTokenType::COMMA: return "COMMA";
        case DbTokenType::SEMICOLON: return "SEMICOLON";
        case DbTokenType::LPAREN: return "LPAREN";
        case DbTokenType::RPAREN: return "RPAREN";
        case DbTokenType::LBRACKET: return "LBRACKET";
        case DbTokenType::RBRACKET: return "RBRACKET";
        case DbTokenType::EOF_TOKEN: return "EOF";
        case DbTokenType::ERROR_TOKEN: return "ERROR";
    }
    return "UNKNOWN";
}

DbLexer::DbLexer() : pos_(0), line_(1), col_(1) {
    initKeywords();
}

DbLexer::DbLexer(const std::string& source) : source_(source), pos_(0), line_(1), col_(1) {
    initKeywords();
}

void DbLexer::setSource(const std::string& source) {
    source_ = source;
    pos_ = 0;
    line_ = 1;
    col_ = 1;
}

void DbLexer::initKeywords() {
    // DDL
    keywords_["create"] = DbTokenType::CREATE;
    keywords_["table"] = DbTokenType::TABLE;
    keywords_["drop"] = DbTokenType::DROP;
    keywords_["describe"] = DbTokenType::DESCRIBE;
    keywords_["show"] = DbTokenType::SHOW;
    keywords_["tables"] = DbTokenType::TABLES;
    keywords_["index"] = DbTokenType::INDEX;
    keywords_["unique"] = DbTokenType::UNIQUE;

    // DML
    keywords_["select"] = DbTokenType::SELECT;
    keywords_["insert"] = DbTokenType::INSERT;
    keywords_["into"] = DbTokenType::INTO;
    keywords_["values"] = DbTokenType::VALUES;
    keywords_["update"] = DbTokenType::UPDATE;
    keywords_["set"] = DbTokenType::SET;
    keywords_["delete"] = DbTokenType::DELETE_KW;
    keywords_["from"] = DbTokenType::FROM;
    keywords_["where"] = DbTokenType::WHERE;

    // Joins
    keywords_["join"] = DbTokenType::JOIN;
    keywords_["inner"] = DbTokenType::INNER;
    keywords_["left"] = DbTokenType::LEFT;
    keywords_["right"] = DbTokenType::RIGHT;
    keywords_["outer"] = DbTokenType::OUTER;
    keywords_["cross"] = DbTokenType::CROSS;
    keywords_["on"] = DbTokenType::ON;

    // Clauses
    keywords_["groupby"] = DbTokenType::GROUPBY;
    keywords_["orderby"] = DbTokenType::ORDERBY;
    keywords_["having"] = DbTokenType::HAVING;
    keywords_["limit"] = DbTokenType::LIMIT;
    keywords_["offset"] = DbTokenType::OFFSET;
    keywords_["as"] = DbTokenType::AS;
    keywords_["asc"] = DbTokenType::ASC;
    keywords_["desc"] = DbTokenType::DESC;
    keywords_["distinct"] = DbTokenType::DISTINCT;

    // Logical
    keywords_["and"] = DbTokenType::AND;
    keywords_["or"] = DbTokenType::OR;
    keywords_["not"] = DbTokenType::NOT;

    // Predicates
    keywords_["between"] = DbTokenType::BETWEEN;
    keywords_["in"] = DbTokenType::IN;
    keywords_["like"] = DbTokenType::LIKE;
    keywords_["exists"] = DbTokenType::EXISTS;
    keywords_["is"] = DbTokenType::IS;

    // Set operations
    keywords_["union"] = DbTokenType::UNION;
    keywords_["intersect"] = DbTokenType::INTERSECT;
    keywords_["except"] = DbTokenType::EXCEPT;

    // Transaction
    keywords_["begin"] = DbTokenType::BEGIN_KW;
    keywords_["commit"] = DbTokenType::COMMIT;
    keywords_["rollback"] = DbTokenType::ROLLBACK;

    // Types
    keywords_["int"] = DbTokenType::INT_TYPE;
    keywords_["double"] = DbTokenType::DOUBLE_TYPE;
    keywords_["string"] = DbTokenType::STRING_TYPE;
    keywords_["bool"] = DbTokenType::BOOL_TYPE;

    // Aggregates
    keywords_["count"] = DbTokenType::COUNT;
    keywords_["sum"] = DbTokenType::SUM;
    keywords_["avg"] = DbTokenType::AVG;
    keywords_["min"] = DbTokenType::MIN_FN;
    keywords_["max"] = DbTokenType::MAX_FN;

    // Boolean literals
    keywords_["true"] = DbTokenType::BOOL_LIT;
    keywords_["false"] = DbTokenType::BOOL_LIT;
    keywords_["null"] = DbTokenType::NULL_LIT;

    // Control flow (from original language)
    keywords_["if"] = DbTokenType::IF;
    keywords_["then"] = DbTokenType::THEN;
    keywords_["else"] = DbTokenType::ELSE;
    keywords_["fi"] = DbTokenType::FI;
    keywords_["while"] = DbTokenType::WHILE;
    keywords_["do"] = DbTokenType::DO;
    keywords_["od"] = DbTokenType::OD;
    keywords_["def"] = DbTokenType::DEF;
    keywords_["fed"] = DbTokenType::FED;
    keywords_["return"] = DbTokenType::RETURN;
    keywords_["print"] = DbTokenType::PRINT;

    // String functions
    keywords_["upper"] = DbTokenType::UPPER;
    keywords_["lower"] = DbTokenType::LOWER;
    keywords_["length"] = DbTokenType::LENGTH;
    keywords_["substr"] = DbTokenType::SUBSTR;
    keywords_["concat"] = DbTokenType::CONCAT;
    keywords_["trim"] = DbTokenType::TRIM;
    keywords_["replace"] = DbTokenType::REPLACE;

    // Utility functions
    keywords_["coalesce"] = DbTokenType::COALESCE;
    keywords_["nullif"] = DbTokenType::NULLIF;
    keywords_["typeof"] = DbTokenType::TYPEOF;
    keywords_["cast"] = DbTokenType::CAST;
    keywords_["lpad"] = DbTokenType::LPAD;
    keywords_["rpad"] = DbTokenType::RPAD;
    keywords_["reverse"] = DbTokenType::REVERSE;
    keywords_["repeat"] = DbTokenType::REPEAT_FN;
    keywords_["power"] = DbTokenType::POWER;
    keywords_["sqrt"] = DbTokenType::SQRT;
    keywords_["log"] = DbTokenType::LOG_FN;
    keywords_["pi"] = DbTokenType::PI_FN;
    keywords_["random"] = DbTokenType::RANDOM;
    keywords_["now"] = DbTokenType::NOW;
    keywords_["iif"] = DbTokenType::IIF;

    // CASE expression
    keywords_["case"] = DbTokenType::CASE;
    keywords_["when"] = DbTokenType::WHEN;
    keywords_["end"] = DbTokenType::END_KW;

    // Pipeline aliases
    keywords_["take"] = DbTokenType::TAKE;
    keywords_["skip"] = DbTokenType::SKIP_KW;
    keywords_["map"] = DbTokenType::MAP;

    // Persistence
    keywords_["save"] = DbTokenType::SAVE;
    keywords_["load"] = DbTokenType::LOAD;
    keywords_["database"] = DbTokenType::DATABASE;

    // Security
    keywords_["user"] = DbTokenType::USER;
    keywords_["password"] = DbTokenType::PASSWORD;
    keywords_["grant"] = DbTokenType::GRANT_KW;
    keywords_["revoke"] = DbTokenType::REVOKE_KW;
    keywords_["login"] = DbTokenType::LOGIN;
    keywords_["logout"] = DbTokenType::LOGOUT;
    keywords_["to"] = DbTokenType::TO;
    keywords_["privileges"] = DbTokenType::PRIVILEGES;

    // Operational
    keywords_["explain"] = DbTokenType::EXPLAIN;
}

char DbLexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[pos_];
}

char DbLexer::peekNext() const {
    if (pos_ + 1 >= source_.size()) return '\0';
    return source_[pos_ + 1];
}

char DbLexer::advance() {
    char c = source_[pos_++];
    if (c == '\n') { line_++; col_ = 1; }
    else col_++;
    return c;
}

bool DbLexer::isAtEnd() const {
    return pos_ >= source_.size();
}

void DbLexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peekNext() == '/') {
            skipLineComment();
        } else if (c == '/' && peekNext() == '*') {
            skipBlockComment();
        } else {
            break;
        }
    }
}

void DbLexer::skipLineComment() {
    while (!isAtEnd() && peek() != '\n')
        advance();
}

void DbLexer::skipBlockComment() {
    advance(); // /
    advance(); // *
    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
            advance(); // *
            advance(); // /
            return;
        }
        advance();
    }
}

DbToken DbLexer::makeToken(DbTokenType type, const std::string& value) {
    return DbToken(type, value, line_, col_);
}

std::vector<DbToken> DbLexer::tokenize() {
    std::vector<DbToken> tokens;
    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;
        DbToken token = scanToken();
        if (token.type != DbTokenType::ERROR_TOKEN)
            tokens.push_back(token);
    }
    tokens.push_back(makeToken(DbTokenType::EOF_TOKEN, ""));
    return tokens;
}

DbToken DbLexer::scanToken() {
    char c = peek();

    // Numbers
    if (std::isdigit(c)) return scanNumber();

    // Strings
    if (c == '"') return scanString();

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') return scanIdentifierOrKeyword();

    // Pipeline operator |>
    if (c == '|' && peekNext() == '>') {
        advance(); advance();
        return makeToken(DbTokenType::PIPE, "|>");
    }

    // Two-character operators
    if (c == '<') {
        advance();
        if (peek() == '=') { advance(); return makeToken(DbTokenType::LTE, "<="); }
        if (peek() == '>') { advance(); return makeToken(DbTokenType::NEQ, "<>"); }
        return makeToken(DbTokenType::LT, "<");
    }
    if (c == '>') {
        advance();
        if (peek() == '=') { advance(); return makeToken(DbTokenType::GTE, ">="); }
        return makeToken(DbTokenType::GT, ">");
    }
    if (c == '=') {
        advance();
        if (peek() == '=') { advance(); return makeToken(DbTokenType::EQEQ, "=="); }
        return makeToken(DbTokenType::EQ, "=");
    }
    if (c == '!' && peekNext() == '=') {
        advance(); advance();
        return makeToken(DbTokenType::NEQ, "!=");
    }

    // Single-character tokens
    advance();
    switch (c) {
        case '+': return makeToken(DbTokenType::PLUS, "+");
        case '-': return makeToken(DbTokenType::MINUS, "-");
        case '*': return makeToken(DbTokenType::STAR, "*");
        case '/': return makeToken(DbTokenType::SLASH, "/");
        case '%': return makeToken(DbTokenType::PERCENT, "%");
        case '.': return makeToken(DbTokenType::DOT, ".");
        case ',': return makeToken(DbTokenType::COMMA, ",");
        case ';': return makeToken(DbTokenType::SEMICOLON, ";");
        case '(': return makeToken(DbTokenType::LPAREN, "(");
        case ')': return makeToken(DbTokenType::RPAREN, ")");
        case '[': return makeToken(DbTokenType::LBRACKET, "[");
        case ']': return makeToken(DbTokenType::RBRACKET, "]");
        default:
            return makeToken(DbTokenType::ERROR_TOKEN, std::string(1, c));
    }
}

DbToken DbLexer::scanNumber() {
    std::string num;
    while (!isAtEnd() && std::isdigit(peek()))
        num += advance();

    bool isDouble = false;

    // Decimal part
    if (!isAtEnd() && peek() == '.' && std::isdigit(peekNext())) {
        isDouble = true;
        num += advance(); // .
        while (!isAtEnd() && std::isdigit(peek()))
            num += advance();
    }

    // Scientific notation
    if (!isAtEnd() && (peek() == 'e' || peek() == 'E')) {
        isDouble = true;
        num += advance();
        if (!isAtEnd() && (peek() == '+' || peek() == '-'))
            num += advance();
        while (!isAtEnd() && std::isdigit(peek()))
            num += advance();
    }

    return makeToken(isDouble ? DbTokenType::DOUBLE_LIT : DbTokenType::INT_LIT, num);
}

DbToken DbLexer::scanString() {
    advance(); // opening "
    std::string str;
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\\') {
            advance();
            if (isAtEnd()) break;
            char esc = advance();
            switch (esc) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += esc; break;
            }
        } else {
            str += advance();
        }
    }
    if (!isAtEnd()) advance(); // closing "
    return makeToken(DbTokenType::STRING_LIT, str);
}

DbToken DbLexer::scanIdentifierOrKeyword() {
    std::string id;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
        id += advance();

    // Check for keywords (case-insensitive)
    std::string lower = id;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto it = keywords_.find(lower);
    if (it != keywords_.end()) {
        // Special handling for boolean and null literals
        if (it->second == DbTokenType::BOOL_LIT)
            return makeToken(DbTokenType::BOOL_LIT, lower);
        if (it->second == DbTokenType::NULL_LIT)
            return makeToken(DbTokenType::NULL_LIT, lower);
        return makeToken(it->second, lower);
    }

    return makeToken(DbTokenType::IDENTIFIER, id);
}

} // namespace epee
