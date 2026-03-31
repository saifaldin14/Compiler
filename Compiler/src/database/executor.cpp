/*
 File: executor.cpp
 Project: Épée Database Query Language
 Description: Query executor that evaluates parsed statements against the in-memory database
*/

#include "../../include/database/executor.hpp"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cctype>
#include <sstream>
#include <set>
#include <map>

namespace epee {

// Exception used internally to propagate return values out of function bodies
struct ReturnException {
    Value value;
    explicit ReturnException(const Value& v) : value(v) {}
};

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Executor::Executor() : db_(&ownedDb_) {}

Executor::Executor(Database& db) : db_(&db) {}

// ---------------------------------------------------------------------------
// Top-level dispatch
// ---------------------------------------------------------------------------

QueryResult Executor::execute(const StmtPtr& stmt) {
    if (!stmt) return QueryResult("Null statement", false);

    try {
        if (auto s = std::dynamic_pointer_cast<CreateTableStmt>(stmt))
            return executeCreateTable(*s);
        if (auto s = std::dynamic_pointer_cast<DropTableStmt>(stmt))
            return executeDropTable(*s);
        if (auto s = std::dynamic_pointer_cast<InsertStmt>(stmt))
            return executeInsert(*s);
        if (auto s = std::dynamic_pointer_cast<SelectStmt>(stmt))
            return executeSelect(*s);
        if (auto s = std::dynamic_pointer_cast<UpdateStmt>(stmt))
            return executeUpdate(*s);
        if (auto s = std::dynamic_pointer_cast<DeleteStmt>(stmt))
            return executeDelete(*s);
        if (auto s = std::dynamic_pointer_cast<PipelineStmt>(stmt))
            return executePipeline(*s);
        if (std::dynamic_pointer_cast<BeginStmt>(stmt))
            return executeBegin();
        if (std::dynamic_pointer_cast<CommitStmt>(stmt))
            return executeCommit();
        if (std::dynamic_pointer_cast<RollbackStmt>(stmt))
            return executeRollback();
        if (std::dynamic_pointer_cast<ShowTablesStmt>(stmt))
            return executeShowTables();
        if (auto s = std::dynamic_pointer_cast<DescribeStmt>(stmt))
            return executeDescribe(*s);
        if (auto s = std::dynamic_pointer_cast<PrintStmt>(stmt))
            return executePrint(*s);
        if (auto s = std::dynamic_pointer_cast<VarDeclStmt>(stmt))
            return executeVarDecl(*s);
        if (auto s = std::dynamic_pointer_cast<AssignStmt>(stmt))
            return executeAssign(*s);
        if (auto s = std::dynamic_pointer_cast<IfStmt>(stmt))
            return executeIf(*s);
        if (auto s = std::dynamic_pointer_cast<WhileStmt>(stmt))
            return executeWhile(*s);
        if (auto s = std::dynamic_pointer_cast<FuncDefStmt>(stmt))
            return executeFuncDef(*s);
        if (auto s = std::dynamic_pointer_cast<ReturnStmt>(stmt))
            return executeReturn(*s);
        if (auto s = std::dynamic_pointer_cast<FuncCallStmt>(stmt))
            return executeFuncCall(*s);

        return QueryResult("Unknown statement type", false);
    } catch (const ReturnException&) {
        throw; // Let return exceptions propagate
    } catch (const std::exception& e) {
        return QueryResult(std::string("Error: ") + e.what(), false);
    }
}

QueryResult Executor::executeAll(const std::vector<StmtPtr>& stmts) {
    QueryResult last;
    for (const auto& stmt : stmts) {
        last = execute(stmt);
        if (!last.success) return last;
    }
    return last;
}

// ---------------------------------------------------------------------------
// CREATE TABLE
// ---------------------------------------------------------------------------

QueryResult Executor::executeCreateTable(const CreateTableStmt& stmt) {
    std::vector<Column> columns;
    columns.reserve(stmt.columns.size());
    for (const auto& cd : stmt.columns) {
        Column col(cd.name, cd.type, cd.primaryKey);
        col.nullable = cd.nullable;
        col.unique = cd.unique;
        columns.push_back(col);
    }
    db_->createTable(stmt.tableName, columns);
    return QueryResult("Table '" + stmt.tableName + "' created.");
}

// ---------------------------------------------------------------------------
// DROP TABLE
// ---------------------------------------------------------------------------

QueryResult Executor::executeDropTable(const DropTableStmt& stmt) {
    db_->dropTable(stmt.tableName);
    return QueryResult("Table '" + stmt.tableName + "' dropped.");
}

// ---------------------------------------------------------------------------
// INSERT INTO
// ---------------------------------------------------------------------------

QueryResult Executor::executeInsert(const InsertStmt& stmt) {
    Table& table = db_->getTable(stmt.tableName);
    const auto& tableCols = table.getColumns();
    int inserted = 0;

    for (const auto& valueRow : stmt.valueRows) {
        Row row(tableCols.size());

        if (!stmt.columns.empty()) {
            // Named columns — fill defaults/nulls first
            for (size_t i = 0; i < tableCols.size(); i++)
                row[i] = tableCols[i].defaultValue;

            for (size_t i = 0; i < stmt.columns.size(); i++) {
                int idx = table.getColumnIndex(stmt.columns[i]);
                if (idx < 0)
                    throw std::runtime_error("Unknown column '" + stmt.columns[i] + "'");
                Value val = (i < valueRow.size()) ? evaluate(valueRow[i]) : Value();
                row[static_cast<size_t>(idx)] = val;
            }
        } else {
            // Positional values
            for (size_t i = 0; i < tableCols.size(); i++) {
                if (i < valueRow.size())
                    row[i] = evaluate(valueRow[i]);
                else
                    row[i] = tableCols[i].defaultValue;
            }
        }

        table.insertRow(row);
        inserted++;
    }

    QueryResult result("Inserted " + std::to_string(inserted) + " row(s).");
    result.affectedRows = inserted;
    return result;
}

// ---------------------------------------------------------------------------
// SELECT
// ---------------------------------------------------------------------------

QueryResult Executor::executeSelect(const SelectStmt& stmt) {
    // Start with the base table
    QueryResult result;
    std::vector<std::string> colNames;
    std::vector<Row> rows;

    if (!stmt.fromTable.empty()) {
        const Table& table = db_->getTable(stmt.fromTable);
        result = table.selectAll();
        colNames = result.columnNames;
        rows = result.rows;
    }

    // Process JOINs
    for (const auto& join : stmt.joins) {
        const Table& rightTable = db_->getTable(join.tableName);

        // Build a temporary Table-like representation from current result for join
        // We'll work directly with rows and colNames
        std::vector<std::string> rightCols;
        for (const auto& c : rightTable.getColumns())
            rightCols.push_back(join.tableName + "." + c.name);

        std::vector<std::string> leftCols = colNames;
        // Prefix left columns with table name if not already prefixed
        if (!stmt.fromTable.empty()) {
            for (auto& c : leftCols) {
                if (c.find('.') == std::string::npos)
                    c = stmt.fromTable + "." + c;
            }
        }

        std::vector<std::string> joinedCols;
        joinedCols.insert(joinedCols.end(), leftCols.begin(), leftCols.end());
        joinedCols.insert(joinedCols.end(), rightCols.begin(), rightCols.end());

        std::vector<Row> joinedRows;
        const auto& rightRows = rightTable.getRows();

        if (join.joinType == "cross") {
            for (const auto& lrow : rows) {
                for (const auto& rrow : rightRows) {
                    Row combined = lrow;
                    combined.insert(combined.end(), rrow.begin(), rrow.end());
                    joinedRows.push_back(combined);
                }
            }
        } else if (join.joinType == "inner") {
            for (const auto& lrow : rows) {
                for (const auto& rrow : rightRows) {
                    Row combined = lrow;
                    combined.insert(combined.end(), rrow.begin(), rrow.end());
                    Value cond = evaluate(join.onCondition, combined, joinedCols);
                    if (cond.asBool())
                        joinedRows.push_back(combined);
                }
            }
        } else if (join.joinType == "left") {
            for (const auto& lrow : rows) {
                bool matched = false;
                for (const auto& rrow : rightRows) {
                    Row combined = lrow;
                    combined.insert(combined.end(), rrow.begin(), rrow.end());
                    Value cond = evaluate(join.onCondition, combined, joinedCols);
                    if (cond.asBool()) {
                        joinedRows.push_back(combined);
                        matched = true;
                    }
                }
                if (!matched) {
                    Row combined = lrow;
                    for (size_t i = 0; i < rightCols.size(); i++)
                        combined.push_back(Value());
                    joinedRows.push_back(combined);
                }
            }
        } else if (join.joinType == "right") {
            for (const auto& rrow : rightRows) {
                bool matched = false;
                for (const auto& lrow : rows) {
                    Row combined = lrow;
                    combined.insert(combined.end(), rrow.begin(), rrow.end());
                    Value cond = evaluate(join.onCondition, combined, joinedCols);
                    if (cond.asBool()) {
                        joinedRows.push_back(combined);
                        matched = true;
                    }
                }
                if (!matched) {
                    Row combined;
                    for (size_t i = 0; i < leftCols.size(); i++)
                        combined.push_back(Value());
                    combined.insert(combined.end(), rrow.begin(), rrow.end());
                    joinedRows.push_back(combined);
                }
            }
        }

        colNames = joinedCols;
        rows = joinedRows;
    }

    // WHERE filter
    if (stmt.whereClause) {
        auto pred = buildPredicate(stmt.whereClause, colNames);
        std::vector<Row> filtered;
        for (const auto& row : rows) {
            if (pred(row)) filtered.push_back(row);
        }
        rows = std::move(filtered);
    }

    // GROUP BY
    if (!stmt.groupBy.empty()) {
        QueryResult grouped;
        grouped.columnNames = colNames;
        grouped.rows = rows;
        QueryResult groupedResult = groupAndAggregate(grouped, stmt.groupBy, stmt.columns, stmt.havingClause);

        if (!stmt.orderBy.empty())
            sortResult(groupedResult, stmt.orderBy);

        if (stmt.distinct) {
            std::vector<Row> unique;
            std::set<std::vector<std::string>> seen;
            for (const auto& row : groupedResult.rows) {
                std::vector<std::string> key;
                for (const auto& v : row) key.push_back(v.asString());
                if (seen.insert(key).second)
                    unique.push_back(row);
            }
            groupedResult.rows = std::move(unique);
        }

        if (stmt.offset > 0) {
            int off = std::min(stmt.offset, static_cast<int>(groupedResult.rows.size()));
            groupedResult.rows.erase(groupedResult.rows.begin(), groupedResult.rows.begin() + off);
        }

        if (stmt.limit >= 0 && static_cast<int>(groupedResult.rows.size()) > stmt.limit)
            groupedResult.rows.resize(static_cast<size_t>(stmt.limit));

        groupedResult.message = std::to_string(groupedResult.rows.size()) + " row(s) returned.";
        return groupedResult;
    }

    // Check if there are aggregate functions without GROUP BY
    bool hasAggregates = false;
    for (const auto& col : stmt.columns) {
        if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(col)) {
            std::string lower = fc->name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower == "count" || lower == "sum" || lower == "avg" ||
                lower == "min" || lower == "max") {
                hasAggregates = true;
                break;
            }
        }
        if (auto alias = std::dynamic_pointer_cast<AliasExpr>(col)) {
            if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(alias->expr)) {
                std::string lower = fc->name;
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower == "count" || lower == "sum" || lower == "avg" ||
                    lower == "min" || lower == "max") {
                    hasAggregates = true;
                    break;
                }
            }
        }
    }

    // Project columns
    QueryResult projected;
    bool hasStar = false;
    for (const auto& col : stmt.columns) {
        if (std::dynamic_pointer_cast<StarExpr>(col)) {
            hasStar = true;
            break;
        }
    }

    if (hasStar) {
        projected.columnNames = colNames;
        projected.rows = rows;
    } else if (hasAggregates) {
        // Aggregate across all rows (no GROUP BY)
        Row aggRow;
        for (const auto& col : stmt.columns) {
            std::string name = getExprName(col);
            projected.columnNames.push_back(name);

            ExprPtr exprToEval = col;
            if (auto alias = std::dynamic_pointer_cast<AliasExpr>(col))
                exprToEval = alias->expr;

            if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(exprToEval)) {
                aggRow.push_back(evaluateAggregate(*fc, rows, colNames));
            } else {
                aggRow.push_back(rows.empty() ? Value() : evaluate(col, rows[0], colNames));
            }
        }
        projected.rows.push_back(aggRow);
    } else {
        for (const auto& col : stmt.columns)
            projected.columnNames.push_back(getExprName(col));

        for (const auto& row : rows) {
            Row projRow;
            for (const auto& col : stmt.columns)
                projRow.push_back(evaluate(col, row, colNames));
            projected.rows.push_back(projRow);
        }
    }

    // ORDER BY
    if (!stmt.orderBy.empty())
        sortResult(projected, stmt.orderBy);

    // DISTINCT
    if (stmt.distinct) {
        std::vector<Row> unique;
        std::set<std::vector<std::string>> seen;
        for (const auto& row : projected.rows) {
            std::vector<std::string> key;
            for (const auto& v : row) key.push_back(v.asString());
            if (seen.insert(key).second)
                unique.push_back(row);
        }
        projected.rows = std::move(unique);
    }

    // OFFSET
    if (stmt.offset > 0) {
        int off = std::min(stmt.offset, static_cast<int>(projected.rows.size()));
        projected.rows.erase(projected.rows.begin(), projected.rows.begin() + off);
    }

    // LIMIT
    if (stmt.limit >= 0 && static_cast<int>(projected.rows.size()) > stmt.limit)
        projected.rows.resize(static_cast<size_t>(stmt.limit));

    projected.message = std::to_string(projected.rows.size()) + " row(s) returned.";
    return projected;
}

