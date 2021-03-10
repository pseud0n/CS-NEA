#ifndef BYTECODE_SCOPE_STACK_CPP
#define BYTECODE_SCOPE_STACK_CPP

#define MAX_RECURSION_DEPTH 100

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include "../objects/external_object.h"
#include "../forward_decl.h"
#include "instructions_enum.h"

using namespace std::string_literals;

/*
#if __has_include("../printer.h")
#define CAN_PRINT_VEC_AND_UMAP // Can be defined elsewhere
#include "../printer.h"
#endif
*/
#define _THROW_ERROR(error, scope) \
	scope.exception.emplace(error); \
	throw std::runtime_error("ERROR");

#define THROW_ERROR(error) \
	_THROW_ERROR(error, UL::Bytecode::scopes)

#define INTERNAL_THROW_ERROR(error) \
	_THROW_ERROR(error, scopes)
//UL::Bytecode::scopes.exception.emplace(error); \
//--UL::Bytecode::scopes; // also checks for catch

namespace Operators {
#include "operators.cpp"
// Avoids rebuilding arrays every time an operator is encountered
// Bytecode generator is in C; cannot directly use C++ constructs
}

class ScopeStack {
private:
	/*
	Variant cannot store reference, but values used will all be lvalues
	An object can either reference an object which is part of the current scope
	or that is on the evaluation stack.
	An identifier is initiallly just a string. When it is evaluated it becomes
	a pointer to value is stores, which now exists primarily as a value to the
	name.
	When a temporary is created, e.g. a string, it is assigned to a custom name
	that is inaccessible.

	Example: a = "hello";
	 - Add new ID to evaluation stack: "a"
	 - Add new temporary variable: ".0":5
	 - Assign: Add new pair, a:".0" becoming a:5
	 - ".0" may be used in an expression so it's not deleted

	Example: a = 5; f = Fn("x")->{ x; }; f(a) = 10;
	 - Add id a
	 - Add 5 to eval stack (don't bother giving it a name as it's not
	   assignable)
	 - Add pair a:5
	 - End expression; delete null
	 - Add id f
	 - Add id Fn
	 - Open call
	 - Add string "x", don't give it a name
	 - Call Fn with one argument to generate new nontrivial temporary
	 - Scope is now {a:5,.0:<fn header>}
	 - Create a new scope and add it to evaluation stack as .1
	 - Add "x" to scope as ExternalObject
	 - Return it by not removing it from evaluation stack
	 - Delete scope and call .0 with .1 to make temporary function
	 - Bind to .2
	 - Do f = .2
	 - Delete top item at end of expression: function only exits in map
	 - Call f with a, evaluating value of a to ExternalObject
	 - Function returns same ExternalObject
	 - Reassign object to 10, replacing original value
	 - Destruct previous object assuming not cached
	*/
	
public:
//#ifdef CAN_PRINT_VEC_AND_UMAP
	friend std::ostream& operator <<(std::ostream&, const ScopeStack&);
//#endif
	using VariantT = std::variant<Aliases::CustomT::iterator, std::string>;

	static inline size_t instruction_index = 0;
	// This can be modified by external functions, e.g. 'Call' for CodeBlock

	// map of variables, evaluation stack, 
	//std::vector<std::vector<std::unordered_map<std::string, ExternalObject>>> scope_stacks;
	std::vector<Aliases::CustomT> scope_stacks;
	std::vector<size_t> call_stack;
	// Stores return location of current code block
	// Always one element shorter than scope_stacks
	
	std::optional<ExternalObject> exception = std::nullopt;
	// Only one exception active
	size_t remaining_throws = 1;
	// May need to exit several scopes so it keeps going
	// Default: 1 (destroyed after first catch)

	std::vector<unsigned char> bytecode;
	std::vector<std::string> identifiers, numbers, strings, attrs;

	std::vector<size_t> code_reduction_stack, // Where to return when current code block exits
						call_reduction_stack,
						list_reduction_stack;

