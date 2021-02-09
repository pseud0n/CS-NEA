#ifndef INSTRUCTIONS_CPP
#define INSTRUCTIONS_CPP

#define PAIR std::make_pair

namespace Bytecode {
	#include "scope_stack.cpp"

	std::vector<const char*>
		identifiers,
		strings,
		integers,
		decimals,
		attributes,
		instruction_names = {
			"push_id",
			"push_str",
			"push_int",
			"push_dec",
			"push_attr",
			"mark_call",
			"make_call",
			"mark_code",
			"make_code",
			"get_attr",
			"assign",
			"operator",
			"end_exp"
		};
	
	std::vector<std::vector<ExternalObject>> scope_stacks{{}};
	ExternalObject interrupt;

	using OperatorT = std::pair<const char*, const char*>;

	template <size_t Size>
	using OperatorArrayT = std::array<OperatorT, Size>;

	/*
	std::array<std::pair<const char*, const char*>>
		binary_operators, post_unary_operators, pre_unary_operators;
		*/

	std::array<std::pair<const char*, const char*>, 7> binary_operators { // 0-127
		PAIR("->", "Then"),
		PAIR("+", "Add"),
		PAIR("-", "Sub"),
		PAIR("*", "Mul"),
		PAIR("/", "Div"),
		PAIR("//", "Quot"),
		PAIR("%", "Rem")
	};

	std::array<OperatorT, 3> post_unary_operators { // 128-191
		PAIR("++", "PostInc"),
		PAIR("--", "PostDec"),
		PAIR("!", "Fact")
	};

	std::array<OperatorT, 3> pre_unary_operators { // 192-255
		PAIR("++", "PreInc"),
		PAIR("--", "PreDec"),
		PAIR("-", "Neg")
	};

/*
b = 'a';
print((10+b.Length())b);

identifiers = {"b", "print"}
strings = {"a"}
integers = {"10"}
attrs = {"Length"}

push_id
b
push_str
a
assign
end_exp
push_id
print
mark_call
push_int
10
push_id
b
push_attr
Length
get_attr
mark_call
make_call
operator
+
push_id
b
operator
->
make_call
end_exp

*/
} // Bytecode

#undef PAIR

#endif