// ---------------------------------------------------------------------------
// UPDATE
// ---------------------------------------------------------------------------

QueryResult Executor::executeUpdate(const UpdateStmt& stmt) {
    Table& table = db_->getTable(stmt.tableName);
    const auto& cols = table.getColumns();
    std::vector<std::string> colNames;
    for (const auto& c : cols) colNames.push_back(c.name);

    auto predicate = stmt.whereClause
        ? buildPredicate(stmt.whereClause, colNames)
        : [](const Row&) { return true; };

    // Build update pairs
    std::vector<std::pair<int, Value>> updates;
    // We need to evaluate per-row, so use updateRows with a custom lambda
    int count = 0;
    auto& tableRows = const_cast<std::vector<Row>&>(table.getRows());
    for (auto& row : tableRows) {
        if (predicate(row)) {
            for (const auto& [colName, expr] : stmt.assignments) {
                int idx = table.getColumnIndex(colName);
                if (idx < 0)
                    throw std::runtime_error("Unknown column '" + colName + "'");
                row[static_cast<size_t>(idx)] = evaluate(expr, row, colNames);
            }
            count++;
        }
    }

    QueryResult result("Updated " + std::to_string(count) + " row(s).");
    result.affectedRows = count;
    return result;
}

// ---------------------------------------------------------------------------
// DELETE
// ---------------------------------------------------------------------------

