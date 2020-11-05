main: main.cpp *.h
	clang++-7  main.cpp -o main -pthread -std=c++17
make clean:
	rm -f main