////////////////////////////////////////////////////////////////////////////////

/*
Made by Alex Scorza, 2020


(Run from this directory) ./go.sh
Output in output/

If permission to ouput/program.out is denied:
delete it:
	make clean
Look in README for more information

https://www.programiz.com/cpp-programming/online-compiler/
*/

#ifndef MAIN_SETUP_H
#define MAIN_SETUP_H

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/functional/hash.hpp>

#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <forward_list>
#include <type_traits>
#include <filesystem>
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <typeinfo>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <list>
#include <set>

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>

// Setting up namespaces & aliases

using std::clog;
using std::cout;
using std::cerr;

#ifndef SHOW_CLOG
struct Clearer {
	Clearer() {
		clog.setstate(std::ios_base::failbit);
	}
} _output_clearer; // Hack to do action outside of main
#endif

bool setup_everything = false;

std::ios_base::fmtflags basic_flags(clog.flags());

namespace bmp = boost::multiprecision;
// for pow & cpp_int
using namespace std::string_literals;
// "dfdsgfsd"s ≡ std::string("dfdsgfsd") (for a literal only)
// chrono_literals definition for numerical only, not string (phew!)
namespace fs = std::filesystem;

#include "standalone/type_name.cpp"
#include "standalone/debug_object.h"
#include "standalone/decl_display.h"
#include "standalone/plumber.h"
#include "standalone/hashable_array.h"

Plumber plumber;

#define TEST_REPR(str) clog << "--------TEST: " << str << "\n\n"

#define MKRRY	UL::ExternalObject::make_array
#define MKRRY_W	UL::ExternalObject::make_array_w
#define MKDCT	UL::ExternalObject::make_dict<Aliases::DictT>
#define MKCSTM	UL::ExternalObject::make_dict<Aliases::CustomT>
#define MKPR	UL::ExternalObject::make_pair

#define MIN_CACHED_INTEGER_VALUE 0
#define MAX_CACHED_INTEGER_VALUE 10

//#include "big_dec.h"

#define GENERATE_SWITCH(enum_val) \
	switch(enum_val) {	\
		case Types::null_type:	\
			break;	\
		case Types::number: SWITCH_MACRO(Aliases::NumT); break;	\
		case Types::string: SWITCH_MACRO(Aliases::StringT); break; \
		case Types::boolean: SWITCH_MACRO(Aliases::BoolT); break; \
		case Types::cpp_function: SWITCH_MACRO(Aliases::CppFunctionT); break; \
		case Types::cpp_function_view: SWITCH_MACRO(Aliases::CppFunctionViewT); break; \
		case Types::pair: SWITCH_MACRO(Aliases::PairT); break; \
		case Types::array: SWITCH_MACRO(Aliases::ArrayT); break; \
		case Types::dictionary: SWITCH_MACRO(Aliases::DictT); break; \
		case Types::base_exception: SWITCH_MACRO(Aliases::BaseExceptionT); break; \
		case Types::custom: SWITCH_MACRO(Aliases::CustomT); break; \
		case Types::code_block: SWITCH_MACRO(Aliases::CodeBlockT); break; \
	}


namespace UL {
#include "forward_decl.h" // Header file to forward-declare object
#include "objects/internal_object.h"
#include "objects/external_object.h"
#include "cpp_function.h"
#include "function_view.h"
#include "objects/conversions.h"
#include "cache/decl.h"
#include "exceptions.h"
#include "code_block.h"
#include "base_exception.cpp"
#include "default_exceptions.cpp"

	using ObjectT = std::variant<ExternalObject, std::string*>;

	std::map<Types, ExternalObject*> enum_to_class;
}

#include "hash.h"

#include "printer.h"


//(UL::InternalObject<UL::Aliases::NumT>*)self.io_ptr

#define CASE(enum_type, type) \
	case Types::enum_type: SWITCH_MACRO(Aliases::type); break;

