/*
 File: dbParser.cpp
 Project: Épée Database Query Language
 Description: Implementation of the recursive descent parser for database
              statements, pipeline queries, and control flow.
*/

#include "../../include/database/dbParser.hpp"

namespace epee {

// ── Constructors & Setup ─────────────────────────────────────────────

DbParser::DbParser() : pos_(0) {}

DbParser::DbParser(const std::vector<DbToken>& tokens)
    : tokens_(tokens), pos_(0) {}

void DbParser::setTokens(const std::vector<DbToken>& tokens) {
    tokens_ = tokens;
    pos_ = 0;
    errors_.clear();
}

// ── Token Navigation ─────────────────────────────────────────────────

static const DbToken kEofToken(DbTokenType::EOF_TOKEN, "", 0, 0);

const DbToken& DbParser::peek() const {
    if (pos_ < tokens_.size()) return tokens_[pos_];
    return kEofToken;
}

const DbToken& DbParser::peekNext() const {
    if (pos_ + 1 < tokens_.size()) return tokens_[pos_ + 1];
    return kEofToken;
}

const DbToken& DbParser::advance() {
    if (pos_ < tokens_.size()) return tokens_[pos_++];
    return kEofToken;
}

bool DbParser::check(DbTokenType type) const {
    return peek().type == type;
}

bool DbParser::match(DbTokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

const DbToken& DbParser::expect(DbTokenType type, const std::string& msg) {
    if (check(type)) return advance();
    error(msg + " (got '" + peek().value + "' at line " +
          std::to_string(peek().line) + ", col " +
          std::to_string(peek().col) + ")");
    // Return current token to allow continued parsing
    return peek();
}

bool DbParser::isAtEnd() const {
    return pos_ >= tokens_.size() || peek().type == DbTokenType::EOF_TOKEN;
}

void DbParser::error(const std::string& msg) {
    errors_.push_back(msg);
}

// ── Top-Level Parse ──────────────────────────────────────────────────

std::vector<StmtPtr> DbParser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(stmt);
        } else {
            // Error recovery: skip to next semicolon or known keyword
            if (!isAtEnd()) advance();
        }
    }
    return statements;
}

// ── Statement Dispatch ───────────────────────────────────────────────

StmtPtr DbParser::parseStatement() {
    if (isAtEnd()) return nullptr;

    switch (peek().type) {
        case DbTokenType::CREATE:    return parseCreateTable();
        case DbTokenType::DROP:      return parseDropTable();
        case DbTokenType::INSERT:    return parseInsert();
        case DbTokenType::SELECT:    return parseSelect();
        case DbTokenType::UPDATE:    return parseUpdate();
        case DbTokenType::DELETE_KW: return parseDelete();
        case DbTokenType::BEGIN_KW:  return parseBegin();
        case DbTokenType::COMMIT:    return parseCommit();
        case DbTokenType::ROLLBACK:  return parseRollback();
        case DbTokenType::SHOW:      return parseShowTables();
        case DbTokenType::DESCRIBE:  return parseDescribe();
        case DbTokenType::SAVE:      return parseSaveDatabase();
        case DbTokenType::LOAD:      return parseLoadDatabase();
        case DbTokenType::IF:        return parseIf();
        case DbTokenType::WHILE:     return parseWhile();
        case DbTokenType::DEF:       return parseFuncDef();
        case DbTokenType::RETURN:    return parseReturn();
        case DbTokenType::PRINT:     return parsePrint();

        case DbTokenType::INT_TYPE:    return parseVarDecl(ValueType::INT);
        case DbTokenType::DOUBLE_TYPE: return parseVarDecl(ValueType::DOUBLE);
        case DbTokenType::STRING_TYPE: return parseVarDecl(ValueType::STRING);
        case DbTokenType::BOOL_TYPE:   return parseVarDecl(ValueType::BOOL);

        case DbTokenType::IDENTIFIER:  return parseAssignOrPipeline();

        case DbTokenType::SEMICOLON:
            advance(); // skip stray semicolons
            return parseStatement();

        default:
            error("Unexpected token '" + peek().value + "' at line " +
                  std::to_string(peek().line));
            return nullptr;
    }
}

// ── CREATE TABLE ─────────────────────────────────────────────────────

