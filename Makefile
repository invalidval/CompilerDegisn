CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -Ibuild

SRC := src/main.cpp src/ast.cpp src/semantic_annotator.cpp src/code_generator.cpp src/symbol_table.cpp src/error_handler.cpp src/codegen_utils.cpp
GEN := build/parser.cpp build/lexer.cpp

TARGET := pascc

.PHONY: all clean test

all: build $(TARGET)

build:
	@mkdir -p build
	bison -d -o build/parser.cpp src/parser.y
	flex -o build/lexer.cpp src/lexer.l

$(TARGET): $(SRC) $(GEN)
	$(CXX) $(CXXFLAGS) $(SRC) $(GEN) -o $(TARGET)

clean:
	rm -rf build $(TARGET)

test: $(TARGET)
	./$(TARGET) --help