	std::vector<VariantT> evaluation_stack;
	// Where operands live (as unevaluated identifiers or iterators)

	size_t call_marker,
		   list_marker,
		   code_marker;

	ScopeStack() {
		scope_stacks.emplace_back();
	}

	bool instruction_takes_arg(Instructions instruction) {
		if (instruction >= I_ASSIGN)
			return false;
		return true;
	}

	ExternalObject* get_scoped_varable(const std::string& name) {
		/*
		Check to see if a variable is in the current scope. Assignment can be
		done with either = or :=.
		Using = checks if the value exits inthe CURRENT scope and raises an
		error if it is not.
		*/
		decltype(scope_stacks)::value_type::iterator dict_it;
		if ((dict_it = scope_stacks.back().find(name)) != scope_stacks.back().end()) {
			print("Found", name);
			return &dict_it->second;
		}
		return nullptr;
	}

	void add_scope(size_t);

	void del_scope() {
		print("DELETED SCOPE", top_scope(), call_stack, scope_stacks.size());
		if (scope_stacks.size() == 1) {
			if (exception) {
				// If exception not removed during exit, program fails
				std::ostringstream osstream;
				osstream << "failure: " << *exception;
				throw std::runtime_error(osstream.str());
			} else {
				throw std::runtime_error("success");
			}
		}
		scope_stacks.pop_back();
		instruction_index = call_stack.back();
		call_stack.pop_back();
		// Only empty if scope_stacks has length 1, which is already handled

		/*
		When the top stack is deleted, if it's the last stack then the program
		ends. Failure depends on whether or not there is an uncaught exception
		or whether the program fails 
		*/
	}

	size_t next_temporary() {
		decltype(scope_stacks)::value_type::iterator it, end_it = scope_stacks.back().end();
		std::string dot = ".";
		for (size_t i = 0; ; ++i) {
			std::string name = dot + std::to_string(i);
			it = top_scope().find(name);
			if (it == end_it) return i;
		}
	}

	auto add_temporary(ExternalObject temp) {
		size_t name_number = next_temporary();	
		[[maybe_unused]] auto [it, success] = top_scope().emplace("."s + std::to_string(name_number), temp);
		// it: iterator to inserted item, success: whether the emplacement was successful (unused)
		return it;
	}

	void delete_temporaries() {
		for (auto it = top_scope().begin(); it != top_scope().end(); ) {
			// Implemenation from en.cppreference.com/w/cpp/container/unordered_map/erase
			if (it->first[0] == '.')
				it = top_scope().erase(it);
			else 
				++it;
		}
	}

	template <typename T>
	void push_op(T&& obj) {
		evaluation_stack.emplace_back();
		evaluation_stack.back().emplace<0>(add_temporary(std::forward<T>(obj)));
	}

	void push_id(std::string& str) {
		evaluation_stack.emplace_back();
		print("Pushed id", str);
		evaluation_stack.back().emplace<1>(std::string(str));
	}

	template <typename T>
	void replace_top_object(T&& obj) {
		print(get_top());
		if (get_top().index() == 0) {
			std::get<0>(get_top())->second = std::forward<T>(obj);
		} else {
			top_scope()[std::get<1>(get_top())].reassign(obj);			
		}
		// Reassigning automatically deletes original object
	}

	VariantT& get_top() {
		return evaluation_stack.back();
	}

	ExternalObject& get_top_as_object() {
		return get_as_object(get_top());
	}

	void pop_top() {
		evaluation_stack.pop_back();
	}

	void add_marker(std::vector<size_t>& stack) {
		stack.push_back(evaluation_stack.size());
		print("Target size:", evaluation_stack.size());
	}

	/*
	void pop_assign_penultimate(ExternalObject* location) {
		auto penultimate_iterator = evaluation_stack.rbegin()[1];
		*location = *penultimate_iterator;
		evaluation_stack.erase(penultimate_iterator);
	}
	*/

