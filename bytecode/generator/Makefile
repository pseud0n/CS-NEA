CODE_FILES := $(shell find -regextype posix-egrep -regex ".*/[a-z_]*[a-z]\.(cpp|h)")
# All nested header files (.h) and source files (.cpp)

#all: parser.out

FLEX_PATH = $(shell dirname $(shell which flex))
LIB_PATH = $(shell readlink -f "$(FLEX_PATH)/../lib")

../../output/parser.out: calc.tab.h lex.yy.c ../../output/operators.out ../instructions_enum.h dir_macros.h Makefile
	@echo "Flex:" $(LIB_PATH)
	@echo "Flex:" $(shell ls $(LIB_PATH))
	clang -o ../../output/parser.out calc.tab.c lex.yy.c -L $(LIB_PATH) -lfl -lm -Wall -g \

lex.yy.c: calc.l calc.tab.h
	flex calc.l

calc.tab.h: calc.y
	bison calc.y -d

../../output/operators.out: ../operators.cpp
	clang++ -o ../../output/operators.out ../operators.cpp -std=c++17 -Wall -Wpedantic -Wextra

make clean:
	rm -fr calc.output calc.tab.c calc.tab.h lex.yy.c bytecode-files/*