QueryResult Executor::executeDelete(const DeleteStmt& stmt) {
    Table& table = db_->getTable(stmt.tableName);
    const auto& cols = table.getColumns();
    std::vector<std::string> colNames;
    for (const auto& c : cols) colNames.push_back(c.name);

    auto predicate = stmt.whereClause
        ? buildPredicate(stmt.whereClause, colNames)
        : [](const Row&) { return true; };

    int count = table.deleteRows(predicate);
    QueryResult result("Deleted " + std::to_string(count) + " row(s).");
    result.affectedRows = count;
    return result;
}

// ---------------------------------------------------------------------------
// PIPELINE
// ---------------------------------------------------------------------------

QueryResult Executor::executePipeline(const PipelineStmt& stmt) {
    Table& table = db_->getTable(stmt.tableName);
    QueryResult current = table.selectAll();

    for (const auto& stage : stmt.stages) {
        current = applyPipelineStage(stage, current, stmt.tableName);
        if (!current.success) return current;
    }

    return current;
}

QueryResult Executor::applyPipelineStage(const PipelineStage& stage,
                                          QueryResult& current,
                                          const std::string& originalTable) {
    switch (stage.type) {
    case PipelineStage::Type::WHERE: {
        auto pred = buildPredicate(stage.condition, current.columnNames);
        std::vector<Row> filtered;
        for (const auto& row : current.rows) {
            if (pred(row)) filtered.push_back(row);
        }
        current.rows = std::move(filtered);
        return current;
    }

    case PipelineStage::Type::SELECT: {
        QueryResult projected;
        bool hasStar = false;
        for (const auto& col : stage.columns) {
            if (std::dynamic_pointer_cast<StarExpr>(col)) {
                hasStar = true;
                break;
            }
        }

        if (hasStar) {
            projected = current;
        } else {
            // Check for aggregates
            bool hasAgg = false;
            for (const auto& col : stage.columns) {
                ExprPtr inner = col;
                if (auto alias = std::dynamic_pointer_cast<AliasExpr>(col))
                    inner = alias->expr;
                if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(inner)) {
                    std::string lower = fc->name;
                    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                    if (lower == "count" || lower == "sum" || lower == "avg" ||
                        lower == "min" || lower == "max") {
                        hasAgg = true;
                        break;
                    }
                }
            }

            for (const auto& col : stage.columns)
                projected.columnNames.push_back(getExprName(col));

            if (hasAgg) {
                Row aggRow;
                for (const auto& col : stage.columns) {
                    ExprPtr inner = col;
                    if (auto alias = std::dynamic_pointer_cast<AliasExpr>(col))
                        inner = alias->expr;
                    if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(inner))
                        aggRow.push_back(evaluateAggregate(*fc, current.rows, current.columnNames));
                    else
                        aggRow.push_back(current.rows.empty() ? Value() : evaluate(col, current.rows[0], current.columnNames));
                }
                projected.rows.push_back(aggRow);
            } else {
                for (const auto& row : current.rows) {
                    Row projRow;
                    for (const auto& col : stage.columns)
                        projRow.push_back(evaluate(col, row, current.columnNames));
                    projected.rows.push_back(projRow);
                }
            }
        }

        if (stage.selectDistinct) {
            std::vector<Row> unique;
            std::set<std::vector<std::string>> seen;
            for (const auto& row : projected.rows) {
                std::vector<std::string> key;
                for (const auto& v : row) key.push_back(v.asString());
                if (seen.insert(key).second)
                    unique.push_back(row);
            }
            projected.rows = std::move(unique);
        }

        return projected;
    }

    case PipelineStage::Type::ORDERBY: {
        sortResult(current, stage.orderCols);
        return current;
    }

    case PipelineStage::Type::LIMIT: {
        if (stage.limitCount >= 0 && static_cast<int>(current.rows.size()) > stage.limitCount)
            current.rows.resize(static_cast<size_t>(stage.limitCount));
        return current;
    }

    case PipelineStage::Type::OFFSET: {
        if (stage.offsetCount > 0) {
            int off = std::min(stage.offsetCount, static_cast<int>(current.rows.size()));
            current.rows.erase(current.rows.begin(), current.rows.begin() + off);
        }
        return current;
    }

    case PipelineStage::Type::GROUPBY: {
        // GROUP BY without explicit select — keep all columns
        return groupAndAggregate(current, stage.groupCols, {}, nullptr);
    }

    case PipelineStage::Type::HAVING: {
        auto pred = buildPredicate(stage.condition, current.columnNames);
        std::vector<Row> filtered;
        for (const auto& row : current.rows) {
            if (pred(row)) filtered.push_back(row);
        }
        current.rows = std::move(filtered);
        return current;
    }

    case PipelineStage::Type::JOIN: {
        const Table& rightTable = db_->getTable(stage.joinTable);

        QueryResult joinResult;
        std::vector<std::string> rightCols;
        for (const auto& c : rightTable.getColumns())
            rightCols.push_back(stage.joinTable + "." + c.name);

        joinResult.columnNames = current.columnNames;
        joinResult.columnNames.insert(joinResult.columnNames.end(), rightCols.begin(), rightCols.end());

        const auto& rightRows = rightTable.getRows();

        if (stage.joinType == "cross") {
            for (const auto& lr : current.rows) {
                for (const auto& rr : rightRows) {
                    Row combined = lr;
                    combined.insert(combined.end(), rr.begin(), rr.end());
                    joinResult.rows.push_back(combined);
                }
            }
        } else if (stage.joinType == "inner") {
            for (const auto& lr : current.rows) {
                for (const auto& rr : rightRows) {
                    Row combined = lr;
                    combined.insert(combined.end(), rr.begin(), rr.end());
                    Value cond = evaluate(stage.joinCondition, combined, joinResult.columnNames);
                    if (cond.asBool())
                        joinResult.rows.push_back(combined);
                }
            }
        } else if (stage.joinType == "left") {
            for (const auto& lr : current.rows) {
                bool matched = false;
                for (const auto& rr : rightRows) {
                    Row combined = lr;
                    combined.insert(combined.end(), rr.begin(), rr.end());
                    Value cond = evaluate(stage.joinCondition, combined, joinResult.columnNames);
                    if (cond.asBool()) {
                        joinResult.rows.push_back(combined);
                        matched = true;
                    }
                }
                if (!matched) {
                    Row combined = lr;
                    for (size_t i = 0; i < rightCols.size(); i++)
                        combined.push_back(Value());
                    joinResult.rows.push_back(combined);
                }
            }
        } else if (stage.joinType == "right") {
            for (const auto& rr : rightRows) {
                bool matched = false;
                for (const auto& lr : current.rows) {
                    Row combined = lr;
                    combined.insert(combined.end(), rr.begin(), rr.end());
                    Value cond = evaluate(stage.joinCondition, combined, joinResult.columnNames);
                    if (cond.asBool()) {
                        joinResult.rows.push_back(combined);
                        matched = true;
                    }
                }
                if (!matched) {
                    Row combined;
                    for (size_t i = 0; i < current.columnNames.size(); i++)
                        combined.push_back(Value());
                    combined.insert(combined.end(), rr.begin(), rr.end());
                    joinResult.rows.push_back(combined);
                }
            }
        }

        return joinResult;
    }

    case PipelineStage::Type::UPDATE: {
        if (!db_->hasTable(originalTable))
            return QueryResult("Cannot update: table '" + originalTable + "' not found", false);

        Table& table = db_->getTable(originalTable);
        const auto& tableCols = table.getColumns();
        std::vector<std::string> tableColNames;
        for (const auto& c : tableCols) tableColNames.push_back(c.name);

        // Build a set of row-signatures from the current (filtered) pipeline result
        // to identify which rows to update
        auto& tableRows = const_cast<std::vector<Row>&>(table.getRows());
        int count = 0;

        // Match rows by content
        std::set<size_t> matchIndices;
        for (size_t ti = 0; ti < tableRows.size(); ti++) {
            for (const auto& prow : current.rows) {
                bool match = true;
                size_t limit = std::min(tableRows[ti].size(), prow.size());
                for (size_t c = 0; c < limit; c++) {
                    if (!(tableRows[ti][c] == prow[c])) { match = false; break; }
                }
                if (match) { matchIndices.insert(ti); break; }
            }
        }

        for (size_t ti : matchIndices) {
            for (const auto& [colName, expr] : stage.assignments) {
                int idx = table.getColumnIndex(colName);
                if (idx < 0)
                    throw std::runtime_error("Unknown column '" + colName + "'");
                tableRows[ti][static_cast<size_t>(idx)] = evaluate(expr, tableRows[ti], tableColNames);
            }
            count++;
        }

        QueryResult result("Updated " + std::to_string(count) + " row(s).");
        result.affectedRows = count;
        return result;
    }

    case PipelineStage::Type::DELETE_STAGE: {
        if (!db_->hasTable(originalTable))
            return QueryResult("Cannot delete: table '" + originalTable + "' not found", false);

        Table& table = db_->getTable(originalTable);
        auto& tableRows = const_cast<std::vector<Row>&>(table.getRows());

        // Match pipeline rows against table rows and delete
        int count = 0;
        auto it = tableRows.begin();
        while (it != tableRows.end()) {
            bool found = false;
            for (const auto& prow : current.rows) {
                bool match = true;
                size_t limit = std::min(it->size(), prow.size());
                for (size_t c = 0; c < limit; c++) {
                    if (!((*it)[c] == prow[c])) { match = false; break; }
                }
                if (match) { found = true; break; }
            }
            if (found) {
                it = tableRows.erase(it);
                count++;
            } else {
                ++it;
            }
        }

        QueryResult result("Deleted " + std::to_string(count) + " row(s).");
        result.affectedRows = count;
        return result;
    }

    case PipelineStage::Type::PRINT: {
        std::cout << current.toPrettyTable();
        return current;
    }

    case PipelineStage::Type::DISTINCT: {
        std::vector<Row> unique;
        std::set<std::vector<std::string>> seen;
        for (const auto& row : current.rows) {
            std::vector<std::string> key;
            for (const auto& v : row) key.push_back(v.asString());
            if (seen.insert(key).second)
                unique.push_back(row);
        }
        current.rows = std::move(unique);
        return current;
    }

    case PipelineStage::Type::COUNT_STAGE: {
        QueryResult countResult;
        countResult.columnNames = {"count"};
        Row r;
        r.push_back(Value(static_cast<int>(current.rows.size())));
        countResult.rows.push_back(r);
        return countResult;
    }
    }

    return current;
}

