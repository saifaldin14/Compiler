/*
 File: executor.hpp
 Project: Épée Database Query Language
 Description: Query executor that evaluates parsed statements against the in-memory database
*/

#ifndef EPEE_EXECUTOR_H
#define EPEE_EXECUTOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <set>
#include "table.hpp"
#include "dbParser.hpp"
#include "value.hpp"

namespace epee {

class Executor {
public:
    Executor();
    explicit Executor(Database& db);

    QueryResult execute(const StmtPtr& stmt);
    QueryResult executeAll(const std::vector<StmtPtr>& stmts);

    Database& getDatabase() { return *db_; }

private:
    Database* db_;
    Database ownedDb_;

    // Variable storage for imperative code
    std::unordered_map<std::string, Value> variables_;
    // Function storage
    std::unordered_map<std::string, std::shared_ptr<FuncDefStmt>> functions_;

    // Statement executors
    QueryResult executeCreateTable(const CreateTableStmt& stmt);
    QueryResult executeDropTable(const DropTableStmt& stmt);
    QueryResult executeInsert(const InsertStmt& stmt);
    QueryResult executeSelect(const SelectStmt& stmt);
    QueryResult executeUpdate(const UpdateStmt& stmt);
    QueryResult executeDelete(const DeleteStmt& stmt);
    QueryResult executePipeline(const PipelineStmt& stmt);
    QueryResult executeBegin();
    QueryResult executeCommit();
    QueryResult executeRollback();
    QueryResult executeShowTables();
    QueryResult executeDescribe(const DescribeStmt& stmt);
    QueryResult executePrint(const PrintStmt& stmt);
    QueryResult executeVarDecl(const VarDeclStmt& stmt);
    QueryResult executeAssign(const AssignStmt& stmt);
    QueryResult executeIf(const IfStmt& stmt);
    QueryResult executeWhile(const WhileStmt& stmt);
    QueryResult executeFuncDef(const FuncDefStmt& stmt);
    QueryResult executeReturn(const ReturnStmt& stmt);
    QueryResult executeFuncCall(const FuncCallStmt& stmt);

    // Expression evaluation
    Value evaluate(const ExprPtr& expr, const Row& row = {},
                   const std::vector<std::string>& colNames = {}) const;

    // Helper: evaluate with joined table context
    Value evaluateWithContext(const ExprPtr& expr, const Row& row,
                             const std::vector<std::string>& colNames) const;

    // Aggregate evaluation
    Value evaluateAggregate(const FunctionCallExpr& func,
                           const std::vector<Row>& rows,
                           const std::vector<std::string>& colNames) const;

    // String function evaluation
    Value evaluateStringFunc(const std::string& name,
                            const std::vector<Value>& args) const;

    // Build predicate from WHERE clause
    std::function<bool(const Row&)> buildPredicate(
        const ExprPtr& expr, const std::vector<std::string>& colNames) const;

    // Helper for column resolution
    int resolveColumn(const std::string& name,
                      const std::vector<std::string>& colNames) const;

    // Get column name from expression
    std::string getExprName(const ExprPtr& expr) const;

    // Pipeline helpers
    QueryResult applyPipelineStage(const PipelineStage& stage,
                                   QueryResult& current,
                                   const std::string& originalTable);

    // Sorting helper
    void sortResult(QueryResult& result,
                    const std::vector<std::pair<ExprPtr, bool>>& orderCols);

    // Grouping helper
    QueryResult groupAndAggregate(QueryResult& input,
                                  const std::vector<ExprPtr>& groupCols,
                                  const std::vector<ExprPtr>& selectCols,
                                  const ExprPtr& havingClause);

    // Join helper
    QueryResult performJoin(const Table& leftTable, const Table& rightTable,
                           const ExprPtr& onCondition, const std::string& joinType);
};

} // namespace epee

#endif /* EPEE_EXECUTOR_H */
