```
     ██████╗ ██████╗ ██████╗ ██████╗
    ██╔════╝ ██╔══██╗██╔════╝██╔════╝
    █████╗   ██████╔╝█████╗  █████╗
    ██╔══╝   ██╔═══╝ ██╔══╝  ██╔══╝
    ███████╗ ██║     ███████╗███████╗
    ╚══════╝ ╚═╝     ╚══════╝╚══════╝
    The Pipeline-First Database Query Language
```

# Épée — The Pipeline-First Database Query Language

> *Originally created by **Saif Al-Din Ali** as a compiler for a pseudocode-style language. Enhanced with a full-featured, pipeline-first database query engine.*

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## What is Épée?

**Épée** is a database query language built from the ground up around the **pipeline operator `|>`**. Instead of writing nested, inside-out SQL, Épée lets you compose queries as a left-to-right chain of transformations — the way you actually *think* about data.

### SQL vs Épée — See the Difference

**Traditional SQL:**
```sql
SELECT name, salary * 12 AS annual_pay
  FROM employees
 WHERE department = 'Engineering' AND salary > 80000
 ORDER BY annual_pay DESC
 LIMIT 5;
```

**Épée:**
```
employees
    |> where(department == "Engineering" and salary > 80000.0)
    |> select(name, salary * 12 as annual_pay)
    |> orderby(annual_pay desc)
    |> limit(5)
    |> print;
```

Each stage feeds its output into the next. No nesting. No subqueries. Just a clean, readable pipeline.

---

## Table of Contents