StmtPtr DbParser::parseCreateTable() {
    advance(); // CREATE
    expect(DbTokenType::TABLE, "Expected 'TABLE' after 'CREATE'");

    auto stmt = std::make_shared<CreateTableStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

    expect(DbTokenType::LPAREN, "Expected '(' after table name");

    do {
        CreateTableStmt::ColumnDef col;
        col.name = expect(DbTokenType::IDENTIFIER, "Expected column name").value;
        col.type = parseType();

        // Column constraints
        while (!isAtEnd() && !check(DbTokenType::COMMA) &&
               !check(DbTokenType::RPAREN)) {
            if (peek().value == "primary" || peek().type == DbTokenType::IDENTIFIER) {
                // PRIMARY KEY (two words, both identifiers from lexer perspective)
                if (peek().value == "primary") {
                    advance();
                    if (peek().value == "key" || peek().type == DbTokenType::IDENTIFIER) {
                        advance();
                    }
                    col.primaryKey = true;
                    col.nullable = false;
                    continue;
                }
            }
            if (check(DbTokenType::UNIQUE)) {
                advance();
                col.unique = true;
                continue;
            }
            if (check(DbTokenType::NOT)) {
                advance();
                if (check(DbTokenType::NULL_LIT)) {
                    advance();
                    col.nullable = false;
                } else {
                    error("Expected 'null' after 'not'");
                }
                continue;
            }
            break;
        }

        stmt->columns.push_back(col);
    } while (match(DbTokenType::COMMA));

    expect(DbTokenType::RPAREN, "Expected ')' after column definitions");
    expect(DbTokenType::SEMICOLON, "Expected ';' after CREATE TABLE");
    return stmt;
}

// ── DROP TABLE ───────────────────────────────────────────────────────

StmtPtr DbParser::parseDropTable() {
    advance(); // DROP
    expect(DbTokenType::TABLE, "Expected 'TABLE' after 'DROP'");

    auto stmt = std::make_shared<DropTableStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;
    expect(DbTokenType::SEMICOLON, "Expected ';' after DROP TABLE");
    return stmt;
}

// ── INSERT ───────────────────────────────────────────────────────────