// ---------------------------------------------------------------------------
// Transaction support
// ---------------------------------------------------------------------------

QueryResult Executor::executeBegin() {
    db_->beginTransaction();
    return QueryResult("Transaction started.");
}

QueryResult Executor::executeCommit() {
    db_->commitTransaction();
    return QueryResult("Transaction committed.");
}

QueryResult Executor::executeRollback() {
    db_->rollbackTransaction();
    return QueryResult("Transaction rolled back.");
}

// ---------------------------------------------------------------------------
// SHOW TABLES / DESCRIBE
// ---------------------------------------------------------------------------

QueryResult Executor::executeShowTables() {
    return db_->showTables();
}

QueryResult Executor::executeDescribe(const DescribeStmt& stmt) {
    const Table& table = db_->getTable(stmt.tableName);
    return table.describe();
}

// ---------------------------------------------------------------------------
// PRINT
// ---------------------------------------------------------------------------

QueryResult Executor::executePrint(const PrintStmt& stmt) {
    Value val = evaluate(stmt.expr);
    std::cout << val.asString() << "\n";
    QueryResult result(val.asString());
    result.columnNames = {"output"};
    result.rows.push_back({val});
    return result;
}

// ---------------------------------------------------------------------------
// Variable / assignment / control flow / functions
// ---------------------------------------------------------------------------

QueryResult Executor::executeVarDecl(const VarDeclStmt& stmt) {
    for (const auto& name : stmt.names) {
        switch (stmt.type) {
            case ValueType::INT:    variables_[name] = Value(0); break;
            case ValueType::DOUBLE: variables_[name] = Value(0.0); break;
            case ValueType::STRING: variables_[name] = Value(std::string("")); break;
            case ValueType::BOOL:   variables_[name] = Value(false); break;
            default:                variables_[name] = Value(); break;
        }
    }
    return QueryResult("Variable(s) declared.");
}