- [Quick Start](#quick-start)
- [The Pipeline Operator](#the-pipeline-operator)
- [Data Types](#data-types)
- [DDL — Defining Your Schema](#ddl--defining-your-schema)
- [DML — Manipulating Data](#dml--manipulating-data)
- [Pipeline Stages](#pipeline-stages)
- [JOINs](#joins)
- [Aggregate Functions](#aggregate-functions)
- [String Functions](#string-functions)
- [Computed Columns & Aliases](#computed-columns--aliases)
- [Predicates](#predicates)
- [Transactions](#transactions)
- [Imperative Features](#imperative-features)
- [Comments](#comments)
- [Interactive REPL](#interactive-repl)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Legacy Compiler](#legacy-compiler)
- [Running Tests](#running-tests)
- [Author & Credits](#author--credits)

---

## Quick Start

### Build

```bash
make            # Compile everything with g++ (C++17)
```

### Run

```bash
./epee                        # Launch interactive REPL
./epee queries.ep             # Execute a .ep query file
./epee --compile program.ep   # Legacy compiler mode (lexer → parser → semantic → TAC)
./epee --help                 # Show usage information
```

---

## The Pipeline Operator

The **`|>`** operator is Épée's defining feature. It takes the result on the left and passes it as input to the stage on the right, creating a fluent chain of data transformations.

```
table_name
    |> stage1(...)
    |> stage2(...)
    |> stage3(...)
    |> print;
```

### Why Pipelines?

| Aspect | SQL | Épée Pipelines |
|---|---|---|
| Read order | Inside-out, bottom-up | Left-to-right, top-to-bottom |
| Composability | Subqueries, CTEs | Just add another `\|>` stage |
| Mental model | Declarative set algebra | Sequential data flow |

### A Real-World Example

Find the top 5 highest-paid engineers, compute their annual salary and bonus, and display the results:

```
employees
    |> where(department == "Engineering" and salary > 80000.0)
    |> select(name, title, salary, salary * 12 as annual_salary, salary * 0.1 as bonus)
    |> orderby(annual_salary desc)
    |> limit(5)
    |> print;
```

Output is a pretty-printed ASCII table:

```
+----------+------------+----------+---------------+---------+
| name     | title      | salary   | annual_salary | bonus   |
+----------+------------+----------+---------------+---------+
| Charlie  | Lead Dev   | 110000.0 | 1320000.0     | 11000.0 |
| Alice    | Senior Dev | 95000.0  | 1140000.0     | 9500.0  |
| Ivy      | Senior Dev | 92000.0  | 1104000.0     | 9200.0  |
| Grace    | Mid Dev    | 85000.0  | 1020000.0     | 8500.0  |
+----------+------------+----------+---------------+---------+
```

---

## Data Types

| Type | Description | Example |
|---|---|---|
| `int` | Integer numbers | `42`, `-7`, `0` |
| `double` | Floating-point numbers | `3.14`, `99000.0` |
| `string` | Text (double-quoted) | `"hello world"` |
| `bool` | Boolean values | `true`, `false` |
| `null` | Null / missing value | `null` |

---

## DDL — Defining Your Schema

### CREATE TABLE

```
create table employees (
    id int,
    name string,
    department string,
    salary double,
    active bool
);
```

Column constraints including `PRIMARY KEY`, `UNIQUE`, and `NOT NULL` are supported.

### DROP TABLE

```
drop table employees;
```

### SHOW TABLES

```
show tables;
```

### DESCRIBE

```
describe employees;
```

Displays the column names and their types for the given table.

---

## DML — Manipulating Data

### INSERT

```
insert into employees values (1, "Alice", "Engineering", 95000.0, true);
insert into employees values (2, "Bob", "Marketing", 78000.0, true);
```

### SELECT (SQL-style)

```
// All rows
select * from employees;

// Specific columns with filtering, sorting, and pagination
select name, salary from employees
    where salary > 80000.0
    orderby salary desc
    limit 5
    offset 2;

// Distinct values
select distinct department from employees;

// Aggregation with GROUP BY and HAVING
select department, count(*) as headcount, avg(salary) as avg_sal
    from employees
    groupby department;
```

### UPDATE (pipeline-style)

```
employees |> where(department == "Engineering") |> update(salary = salary * 1.1);
```

### DELETE (pipeline-style)

```
employees |> where(active == false) |> delete;
```

---

## Pipeline Stages

Every stage receives a table as input and produces a table as output.

| Stage | Description | Example |
|---|---|---|
| `where(cond)` | Filter rows by condition | `where(salary > 80000.0)` |
| `select(cols)` | Project columns, compute expressions | `select(name, salary * 12 as annual)` |
| `orderby(col dir)` | Sort results (`asc` or `desc`) | `orderby(salary desc)` |
| `limit(n)` | Return at most *n* rows | `limit(10)` |
| `offset(n)` | Skip the first *n* rows | `offset(5)` |
| `groupby(cols)` | Group rows by column(s) | `groupby(department)` |
| `having(cond)` | Filter groups after aggregation | `having(count(*) > 3)` |
| `join(...)` | Join with another table | `join(departments on dept_id == departments.id)` |
| `update(assignments)` | Update matching rows | `update(price = price * 0.9)` |
| `delete` | Delete matching rows | *(no arguments)* |
| `distinct` | Remove duplicate rows | *(no arguments)* |
| `count` | Count rows and print the result | *(no arguments)* |
| `print` | Output as formatted ASCII table | *(no arguments)* |

### Chaining Example

```
products
    |> where(category == "Electronics")
    |> select(name, price, price * 0.9 as sale_price)
    |> orderby(sale_price asc)
    |> limit(10)
    |> print;
```

---

## JOINs

Épée supports four join types, usable in both SQL-style and pipeline syntax.

| Join Type | Description |
|---|---|
| `INNER JOIN` | Rows matching in both tables |
| `LEFT JOIN` | All rows from left + matches from right |
| `RIGHT JOIN` | All rows from right + matches from left |
| `CROSS JOIN` | Cartesian product of both tables |

### SQL-style JOIN

```
select employees.name, departments.name, employees.salary
    from employees
    inner join departments on employees.dept_id == departments.id;
```

### Pipeline-style JOIN

```
employees
    |> join(departments on employees.dept_id == departments.id)
    |> select(employees.name, departments.name, employees.salary)
    |> orderby(employees.salary desc)
    |> print;
```

---

## Aggregate Functions

Use these in `select` or `having` clauses, typically combined with `groupby`.

| Function | Description |
|---|---|
| `count(*)` | Count all rows |
| `count(col)` | Count non-null values in a column |
| `sum(col)` | Sum of a numeric column |
| `avg(col)` | Average of a numeric column |
| `min(col)` | Minimum value |
| `max(col)` | Maximum value |

### Example: Department Statistics

```
employees
    |> groupby(department)
    |> select(department, count(*) as headcount, avg(salary) as avg_sal, sum(salary) as total_sal)
    |> orderby(total_sal desc)
    |> print;
```

---

## String Functions

| Function | Description | Example |
|---|---|---|
| `upper(s)` | Convert to uppercase | `upper(name)` |
| `lower(s)` | Convert to lowercase | `lower(department)` |
| `length(s)` | String length | `length(name)` |
| `substr(s, start, len)` | Extract substring | `substr(name, 1, 3)` |
| `concat(a, b)` | Concatenate strings | `concat(first, last)` |
| `trim(s)` | Remove leading/trailing whitespace | `trim(name)` |
| `replace(s, old, new)` | Replace occurrences | `replace(name, "old", "new")` |

### Example

```
employees
    |> select(upper(name) as upper_name, lower(department) as dept, length(name) as name_len)
    |> orderby(name_len desc)
    |> limit(5)
    |> print;
```

---

## Computed Columns & Aliases

Create derived columns with expressions and name them with `as`:

```
employees
    |> select(name, salary, salary * 12 as annual_pay, salary * 0.1 as bonus)
    |> orderby(annual_pay desc)
    |> limit(5)
    |> print;
```

Computed columns support arithmetic (`+`, `-`, `*`, `/`), string functions, and aggregate functions.

---

## Predicates

### Comparison Operators

`==`, `!=`, `>`, `<`, `>=`, `<=`

### Logical Operators

`and`, `or`, `not`

### LIKE — Pattern Matching

The `%` wildcard matches any sequence of characters:

```
products |> where(name like "%Pro%") |> select(name, price) |> print;
```

### BETWEEN — Range Queries

```
products |> where(price between 50.0 and 200.0) |> select(name, price) |> print;
```

### IN — Value Lists

```
employees |> where(department in ("Engineering", "Sales")) |> print;
```

### IS NULL / IS NOT NULL

```
employees |> where(manager is null) |> print;
employees |> where(email is not null) |> print;
```

---

## Transactions

Wrap multiple operations in an atomic transaction:

```
begin;
    products |> where(category == "Electronics") |> update(price = price * 0.9);
    products |> where(in_stock == false) |> delete;
commit;
```

Discard changes with `rollback;`:

```
begin;
    products |> where(price < 10.0) |> delete;
rollback;  // Nothing was deleted
```

---

## Imperative Features

Épée retains the imperative features from its compiler roots. Mix general-purpose programming with database queries in the same file.

### Variables

```
int count;
double total;
string greeting;
bool is_done;

count = 42;
total = 99.5;
greeting = "Hello, Épée!";
is_done = false;
```

### Control Flow

```
if (count > 10) then
    print "Large count";
else
    print "Small count";
fi;
```

### While Loops

```
int i;
i = 0;
while (i < 5) do
    print i;
    i = i + 1;
done;
```

### Functions & Recursion

```
def int gcd(int a, int b)
    if (a == b) then
        return(a)
    fi;
    if (a > b) then
        return(gcd(a - b, b))
    else
        return(gcd(a, b - a))
    fi;
fed;

print gcd(21, 15);
```

---

## Comments

```
// This is a line comment

/* This is a
   block comment */
```

---

## Interactive REPL

Launch the REPL with no arguments:

```bash
./epee
```

```
  ╔══════════════════════════════════════════╗
  ║  Épée Database Query Language  v1.0     ║
  ║  Type 'help' for commands, 'exit' to    ║
  ║  quit.                                  ║
  ╚══════════════════════════════════════════╝

épée> create table demo (id int, name string);
Table 'demo' created.

épée> insert into demo values (1, "Alice");
1 row inserted.

épée> demo |> select(name) |> print;
+-------+
| name  |
+-------+
| Alice |
+-------+

épée> help
épée> exit
```

**REPL Commands:**
- `help` — Show available commands and syntax reference
- `exit` / `quit` — Exit the REPL
- Multi-line statements are supported (terminate with `;`)

---

## Architecture

Épée is powered by **two independent pipelines** sharing a single entry point:

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│                  (CLI & Mode Router)                    │
└──────────┬──────────────────────────────┬───────────────┘
           │                              │
     ┌─────▼──────┐               ┌──────▼──────┐
     │  Database   │               │   Legacy    │
     │   Engine    │               │  Compiler   │
     └─────┬──────┘               └──────┬──────┘
           │                              │
   ┌───────▼───────┐            ┌────────▼────────┐
   │   DbLexer     │            │     Lexer       │
   │  Tokenizer    │            │  (Tokenizer)    │
   └───────┬───────┘            └────────┬────────┘
   ┌───────▼───────┐            ┌────────▼────────┐
   │   DbParser    │            │     Parser      │
   │ (AST Builder) │            │ (Syntax Tree)   │
   └───────┬───────┘            └────────┬────────┘
   ┌───────▼───────┐            ┌────────▼────────┐
   │   Executor    │            │    Analyzer     │
   │ (Query Engine)│            │  (Semantics)    │
   └───────┬───────┘            └────────┬────────┘
   ┌───────▼───────┐            ┌────────▼────────┐
   │  In-Memory    │            │ ThreeAddress    │
   │   Storage     │            │    Code Gen     │
   └───────────────┘            └─────────────────┘
```

**Database Engine:** `DbLexer` → `DbParser` → `Executor` with an in-memory columnar store. Supports the full query language, pipeline operator, transactions, and REPL.

**Legacy Compiler:** `Lexer` → `Parser` → `Analyzer` → `ThreeAddressCode`. Compiles the original Épée pseudocode language into three-address intermediate code. Access via `--compile`.

---

## Project Structure

```
Compiler/
├── Makefile                    # Build system
├── README.md                   # This file
├── epee                        # Compiled binary
├── Compiler/
│   ├── include/
│   │   ├── database/           # Database engine headers
│   │   │   ├── dbLexer.hpp
│   │   │   ├── dbParser.hpp
│   │   │   ├── executor.hpp
│   │   │   ├── repl.hpp
│   │   │   ├── table.hpp
│   │   │   └── value.hpp
│   │   ├── lexicalAnalysis/    # Compiler lexer headers
│   │   ├── syntaxAnalysis/     # Compiler parser headers
│   │   ├── semanticAnalysis/   # Semantic analyzer headers
│   │   ├── intermediateCode/   # TAC generator headers
│   │   └── tokens/             # Token type definitions
│   ├── src/
│   │   ├── main.cpp            # Entry point & CLI router
│   │   ├── database/           # Database engine implementation
│   │   │   ├── dbLexer.cpp
│   │   │   ├── dbParser.cpp
│   │   │   ├── executor.cpp
│   │   │   ├── repl.cpp
│   │   │   ├── table.cpp
│   │   │   └── value.cpp
│   │   ├── lexicalAnalysis/    # Compiler lexer implementation
│   │   ├── syntaxAnalysis/     # Compiler parser implementation
│   │   ├── semanticAnalysis/   # Semantic analysis implementation
│   │   ├── intermediateCode/   # Code generation implementation
│   │   └── tokens/             # Token utilities
│   ├── input/                  # Test files (.ep)
│   │   ├── TestDB1.ep          # Basic table operations
│   │   ├── TestDB2.ep          # Pipeline queries
│   │   ├── TestDB3.ep          # Joins and aggregations
│   │   ├── TestDB4.ep          # Advanced features
│   │   └── Test1–11.ep         # Legacy compiler tests
│   └── output/                 # Generated output files
└── Compiler.xcodeproj/         # Xcode project (macOS)
```

---

## Legacy Compiler

The original Épée compiler is still accessible via `--compile`. It processes pseudocode-style programs through a full compilation pipeline:

```bash
./epee --compile Compiler/input/Test1.ep
```

**Pipeline:** Lexical Analysis → Syntax Analysis (FIRST/FOLLOW sets) → Semantic Analysis (scope, type checking) → Three-Address Code Generation

**Output:** Symbol table and intermediate code files.

```
def int gcd(int a, int b)
    if (a == b) then
        return(a)
    fi;
    if (a > b) then
        return(gcd(a - b, b))
    else
        return(gcd(a, b - a))
    fi;
fed;

print gcd(21, 15);
```

---

## Running Tests

```bash
make test    # Run all database test suites
```

This executes `TestDB1.ep` through `TestDB4.ep`, covering table operations, pipeline queries, joins, aggregations, transactions, and more.

---

## Author & Credits

**Original Author:** [Saif Al-Din Ali](https://github.com/saifali96)
- Designed and implemented the Épée language and compiler (lexer, parser, semantic analyzer, three-address code generator).

**Enhanced Version:** Extended with a pipeline-first database query engine featuring an interactive REPL, in-memory storage, JOINs, aggregations, transactions, string functions, and the signature `|>` pipeline operator.
