#ifndef CODE_BLOCK_H
#define CODE_BLOCK_H

#include "objects/external_object.h"
#include <forward_list>
#include <iostream>

class CodeBlock {
public:
	std::vector<std::pair<ExternalObject, CodeBlock>> catches;
	size_t start_location;
	bool is_null;

	CodeBlock(size_t, bool=false);

	void operator ()() const;

	CodeBlock(const CodeBlock&) = default;
	CodeBlock(CodeBlock&&) noexcept = default;
};

bool operator ==(const CodeBlock& o1, const CodeBlock& o2) {
	return &o1 == &o2; // Cannot know if functions are the same, so just check if the internal pointers are equal
}

std::ostream& operator <<(std::ostream& stream, const CodeBlock& object) {
	return stream << "{" << &object << ":"<< object.start_location << "}";
}

#endif