QueryResult Executor::executeAssign(const AssignStmt& stmt) {
    Value val = evaluate(stmt.value);
    variables_[stmt.varName] = val;
    return QueryResult("Variable '" + stmt.varName + "' assigned.");
}

QueryResult Executor::executeIf(const IfStmt& stmt) {
    Value cond = evaluate(stmt.condition);
    if (cond.asBool()) {
        return executeAll(stmt.thenBody);
    } else if (!stmt.elseBody.empty()) {
        return executeAll(stmt.elseBody);
    }
    return QueryResult("OK");
}

QueryResult Executor::executeWhile(const WhileStmt& stmt) {
    QueryResult last("OK");
    while (true) {
        Value cond = evaluate(stmt.condition);
        if (!cond.asBool()) break;
        last = executeAll(stmt.body);
        if (!last.success) return last;
    }
    return last;
}

QueryResult Executor::executeFuncDef(const FuncDefStmt& stmt) {
    functions_[stmt.name] = std::make_shared<FuncDefStmt>(stmt);
    return QueryResult("Function '" + stmt.name + "' defined.");
}

QueryResult Executor::executeReturn(const ReturnStmt& stmt) {
    Value val = stmt.value ? evaluate(stmt.value) : Value();
    throw ReturnException(val);
}

QueryResult Executor::executeFuncCall(const FuncCallStmt& stmt) {
    auto it = functions_.find(stmt.name);
    if (it == functions_.end())
        return QueryResult("Error: undefined function '" + stmt.name + "'", false);

    const auto& funcDef = it->second;

    // Save current variables and set up params
    auto savedVars = variables_;

    for (size_t i = 0; i < funcDef->params.size(); i++) {
        Value argVal = (i < stmt.args.size()) ? evaluate(stmt.args[i]) : Value();
        variables_[funcDef->params[i].second] = argVal;
    }

    QueryResult result("OK");
    try {
        result = executeAll(funcDef->body);
    } catch (const ReturnException& ret) {
        result = QueryResult(ret.value.asString());
        result.columnNames = {"result"};
        result.rows.push_back({ret.value});
    }

    variables_ = savedVars;
    return result;
}

// ---------------------------------------------------------------------------
// Expression evaluation
// ---------------------------------------------------------------------------

Value Executor::evaluate(const ExprPtr& expr, const Row& row,
                         const std::vector<std::string>& colNames) const {
    if (!expr) return Value();

    // LiteralExpr
    if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr))
        return lit->value;

    // ColumnExpr
    if (auto col = std::dynamic_pointer_cast<ColumnExpr>(expr)) {
        std::string name = col->fullName();
        int idx = resolveColumn(name, colNames);
        if (idx >= 0 && idx < static_cast<int>(row.size()))
            return row[static_cast<size_t>(idx)];

        // Try variable lookup
        auto vit = variables_.find(col->columnName);
        if (vit != variables_.end()) return vit->second;
        if (!col->tableName.empty()) {
            vit = variables_.find(name);
            if (vit != variables_.end()) return vit->second;
        }

        // Column not found — if row is empty, this might be a variable context
        if (row.empty() && colNames.empty()) {
            throw std::runtime_error("Undefined variable '" + name + "'");
        }
        return Value(); // NULL for missing column
    }

    // BinaryExpr
    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        // Short-circuit for logical operators
        if (bin->op == "and") {
            Value left = evaluate(bin->left, row, colNames);
            if (!left.asBool()) return Value(false);
            return Value(evaluate(bin->right, row, colNames).asBool());
        }
        if (bin->op == "or") {
            Value left = evaluate(bin->left, row, colNames);
            if (left.asBool()) return Value(true);
            return Value(evaluate(bin->right, row, colNames).asBool());
        }

        Value left = evaluate(bin->left, row, colNames);
        Value right = evaluate(bin->right, row, colNames);

        if (bin->op == "+")  return left + right;
        if (bin->op == "-")  return left - right;
        if (bin->op == "*")  return left * right;
        if (bin->op == "/")  return left / right;
        if (bin->op == "%")  return left % right;
        if (bin->op == "==" || bin->op == "=")  return Value(left == right);
        if (bin->op == "!=" || bin->op == "<>") return Value(left != right);
        if (bin->op == "<")  return Value(left < right);
        if (bin->op == ">")  return Value(left > right);
        if (bin->op == "<=") return Value(left <= right);
        if (bin->op == ">=") return Value(left >= right);

        throw std::runtime_error("Unknown binary operator: " + bin->op);
    }

    // UnaryExpr
    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        Value val = evaluate(un->operand, row, colNames);
        if (un->op == "-") return -val;
        if (un->op == "not" || un->op == "!") return Value(!val.asBool());
        throw std::runtime_error("Unknown unary operator: " + un->op);
    }

    // FunctionCallExpr
    if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(expr)) {
        std::string lowerName = fc->name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        // Check for user-defined function first
        auto fit = functions_.find(fc->name);
        if (fit != functions_.end()) {
            // User-defined function call within const evaluate():
            // The header declares evaluate() as const, but calling user-defined
            // functions requires modifying variables_ for parameter binding.
            // We save/restore state to maintain logical const-ness.
            const auto& funcDef = fit->second;
            auto savedVars = variables_;
            auto& mutVars = const_cast<std::unordered_map<std::string, Value>&>(variables_);
            for (size_t i = 0; i < funcDef->params.size(); i++) {
                Value argVal = (i < fc->args.size()) ? evaluate(fc->args[i], row, colNames) : Value();
                mutVars[funcDef->params[i].second] = argVal;
            }
            Value result;
            try {
                const_cast<Executor*>(this)->executeAll(funcDef->body);
            } catch (const ReturnException& ret) {
                result = ret.value;
            }
            mutVars = savedVars;
            return result;
        }

        // Aggregate functions — when evaluated per-row, operate on single value
        if (lowerName == "count" || lowerName == "sum" || lowerName == "avg" ||
            lowerName == "min" || lowerName == "max") {
            // In per-row context, just return the value (single row aggregate)
            if (fc->args.empty() || std::dynamic_pointer_cast<StarExpr>(fc->args[0])) {
                if (lowerName == "count") return Value(1);
                return Value();
            }
            return evaluate(fc->args[0], row, colNames);
        }

        // String functions
        std::vector<Value> argVals;
        for (const auto& arg : fc->args)
            argVals.push_back(evaluate(arg, row, colNames));
        return evaluateStringFunc(lowerName, argVals);
    }

    // AliasExpr
    if (auto alias = std::dynamic_pointer_cast<AliasExpr>(expr))
        return evaluate(alias->expr, row, colNames);

    // StarExpr — shouldn't be evaluated directly
    if (std::dynamic_pointer_cast<StarExpr>(expr))
        return Value();

    // BetweenExpr
    if (auto bet = std::dynamic_pointer_cast<BetweenExpr>(expr)) {
        Value val  = evaluate(bet->expr, row, colNames);
        Value low  = evaluate(bet->low, row, colNames);
        Value high = evaluate(bet->high, row, colNames);
        return Value(val.between(low, high));
    }

    // InExpr
    if (auto in = std::dynamic_pointer_cast<InExpr>(expr)) {
        Value val = evaluate(in->expr, row, colNames);
        for (const auto& v : in->values) {
            if (val == evaluate(v, row, colNames))
                return Value(true);
        }
        return Value(false);
    }

    // LikeExpr
    if (auto lk = std::dynamic_pointer_cast<LikeExpr>(expr)) {
        Value val = evaluate(lk->expr, row, colNames);
        return Value(val.like(lk->pattern));
    }

    // IsNullExpr
    if (auto isn = std::dynamic_pointer_cast<IsNullExpr>(expr)) {
        Value val = evaluate(isn->expr, row, colNames);
        bool result = val.isNull();
        return Value(isn->isNot ? !result : result);
    }

    return Value();
}

