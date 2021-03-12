CODE_FILES := $(shell find . -regextype posix-egrep -regex ".*/[a-z_]*[a-z]\.(cpp|h)")

EXCLUDE = ./bytecode/operators.cpp $(shell find ./bytecode/generator -type f)

CODE_FILES_FILTERED := $(filter-out $(EXCLUDE), $(CODE_FILES))
# All nested header files (.h) and source files (.cpp)
# Excludes parser files & operators.cpp

CPPFLAGS = -std=c++17 -g -Wall -Wpedantic -Wextra -pthread

MAIN_DEFS = -D DO_CACHE_DECL -D SHOW_BC #-D SHOW_CLOG
# Possible defines: -D PLUMBER_DEBUG -D DO_CACHE_DECL -D TRACKER_DEBUG -D SHOW_CLOG -D SHOW_BC

all: ./output/program.out ./output/operators.out ./output/parser.out

./output/program.out: $(CODE_FILES_FILTERED) Makefile # ./object_files/main_setup.o # ./object_files/lookup_setup.o
	clang++ main.cpp -o ./output/program.out $(CPPFLAGS) $(MAIN_DEFS)

./output/parser.out ./output/operators.out: FORCE
	make -C ./bytecode/generator

FORCE: ;

make clean:
	rm -f ./output/*.out
	output/clear-files.sh
	make -C ./bytecode/generator clean
	rm -f object_files/*