	ExternalObject& get_as_object(VariantT& variant) {
		if (get_top().index() == 0)
			return std::get<0>(get_top())->second;
		//const std::string& str = std::get<1>(get_top());
		return find_object(std::get<1>(get_top()));
	}

	void operator ()();

			decltype(scope_stacks)::value_type& top_scope() {
		return scope_stacks.back(); // Reference to top scope
	}

	ExternalObject* find_object_no_throw(const std::string&);

	ExternalObject& find_object(const std::string& name);

	void throw_error() {
		print("RAISED AN ERROR");
		std::ostringstream osstream;
		osstream << *exception;
		del_scope();
		// 1 hour 6 minutes
		//throw std::runtime_error(osstream.str());
	}
};

ScopeStack scopes;

void ScopeStack::add_scope(size_t return_pos) {
	if (scope_stacks.size() > MAX_RECURSION_DEPTH) {
		cout << "OH NO" << std::endl;
		cout << std::flush;
		INTERNAL_THROW_ERROR("Maximum recursion depth exceeded! ("s + std::to_string(MAX_RECURSION_DEPTH) + " stacks)");
		return;
	}
	print("CREATED SCOPE");
	scope_stacks.emplace_back();
	call_stack.push_back(return_pos);
}

ExternalObject* ScopeStack::find_object_no_throw(const std::string& name) {
	print("Looking for:", name);
	decltype(scope_stacks)::value_type::iterator dict_it;
	for (auto it = scope_stacks.rbegin(); it != scope_stacks.rend(); ++it) {
		if ((dict_it = it->find(name)) != it->end()) {
			print("Found", name);
			return &dict_it->second;
		}
	}
	//decltype(all_builtins)::iterator
	auto all_builtins_it = all_builtins.find(name);
	if (all_builtins_it != all_builtins.end()) {
		print("Found", name);
		return &all_builtins_it->second;
	}
	return 0;
}
 ExternalObject& ScopeStack::find_object(const std::string& name) {
	ExternalObject *found = find_object_no_throw(name);
	if (found)
		return *found;
	INTERNAL_THROW_ERROR("Could not find name "s + name)		
}