StmtPtr DbParser::parseInsert() {
    advance(); // INSERT
    expect(DbTokenType::INTO, "Expected 'INTO' after 'INSERT'");

    auto stmt = std::make_shared<InsertStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

    // Optional column list: INSERT INTO t (c1, c2)
    if (check(DbTokenType::LPAREN)) {
        advance();
        do {
            stmt->columns.push_back(
                expect(DbTokenType::IDENTIFIER, "Expected column name").value);
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after column list");
    }

    expect(DbTokenType::VALUES, "Expected 'VALUES'");

    // Parse value rows: (v1, v2), (v3, v4), ...
    do {
        expect(DbTokenType::LPAREN, "Expected '(' before values");
        std::vector<ExprPtr> row;
        do {
            row.push_back(parseExpression());
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after values");
        stmt->valueRows.push_back(std::move(row));
    } while (match(DbTokenType::COMMA));

    expect(DbTokenType::SEMICOLON, "Expected ';' after INSERT");
    return stmt;
}

// ── SELECT ───────────────────────────────────────────────────────────

StmtPtr DbParser::parseSelect() {
    advance(); // SELECT
    auto stmt = std::make_shared<SelectStmt>();

    // DISTINCT
    if (check(DbTokenType::DISTINCT)) {
        advance();
        stmt->distinct = true;
    }

    // Column list
    stmt->columns = parseExpressionList();

    // FROM
    expect(DbTokenType::FROM, "Expected 'FROM'");
    stmt->fromTable = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

    // JOINs
    while (!isAtEnd()) {
        std::string joinType;
        if (check(DbTokenType::INNER)) { advance(); joinType = "inner"; }
        else if (check(DbTokenType::LEFT))  { advance(); joinType = "left"; }
        else if (check(DbTokenType::RIGHT)) { advance(); joinType = "right"; }
        else if (check(DbTokenType::CROSS)) { advance(); joinType = "cross"; }
        else if (check(DbTokenType::JOIN))  { joinType = "inner"; }
        else break;

        // Consume optional OUTER after LEFT/RIGHT
        if (joinType == "left" || joinType == "right") {
            match(DbTokenType::OUTER);
        }

        expect(DbTokenType::JOIN, "Expected 'JOIN'");

        SelectStmt::JoinClause jc;
        jc.joinType = joinType;
        jc.tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

        if (joinType != "cross") {
            expect(DbTokenType::ON, "Expected 'ON' after join table");
            jc.onCondition = parseExpression();
        }
        stmt->joins.push_back(std::move(jc));
    }

    // WHERE
    if (check(DbTokenType::WHERE)) {
        advance();
        stmt->whereClause = parseExpression();
    }

    // GROUP BY
    if (check(DbTokenType::GROUPBY)) {
        advance();
        do {
            stmt->groupBy.push_back(parseExpression());
        } while (match(DbTokenType::COMMA));
    }

    // HAVING
    if (check(DbTokenType::HAVING)) {
        advance();
        stmt->havingClause = parseExpression();
    }

    // ORDER BY
    if (check(DbTokenType::ORDERBY)) {
        advance();
        do {
            auto expr = parseExpression();
            bool asc = true;
            if (check(DbTokenType::ASC))       { advance(); }
            else if (check(DbTokenType::DESC))  { advance(); asc = false; }
            stmt->orderBy.push_back({expr, asc});
        } while (match(DbTokenType::COMMA));
    }

    // LIMIT
    if (check(DbTokenType::LIMIT)) {
        advance();
        const auto& tok = expect(DbTokenType::INT_LIT, "Expected integer after LIMIT");
        try { stmt->limit = std::stoi(tok.value); }
        catch (...) { error("Invalid LIMIT value"); }
    }

    // OFFSET
    if (check(DbTokenType::OFFSET)) {
        advance();
        const auto& tok = expect(DbTokenType::INT_LIT, "Expected integer after OFFSET");
        try { stmt->offset = std::stoi(tok.value); }
        catch (...) { error("Invalid OFFSET value"); }
    }

    expect(DbTokenType::SEMICOLON, "Expected ';' after SELECT");
    return stmt;
}

// ── UPDATE ───────────────────────────────────────────────────────────

StmtPtr DbParser::parseUpdate() {
    advance(); // UPDATE
    auto stmt = std::make_shared<UpdateStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

    expect(DbTokenType::SET, "Expected 'SET'");

    do {
        std::string col = expect(DbTokenType::IDENTIFIER, "Expected column name").value;
        expect(DbTokenType::EQ, "Expected '=' in SET clause");
        auto val = parseExpression();
        stmt->assignments.push_back({col, val});
    } while (match(DbTokenType::COMMA));

    if (check(DbTokenType::WHERE)) {
        advance();
        stmt->whereClause = parseExpression();
    }

    expect(DbTokenType::SEMICOLON, "Expected ';' after UPDATE");
    return stmt;
}

// ── DELETE ───────────────────────────────────────────────────────────

StmtPtr DbParser::parseDelete() {
    advance(); // DELETE
    expect(DbTokenType::FROM, "Expected 'FROM' after 'DELETE'");

    auto stmt = std::make_shared<DeleteStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;

    if (check(DbTokenType::WHERE)) {
        advance();
        stmt->whereClause = parseExpression();
    }

    expect(DbTokenType::SEMICOLON, "Expected ';' after DELETE");
    return stmt;
}

// ── Pipeline ─────────────────────────────────────────────────────────

StmtPtr DbParser::parsePipeline(const std::string& tableName) {
    auto stmt = std::make_shared<PipelineStmt>();
    stmt->tableName = tableName;

    while (check(DbTokenType::PIPE)) {
        advance(); // |>
        stmt->stages.push_back(parsePipelineStage());
    }

    expect(DbTokenType::SEMICOLON, "Expected ';' after pipeline");
    return stmt;
}

PipelineStage DbParser::parsePipelineStage() {
    PipelineStage stage;
    const auto& tok = peek();

    if (tok.type == DbTokenType::WHERE) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'where'");
        stage.type = PipelineStage::Type::WHERE;
        stage.condition = parseExpression();
        expect(DbTokenType::RPAREN, "Expected ')' after where condition");
    }
    else if (tok.type == DbTokenType::SELECT) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'select'");
        stage.type = PipelineStage::Type::SELECT;
        stage.columns = parseExpressionList();
        expect(DbTokenType::RPAREN, "Expected ')' after select columns");
    }
    else if (tok.type == DbTokenType::ORDERBY) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'orderby'");
        stage.type = PipelineStage::Type::ORDERBY;
        do {
            auto expr = parseExpression();
            bool asc = true;
            if (check(DbTokenType::ASC))       { advance(); }
            else if (check(DbTokenType::DESC))  { advance(); asc = false; }
            stage.orderCols.push_back({expr, asc});
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after orderby columns");
    }
    else if (tok.type == DbTokenType::LIMIT) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'limit'");
        stage.type = PipelineStage::Type::LIMIT;
        const auto& val = expect(DbTokenType::INT_LIT, "Expected integer for limit");
        try { stage.limitCount = std::stoi(val.value); }
        catch (...) { error("Invalid limit value"); }
        expect(DbTokenType::RPAREN, "Expected ')' after limit value");
    }
    else if (tok.type == DbTokenType::OFFSET) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'offset'");
        stage.type = PipelineStage::Type::OFFSET;
        const auto& val = expect(DbTokenType::INT_LIT, "Expected integer for offset");
        try { stage.offsetCount = std::stoi(val.value); }
        catch (...) { error("Invalid offset value"); }
        expect(DbTokenType::RPAREN, "Expected ')' after offset value");
    }
    else if (tok.type == DbTokenType::GROUPBY) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'groupby'");
        stage.type = PipelineStage::Type::GROUPBY;
        do {
            stage.groupCols.push_back(parseExpression());
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after groupby columns");
    }
    else if (tok.type == DbTokenType::HAVING) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'having'");
        stage.type = PipelineStage::Type::HAVING;
        stage.condition = parseExpression();
        expect(DbTokenType::RPAREN, "Expected ')' after having condition");
    }
    else if (tok.type == DbTokenType::JOIN) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'join'");
        stage.type = PipelineStage::Type::JOIN;
        stage.joinType = "inner";

        stage.joinTable = expect(DbTokenType::IDENTIFIER, "Expected table name in join").value;
        expect(DbTokenType::ON, "Expected 'on' in join");
        stage.joinCondition = parseExpression();
        expect(DbTokenType::RPAREN, "Expected ')' after join");
    }
    else if (tok.type == DbTokenType::UPDATE) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'update'");
        stage.type = PipelineStage::Type::UPDATE;
        do {
            std::string col = expect(DbTokenType::IDENTIFIER, "Expected column name").value;
            expect(DbTokenType::EQ, "Expected '=' in update");
            auto val = parseExpression();
            stage.assignments.push_back({col, val});
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after update");
    }
    else if (tok.type == DbTokenType::DELETE_KW) {
        advance();
        stage.type = PipelineStage::Type::DELETE_STAGE;
    }
    else if (tok.type == DbTokenType::PRINT) {
        advance();
        stage.type = PipelineStage::Type::PRINT;
    }
    else if (tok.type == DbTokenType::DISTINCT) {
        advance();
        stage.type = PipelineStage::Type::DISTINCT;
    }
    else if (tok.type == DbTokenType::COUNT) {
        advance();
        stage.type = PipelineStage::Type::COUNT_STAGE;
    }
    else if (tok.type == DbTokenType::TAKE) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'take'");
        stage.type = PipelineStage::Type::TAKE;
        const auto& val = expect(DbTokenType::INT_LIT, "Expected integer for take");
        try { stage.limitCount = std::stoi(val.value); }
        catch (...) { error("Invalid take value"); }
        expect(DbTokenType::RPAREN, "Expected ')' after take value");
    }
    else if (tok.type == DbTokenType::SKIP_KW) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'skip'");
        stage.type = PipelineStage::Type::SKIP_STAGE;
        const auto& val = expect(DbTokenType::INT_LIT, "Expected integer for skip");
        try { stage.offsetCount = std::stoi(val.value); }
        catch (...) { error("Invalid skip value"); }
        expect(DbTokenType::RPAREN, "Expected ')' after skip value");
    }
    else if (tok.type == DbTokenType::MAP) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after 'map'");
        stage.type = PipelineStage::Type::MAP;
        stage.columns = parseExpressionList();
        expect(DbTokenType::RPAREN, "Expected ')' after map columns");
    }
    else {
        error("Unknown pipeline stage '" + tok.value + "'");
        advance();
        stage.type = PipelineStage::Type::PRINT;  // fallback
    }

    return stage;
}

