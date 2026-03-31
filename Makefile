CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -O2
INCLUDES = -I Compiler/include

# Source files for the original compiler
COMPILER_SRCS = \
    Compiler/src/tokens/tokenType.cpp \
    Compiler/src/tokens/number.cpp \
    Compiler/src/tokens/fileReader.cpp \
    Compiler/src/lexicalAnalysis/lexer.cpp \
    Compiler/src/lexicalAnalysis/transitionTableStates.cpp \
    Compiler/src/lexicalAnalysis/transitionTableNode.cpp \
    Compiler/src/lexicalAnalysis/transitionTableEntry.cpp \
    Compiler/src/syntaxAnalysis/parser.cpp \
    Compiler/src/syntaxAnalysis/syntaxTree.cpp \
    Compiler/src/syntaxAnalysis/syntaxTreeNode.cpp \
    Compiler/src/syntaxAnalysis/syntaxProp.cpp \
    Compiler/src/syntaxAnalysis/leaf.cpp \
    Compiler/src/semanticAnalysis/analyzer.cpp \
    Compiler/src/semanticAnalysis/scopeVariable.cpp \
    Compiler/src/intermediateCode/threeAddressCode.cpp \
    Compiler/src/intermediateCode/saveToken.cpp \
    Compiler/src/intermediateCode/compareToken.cpp

# Source files for the database engine
DB_SRCS = \
    Compiler/src/database/value.cpp \
    Compiler/src/database/table.cpp \
    Compiler/src/database/btree.cpp \
    Compiler/src/database/dbLexer.cpp \
    Compiler/src/database/dbParser.cpp \
    Compiler/src/database/executor.cpp \
    Compiler/src/database/repl.cpp \
    Compiler/src/database/storage.cpp \
    Compiler/src/database/wal.cpp \
    Compiler/src/database/security.cpp \
    Compiler/src/database/logger.cpp

# All source files
ALL_SRCS = $(COMPILER_SRCS) $(DB_SRCS) Compiler/src/main.cpp

# Object files
ALL_OBJS = $(ALL_SRCS:.cpp=.o)

# Target
TARGET = epee

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(ALL_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	find . -name '*.o' -delete
	rm -f $(TARGET)

test: $(TARGET)
	@echo "=== Running Database Tests ==="
	@echo "--- Test: Basic Table Operations ---"
	./$(TARGET) Compiler/input/TestDB1.ep
	@echo ""
	@echo "--- Test: Pipeline Queries ---"
	./$(TARGET) Compiler/input/TestDB2.ep
	@echo ""
	@echo "--- Test: Joins and Aggregations ---"
	./$(TARGET) Compiler/input/TestDB3.ep
	@echo ""
	@echo "--- Test: Advanced Features ---"
	./$(TARGET) Compiler/input/TestDB4.ep
	@echo ""
	@echo "--- Test: New Language Features ---"
	./$(TARGET) Compiler/input/TestDB5.ep
	@echo ""
	@echo "--- Test: Production Features (Persistence, Indexing, Security, Ops) ---"
	./$(TARGET) Compiler/input/TestDB6.ep
	@echo ""
	@echo "=== All tests complete ==="