Value Executor::evaluateWithContext(const ExprPtr& expr, const Row& row,
                                    const std::vector<std::string>& colNames) const {
    return evaluate(expr, row, colNames);
}

// ---------------------------------------------------------------------------
// Aggregate evaluation
// ---------------------------------------------------------------------------

Value Executor::evaluateAggregate(const FunctionCallExpr& func,
                                  const std::vector<Row>& rows,
                                  const std::vector<std::string>& colNames) const {
    std::string lowerName = func.name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    if (lowerName == "count") {
        if (func.args.empty() || std::dynamic_pointer_cast<StarExpr>(func.args[0]))
            return Value(static_cast<int>(rows.size()));
        // COUNT(expr) — count non-null
        int count = 0;
        for (const auto& row : rows) {
            Value v = evaluate(func.args[0], row, colNames);
            if (!v.isNull()) count++;
        }
        return Value(count);
    }

    if (rows.empty()) return Value();

    if (lowerName == "sum") {
        double sum = 0.0;
        bool hasInt = true;
        for (const auto& row : rows) {
            Value v = evaluate(func.args[0], row, colNames);
            if (v.isNull()) continue;
            if (v.isDouble()) hasInt = false;
            sum += v.asDouble();
        }
        if (hasInt) return Value(static_cast<int>(sum));
        return Value(sum);
    }

    if (lowerName == "avg") {
        double sum = 0.0;
        int count = 0;
        for (const auto& row : rows) {
            Value v = evaluate(func.args[0], row, colNames);
            if (v.isNull()) continue;
            sum += v.asDouble();
            count++;
        }
        if (count == 0) return Value();
        return Value(sum / count);
    }

    if (lowerName == "min") {
        Value minVal;
        bool first = true;
        for (const auto& row : rows) {
            Value v = evaluate(func.args[0], row, colNames);
            if (v.isNull()) continue;
            if (first || v < minVal) {
                minVal = v;
                first = false;
            }
        }
        return minVal;
    }

    if (lowerName == "max") {
        Value maxVal;
        bool first = true;
        for (const auto& row : rows) {
            Value v = evaluate(func.args[0], row, colNames);
            if (v.isNull()) continue;
            if (first || v > maxVal) {
                maxVal = v;
                first = false;
            }
        }
        return maxVal;
    }

    return Value();
}

// ---------------------------------------------------------------------------
// String function evaluation
// ---------------------------------------------------------------------------

Value Executor::evaluateStringFunc(const std::string& name,
                                   const std::vector<Value>& args) const {
    if (name == "upper") {
        if (args.empty()) return Value();
        std::string s = args[0].asString();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return Value(s);
    }
    if (name == "lower") {
        if (args.empty()) return Value();
        std::string s = args[0].asString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return Value(s);
    }
    if (name == "length" || name == "len") {
        if (args.empty()) return Value();
        return Value(static_cast<int>(args[0].asString().size()));
    }
    if (name == "substr" || name == "substring") {
        if (args.size() < 2) return Value();
        std::string s = args[0].asString();
        int start = args[1].asInt();
        if (start < 0) start = 0;
        if (static_cast<size_t>(start) >= s.size()) return Value(std::string(""));
        if (args.size() >= 3) {
            int len = args[2].asInt();
            return Value(s.substr(static_cast<size_t>(start), static_cast<size_t>(len)));
        }
        return Value(s.substr(static_cast<size_t>(start)));
    }
    if (name == "concat") {
        std::string result;
        for (const auto& a : args)
            result += a.asString();
        return Value(result);
    }
    if (name == "trim") {
        if (args.empty()) return Value();
        std::string s = args[0].asString();
        size_t start = s.find_first_not_of(" \t\n\r");
        size_t end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) return Value(std::string(""));
        return Value(s.substr(start, end - start + 1));
    }
    if (name == "replace") {
        if (args.size() < 3) return Value();
        std::string s = args[0].asString();
        std::string oldStr = args[1].asString();
        std::string newStr = args[2].asString();
        if (oldStr.empty()) return Value(s);
        size_t pos = 0;
        while ((pos = s.find(oldStr, pos)) != std::string::npos) {
            s.replace(pos, oldStr.size(), newStr);
            pos += newStr.size();
        }
        return Value(s);
    }
    if (name == "abs") {
        if (args.empty()) return Value();
        if (args[0].isInt()) return Value(std::abs(args[0].asInt()));
        if (args[0].isDouble()) return Value(std::abs(args[0].asDouble()));
        return Value();
    }
    if (name == "round") {
        if (args.empty()) return Value();
        return Value(static_cast<int>(std::round(args[0].asDouble())));
    }
    if (name == "floor") {
        if (args.empty()) return Value();
        return Value(static_cast<int>(std::floor(args[0].asDouble())));
    }
    if (name == "ceil" || name == "ceiling") {
        if (args.empty()) return Value();
        return Value(static_cast<int>(std::ceil(args[0].asDouble())));
    }

    throw std::runtime_error("Unknown function: " + name);
}

// ---------------------------------------------------------------------------
// Predicate builder
// ---------------------------------------------------------------------------

std::function<bool(const Row&)> Executor::buildPredicate(
    const ExprPtr& expr, const std::vector<std::string>& colNames) const {
    return [this, expr, colNames](const Row& row) -> bool {
        Value result = evaluate(expr, row, colNames);
        return result.asBool();
    };
}

// ---------------------------------------------------------------------------
// Column resolution
// ---------------------------------------------------------------------------