void ScopeStack::operator ()() {
	/*
	Function to do the parsing of a bytecode given names of all data.
	*/

	print("------------------");
	print(all_builtins);
	while (instruction_index < bytecode.size()) {
		print("START", instruction_index, char_hex(bytecode[instruction_index]), evaluation_stack, top_scope(), call_stack);
		//std::cin.get();
		switch(bytecode[instruction_index]) {
			case I_NOP:
				push_op(nullptr);
				// Every line pushes a value to the stack
				// A line containing only ';' creates a NOP statement
				break;
			case I_PUSH_ID:
				print(identifiers);
				push_id(identifiers[bytecode[++instruction_index]]);
				break;
			case I_PUSH_ATTR:
			{
				const char *attr_name = attrs[bytecode[++instruction_index]].c_str();
				print("Attr name:", attr_name);
				ExternalObject top_attr_gotten = get_top_as_object().get_attr(attr_name);
				// Forces top object to get evaluated
				print("Result:", top_attr_gotten);
				pop_top();
				push_op(top_attr_gotten);
				break;
			}
			case I_PUSH_INT:
				push_op(Aliases::NumT(numbers[bytecode[++instruction_index]]));
				// Construct new number from a string
				break;
			case I_PUSH_STR:
				print(strings);
				push_op(std::string(strings[bytecode[++instruction_index]]));
				break;
			case I_OP:
			{
				/*
				C code accesses operators with program call call C++ can
				just #include it
				*/
				size_t index = bytecode[++instruction_index] - LOWER_OFFSET;
				print("OPERATOR:", index);
				const char *attr_name;
				if (index < Operators::pre_unary_operators.size()) {
					attr_name = Operators::pre_unary_operators[index].second;
					print("Attr name:", attr_name);
					ExternalObject& operand = get_top_as_object();
					auto args = make_eo_vec(operand);
					print(operand.get_type(), operand.get_type().attrs_of()["MRO"].get<Aliases::ArrayT>()[1].attrs_of());
					std::cin.get();
					ExternalObject result = ExternalObject(operand.get_type().get_attr(attr_name))(args);
					pop_top(); // No longer needed!
					push_op(result);
					std::cin.get();
				} else if (index < Operators::pre_unary_operators.size() + Operators::post_unary_operators.size()) {
					attr_name = Operators::post_unary_operators[index + Operators::post_unary_operators.size()].second;
					get_top() = add_temporary(get_top_as_object().get_type().get_attr(attr_name));
				} else {
					ExternalObject popped_top = std::move(get_top_as_object());
					pop_top();
					attr_name = Operators::binary_operators[index - Operators::pre_unary_operators.size() - Operators::post_unary_operators.size()].second;
					print("Binary operator:", attr_name, index, index - Operators::pre_unary_operators.size() - Operators::post_unary_operators.size());
					print(get_top_as_object());
					print(get_top_as_object().attrs_of());
					auto args = make_eo_vec(get_top_as_object(), popped_top);
					ExternalObject result = ExternalObject(get_top_as_object().get_type().get_attr(attr_name))(args);
					print("Result:", result);
					get_top() = add_temporary(result);
				}
				break;
			}
			case I_ASSIGN: // 06
			{
				/*
				Suppose the top objects are {..., 100, 101} where an object
				holding 100 is assigned to 101.
				The top object is erased and stored in a separate variable.
				Assignment does not involve any copying so the references
				to 100 are now references to 101.
				a = 100; b = 101; a = b;
				Now a === b, so if either variable is modified, both are
				affected.

				This is done by calling the destructor of 100's io_ptr then
				assigning it to 101's, adding the number of references.
				*/
				auto penultimate_iterator = evaluation_stack.end() - 2;
				ExternalObject top_as_object = get_top_as_object();
				print("PI:", *penultimate_iterator);
				if (penultimate_iterator->index() == 0) {
					/*
					If it's an rvalue,
						e.g. in `foo[x] = 10`, `exc.Message() = 5`
					In this case, the original object should be modified.
					If a copy is returned, then modifying th InternalObject
					will not affect the object copied from and it returns a
					reference, other references to the object should now
					point to the modified version of the object.

					An immovable object cannot be reassigned to.
					*/
					std::get<0>(*penultimate_iterator)->second.reassign(top_as_object);
					pop_top();
				} else {
					print("ID", *penultimate_iterator);
					/*
					E.g. {..., ID "a", 50}
					Reassign kv in current scope.
					Reassign the object to the current scope with the given
					name which automatically deletes the previous
					ExternalObject
					The value must be found in the current scope and if not,
					an error is raised.
					*/
					std::string& name = std::get<1>(*penultimate_iterator);
					ExternalObject *maybe_value = get_scoped_varable(name);
					if (!maybe_value) {
						INTERNAL_THROW_ERROR("Could not find variable "s + name + " in scope (= is only for reassignment, consider :=)");
					}
					top_scope()[std::get<1>(*penultimate_iterator)] = top_as_object;
					//pop_top();
					evaluation_stack.erase(penultimate_iterator);
				}
				break;
			}
			case I_LET:
			{
				auto penultimate_iterator = evaluation_stack.end() - 2;
				ExternalObject top_as_object = get_top_as_object();
				pop_top();
				if (penultimate_iterator->index() == 0) {
					INTERNAL_THROW_ERROR(":= is used for assigning to an identifier, not an reference expression"s)
				} else {
					print("String, good");
					std::string& name = std::get<1>(*penultimate_iterator);
					top_scope().erase(name);
					top_scope()[name] = top_as_object;
					// Leave penultimate object (now top) which is a string
					// This allows chaining, e.g. a := b := 1 

				}
				break;

			}
			case I_MARK_CALL:
				/*
				... f(1,2,3)
				{..., ID f}
				When a call is met, add to the call stack the current length
				of the evaluation stack. When a closing bracket is met, pop
				from the evaluation stack to a new vector of arguments until
				the number of elements matches the target.
				A stack is used instead of a single value in the case of
				calls within arguments of another call.
				*/
				add_marker(call_reduction_stack);
				break;
			case I_MAKE_CALL:
			{
				std::vector<ExternalObject> arguments;
				print(call_reduction_stack, evaluation_stack.size());
				while (evaluation_stack.size() > call_reduction_stack.back()) {
					arguments.insert(arguments.begin(), get_top_as_object());
					print("Popping", get_top());
					pop_top();
				}
				// Operand on top of evaluation stack now callable
				call_reduction_stack.pop_back(); // Call completed
				print("Arguments:", arguments, "top:", get_top(), get_top().index());
				print("top:");
				if (get_top().index() == 0) {
					print("EO:", std::get<0>(get_top())->second);
				} else {
					print("ID:", std::get<1>(get_top()));
				}
				ExternalObject& top_as_object = get_top_as_object();
				print("Top as object:", top_as_object);
				print("Arguments:", arguments);
				ExternalObject call_result = top_as_object(arguments);
				print("INDEX NOW IS", instruction_index);
				print("Call result:", call_result);
				pop_top();
				//replace_top_object(call_result);
				push_op(call_result);
				break;
			}
			case I_MARK_LIST:
				add_marker(list_reduction_stack);
				break;
			case I_MAKE_LIST:
			{
				/*
				I.e. a closing square bracket.
				Simply add each element to the start of a new list.

				Create a new code block with the correct start and end
				points.
				The index of the bytecode instruction is available outside
				of this function so that e.g. code blocks can modify it if
				the code is exited.
				*/
				Aliases::ArrayT array;
				size_t target_length = list_reduction_stack.back();
				while (evaluation_stack.size() > target_length) {
					array.emplace(0, std::move(get_top_as_object()));
					pop_top();
				}
				push_op(std::move(array));
				list_reduction_stack.pop_back();
				break;
			}
			case I_MARK_CODE:
			{
				/*
				Advance until the next closing brace that matches this one
				*/
				print("MET {");
				size_t start_pos = instruction_index;
				size_t brace_diff = 0; // One more '{' than '}' initially
				for (;;) {
					print("Advance", instruction_index, char_hex(bytecode[instruction_index]), brace_diff);
					if (bytecode[instruction_index] == I_MARK_CODE)
						++brace_diff; // '{'
					else if (bytecode[instruction_index] == I_MAKE_CODE)
						--brace_diff; // '}'
					if (brace_diff == 0)
						break;
					instruction_index += 1 + instruction_takes_arg(static_cast<Instructions>(bytecode[instruction_index]));
					// Implicit: false->0, true->1 : false->+1, true->+2
					// 05 0c is binary + so 0c does not always represent '}'
				}
				// Don't advance past '}'; done at start of next iteration
				print("Index:", instruction_index);
				print("Instruction:", char_hex(bytecode[instruction_index]));
				CodeBlock code_block(start_pos);
				// Ending instruction is one after start
				push_op(std::move(code_block));
				/*
				Location where code starts (line to go to when called)
				In a case such as `{};`, the starting location is the
				closing brace so an extra I_NOP is inserted
				*/
				break;
			}
			case I_MAKE_CODE:
			{
				/*
				If met naturally, it's when a code block ends execution.
				After exit, go to the place it was called from.
				As an example:
				{}();
				When the code is executed (after  ')'), the execution
				continues at the ';' by jumping.
				*/
				del_scope();
				break;
			}
			case I_ADD_BYTE:
				break;
			case I_END_EXP:
				pop_top();
				delete_temporaries();
				// Value is pushed for every expression (include NOP)
				break;
		}
		++instruction_index;
	}
	print("------------------");
#ifdef SHOW_CLOG
	std::cin.get();
#endif
}


#endif
