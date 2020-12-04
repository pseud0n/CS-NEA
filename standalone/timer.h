#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	Timer() : start(std::chrono::high_resolution_clock::now()) {}
	~Timer() {
		std::cout <<
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - start
			).count() << "ms\n";
		}
};

#endif