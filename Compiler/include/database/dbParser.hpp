/*
 File: dbParser.hpp
 Project: Épée Database Query Language
 Description: Parser for database statements and pipeline queries
*/

#ifndef EPEE_DB_PARSER_H
#define EPEE_DB_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <variant>
#include "dbLexer.hpp"
#include "value.hpp"

namespace epee {

// Forward declarations
struct Expression;
using ExprPtr = std::shared_ptr<Expression>;

// Expression types for the AST
struct Expression {
    virtual ~Expression() = default;
};

struct LiteralExpr : Expression {
    Value value;
    explicit LiteralExpr(const Value& v) : value(v) {}
};

struct ColumnExpr : Expression {
    std::string tableName;  // optional: table.column
    std::string columnName;
    ColumnExpr(const std::string& col) : columnName(col) {}
    ColumnExpr(const std::string& tbl, const std::string& col) : tableName(tbl), columnName(col) {}

    std::string fullName() const {
        return tableName.empty() ? columnName : tableName + "." + columnName;
    }
};

struct BinaryExpr : Expression {
    ExprPtr left;
    std::string op;  // +, -, *, /, %, ==, <>, <, >, <=, >=, and, or
    ExprPtr right;
    BinaryExpr(ExprPtr l, const std::string& o, ExprPtr r) : left(l), op(o), right(r) {}
};

struct UnaryExpr : Expression {
    std::string op;  // -, not
    ExprPtr operand;
    UnaryExpr(const std::string& o, ExprPtr e) : op(o), operand(e) {}
};

struct FunctionCallExpr : Expression {
    std::string name;  // count, sum, avg, min, max, upper, lower, length, etc.
    std::vector<ExprPtr> args;
    FunctionCallExpr(const std::string& n, std::vector<ExprPtr> a) : name(n), args(std::move(a)) {}
};

struct AliasExpr : Expression {
    ExprPtr expr;
    std::string alias;
    AliasExpr(ExprPtr e, const std::string& a) : expr(e), alias(a) {}
};

struct StarExpr : Expression {};

struct BetweenExpr : Expression {
    ExprPtr expr;
    ExprPtr low;
    ExprPtr high;
    bool negated;
    BetweenExpr(ExprPtr e, ExprPtr l, ExprPtr h, bool neg = false)
        : expr(e), low(l), high(h), negated(neg) {}
};

struct InExpr : Expression {
    ExprPtr expr;
    std::vector<ExprPtr> values;
    bool negated;
    InExpr(ExprPtr e, std::vector<ExprPtr> v, bool neg = false)
        : expr(e), values(std::move(v)), negated(neg) {}
};

struct LikeExpr : Expression {
    ExprPtr expr;
    std::string pattern;
    bool negated;
    LikeExpr(ExprPtr e, const std::string& p, bool neg = false) : expr(e), pattern(p), negated(neg) {}
};

struct IsNullExpr : Expression {
    ExprPtr expr;
    bool isNot;
    IsNullExpr(ExprPtr e, bool n) : expr(e), isNot(n) {}
};

struct CaseExpr : Expression {
    struct WhenClause {
        ExprPtr condition;
        ExprPtr result;
    };
    std::vector<WhenClause> whenClauses;
    ExprPtr elseResult;  // can be null
};

// Statement types
struct Statement {
    virtual ~Statement() = default;
};

using StmtPtr = std::shared_ptr<Statement>;

struct CreateTableStmt : Statement {
    std::string tableName;
    struct ColumnDef {
        std::string name;
        ValueType type;
        bool primaryKey = false;
        bool unique = false;
        bool nullable = true;
    };
    std::vector<ColumnDef> columns;
};

struct DropTableStmt : Statement {
    std::string tableName;
};

struct InsertStmt : Statement {
    std::string tableName;
    std::vector<std::string> columns;  // optional column list
    std::vector<std::vector<ExprPtr>> valueRows;  // multiple value rows
};

struct SelectStmt : Statement {
    bool distinct = false;
    std::vector<ExprPtr> columns;  // expressions or * 
    std::string fromTable;
    ExprPtr whereClause;
    std::vector<ExprPtr> groupBy;
    ExprPtr havingClause;
    std::vector<std::pair<ExprPtr, bool>> orderBy;  // expr, ascending
    int limit = -1;
    int offset = 0;