// ── Transaction Statements ───────────────────────────────────────────

StmtPtr DbParser::parseBegin() {
    advance(); // BEGIN
    expect(DbTokenType::SEMICOLON, "Expected ';' after BEGIN");
    return std::make_shared<BeginStmt>();
}

StmtPtr DbParser::parseCommit() {
    advance(); // COMMIT
    expect(DbTokenType::SEMICOLON, "Expected ';' after COMMIT");
    return std::make_shared<CommitStmt>();
}

StmtPtr DbParser::parseRollback() {
    advance(); // ROLLBACK
    expect(DbTokenType::SEMICOLON, "Expected ';' after ROLLBACK");
    return std::make_shared<RollbackStmt>();
}

// ── SHOW TABLES / DESCRIBE ───────────────────────────────────────────

StmtPtr DbParser::parseShowTables() {
    advance(); // SHOW
    expect(DbTokenType::TABLES, "Expected 'TABLES' after 'SHOW'");
    expect(DbTokenType::SEMICOLON, "Expected ';' after SHOW TABLES");
    return std::make_shared<ShowTablesStmt>();
}

StmtPtr DbParser::parseDescribe() {
    advance(); // DESCRIBE
    auto stmt = std::make_shared<DescribeStmt>();
    stmt->tableName = expect(DbTokenType::IDENTIFIER, "Expected table name").value;
    expect(DbTokenType::SEMICOLON, "Expected ';' after DESCRIBE");
    return stmt;
}

// ── PRINT ────────────────────────────────────────────────────────────

StmtPtr DbParser::parsePrint() {
    advance(); // PRINT
    auto stmt = std::make_shared<PrintStmt>();
    stmt->expr = parseExpression();
    expect(DbTokenType::SEMICOLON, "Expected ';' after PRINT");
    return stmt;
}

StmtPtr DbParser::parseSaveDatabase() {
    advance(); // SAVE
    expect(DbTokenType::DATABASE, "Expected 'DATABASE' after 'SAVE'");
    auto stmt = std::make_shared<SaveDatabaseStmt>();
    stmt->filepath = expect(DbTokenType::STRING_LIT, "Expected file path string").value;
    expect(DbTokenType::SEMICOLON, "Expected ';' after SAVE DATABASE");
    return stmt;
}

