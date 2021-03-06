// 'Plumber' cos it finds leaks, geddit?

#ifndef STANDALONE_MEMORY_MANAGER_H
#define STANDALONE_MEMORY_MANAGER_H

#define FBG_DEFAULT 	"\x1B[0m"
#define FG_NEW			"\x1B[91m"
#define FG_DELETE 		"\x1B[92m"
#define BG_WARNING		"\x1B[101m"

/*
Name            FG  BG
-----------------------
Black           30  40
Red             31  41
Green           32  42
Yellow          33  43
Blue            34  44
Magenta         35  45
Cyan            36  46
White           37  47
Bright Black    90  100
Bright Red      91  101
Bright Green    92  102
Bright Yellow   93  103
Bright Blue     94  104
Bright Magenta  95  105
Bright Cyan     96  106
Bright White    97  107
*/

#include <stack>
#include <string>
#include <iostream>


class Plumber {
	private:
		//typedef unsigned int MemoryT;
		
		static inline std::stack<Plumber*> instances;
		size_t memory_usage;

		friend std::ostream& operator <<(std::ostream&, const Plumber&);
	public:
		static void add(size_t value) {
			clog << "LEN " << instances.size() << "\n";
			if (instances.size())
				clog << "TOP " << instances.top() << "\n";
			//instances.top()->memory_usage += value;
		}

		static void subtract(size_t value) {
			//instances.top()->memory_usage -= value;
		}

		Plumber() : memory_usage(0) {
			clog << "CONS\n";
			static bool init_stack = [](){
				instances = std::stack<Plumber*>();
				return 0;
			};
			instances.push(this);
		}
		
		~Plumber() {
			clog << *this << " " << instances.size() << " " << *instances.top() << "\n";
			if (memory_usage)
				clog << BG_WARNING;
			clog << "\nEXIT MEMORY INCREASE: " << memory_usage << " bytes" FBG_DEFAULT "\n";
			instances.pop();
		}
		
		size_t get() const {
			return memory_usage;
		}
};

std::ostream& operator<<(std::ostream& stream, const Plumber& instance) {
	return stream << instance.memory_usage << " bytes";
}

void* operator new(size_t size) {
	#ifdef PLUMBER_DEBUG
	clog << FG_NEW "(+" << size << ")" << FBG_DEFAULT;
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
	clog << FG_DELETE "(-" << size << ")" << FBG_DEFAULT;
	#endif
	Plumber::subtract(size);
	free((size_t*)memory - 1);
}

//Plumber::instances = std::stack<Plumber*>();

#endif