int Executor::resolveColumn(const std::string& name,
                            const std::vector<std::string>& colNames) const {
    // Exact match
    for (size_t i = 0; i < colNames.size(); i++) {
        if (colNames[i] == name) return static_cast<int>(i);
    }

    // Try bare name against qualified columns (table.col matches col)
    for (size_t i = 0; i < colNames.size(); i++) {
        size_t dotPos = colNames[i].find('.');
        if (dotPos != std::string::npos) {
            std::string bare = colNames[i].substr(dotPos + 1);
            if (bare == name) return static_cast<int>(i);
        }
    }

    // Try qualified name against bare columns
    size_t dotPos = name.find('.');
    if (dotPos != std::string::npos) {
        std::string bare = name.substr(dotPos + 1);
        for (size_t i = 0; i < colNames.size(); i++) {
            if (colNames[i] == bare) return static_cast<int>(i);
        }
    }

    return -1;
}

// ---------------------------------------------------------------------------
// Get expression name (for column headers)
// ---------------------------------------------------------------------------

std::string Executor::getExprName(const ExprPtr& expr) const {
    if (!expr) return "?";

    if (auto alias = std::dynamic_pointer_cast<AliasExpr>(expr))
        return alias->alias;

    if (auto col = std::dynamic_pointer_cast<ColumnExpr>(expr))
        return col->fullName();

    if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr))
        return lit->value.asString();

    if (std::dynamic_pointer_cast<StarExpr>(expr))
        return "*";

    if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(expr)) {
        std::string result = fc->name + "(";
        for (size_t i = 0; i < fc->args.size(); i++) {
            if (i > 0) result += ", ";
            result += getExprName(fc->args[i]);
        }
        result += ")";
        return result;
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr))
        return getExprName(bin->left) + " " + bin->op + " " + getExprName(bin->right);

    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr))
        return un->op + getExprName(un->operand);

    return "expr";
}

// ---------------------------------------------------------------------------
// Sorting helper
// ---------------------------------------------------------------------------

void Executor::sortResult(QueryResult& result,
                          const std::vector<std::pair<ExprPtr, bool>>& orderCols) {
    const auto& colNames = result.columnNames;
    std::sort(result.rows.begin(), result.rows.end(),
        [this, &orderCols, &colNames](const Row& a, const Row& b) -> bool {
            for (const auto& [expr, ascending] : orderCols) {
                Value va = evaluate(expr, a, colNames);
                Value vb = evaluate(expr, b, colNames);

                if (va == vb) continue;

                // Handle NULLs: nulls sort last
                if (va.isNull()) return !ascending;
                if (vb.isNull()) return ascending;

                if (ascending)
                    return va < vb;
                else
                    return vb < va;
            }
            return false;
        });
}

// ---------------------------------------------------------------------------
// Grouping and aggregation
// ---------------------------------------------------------------------------

// File-local helper: evaluate expression with aggregate support in group context
static Value evaluateGroupExprHelper(const ExprPtr& expr,
                                      const std::vector<Row>& groupRows,
                                      const std::vector<std::string>& colNames);

QueryResult Executor::groupAndAggregate(QueryResult& input,
                                         const std::vector<ExprPtr>& groupCols,
                                         const std::vector<ExprPtr>& selectCols,
                                         const ExprPtr& havingClause) {
    const auto& colNames = input.columnNames;

    // Build groups: key -> rows
    std::map<std::vector<std::string>, std::vector<Row>> groups;
    std::vector<std::vector<std::string>> groupOrder; // preserve insertion order

    for (const auto& row : input.rows) {
        std::vector<std::string> key;
        for (const auto& gc : groupCols) {
            Value v = evaluate(gc, row, colNames);
            key.push_back(v.asString());
        }
        if (groups.find(key) == groups.end())
            groupOrder.push_back(key);
        groups[key].push_back(row);
    }

    // Determine output columns
    QueryResult result;
    const auto& outCols = selectCols.empty() ? groupCols : selectCols;

    // Check if outCols has a star
    bool hasStar = false;
    for (const auto& oc : outCols) {
        if (std::dynamic_pointer_cast<StarExpr>(oc)) { hasStar = true; break; }
    }

    if (hasStar || selectCols.empty()) {
        // Use group column names + keep same column layout
        result.columnNames = colNames;
        for (const auto& key : groupOrder) {
            const auto& groupRows = groups[key];
            if (!groupRows.empty()) {
                if (havingClause) {
                    Value hv = evaluate(havingClause, groupRows[0], colNames);
                    if (!hv.asBool()) continue;
                }
                result.rows.push_back(groupRows[0]);
            }
        }
    } else {
        for (const auto& oc : outCols)
            result.columnNames.push_back(getExprName(oc));

        for (const auto& key : groupOrder) {
            const auto& groupRows = groups[key];

            // HAVING filter
            if (havingClause) {
                // Evaluate having with aggregate support
                // We need a row context — use the first row of the group
                // but aggregates should operate over groupRows
                // Create a temporary row for the having evaluation
                Value hv = evaluateGroupExprHelper(havingClause, groupRows, colNames);
                if (!hv.asBool()) continue;
            }

            Row outRow;
            for (const auto& oc : outCols) {
                ExprPtr inner = oc;
                if (auto alias = std::dynamic_pointer_cast<AliasExpr>(oc))
                    inner = alias->expr;

                if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(inner)) {
                    std::string lower = fc->name;
                    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                    if (lower == "count" || lower == "sum" || lower == "avg" ||
                        lower == "min" || lower == "max") {
                        outRow.push_back(evaluateAggregate(*fc, groupRows, colNames));
                        continue;
                    }
                }
                // Non-aggregate: use first row of group
                outRow.push_back(groupRows.empty() ? Value() : evaluate(oc, groupRows[0], colNames));
            }
            result.rows.push_back(outRow);
        }
    }

    return result;
}