#define _GENERATE_SWITCH(enum_val)								\
	switch (enum_val) {											\
		case Types::null_type:										\
			break;												\
		case Types::number:										\
			SWITCH_MACRO(Aliases::NumT);						\
			break;												\
		case Types::string:										\
			SWITCH_MACRO(Aliases::StringT);						\
			break;												\
		case Types::boolean:									\
			SWITCH_MACRO(Aliases::BoolT);						\
			break;												\
		case Types::cpp_function:								\
			SWITCH_MACRO(Aliases::CppFunctionT);				\
			break;												\
		case Types::bytecode_function:							\
			SWITCH_MACRO(Aliases::ByteCodeFunctionT);			\
			break;												\
		case Types::list:										\
			SWITCH_MACRO(Aliases::ListT);						\
			break;												\
		case Types::array:										\
			SWITCH_MACRO(Aliases::ArrayT);						\
			break;												\
		CASE(Types::base_exception, Aliases::BaseExceptionT)	\
		default:												\
			break;												\
	}

		/*
		CASE(boolean, BoolT)	\
		CASE(cpp_function, CppFunctionT)	\
		CASE(cpp_function_view, CppFunctionViewT)	\
		CASE(pair, PairT)	\
		CASE(array, ArrayT)	\
		CASE(dictionary, DictT)	\
		CASE(base_exception, BaseExceptionT)	\
		CASE(custom, CustomT)	\
	};*/

#undef CASE

namespace UL {
	namespace Tracker {
		/*
		For debugging, vector used, which has O(n) complexity, but it prints
		the values in the right order, which is probably more important when
		debugging
		*/

		#ifdef TRACKER_DEBUG
		std::vector<Object*> stored_objects{};
		std::vector<unsigned int> object_counts{};
		#endif


		/*
		std::vector<Object*> stored_objects
		#ifdef TRACKER_DEBUG
		= { }
		#endif
		;
		std::vector<unsigned int> object_counts
		#ifdef TRACKER_DEBUG
		= { }
		#endif
		;
		*/

		void add_pair(Object* object) {
			#ifdef TRACKER_DEBUG
			clog << "Adding pair: " << *object << "\n";
			stored_objects.push_back(object);
			object_counts.push_back(1);
			clog << "Added pair!\n";
			#endif
		}

		void remove_pair(Object*) {
			#ifdef TRACKER_DEBUG
			stored_objects.pop_back();
			object_counts.pop_back();
			#endif
		}

		void repr() {
			#ifdef TRACKER_DEBUG
			clog << "{";
			for (size_t i = 0; i < stored_objects.size(); ++i) {
				if (i != 0) clog << ", ";
				clog << stored_objects[i] << " (" << *stored_objects[i] << ", " << (stored_objects[i]->is_const ? "const" : "non-const") << ") : " << object_counts[i];
			}
			clog << "}\n";
			#endif
		}
	} // namespace Tracker
	#define UL_LMBD [&](UL::CppFunction const* argument_data, std::vector<UL::ExternalObject>& arguments) -> UL::ExternalObject
	//Macro for defining the lambda for a CppFunction as `DY_LMBD { stuff }`
	using LambdaT = std::function<ExternalObject(UL::CppFunction const*, std::vector<UL::ExternalObject>&)>;
} // namespace UL


namespace Utils {
	/* Commented is a 'possible implementation' of std::apply from <utility> from https://en.cppreference.com/w/cpp/utility/apply
	`inplace_tuple_slice_apply` is a modification of this
	*******************************************************

	namespace detail {
	template <class F, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
	}
	}

	template <class F, class Tuple>
	constexpr decltype(auto) apply(F&& f, Tuple&& t)
	{
		return detail::apply_impl(
			std::forward<F>(f), std::forward<Tuple>(t),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
	}
	*/


	// Adapted from 'possible implementation' from the C++ reference which is why this code is so much better than the rest of it :)

	namespace {
		template <size_t N, typename Lambda1T, typename Lambda2T, typename TupleT, std::size_t... FirstPartIndices, std::size_t... SecondPartIndices>
		constexpr void apply_impl(Lambda1T&& lambda1, Lambda2T&& lambda2, TupleT&& tuple, std::index_sequence<FirstPartIndices...>, std::index_sequence<SecondPartIndices...>) {
			std::invoke(std::forward<Lambda1T>(lambda1), std::get<FirstPartIndices>(std::forward<TupleT>(tuple))...);
			// Invoke first function with elements from the tuple with indexes in the first index_sequence
			std::invoke(std::forward<Lambda2T>(lambda2), std::get<SecondPartIndices + N>(std::forward<TupleT>(tuple))...);
			// Invoke second function  with elements from the tuple with indexes in the second index_sequence
		}
	}