StmtPtr DbParser::parseLoadDatabase() {
    advance(); // LOAD
    expect(DbTokenType::DATABASE, "Expected 'DATABASE' after 'LOAD'");
    auto stmt = std::make_shared<LoadDatabaseStmt>();
    stmt->filepath = expect(DbTokenType::STRING_LIT, "Expected file path string").value;
    expect(DbTokenType::SEMICOLON, "Expected ';' after LOAD DATABASE");
    return stmt;
}

// ── Variable Declarations ────────────────────────────────────────────

StmtPtr DbParser::parseVarDecl(ValueType type) {
    advance(); // type keyword
    auto stmt = std::make_shared<VarDeclStmt>();
    stmt->type = type;

    do {
        stmt->names.push_back(
            expect(DbTokenType::IDENTIFIER, "Expected variable name").value);
    } while (match(DbTokenType::COMMA));

    expect(DbTokenType::SEMICOLON, "Expected ';' after variable declaration");
    return stmt;
}

// ── Assignment or Pipeline ───────────────────────────────────────────

StmtPtr DbParser::parseAssignOrPipeline() {
    std::string name = peek().value;
    advance(); // identifier

    // Assignment: x = expr;
    if (check(DbTokenType::EQ)) {
        advance(); // =
        auto stmt = std::make_shared<AssignStmt>();
        stmt->varName = name;
        stmt->value = parseExpression();
        expect(DbTokenType::SEMICOLON, "Expected ';' after assignment");
        return stmt;
    }

    // Pipeline: table |> stage |> stage ...;
    if (check(DbTokenType::PIPE)) {
        return parsePipeline(name);
    }

    // Function call: name(args);
    if (check(DbTokenType::LPAREN)) {
        advance(); // (
        auto stmt = std::make_shared<FuncCallStmt>();
        stmt->name = name;
        if (!check(DbTokenType::RPAREN)) {
            do {
                stmt->args.push_back(parseExpression());
            } while (match(DbTokenType::COMMA));
        }
        expect(DbTokenType::RPAREN, "Expected ')' after function arguments");
        expect(DbTokenType::SEMICOLON, "Expected ';' after function call");
        return stmt;
    }

    error("Expected '=', '|>', or '(' after identifier '" + name + "'");
    return nullptr;
}

// ── If / Else ────────────────────────────────────────────────────────

StmtPtr DbParser::parseIf() {
    advance(); // IF
    auto stmt = std::make_shared<IfStmt>();

    expect(DbTokenType::LPAREN, "Expected '(' after 'if'");
    stmt->condition = parseExpression();
    expect(DbTokenType::RPAREN, "Expected ')' after if condition");

    expect(DbTokenType::THEN, "Expected 'then'");
    stmt->thenBody = parseBlock(DbTokenType::FI);

    // Check for else before consuming FI
    if (check(DbTokenType::ELSE)) {
        advance(); // ELSE
        stmt->elseBody = parseBlock(DbTokenType::FI);
    }

    expect(DbTokenType::FI, "Expected 'fi' to close if");
    expect(DbTokenType::SEMICOLON, "Expected ';' after 'fi'");
    return stmt;
}

// ── While ────────────────────────────────────────────────────────────

StmtPtr DbParser::parseWhile() {
    advance(); // WHILE
    auto stmt = std::make_shared<WhileStmt>();

    expect(DbTokenType::LPAREN, "Expected '(' after 'while'");
    stmt->condition = parseExpression();
    expect(DbTokenType::RPAREN, "Expected ')' after while condition");

    expect(DbTokenType::DO, "Expected 'do'");
    stmt->body = parseBlock(DbTokenType::OD);

    expect(DbTokenType::OD, "Expected 'od' to close while");
    expect(DbTokenType::SEMICOLON, "Expected ';' after 'od'");
    return stmt;
}

// ── Function Definition ──────────────────────────────────────────────

StmtPtr DbParser::parseFuncDef() {
    advance(); // DEF
    auto stmt = std::make_shared<FuncDefStmt>();

    stmt->returnType = parseType();
    stmt->name = expect(DbTokenType::IDENTIFIER, "Expected function name").value;

    expect(DbTokenType::LPAREN, "Expected '(' after function name");
    if (!check(DbTokenType::RPAREN)) {
        do {
            ValueType ptype = parseType();
            std::string pname = expect(DbTokenType::IDENTIFIER, "Expected parameter name").value;
            stmt->params.push_back({ptype, pname});
        } while (match(DbTokenType::COMMA));
    }
    expect(DbTokenType::RPAREN, "Expected ')' after parameters");

    stmt->body = parseBlock(DbTokenType::FED);

    expect(DbTokenType::FED, "Expected 'fed' to close function");
    expect(DbTokenType::SEMICOLON, "Expected ';' after 'fed'");
    return stmt;
}