    // Join info
    struct JoinClause {
        std::string joinType;  // "inner", "left", "right", "cross"
        std::string tableName;
        ExprPtr onCondition;
    };
    std::vector<JoinClause> joins;
};

struct UpdateStmt : Statement {
    std::string tableName;
    std::vector<std::pair<std::string, ExprPtr>> assignments;
    ExprPtr whereClause;
};

struct DeleteStmt : Statement {
    std::string tableName;
    ExprPtr whereClause;
};

// Pipeline statement: table |> stage1 |> stage2 |> ...
struct PipelineStage {
    enum class Type {
        WHERE, SELECT, ORDERBY, LIMIT, OFFSET, GROUPBY, HAVING,
        JOIN, UPDATE, DELETE_STAGE, PRINT, DISTINCT, COUNT_STAGE,
        MAP, TAKE, SKIP_STAGE
    };
    Type type;

    // WHERE, HAVING
    ExprPtr condition;

    // SELECT
    std::vector<ExprPtr> columns;
    bool selectDistinct = false;

    // ORDERBY
    std::vector<std::pair<ExprPtr, bool>> orderCols;  // expr, ascending

    // LIMIT, OFFSET
    int limitCount = -1;
    int offsetCount = 0;

    // GROUPBY
    std::vector<ExprPtr> groupCols;

    // JOIN
    std::string joinType;
    std::string joinTable;
    ExprPtr joinCondition;

    // UPDATE
    std::vector<std::pair<std::string, ExprPtr>> assignments;
};

struct PipelineStmt : Statement {
    std::string tableName;
    std::vector<PipelineStage> stages;
};

struct BeginStmt : Statement {};
struct CommitStmt : Statement {};
struct RollbackStmt : Statement {};

struct ShowTablesStmt : Statement {};
struct DescribeStmt : Statement {
    std::string tableName;
};

struct PrintStmt : Statement {
    ExprPtr expr;
};

// Variable declaration and assignment (from original language)
struct VarDeclStmt : Statement {
    ValueType type;
    std::vector<std::string> names;
};

struct AssignStmt : Statement {
    std::string varName;
    ExprPtr value;
};

// Control flow
struct IfStmt : Statement {
    ExprPtr condition;
    std::vector<StmtPtr> thenBody;
    std::vector<StmtPtr> elseBody;
};

struct WhileStmt : Statement {
    ExprPtr condition;
    std::vector<StmtPtr> body;
};

struct FuncDefStmt : Statement {
    std::string name;
    ValueType returnType;
    std::vector<std::pair<ValueType, std::string>> params;
    std::vector<StmtPtr> body;
};

struct ReturnStmt : Statement {
    ExprPtr value;
};

struct FuncCallStmt : Statement {
    std::string name;
    std::vector<ExprPtr> args;
};

// The Parser
class DbParser {
public:
    DbParser();
    explicit DbParser(const std::vector<DbToken>& tokens);

    std::vector<StmtPtr> parse();
    void setTokens(const std::vector<DbToken>& tokens);
    bool hasErrors() const { return !errors_.empty(); }
    const std::vector<std::string>& getErrors() const { return errors_; }

private:
    std::vector<DbToken> tokens_;
    size_t pos_;
    std::vector<std::string> errors_;

    // Token navigation
    const DbToken& peek() const;
    const DbToken& peekNext() const;
    const DbToken& advance();
    bool check(DbTokenType type) const;
    bool match(DbTokenType type);
    const DbToken& expect(DbTokenType type, const std::string& msg);
    bool isAtEnd() const;

    void error(const std::string& msg);

    // Statement parsers
    StmtPtr parseStatement();
    StmtPtr parseCreateTable();
    StmtPtr parseDropTable();
    StmtPtr parseInsert();
    StmtPtr parseSelect();
    StmtPtr parseUpdate();
    StmtPtr parseDelete();
    StmtPtr parsePipeline(const std::string& tableName);
    StmtPtr parseBegin();
    StmtPtr parseCommit();
    StmtPtr parseRollback();
    StmtPtr parseShowTables();
    StmtPtr parseDescribe();
    StmtPtr parseVarDecl(ValueType type);
    StmtPtr parseAssignOrPipeline();
    StmtPtr parseIf();
    StmtPtr parseWhile();
    StmtPtr parseFuncDef();
    StmtPtr parseReturn();
    StmtPtr parsePrint();

    PipelineStage parsePipelineStage();

    // Expression parsers
    ExprPtr parseExpression();
    ExprPtr parseOrExpr();
    ExprPtr parseAndExpr();
    ExprPtr parseNotExpr();
    ExprPtr parseComparisonExpr();
    ExprPtr parseAddExpr();
    ExprPtr parseMulExpr();
    ExprPtr parseUnaryExpr();
    ExprPtr parsePrimaryExpr();
    ExprPtr parseColumnOrFunction();

    std::vector<ExprPtr> parseExpressionList();
    ValueType parseType();

    std::vector<StmtPtr> parseBlock(DbTokenType terminator);
};

} // namespace epee

#endif /* EPEE_DB_PARSER_H */