	template <size_t N, typename Lambda1T, typename Lambda2T, typename TupleT>
	// Truth be told, I don't know the actual function signatures but the compiler obviously does :)
	constexpr void inplace_tuple_slice_apply(Lambda1T&& lambda1, Lambda2T&& lambda2, TupleT&& tuple) {
		apply_impl<N, Lambda1T, Lambda2T, TupleT>(
			std::forward<Lambda1T>(lambda1),
			std::forward<Lambda2T>(lambda2),
			// template<std::size_t... Ints> using index_sequence = std::integer_sequence<std::size_t, Ints...>;
			std::forward<TupleT>(tuple), // Perfect forwarding of tuple to account for lvalue reference or rvalue reference for tuple
			std::make_index_sequence<N>{ }, // Compile-time size_t sequence up to `N`
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TupleT> > - N>{ }); // Compile-time size_t sequence up to tuple length - `N`
	}

}


namespace UL {
	std::unordered_map<Types, ExternalObject> builtin_objects;
	std::unordered_map<Types, Aliases::CustomT> builtin_dicts;


	/*
	const std::unordered_map<UL::Types, const std::unordered_set<UL::Types> > conversion_table {
		{Types::null,			   	{Types::number, Types::string}},
		{Types::number,			 	{Types::string}},
		{Types::string,			 	{Types::number, Types::string, Types::list}},
		{Types::cpp_function,	   	{Types::string}},
		{Types::bytecode_function,  {Types::string}},
		{Types::list,			   	{Types::string}}
	};
	*/

	namespace Conversions {
		bool can_convert(int, Types type) { return type == Types::number || type == Types::string; }
		bool can_convert(double, Types type) { return can_convert(0, type); }

		bool can_convert(std::string, Types) { return true; }
		bool can_convert(char*, Types) { return true; }
		bool can_convert(const char*, Types) { return true; }
		bool can_convert(std::string&, Types) { return true; }
	}

	template <typename... Ts>
	std::vector<ExternalObject> make_eo_vec(Ts&&... elements) {
		// I think that this is the best I can do here
		std::vector<ExternalObject> temp(sizeof...(elements));
		// Default-construct
		size_t i = 0;
		((
		  [&]() {
			if constexpr(std::is_same_v<ExternalObject, std::remove_reference_t<Ts>>)
				temp[i++] = std::forward<Ts>(elements);
			else
				temp[i++] = ExternalObject::emplace<Ts>(std::forward<Ts>(elements));
		  }()
        ), ...);
		// Construct from lvalue or rvalue references and emplace
		// Using rvalue references: Constructs InternalObject then moves
		return temp;
		// Deletes null ExternalObjects left over from std::move (if && not &)
	}

	//std::unordered_map<std::string, ExternalObject> all_builtins;

#define ADD_CLASS_REF(ref_name, string) ExternalObject& ref_name = all_builtins[string];

	namespace Classes {
		ADD_CLASS_REF(object, "Object")
		ADD_CLASS_REF(cls, "Class")
		ADD_CLASS_REF(null_type, "NullType")
		ADD_CLASS_REF(integer, "Integer")
		ADD_CLASS_REF(string, "String")
		ADD_CLASS_REF(boolean, "Boolean")
		ADD_CLASS_REF(cpp_function, "Function")
		ADD_CLASS_REF(cpp_function_view, "FunctionView")
		ADD_CLASS_REF(code_block, "CodeBlock")
		ADD_CLASS_REF(pair, "Pair")
		ADD_CLASS_REF(array, "Array")
		ADD_CLASS_REF(dict, "Dict")
		ADD_CLASS_REF(base_exception, "ExcBase")
		ADD_CLASS_REF(custom, "Generic")
	}

	struct ByteCodeFunction {
		//Functor class which represents a function written in bytecode
		size_t start_line, code_length;
	};