// ── Return ───────────────────────────────────────────────────────────

StmtPtr DbParser::parseReturn() {
    advance(); // RETURN
    auto stmt = std::make_shared<ReturnStmt>();

    expect(DbTokenType::LPAREN, "Expected '(' after 'return'");
    stmt->value = parseExpression();
    expect(DbTokenType::RPAREN, "Expected ')' after return value");

    expect(DbTokenType::SEMICOLON, "Expected ';' after return");
    return stmt;
}

// ── Block Parsing ────────────────────────────────────────────────────

std::vector<StmtPtr> DbParser::parseBlock(DbTokenType terminator) {
    std::vector<StmtPtr> body;
    while (!isAtEnd() && !check(terminator) && !check(DbTokenType::ELSE)) {
        auto stmt = parseStatement();
        if (stmt) body.push_back(stmt);
        else if (!isAtEnd() && !check(terminator) && !check(DbTokenType::ELSE))
            advance(); // skip bad token for recovery
    }
    return body;
}

// ── Type Parsing ─────────────────────────────────────────────────────

ValueType DbParser::parseType() {
    switch (peek().type) {
        case DbTokenType::INT_TYPE:    advance(); return ValueType::INT;
        case DbTokenType::DOUBLE_TYPE: advance(); return ValueType::DOUBLE;
        case DbTokenType::STRING_TYPE: advance(); return ValueType::STRING;
        case DbTokenType::BOOL_TYPE:   advance(); return ValueType::BOOL;
        default:
            error("Expected type (int, double, string, bool)");
            return ValueType::INT;
    }
}

// ── Expression List ──────────────────────────────────────────────────

std::vector<ExprPtr> DbParser::parseExpressionList() {
    std::vector<ExprPtr> exprs;
    do {
        exprs.push_back(parseExpression());
    } while (match(DbTokenType::COMMA));
    return exprs;
}

// ══════════════════════════════════════════════════════════════════════
//  Expression Parser — Recursive Descent with Precedence Climbing
// ══════════════════════════════════════════════════════════════════════

ExprPtr DbParser::parseExpression() {
    auto expr = parseOrExpr();

    // Handle alias: expr AS name
    if (check(DbTokenType::AS)) {
        advance();
        std::string alias = expect(DbTokenType::IDENTIFIER, "Expected alias name").value;
        return std::make_shared<AliasExpr>(expr, alias);
    }

    return expr;
}

// ── OR ───────────────────────────────────────────────────────────────

ExprPtr DbParser::parseOrExpr() {
    auto left = parseAndExpr();
    while (check(DbTokenType::OR)) {
        advance();
        auto right = parseAndExpr();
        left = std::make_shared<BinaryExpr>(left, "or", right);
    }
    return left;
}

// ── AND ──────────────────────────────────────────────────────────────

ExprPtr DbParser::parseAndExpr() {
    auto left = parseNotExpr();
    while (check(DbTokenType::AND)) {
        advance();
        auto right = parseNotExpr();
        left = std::make_shared<BinaryExpr>(left, "and", right);
    }
    return left;
}

// ── NOT ──────────────────────────────────────────────────────────────

ExprPtr DbParser::parseNotExpr() {
    if (check(DbTokenType::NOT)) {
        advance();
        auto operand = parseNotExpr();
        return std::make_shared<UnaryExpr>("not", operand);
    }
    return parseComparisonExpr();
}

// ── Comparison / BETWEEN / IN / LIKE / IS NULL ───────────────────────

