#ifndef PRE_UNARY_CPP
#define PRE_UNARY_CPP

#include <iostream>
#include <utility>
#include <array>
#include <cstdlib>
#include <cstring>

#define PAIR std::make_pair
#define LOWER_OFFSET 3

using OperatorT = std::pair<const char*, const char*>;
// reserved: -1 (255), 1, 2, 126, 127, 128, 130 (ctrl+c terminated), 128+n
std::array<OperatorT, 5> pre_unary_operators {
	PAIR("++", "PreInc"),
	PAIR("--", "PreDec"),
	PAIR("-", "Neg"),
	PAIR("*", "Expand"),
	PAIR("@", "Copy")
};	

std::array<OperatorT, 4> post_unary_operators {
	PAIR("++", "PostInc"),
	PAIR("--", "PostDec"),
	PAIR("!", "Fact"),
	PAIR("NOT", "Not")
};

std::array<OperatorT, 25> binary_operators {
	PAIR("+", "Add"),
	PAIR("-", "Sub"),
	PAIR("*", "Mul"),
	PAIR("/", "Div"),
	PAIR("//", "Quot"),
	PAIR("%", "Rem"),
	PAIR("**", "Pow"),
	PAIR("..", "Dot2"),
	PAIR("...", "Dot3"),
	PAIR("AND", "And"), // both 1
	PAIR("OR", "Or"), // not both 0
	PAIR("NOR", "Nor"), // both 0
	PAIR("XOR", "Xor"), // unequal
	PAIR("NAND", "Nand"), // not both  1
	PAIR("XNOR", "Xnor"), // equal
	PAIR("IN", "In"),
	PAIR("===", "Is"),
	PAIR("!==", "IsNot"),
	PAIR("==", "Eq"),
	PAIR("!=", "NotEq"),
	PAIR(">=", "GreaterEq"),
	PAIR("<=", "LessEq"),
	PAIR(">", "Greater"),
	PAIR("<", "Less"),
	PAIR(":", "Pair")
};

constexpr size_t binary_start = pre_unary_operators.size() + post_unary_operators.size();
// If n >= binary_start, n is binary

template <size_t array_len>
int index_of(const char* string, std::array<OperatorT, array_len> array) {
	for (size_t i = 0; i < array.size(); ++i) {
		if (!std::strcmp(array[i].first, string))
			return i;
	}
	return -1;
}

int main(int argc, char** argv) {
	// e.g. {(file name), 0, '++'} or {(file name), 15}
	if (argc < 3) return -1;
	/*
	if (argc == 2) {
		size_t index = std::stoul(argv[1]) - LOWER_OFFSET; // index is at least LOWER_OFFSET
		if (index < pre_unary_operators.size())
			std::cout << pre_unary_operators[index].second();
		else if (index < post_unary_operators.size() + pre_unary_operators.size())
			return post_unary_operators[index - pre_unary_operators.size()];
		return binary_operators[]
	}
	*/
	switch (std::atoi(argv[1])) {
		case 0:
			return index_of(argv[2], pre_unary_operators) + LOWER_OFFSET;
		case 1:
			return index_of(argv[2], post_unary_operators) + pre_unary_operators.size() + LOWER_OFFSET;
		case 2:
			return index_of(argv[2], binary_operators) + pre_unary_operators.size() + post_unary_operators.size() + LOWER_OFFSET;
		default:
			return -1;
	}
}

#undef PAIR

#endif
