HEADERS := $(shell find -regextype sed -regex ".*/[a-z_]*[a-z]\.h")

all: a.out

a.out: main.cpp $(HEADERS) Makefile
	clang++-8 main.cpp -o a.out -pthread -std=gnu++2a -g -Wall -Wpedantic -Wextra \
	-D PLUMBER_DEBUG -D DO_CACHE_DECL -D PLUMBER_NO_SIZE_TRACK
#define: PLUMBER_DEBUG, DO_CACHE_DECL TRACKER_DEBUG
make clean:
	rm -f a.out