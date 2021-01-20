HEADERS := $(shell find -regextype posix-egrep -regex ".*/[a-z_]*[a-z]\.(cpp|h)")
# All nested header files (.h) and source files (.cpp)

all: a.out

a.out: main.cpp $(HEADERS) Makefile
	clang++-8 main.cpp -o a.out -pthread -std=gnu++2a -g -Wall -Wpedantic -Wextra \
	-D PLUMBER_DEBUG -D DO_CACHE_DECL
# Define: -D PLUMBER_DEBUG -D DO_CACHE_DECL -D TRACKER_DEBUG 
make clean:
	rm -f a.out