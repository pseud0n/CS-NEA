#ifndef CODE_BLOCK_CPP
#define CODE_BLOCK_CPP

#include "objects/external_object.h"
#include <forward_list>

CodeBlock::CodeBlock(size_t start, bool is_null)
	: start_location(start), is_null(is_null) {
}

void CodeBlock::operator ()() const {
	/*
	The top value of the stack is set to a null value and the data from this
	object is moved to other locations so it can be deleted.
	The code continues and automatically increments the current instruction.
	A new scope is created.
	When the code block ends, the top scope is deleted and the value there
	is moved to replace the top value of the scope below it (which is null)
	*/
	if (is_null) return;
	print("CodeBlock called", start_location);
	Bytecode::scopes.add_scope(Bytecode::scopes.instruction_index);
	Bytecode::scopes.instruction_index = start_location;
}

#endif