ExprPtr DbParser::parseComparisonExpr() {
    auto left = parseAddExpr();

    // BETWEEN expr AND expr
    if (check(DbTokenType::BETWEEN)) {
        advance();
        auto low = parseAddExpr();
        expect(DbTokenType::AND, "Expected 'AND' in BETWEEN");
        auto high = parseAddExpr();
        return std::make_shared<BetweenExpr>(left, low, high, false);
    }

    // NOT BETWEEN / NOT IN / NOT LIKE
    if (check(DbTokenType::NOT)) {
        auto savedPos = pos_;
        advance();

        if (check(DbTokenType::BETWEEN)) {
            advance();
            auto low = parseAddExpr();
            expect(DbTokenType::AND, "Expected 'AND' in NOT BETWEEN");
            auto high = parseAddExpr();
            return std::make_shared<BetweenExpr>(left, low, high, true);
        }
        if (check(DbTokenType::IN)) {
            advance();
            expect(DbTokenType::LPAREN, "Expected '(' after NOT IN");
            std::vector<ExprPtr> vals;
            do {
                vals.push_back(parseExpression());
            } while (match(DbTokenType::COMMA));
            expect(DbTokenType::RPAREN, "Expected ')' after NOT IN list");
            return std::make_shared<InExpr>(left, std::move(vals), true);
        }
        if (check(DbTokenType::LIKE)) {
            advance();
            std::string pattern = expect(DbTokenType::STRING_LIT, "Expected pattern string").value;
            return std::make_shared<LikeExpr>(left, pattern, true);
        }

        // Not a postfix NOT — backtrack
        pos_ = savedPos;
    }

    // IN (val, val, ...)
    if (check(DbTokenType::IN)) {
        advance();
        expect(DbTokenType::LPAREN, "Expected '(' after IN");
        std::vector<ExprPtr> vals;
        do {
            vals.push_back(parseExpression());
        } while (match(DbTokenType::COMMA));
        expect(DbTokenType::RPAREN, "Expected ')' after IN list");
        return std::make_shared<InExpr>(left, std::move(vals), false);
    }

    // LIKE
    if (check(DbTokenType::LIKE)) {
        advance();
        std::string pattern = expect(DbTokenType::STRING_LIT, "Expected pattern string").value;
        return std::make_shared<LikeExpr>(left, pattern, false);
    }

    // IS [NOT] NULL
    if (check(DbTokenType::IS)) {
        advance();
        bool isNot = false;
        if (check(DbTokenType::NOT)) {
            advance();
            isNot = true;
        }
        expect(DbTokenType::NULL_LIT, "Expected 'null' after IS");
        return std::make_shared<IsNullExpr>(left, isNot);
    }

    // Standard comparison operators
    while (true) {
        std::string op;
        if (check(DbTokenType::EQEQ))     { advance(); op = "=="; }
        else if (check(DbTokenType::NEQ))  { advance(); op = "<>"; }
        else if (check(DbTokenType::LT))   { advance(); op = "<"; }
        else if (check(DbTokenType::GT))    { advance(); op = ">"; }
        else if (check(DbTokenType::LTE))  { advance(); op = "<="; }
        else if (check(DbTokenType::GTE))  { advance(); op = ">="; }
        else break;

        auto right = parseAddExpr();
        left = std::make_shared<BinaryExpr>(left, op, right);
    }

    return left;
}

// ── Additive (+, -) ─────────────────────────────────────────────────

ExprPtr DbParser::parseAddExpr() {
    auto left = parseMulExpr();
    while (check(DbTokenType::PLUS) || check(DbTokenType::MINUS)) {
        std::string op = check(DbTokenType::PLUS) ? "+" : "-";
        advance();
        auto right = parseMulExpr();
        left = std::make_shared<BinaryExpr>(left, op, right);
    }
    return left;
}

// ── Multiplicative (*, /, %) ─────────────────────────────────────────

ExprPtr DbParser::parseMulExpr() {
    auto left = parseUnaryExpr();
    while (check(DbTokenType::STAR) || check(DbTokenType::SLASH) ||
           check(DbTokenType::PERCENT)) {
        std::string op;
        if (check(DbTokenType::STAR))         op = "*";
        else if (check(DbTokenType::SLASH))   op = "/";
        else                                   op = "%";
        advance();
        auto right = parseUnaryExpr();
        left = std::make_shared<BinaryExpr>(left, op, right);
    }
    return left;
}

// ── Unary (-, not) ───────────────────────────────────────────────────

ExprPtr DbParser::parseUnaryExpr() {
    if (check(DbTokenType::MINUS)) {
        advance();
        auto operand = parseUnaryExpr();
        return std::make_shared<UnaryExpr>("-", operand);
    }
    return parsePrimaryExpr();
}

// ── Primary Expressions ──────────────────────────────────────────────

