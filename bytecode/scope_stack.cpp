#ifndef BYTECODE_SCOPE_STACK_CPP
#define BYTECODE_SCOPE_STACK_CPP

/*
#if __has_include("../printer.h")
#define CAN_PRINT_VEC_AND_UMAP // Can be defined elsewhere
#include "../printer.h"
#endif
*/

class ScopeStack {
public:
//#ifdef CAN_PRINT_VEC_AND_UMAP
	friend std::ostream& operator <<(std::ostream&, const ScopeStack&);
//#endif

	// map of variables, evaluation stack, 
	std::vector<std::vector<std::unordered_map<std::string, ExternalObject>>> scope_stacks;
	
	ExternalObject exception = nullptr;
	// Only one exception active

	ScopeStack() {
		++*this;
	}

	void operator ++() {
		if (scope_stacks.size() > 100) {
			throw_error(0);
		}
		scope_stacks.emplace_back();
	}

	void operator --() {
		scope_stacks.pop_back();
	}

	decltype(scope_stacks)::value_type& top() {
		return scope_stacks.back();
	}

	void throw_error(ExternalObject object) {
		throw std::runtime_error("!ERROR!");
	}
};

std::ostream& operator <<(std::ostream& stream, const ScopeStack&){
	return stream << "ss.scope_stacks";
}
	

#endif