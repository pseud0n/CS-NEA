// 'Plumber' cos it finds leaks, geddit?

#ifndef STANDALONE_MEMORY_MANAGER_H
#define STANDALONE_MEMORY_MANAGER_H

#include "colours.h"

#include <stack>
#include <string>
#include <iostream>
#include <set>

using std::clog;
using std::cout;

class Plumber {
	friend std::ostream& operator <<(std::ostream&, const Plumber&);
private:
	static inline size_t memory_usage = 0;
	static std::set<void*> pointers;

public:
	static void add(size_t size, void* ptr) {
		memory_usage += size;
		//pointers.insert(ptr);
	}

	static void subtract(size_t size, void* ptr) {
		memory_usage -= size;
		//pointers.erase(ptr);
	}

	Plumber() {
	}
	

	~Plumber() {
		#ifdef PLUMBER_NO_SIZE_TRACK
		clog << "\nEXIT MEMORY NOT TRACKED - undefine 'PLUMBER_NO_SIZE_TRACK' and recompile\n";
		#else
		if (memory_usage)
			clog << BG_WARNING;
		cout << "\nEXIT MEMORY INCREASE: " << memory_usage << " bytes" FBG_DEFAULT "\n";
		#endif
	}
	
	static void get() {
		clog << "MEMORY USAGE: " << memory_usage << "\n";
	}
};

std::ostream& operator<<(std::ostream& stream, const Plumber&) {
	return stream << Plumber::memory_usage << " bytes";
}

#ifdef PLUMBER_NO_SIZE_TRACK

void* operator new(size_t size) {
	#ifdef PLUMBER_DEBUG
	clog << FG_GENERIC "(+" << size << ")" << FBG_DEFAULT;
	#endif
	return malloc(size);
}

void operator delete(void* memory) noexcept {
	free(memory);
}

#else

void* operator new(size_t size) {
	#ifdef PLUMBER_DEBUG
	clog << FG_NEW "(+" << size << ")" << FBG_DEFAULT;
	#endif
	size_t *memory = (size_t*)malloc(size + sizeof(size_t));
	memory[0] = size; // Stores size in first sizeof(size_t) bytes
	Plumber::add(size, memory);
	return (void*)(memory + 1); // Returns pointer as void without first

	// malloc??? in a C++ program? The power of $(GENERIC_DEITY) compels you
}

void operator delete(void* memory) noexcept {
	size_t size = *((size_t*)(memory) - 1);
	#ifdef PLUMBER_DEBUG
	clog << FG_DELETE "(-" << size << ")" << FBG_DEFAULT;
	#endif
	Plumber::subtract(size,(void*)memory);
	free((size_t*)memory - 1);
}

std::set<void*> Plumber::pointers {};

#endif

#endif
