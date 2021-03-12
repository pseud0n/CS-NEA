#include <vector>
#include <string>
#include <fstream>
#include <memory>
#include <string>

void bytes_file_to_word(const char* file_name, std::vector<std::string>& destination) {
	std::ifstream input_file(file_name, std::ios_base::binary);
	std::istreambuf_iterator<char> start(input_file), end;
	//Bytecode::bytecode = decltype(Bytecode::bytecode)(start, end);
	for (auto it = start; it != end;) {
		std::string& current_str_ref = destination.emplace_back();
		// emplace_back returns reference to object emplaced
		// * is overloaded to return reference to object stored by shared_ptr
		while (*it != 0x00) { // New word if found null
			current_str_ref += *it; // Append character
			++it;
		}
		++it;
	}
}
