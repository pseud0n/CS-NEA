// 'Plumber' cos it finds leaks, geddit?

#ifndef STANDALONE_MEMORY_MANAGER_H
#define STANDALONE_MEMORY_MANAGER_H

#include "colours.h"

#include <stack>
#include <string>
#include <iostream>


class Plumber {
	friend std::ostream& operator <<(std::ostream&, const Plumber&);
private:
	static inline size_t memory_usage = 0;

public:
	static void add(size_t size) {
		memory_usage += size;
	}

	static void subtract(size_t size) {
		memory_usage -= size;
	}

	Plumber() {
	}
	

	~Plumber() {
		#ifdef PLUMBER_NO_SIZE_TRACK
		cout << "\nEXIT MEMORY NOT TRACKED - undefine 'PLUMBER_NO_SIZE_TRACK' and recompile\n";
		#else
		if (memory_usage)
			cout << BG_WARNING;
		cout << "\nEXIT MEMORY INCREASE: " << memory_usage << " bytes" FBG_DEFAULT "\n";
		#endif
	}
	
	size_t get() const {
		return memory_usage;
	}
};

std::ostream& operator<<(std::ostream& stream, const Plumber& instance) {
	return stream << Plumber::memory_usage << " bytes";
}

#ifdef PLUMBER_NO_SIZE_TRACK

void* operator new(size_t size) {
	#ifdef PLUMBER_DEBUG
	cout << FG_GENERIC "(+" << size << ")" << FBG_DEFAULT;
	#endif
	return malloc(size);
}

void operator delete(void* memory) noexcept {
	free(memory);
}

#else

void* operator new(size_t size) {
	#ifdef PLUMBER_DEBUG
	cout << FG_NEW "(+" << size << ")" << FBG_DEFAULT;
	#endif
	size_t *memory = (size_t*)malloc(size + sizeof(size_t));
	memory[0] = size; // Stores size in first sizeof(size_t) bytes
	Plumber::add(size);
	return (void*)(memory + 1); // Returns pointer as void without first

	// malloc??? in a C++ program? The power of $(GENERIC_DEITY) compels you
}

void operator delete(void* memory) noexcept {
	size_t size = *((size_t*)(memory) - 1);
	#ifdef PLUMBER_DEBUG
	cout << FG_DELETE "(-" << size << ")" << FBG_DEFAULT;
	#endif
	Plumber::subtract(size);
	free((size_t*)memory - 1);
}

#endif

#endif