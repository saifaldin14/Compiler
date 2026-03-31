# Epee -- The Pipeline-First Database Query Language

Originally created by Saif Al-Din Ali as a compiler for a pseudocode-style
language.  Enhanced with a full-featured, pipeline-first database query engine,
an interactive REPL, and a growing library of built-in functions.

---

## Table of Contents

1. [Overview](#overview)
2. [Building and Running](#building-and-running)
3. [Pipeline Queries](#pipeline-queries)
4. [Data Types](#data-types)
5. [Schema Operations](#schema-operations)
6. [Data Manipulation](#data-manipulation)
7. [Pipeline Stages Reference](#pipeline-stages-reference)
8. [Expressions and Operators](#expressions-and-operators)
9. [CASE / WHEN Expressions](#case--when-expressions)
10. [Aggregate Functions](#aggregate-functions)
11. [Scalar Functions](#scalar-functions)
12. [Predicate Expressions](#predicate-expressions)
13. [Joins](#joins)
14. [Grouping and Aggregation](#grouping-and-aggregation)
15. [Transactions](#transactions)
16. [Variables and Control Flow](#variables-and-control-flow)
17. [User-Defined Functions](#user-defined-functions)
18. [Interactive REPL](#interactive-repl)
19. [Legacy Compiler Mode](#legacy-compiler-mode)
20. [Project Structure](#project-structure)

---

## Overview

Epee is a database query language built around the pipeline operator `|>`.
Queries are written as a left-to-right chain of transformations instead of
the nested, inside-out structure found in SQL.  The engine runs entirely
in-memory, supports transactions, and includes an interactive REPL for
exploratory work.

### SQL versus Epee

Traditional SQL:

```sql
SELECT name, salary * 12 AS annual_pay
  FROM employees
 WHERE department = 'Engineering' AND salary > 80000
 ORDER BY annual_pay DESC
 LIMIT 5;
```

The same query in Epee:

```
employees
    |> where(department == "Engineering" and salary > 80000.0)
    |> select(name, salary * 12 as annual_pay)
    |> orderby(annual_pay desc)
    |> limit(5)
    |> print;
```

Each stage reads left to right: start with the `employees` table, keep rows
matching the condition, project two columns, sort them, take the first five,
and print the result.

---

## Building and Running

Requirements: a C++17 compiler (g++ or clang++) and GNU Make.

```
make            # build the binary
make clean      # remove object files and the binary
make test       # run all built-in test suites
```

Three execution modes:

```
./epee                      # start the interactive REPL
./epee queries.ep           # run a query file
./epee --compile program.ep # run the legacy compiler pipeline
```

---

## Pipeline Queries

A pipeline begins with a table name followed by one or more stages separated
by `|>`.  Each stage transforms the result set from the previous stage.

```
table_name
    |> stage1(...)
    |> stage2(...)
    |> ...
    |> print;
```

The final `print` stage outputs the result as a formatted ASCII table.  A
pipeline that does not end with `print` still executes but produces no visible
output (useful for `update` and `delete` pipelines).

Stages can be written on a single line or across multiple lines for
readability:

```
employees |> where(salary > 50000.0) |> select(name, salary) |> print;
```

---

## Data Types

| Type     | Literal examples          | Notes                              |
|----------|---------------------------|------------------------------------|
| `int`    | `42`, `-7`, `0`           | 32-bit signed integer              |
| `double` | `3.14`, `-0.5`, `1e6`     | 64-bit floating point              |
| `string` | `"hello"`, `"it's fine"`  | Double-quoted, supports `\"`, `\\` |
| `bool`   | `true`, `false`           |                                    |
| `null`   | `null`                    | The absence of a value             |

---

## Schema Operations

### CREATE TABLE

```
create table employees (
    id      int primary key,
    name    string not null,
    email   string unique,
    salary  double,
    active  bool
);
```

Column constraints: `primary key`, `unique`, `not null`.

### DROP TABLE

```
drop table employees;
```

### SHOW TABLES

```
show tables;
```

Displays every table with its column count and row count.

### DESCRIBE

```
describe employees;
```

Shows the column names, types, nullability, primary key, and uniqueness
for the named table.

---

## Data Manipulation

### INSERT

```
insert into employees values (1, "Alice", "alice@co.com", 95000.0, true);
```

Multiple rows:

```
insert into employees values
    (2, "Bob",   "bob@co.com",   78000.0, true),
    (3, "Carol", "carol@co.com", 62000.0, false);
```

Named columns:

```
insert into employees (id, name, salary) values (4, "Dave", 55000.0);
```

### SELECT (SQL-style)

```
select name, salary from employees where salary > 70000.0 orderby salary desc limit 5;
```

```
select distinct department from employees;
```

```
select count(*), avg(salary) from employees;
```

### UPDATE (SQL-style)

```
update employees set salary = salary * 1.1 where active == true;
```

### DELETE (SQL-style)

```
delete from employees where active == false;
```

### Pipeline equivalents

The same operations expressed as pipelines:

```
// SELECT
employees
    |> where(salary > 70000.0)
    |> select(name, salary)
    |> orderby(salary desc)
    |> take(5)
    |> print;

// UPDATE
employees
    |> where(active == true)
    |> update(salary = salary * 1.1);

// DELETE
employees
    |> where(active == false)
    |> delete;
```

---

## Pipeline Stages Reference

### Complete list of pipeline stages

| Stage                      | Description                                      |
|----------------------------|--------------------------------------------------|
| `where(condition)`         | Keep rows satisfying the condition                |
| `select(columns...)`       | Project or compute columns (replaces column set)  |
| `map(columns...)`          | Add computed columns to existing column set       |
| `orderby(col asc/desc)`    | Sort by one or more columns                       |
| `limit(n)` / `take(n)`     | Return at most n rows                             |
| `offset(n)` / `skip(n)`    | Skip the first n rows                             |
| `groupby(columns...)`      | Group rows (combine with a following `select`)    |
| `having(condition)`        | Filter groups after aggregation                   |
| `join(table on condition)` | Inner-join another table                          |
| `distinct`                 | Remove duplicate rows                             |
| `count`                    | Replace the result with a single count row        |
| `update(col = expr, ...)`  | Modify matching rows in the underlying table      |
| `delete`                   | Remove matching rows from the underlying table    |
| `print`                    | Output the result as a formatted table            |

### select versus map

`select` replaces the column set:

```
products |> select(name, price) |> print;
// result has exactly two columns: name, price
```

`map` appends new computed columns to the existing set:

```
products |> map(price * 1.1 as new_price) |> print;
// result has all original columns plus new_price
```

### take and skip

`take(n)` and `skip(n)` are aliases for `limit(n)` and `offset(n)`.

```
employees |> orderby(salary desc) |> skip(10) |> take(5) |> print;
```

---

## Expressions and Operators

### Arithmetic

| Operator | Meaning        | Example            |
|----------|----------------|--------------------|
| `+`      | Addition       | `price + tax`      |
| `-`      | Subtraction    | `total - discount` |
| `*`      | Multiplication | `qty * price`      |
| `/`      | Division       | `total / count`    |
| `%`      | Modulo         | `id % 2`           |

String concatenation: `"hello" + " world"` produces `"hello world"`.

### Comparison

| Operator     | Meaning                |
|--------------|------------------------|
| `==`         | Equal                  |
| `<>` or `!=` | Not equal              |
| `<`          | Less than              |
| `>`          | Greater than           |
| `<=`         | Less than or equal     |
| `>=`         | Greater than or equal  |

### Logical

| Operator | Meaning                       |
|----------|-------------------------------|
| `and`    | Logical AND (short-circuit)   |
| `or`     | Logical OR (short-circuit)    |
| `not`    | Logical negation              |

---

## CASE / WHEN Expressions

Conditional expressions with multiple branches:

```
select
    name,
    case
        when salary > 100000.0 then "Senior"
        when salary > 60000.0  then "Mid"
        else "Junior"
    end as level
from employees;
```

Inside a pipeline:

```
employees
    |> select(
        name,
        case
            when salary > 100000.0 then "Senior"
            when salary > 60000.0  then "Mid"
            else "Junior"
        end as level
    )
    |> print;
```

The `else` clause is optional.  If no `when` branch matches and there is no
`else`, the expression evaluates to `null`.

---

## Aggregate Functions

These functions operate over groups of rows (or the entire result set when
there is no `groupby`).

| Function     | Description                     |
|--------------|---------------------------------|
| `count(*)`   | Number of rows                  |
| `count(col)` | Number of non-null values       |
| `sum(col)`   | Sum of numeric values           |
| `avg(col)`   | Average of numeric values       |
| `min(col)`   | Minimum value                   |
| `max(col)`   | Maximum value                   |

Example:

```
orders
    |> groupby(region)
    |> select(region, count(*) as total, avg(amount) as avg_amount)
    |> orderby(total desc)
    |> print;
```

---

## Scalar Functions

### String functions

| Function                                       | Description                           |
|------------------------------------------------|---------------------------------------|
| `upper(s)`                                     | Convert to uppercase                  |
| `lower(s)`                                     | Convert to lowercase                  |
| `length(s)`                                    | Number of characters                  |
| `substr(s, start)` or `substr(s, start, len)`  | Extract substring                     |
| `left(s, n)`                                   | First n characters                    |
| `right(s, n)`                                  | Last n characters                     |
| `trim(s)`                                      | Remove leading/trailing whitespace    |
| `lpad(s, len, pad)`                            | Left-pad to target length             |
| `rpad(s, len, pad)`                            | Right-pad to target length            |
| `replace(s, old, new)`                         | Replace all occurrences               |
| `reverse(s)`                                   | Reverse the string                    |
| `repeat(s, n)`                                 | Repeat the string n times             |
| `concat(a, b, ...)`                            | Concatenate values                    |

Example:

```
employees
    |> select(
        name,
        upper(name) as shouting,
        left(name, 1) as initial,
        lpad(name, 12, ".") as padded,
        reverse(name) as backwards
    )
    |> take(5)
    |> print;
```

### Math functions

| Function                    | Description                              |
|-----------------------------|------------------------------------------|
| `abs(n)`                    | Absolute value                           |
| `round(n)`                  | Round to nearest integer                 |
| `floor(n)`                  | Round down                               |
| `ceil(n)`                   | Round up                                 |
| `power(base, exp)`          | Raise base to the power of exp           |
| `sqrt(n)`                   | Square root                              |
| `log(n)` or `log(base, n)`  | Natural log, or log with custom base     |
| `pi()`                      | The constant 3.14159...                  |
| `random()`                  | Random double in the range [0.0, 1.0)    |

Example:

```
products
    |> select(name, price, round(sqrt(price)) as root, power(price, 2) as squared)
    |> take(5)
    |> print;
```

### Utility functions

| Function              | Description                                        |
|-----------------------|----------------------------------------------------|
| `coalesce(a, b, ...)` | Return the first non-null argument                 |
| `nullif(a, b)`        | Return null if a equals b, otherwise return a      |
| `typeof(expr)`        | Return the type name as a string                   |
| `cast(expr, "type")`  | Convert to the given type (int/double/string/bool) |
| `iif(cond, t, f)`     | Inline if: return t when cond is true, else f      |
| `now()`               | Current timestamp as a string                      |

Example:

```
contacts
    |> select(name, coalesce(phone, email, "N/A") as contact)
    |> print;

inventory
    |> select(
        product,
        quantity,
        iif(quantity > 100, "In Stock", "Low Stock") as availability,
        typeof(price) as price_type
    )
    |> print;
```

---

## Predicate Expressions

### BETWEEN / NOT BETWEEN

```
products |> where(price between 10.0 and 100.0) |> print;
products |> where(price not between 10.0 and 100.0) |> print;
```

### IN / NOT IN

```
products |> where(category in ("Electronics", "Books")) |> print;
products |> where(category not in ("Electronics", "Books")) |> print;
```

### LIKE / NOT LIKE

Pattern matching with `%` (any sequence of characters) and `_` (single
character).  Matching is case-insensitive.

```
products |> where(name like "Laptop%") |> print;
products |> where(name not like "%Pro%") |> print;
```

### IS NULL / IS NOT NULL

```
products |> where(description is null) |> print;
products |> where(description is not null) |> print;
```

---

## Joins

Four join types are supported.

### SQL-style joins

```
select employees.name, departments.name
    from employees
    inner join departments on employees.dept_id == departments.id;

select employees.name, departments.name
    from employees
    left join departments on employees.dept_id == departments.id;
```

Join types: `inner join`, `left join`, `right join`, `cross join`.

### Pipeline joins

```
employees
    |> join(departments on employees.dept_id == departments.id)
    |> select(employees.name, departments.name, employees.salary)
    |> orderby(employees.salary desc)
    |> print;
```

The pipeline join defaults to inner join.

---

## Grouping and Aggregation

Group rows and compute aggregates per group:

```
orders
    |> groupby(region)
    |> select(region, count(*) as num, sum(total) as revenue)
    |> orderby(revenue desc)
    |> print;
```

Use CASE inside a pipeline for dynamic bucketing:

```
products
    |> select(
        case
            when price > 500.0  then "Premium"
            when price > 100.0  then "Mid-range"
            else "Budget"
        end as tier,
        price
    )
    |> groupby(tier)
    |> select(tier, count(*) as items, avg(price) as avg_price)
    |> print;
```

SQL-style syntax:

```
select dept_id, count(*) as cnt, avg(salary) as avg_sal
    from employees
    groupby dept_id;
```

---

## Transactions

Wrap a group of operations in a transaction.  Use `rollback` to undo changes
if something goes wrong.

```
begin;
    products |> where(category == "Electronics") |> update(price = price * 0.9);
commit;
```

```
begin;
    // something went wrong
rollback;
```

---

## Variables and Control Flow

### Variable declaration and assignment

```
int count;
count = 42;
print count;

string greeting;
greeting = "hello";
print greeting;
```

Supported types: `int`, `double`, `string`, `bool`.

### If / else

```
int x;
x = 10;
if x > 5 then
    print "big";
else
    print "small";
fi;
```

### While loops

```
int i;
i = 0;
while i < 5 do
    print i;
    i = i + 1;
od;
```

---

## User-Defined Functions

Define reusable functions with typed parameters and return values.
Recursion is supported.

```
def int factorial(int n)
    if n <= 1 then
        return 1;
    fi;
    return n * factorial(n - 1);
fed;

print factorial(5);
```

Functions can be called inside pipeline stages:

```
def double discount(double price, double pct)
    return price * (1.0 - pct / 100.0);
fed;

products
    |> select(name, price, discount(price, 15.0) as sale_price)
    |> print;
```

---

## Interactive REPL

Start the REPL with `./epee` (no arguments).

```
$ ./epee
    Epee v2.0 -- Pipeline-First Database Query Language
    Type 'help' for commands, 'exit' to quit.

epee> create table t (id int, name string);
Table 't' created.

epee> insert into t values (1, "Alice"), (2, "Bob");
Inserted 2 row(s).

epee> t |> print;
+----+-------+
| id | name  |
+----+-------+
|  1 | Alice |
|  2 | Bob   |
+----+-------+
2 row(s)

epee> exit
```

Commands: `help`, `exit`, `quit`.

Multi-line input is supported.  The REPL accumulates lines until a semicolon
is found.

---

## Comments

```
// This is a line comment

/* This is a
   block comment */
```

---

## Legacy Compiler Mode

The original compiler pipeline processes a pseudocode language through lexical
analysis, syntax tree construction, semantic analysis, and three-address code
generation.  Activate it with:

```
./epee --compile program.ep
```

This mode is independent of the database engine.

---

## Project Structure

```
Compiler/
  include/
    database/          -- database engine headers
      value.hpp        -- variant value type (int/double/string/bool/null)
      table.hpp        -- table, row, database, query result
      dbLexer.hpp      -- lexer token types and scanner
      dbParser.hpp     -- AST node types and recursive descent parser
      executor.hpp     -- query executor
      repl.hpp         -- interactive REPL
    lexicalAnalysis/   -- legacy compiler lexer
    syntaxAnalysis/    -- legacy compiler parser
    semanticAnalysis/  -- legacy compiler semantic analyzer
    intermediateCode/  -- three-address code generator
    tokens/            -- token type definitions
  src/
    main.cpp           -- entry point (REPL / file / legacy mode)
    database/          -- database engine implementation
    lexicalAnalysis/   -- legacy compiler implementation
    syntaxAnalysis/
    semanticAnalysis/
    intermediateCode/
    tokens/
  input/
    TestDB1.ep         -- basic table operations
    TestDB2.ep         -- pipeline queries
    TestDB3.ep         -- joins and aggregations
    TestDB4.ep         -- transactions, LIKE, BETWEEN, variables
    TestDB5.ep         -- CASE/WHEN, new functions, map/take/skip
    Test1-11.ep        -- legacy compiler tests
Makefile               -- build rules
```

---

## Running Tests

```
make test
```

This executes `TestDB1.ep` through `TestDB5.ep`, covering table operations,
pipeline queries, joins, aggregations, transactions, new functions, and CASE
expressions.

---

## Quick Reference

```
-- Schema
create table T (col type [constraints], ...);
drop table T;
show tables;
describe T;

-- Insert
insert into T values (...);

-- Query (SQL-style)
select cols from T where cond orderby col desc limit n;

-- Query (pipeline)
T |> where(cond) |> select(cols) |> orderby(col desc) |> take(n) |> print;

-- Computed columns
T |> map(price * 1.1 as new_price) |> print;

-- Grouping
T |> groupby(col) |> select(col, count(*) as n, avg(val) as mean) |> print;

-- CASE expression
T |> select(case when x > 0 then "pos" else "neg" end as sign) |> print;

-- Update / Delete
T |> where(cond) |> update(col = expr);
T |> where(cond) |> delete;

-- Transactions
begin; ... commit;

-- Variables and control flow
int x; x = 10;
if x > 5 then print x; fi;
while x > 0 do x = x - 1; od;

-- Functions
def int f(int n) return n * 2; fed;
```