ExprPtr DbParser::parsePrimaryExpr() {
    const auto& tok = peek();

    // Integer literal
    if (tok.type == DbTokenType::INT_LIT) {
        advance();
        try { return std::make_shared<LiteralExpr>(Value(std::stoi(tok.value))); }
        catch (...) {
            error("Invalid integer literal: " + tok.value);
            return std::make_shared<LiteralExpr>(Value(0));
        }
    }

    // Double literal
    if (tok.type == DbTokenType::DOUBLE_LIT) {
        advance();
        try { return std::make_shared<LiteralExpr>(Value(std::stod(tok.value))); }
        catch (...) {
            error("Invalid double literal: " + tok.value);
            return std::make_shared<LiteralExpr>(Value(0.0));
        }
    }

    // String literal
    if (tok.type == DbTokenType::STRING_LIT) {
        advance();
        return std::make_shared<LiteralExpr>(Value(tok.value));
    }

    // Bool literal
    if (tok.type == DbTokenType::BOOL_LIT) {
        advance();
        return std::make_shared<LiteralExpr>(Value(tok.value == "true"));
    }

    // Null literal
    if (tok.type == DbTokenType::NULL_LIT) {
        advance();
        return std::make_shared<LiteralExpr>(Value::null());
    }

    // Star: *
    if (tok.type == DbTokenType::STAR) {
        advance();
        return std::make_shared<StarExpr>();
    }

    // Parenthesized expression
    if (tok.type == DbTokenType::LPAREN) {
        advance();
        auto expr = parseExpression();
        expect(DbTokenType::RPAREN, "Expected ')'");
        return expr;
    }

    // Aggregate / built-in functions (keyword-based tokens)
    if (tok.type == DbTokenType::COUNT || tok.type == DbTokenType::SUM ||
        tok.type == DbTokenType::AVG   || tok.type == DbTokenType::MIN_FN ||
        tok.type == DbTokenType::MAX_FN) {
        return parseColumnOrFunction();
    }
    if (tok.type == DbTokenType::UPPER  || tok.type == DbTokenType::LOWER ||
        tok.type == DbTokenType::LENGTH || tok.type == DbTokenType::SUBSTR ||
        tok.type == DbTokenType::CONCAT || tok.type == DbTokenType::TRIM ||
        tok.type == DbTokenType::REPLACE) {
        return parseColumnOrFunction();
    }
    // New utility/math functions
    if (tok.type == DbTokenType::COALESCE || tok.type == DbTokenType::NULLIF ||
        tok.type == DbTokenType::TYPEOF   || tok.type == DbTokenType::CAST ||
        tok.type == DbTokenType::LEFT_FN  || tok.type == DbTokenType::RIGHT_FN ||
        tok.type == DbTokenType::LPAD     || tok.type == DbTokenType::RPAD ||
        tok.type == DbTokenType::REVERSE  || tok.type == DbTokenType::REPEAT_FN ||
        tok.type == DbTokenType::POWER    || tok.type == DbTokenType::SQRT ||
        tok.type == DbTokenType::LOG_FN   || tok.type == DbTokenType::PI_FN ||
        tok.type == DbTokenType::RANDOM   || tok.type == DbTokenType::NOW ||
        tok.type == DbTokenType::IIF) {
        return parseColumnOrFunction();
    }

    // CASE WHEN ... THEN ... [ELSE ...] END
    if (tok.type == DbTokenType::CASE) {
        advance(); // consume CASE
        auto caseExpr = std::make_shared<CaseExpr>();
        while (check(DbTokenType::WHEN)) {
            advance(); // consume WHEN
            auto cond = parseExpression();
            expect(DbTokenType::THEN, "Expected 'THEN' after WHEN condition");
            auto result = parseExpression();
            caseExpr->whenClauses.push_back({cond, result});
        }
        if (check(DbTokenType::ELSE)) {
            advance();
            caseExpr->elseResult = parseExpression();
        }
        expect(DbTokenType::END_KW, "Expected 'END' to close CASE");
        return caseExpr;
    }

    // Keywords that can also be used as function names when followed by '('
    // (e.g., left(str, n), right(str, n))
    if ((tok.type == DbTokenType::LEFT || tok.type == DbTokenType::RIGHT) &&
        peekNext().type == DbTokenType::LPAREN) {
        return parseColumnOrFunction();
    }

    // Identifier — could be column, table.column, or user-defined function
    if (tok.type == DbTokenType::IDENTIFIER) {
        return parseColumnOrFunction();
    }

    error("Unexpected token in expression: '" + tok.value + "' at line " +
          std::to_string(tok.line));
    advance();
    return std::make_shared<LiteralExpr>(Value::null());
}

// ── Column Reference or Function Call ────────────────────────────────

ExprPtr DbParser::parseColumnOrFunction() {
    const auto& tok = peek();
    std::string name = tok.value;
    advance();

    // Function call: name(args) — works for identifiers and keyword-named functions
    if (check(DbTokenType::LPAREN)) {
        advance(); // (
        std::vector<ExprPtr> args;

        // count(*) special case
        if (check(DbTokenType::STAR)) {
            args.push_back(std::make_shared<StarExpr>());
            advance();
        } else if (!check(DbTokenType::RPAREN)) {
            do {
                args.push_back(parseExpression());
            } while (match(DbTokenType::COMMA));
        }

        expect(DbTokenType::RPAREN, "Expected ')' after function arguments");
        return std::make_shared<FunctionCallExpr>(name, std::move(args));
    }

    // table.column: identifier DOT identifier
    if (tok.type == DbTokenType::IDENTIFIER && check(DbTokenType::DOT)) {
        advance(); // .
        std::string col = expect(DbTokenType::IDENTIFIER, "Expected column name after '.'").value;
        return std::make_shared<ColumnExpr>(name, col);
    }

    // Simple column reference
    return std::make_shared<ColumnExpr>(name);
}

} // namespace epee