// File-local helper implementation
static Value evaluateGroupExprHelper(const ExprPtr& expr,
                                      const std::vector<Row>& groupRows,
                                      const std::vector<std::string>& colNames) {

    if (!expr) return Value();

    if (auto fc = std::dynamic_pointer_cast<FunctionCallExpr>(expr)) {
        std::string lower = fc->name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "count") {
            if (fc->args.empty() || std::dynamic_pointer_cast<StarExpr>(fc->args[0]))
                return Value(static_cast<int>(groupRows.size()));
            int count = 0;
            for (const auto& row : groupRows) {
                // Use a simple column lookup for count(col)
                if (auto col = std::dynamic_pointer_cast<ColumnExpr>(fc->args[0])) {
                    std::string name = col->fullName();
                    for (size_t i = 0; i < colNames.size(); i++) {
                        if (colNames[i] == name || 
                            (colNames[i].find('.') != std::string::npos && 
                             colNames[i].substr(colNames[i].find('.') + 1) == name)) {
                            if (i < row.size() && !row[i].isNull()) count++;
                            break;
                        }
                    }
                } else {
                    count++; // non-column expression, count all
                }
            }
            return Value(count);
        }
        if (lower == "sum" || lower == "avg" || lower == "min" || lower == "max") {
            // Delegate to aggregation over groupRows
            if (groupRows.empty()) return Value();
            // Find the column index
            if (!fc->args.empty()) {
                if (auto col = std::dynamic_pointer_cast<ColumnExpr>(fc->args[0])) {
                    std::string name = col->fullName();
                    int idx = -1;
                    for (size_t i = 0; i < colNames.size(); i++) {
                        if (colNames[i] == name) { idx = static_cast<int>(i); break; }
                        if (colNames[i].find('.') != std::string::npos &&
                            colNames[i].substr(colNames[i].find('.') + 1) == name) {
                            idx = static_cast<int>(i); break;
                        }
                    }
                    if (idx >= 0) {
                        if (lower == "sum") {
                            double sum = 0; bool allInt = true;
                            for (const auto& r : groupRows) {
                                if (static_cast<size_t>(idx) < r.size() && !r[idx].isNull()) {
                                    if (r[idx].isDouble()) allInt = false;
                                    sum += r[idx].asDouble();
                                }
                            }
                            return allInt ? Value(static_cast<int>(sum)) : Value(sum);
                        }
                        if (lower == "avg") {
                            double sum = 0; int cnt = 0;
                            for (const auto& r : groupRows) {
                                if (static_cast<size_t>(idx) < r.size() && !r[idx].isNull()) {
                                    sum += r[idx].asDouble(); cnt++;
                                }
                            }
                            return cnt > 0 ? Value(sum / cnt) : Value();
                        }
                        if (lower == "min") {
                            Value minV; bool first = true;
                            for (const auto& r : groupRows) {
                                if (static_cast<size_t>(idx) < r.size() && !r[idx].isNull()) {
                                    if (first || r[idx] < minV) { minV = r[idx]; first = false; }
                                }
                            }
                            return minV;
                        }
                        if (lower == "max") {
                            Value maxV; bool first = true;
                            for (const auto& r : groupRows) {
                                if (static_cast<size_t>(idx) < r.size() && !r[idx].isNull()) {
                                    if (first || r[idx] > maxV) { maxV = r[idx]; first = false; }
                                }
                            }
                            return maxV;
                        }
                    }
                }
            }
            return Value();
        }
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        Value left = evaluateGroupExprHelper(bin->left, groupRows, colNames);
        Value right = evaluateGroupExprHelper(bin->right, groupRows, colNames);

        if (bin->op == "and") return Value(left.asBool() && right.asBool());
        if (bin->op == "or")  return Value(left.asBool() || right.asBool());
        if (bin->op == "==" || bin->op == "=")  return Value(left == right);
        if (bin->op == "!=" || bin->op == "<>") return Value(left != right);
        if (bin->op == "<")  return Value(left < right);
        if (bin->op == ">")  return Value(left > right);
        if (bin->op == "<=") return Value(left <= right);
        if (bin->op == ">=") return Value(left >= right);
        if (bin->op == "+")  return left + right;
        if (bin->op == "-")  return left - right;
        if (bin->op == "*")  return left * right;
        if (bin->op == "/")  return left / right;
        if (bin->op == "%")  return left % right;
    }

    if (auto un = std::dynamic_pointer_cast<UnaryExpr>(expr)) {
        Value val = evaluateGroupExprHelper(un->operand, groupRows, colNames);
        if (un->op == "-") return -val;
        if (un->op == "not" || un->op == "!") return Value(!val.asBool());
    }

    if (auto lit = std::dynamic_pointer_cast<LiteralExpr>(expr))
        return lit->value;

    // Fall back to evaluating against first row of group
    if (!groupRows.empty()) {
        // Simple column lookup
        if (auto col = std::dynamic_pointer_cast<ColumnExpr>(expr)) {
            std::string name = col->fullName();
            for (size_t i = 0; i < colNames.size(); i++) {
                if (colNames[i] == name && i < groupRows[0].size())
                    return groupRows[0][i];
                if (colNames[i].find('.') != std::string::npos &&
                    colNames[i].substr(colNames[i].find('.') + 1) == name &&
                    i < groupRows[0].size())
                    return groupRows[0][i];
            }
        }
    }
    return Value();
}

// ---------------------------------------------------------------------------
// JOIN helper
// ---------------------------------------------------------------------------

QueryResult Executor::performJoin(const Table& leftTable, const Table& rightTable,
                                  const ExprPtr& onCondition, const std::string& joinType) {
    // For pipeline JOINs, leftTable is actually a QueryResult passed as reference.
    // We overload by working with the QueryResult version in applyPipelineStage.
    // This version works with two Table objects.
    QueryResult result;

    // Build column names
    std::vector<std::string> leftCols;
    for (const auto& c : leftTable.getColumns())
        leftCols.push_back(leftTable.getName() + "." + c.name);

    std::vector<std::string> rightCols;
    for (const auto& c : rightTable.getColumns())
        rightCols.push_back(rightTable.getName() + "." + c.name);

    result.columnNames.insert(result.columnNames.end(), leftCols.begin(), leftCols.end());
    result.columnNames.insert(result.columnNames.end(), rightCols.begin(), rightCols.end());

    const auto& leftRows = leftTable.getRows();
    const auto& rightRows = rightTable.getRows();

    if (joinType == "cross") {
        for (const auto& lr : leftRows) {
            for (const auto& rr : rightRows) {
                Row combined = lr;
                combined.insert(combined.end(), rr.begin(), rr.end());
                result.rows.push_back(combined);
            }
        }
    } else if (joinType == "inner") {
        for (const auto& lr : leftRows) {
            for (const auto& rr : rightRows) {
                Row combined = lr;
                combined.insert(combined.end(), rr.begin(), rr.end());
                Value cond = evaluate(onCondition, combined, result.columnNames);
                if (cond.asBool())
                    result.rows.push_back(combined);
            }
        }
    } else if (joinType == "left") {
        for (const auto& lr : leftRows) {
            bool matched = false;
            for (const auto& rr : rightRows) {
                Row combined = lr;
                combined.insert(combined.end(), rr.begin(), rr.end());
                Value cond = evaluate(onCondition, combined, result.columnNames);
                if (cond.asBool()) {
                    result.rows.push_back(combined);
                    matched = true;
                }
            }
            if (!matched) {
                Row combined = lr;
                for (size_t i = 0; i < rightCols.size(); i++)
                    combined.push_back(Value());
                result.rows.push_back(combined);
            }
        }
    } else if (joinType == "right") {
        for (const auto& rr : rightRows) {
            bool matched = false;
            for (const auto& lr : leftRows) {
                Row combined = lr;
                combined.insert(combined.end(), rr.begin(), rr.end());
                Value cond = evaluate(onCondition, combined, result.columnNames);
                if (cond.asBool()) {
                    result.rows.push_back(combined);
                    matched = true;
                }
            }
            if (!matched) {
                Row combined;
                for (size_t i = 0; i < leftCols.size(); i++)
                    combined.push_back(Value());
                combined.insert(combined.end(), rr.begin(), rr.end());
                result.rows.push_back(combined);
            }
        }
    }

    return result;
}

} // namespace epee