		/*
		When methods are retrieved from an object, the names an definitions need to be known.
		A BuiltinClass is a container to store members.

		All object may have any extra members added at any time.
		When an object is an instance of a class, it is simply added to its member lookup graph (directed, acyclic)

		An object has its own type, which is defined as the first object in its MLG that is not a superclass.
		This is stored as a separate attribute

		Suppose we have `x = SpecialNumber(10)`. SpecialNumber inherits from Number.
		Number is an instance of Class and a subclass of Object.

		When we look for an attribute of x, we first look at the dictionary of x.
		If not found, we then ascend the graph, breadth-first.
		The attribute is looked for in x's type, SpecialNumber, which is the second element, ascending from x.
		
		x.func(*args)
		is the same as `SpecialNumber.func(func, *args)` if func belongs to the class not the instance
		possibly `Number.func(func, *args)`

		This function belongs to the class and so it is atomic.

		x (SpecialNumber) < SpecialNumber (Class) <= Number (Class) < Class (Class) < Class (Class) < ...
																	<= Object (Class) < Class (Class) < ...
																					  <= Object (Class) < ...

		*/
	namespace Bytecode {
#include "bytecode/scope_stack.cpp"
#include "bytecode/bc_data.cpp"
	}

#include "base_exception.cpp"
#include "code_block.cpp"
#include "objects/external_object.cpp"
#include "objects/internal_object.cpp"

#include "cpp_function.cpp"
#include "function_view.cpp"

	template <size_t S>
	bool type_match(std::vector<ExternalObject*>& given, std::array<Types, S>& accepts) {
		for (size_t i = 0; i < given.size(); ++i)
			if (given[i]->type() != accepts[i])
				return false;
		return true;
	}

	bool get_object_as_bool(ExternalObject& object) {
		ExternalObject boolean = ExternalObject(object.get_attr("ToBool"))();
		if (boolean.type() != Types::boolean) {
			THROW_ERROR(Exceptions::expected_bool(object))
		}
		return boolean.get<Aliases::BoolT>();
	}

	ExternalObject *value_ptr;

/*
#define ADD_OBJECT(i_name, e_name)\
	*(value_ptr = &all_builtins[e_name]) = MKCSTM("Name"s,e_name##s);\
	namespace Classes { ExternalObject& i_name = *value_ptr;  }
	

	
namespace Classes {}

ADD_OBJECT(object, "Object")
ADD_OBJECT(cls, "Class")
ADD_OBJECT(string, "String")
ADD_OBJECT(base_exception, "ExcBase")

#undef ADD_OBJECT
*/

	template <typename T, typename R=ExternalObject>
	ExternalObject make_monadic_method (std::function<R(T*)>&& code) {
		return ExternalObject::emplace<Aliases::CppFunctionT>(
			CppFunction::empty_eobject_vec, false, UL_LMBD {
				GetCorrespondingType<T> *self_obj; // Argument stored in function view
				if (!argument_data->assign_args<1>(arguments, self_obj)) {
					return nullptr;
				}
				//print("MMM result:", result);
				return code(self_obj);
				// `code` is a lambda e.g. [](Aliases::NumT x){ return x + 1; }
			}, std::vector<Types>{AssociatedData<T>::enum_type}
		);
		//print("temp:", &temp);
		//return temp;
	}

/*
	void emplace_map(const char* name, const ExternalObject& obj) {
		print("In emplace_map");
		auto external_object = std::forward<ExternalObject>(obj);
		class_ptr->emplace(name, external_object);
	}
*/
} // UL

namespace std {

#define HASH(T) std::hash<T>()(object.get<T>())
	size_t hash<ExternalObject>::operator ()(const ExternalObject& object) const noexcept {
		switch(object.type()) {
			case Types::null_type:
				return 0xDEADBEEF; // Haha
			case Types::boolean:
				return HASH(Aliases::BoolT);
			case Types::number:
				return HASH(Aliases::NumT);
			case Types::string:
				return HASH(Aliases::StringT);
			case Types::cpp_function:
				return HASH(Aliases::CppFunctionT);
			case Types::cpp_function_view:
				return HASH(Aliases::CppFunctionViewT);
			case Types::pair:
			{
				const Aliases::PairT& pair = object.get<Aliases::PairT>();
				return std::hash<ExternalObject>()(pair.first) ^ std::hash<ExternalObject>()(pair.second); // 3: 64 bit
			}
			case Types::array:
				return HASH(Aliases::ArrayT);
			case Types::base_exception:
				return HASH(Aliases::BaseExceptionT);
			default:
			{
				std::ostringstream osstream;
				osstream << "Cannot hash type '" << enum_to_class[object.type()] << "'";
				std::cin.get();
				return 0;
				//THROW_ERROR(osstream.str());
			}
		}
	}
#undef HASH
} // std
#endif
