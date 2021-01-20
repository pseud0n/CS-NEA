////////////////////////////////////////////////////////////////////////////////

/*
Made by Alex Scorza, 2020


manual:
	clear && make && ./a.out
custom:
	./go.sh

If permission to a.out is denied:
delete it:
	make clean

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
#include <functional>
#include <algorithm>
#include <iostream>
#include <optional>
#include <typeinfo>
#include <cassert>
#include <cstring>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <tuple>
#include <list>

// Setting up namespaces & aliases

using std::cout;
using std::cerr;
namespace bmp = boost::multiprecision;
// for pow & cpp_int

using namespace std::string_literals;
// "dfdsgfsd"s ≡ std::string("dfdsgfsd") (for a literal only)
// chrono_literals definition for numerical only, not string (phew!)


#include "standalone/debug_object.h"
#include "standalone/decl_display.h"
#include "standalone/plumber.h"
#include "standalone/dynamic_array.h"

Plumber plumber;

#define TEST_REPR(str) cout << "--------TEST: " << str << "\n\n"

#define MIN_CACHED_INTEGER_VALUE 0
#define MAX_CACHED_INTEGER_VALUE 10

//#include "big_dec.h"
#include "forward_decl.h" // Header file to forward-declare object


namespace UL {
	#include "objects/internal_object.h"
	#include "objects/external_object.h"
	#include "cpp_function.h"
	#include "function_view.h"
	#include "objects/conversions.h"
	#include "cache/decl.h"
	#include "exceptions.h"
}

#include "hash.h"

#include "printer.h"


//(UL::InternalObject<UL::Aliases::NumT>*)self.io_ptr
#define __GENERATE_SWITCH(self) 							\
	switch (self->type()) {									\
		case Types::null:							\
			break;									\
		case Types::number:							\
			SWITCH_MACRO(self->get<Aliases::NumT>());	\
			break;									\
		case Types::string:							\
			SWITCH_MACRO(self->get<Aliases::StringT>());		\
			break;									\
		case Types::cpp_function:					\
			SWITCH_MACRO(self->get<Aliases::CppFunctionT>());	\
			break;									\
		case Types::bytecode_function:				\
			SWITCH_MACRO(SWITCH_MACRO(self->get<Aliases::ByteCodeFunctionT>());	\
			break;									\
		case Types::list:							\
			SWITCH_MACRO(self->get<Aliases::ListT>());		\
			break;									\
		case Types::array:							\
			SWITCH_MACRO(self->get<Aliases::ArrayT>());		\
			break;	\
		default:									\
			break;									\
	}

#define GENERATE_SWITCH(enum_val) 							\
	switch (enum_val) {									\
		case Types::null:							\
			break;									\
		case Types::number:							\
			SWITCH_MACRO(Aliases::NumT);	\
			break;									\
		case Types::string:							\
			SWITCH_MACRO(Aliases::StringT);		\
			break;									\
		case Types::boolean:							\
			SWITCH_MACRO(Aliases::BoolT);		\
			break;									\
		case Types::cpp_function:					\
			SWITCH_MACRO(Aliases::CppFunctionT);	\
			break;									\
		case Types::bytecode_function:				\
			SWITCH_MACRO(Aliases::ByteCodeFunctionT);	\
			break;									\
		case Types::list:							\
			SWITCH_MACRO(Aliases::ListT);		\
			break;									\
		case Types::array:							\
			SWITCH_MACRO(Aliases::ArrayT);		\
			break;	\
		default:									\
			break;									\
	}

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
			cout << "Adding pair: " << *object << "\n";
			stored_objects.push_back(object);
			object_counts.push_back(1);
			cout << "Added pair!\n";
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
			cout << "{";
			for (size_t i = 0; i < stored_objects.size(); ++i) {
				if (i != 0) cout << ", ";
				cout << stored_objects[i] << " (" << *stored_objects[i] << ", " << (stored_objects[i]->is_const ? "const" : "non-const") << ") : " << object_counts[i];
			}
			cout << "}\n";
			#endif
		}
	} // namespace Tracker
} // namespace UL

#define UL_LMBD [](UL::CppFunction const* argument_data, const std::vector<UL::ExternalObject>& arguments) -> UL::ExternalObject
//Macro for defining the lambda for a CppFunction as `DY_LMBD { stuff }`

namespace Utils {
	/* Commented is a 'possible implementation' of std::apply from <utility> from https://en.cppreference.com/w/cpp/utility/apply
	`inplace_tuple_slice_apply` is a modification of this
	*****************************************************************************************************************************

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
		template <size_t N, typename Lambda1Type, typename Lambda2Type, typename TupleType, std::size_t... FirstPartIndices, std::size_t... SecondPartIndices>
		constexpr void apply_impl(Lambda1Type&& lambda1, Lambda2Type&& lambda2, TupleType&& tuple, std::index_sequence<FirstPartIndices...>, std::index_sequence<SecondPartIndices...>) {
			std::invoke(std::forward<Lambda1Type>(lambda1), std::get<FirstPartIndices>(std::forward<TupleType>(tuple))...);
			// Invoke first function with elements from the tuple with indexes in the first index_sequence
			std::invoke(std::forward<Lambda2Type>(lambda2), std::get<SecondPartIndices + N>(std::forward<TupleType>(tuple))...);
			// Invoke second function  with elements from the tuple with indexes in the second index_sequence
		}
	}

	template <size_t N, typename Lambda1Type, typename Lambda2Type, typename TupleType>
	// Truth be told, I don't know the actual function signatures but the compiler obviously does :)
	constexpr void inplace_tuple_slice_apply(Lambda1Type&& lambda1, Lambda2Type&& lambda2, TupleType&& tuple) {
		apply_impl<N, Lambda1Type, Lambda2Type, TupleType>(
			std::forward<Lambda1Type>(lambda1),
			std::forward<Lambda2Type>(lambda2),
			// template<std::size_t... Ints> using index_sequence = std::integer_sequence<std::size_t, Ints...>;
			std::forward<TupleType>(tuple), // Perfect forwarding of tuple to account for lvalue reference or rvalue reference for tuple
			std::make_index_sequence<N>{ }, // Compile-time size_t sequence up to `N`
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TupleType> > - N>{ }); // Compile-time size_t sequence up to tuple length - `N`
	}

}


namespace UL {
	std::unordered_map<Types, ExternalObject> builtin_objects;

	std::unordered_map<Types, Aliases::CustomT> builtin_dicts {
	{Types::string, { } }
	};

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

	//#include "lookup.h"

	template <typename StoredT>
	InternalObject<StoredT>::InternalObject()
		: type(AssociatedData<StoredT>::enum_type), reference_count(1),
		  is_immovable(AssociatedData<StoredT>::is_immovable),
		  stored_value(StoredT()) {}

	template <typename StoredT>
	InternalObject<StoredT>::InternalObject(StoredT construct_from) // e.g. std::string
	/*
		: type(Corresponding<StoredT>::enum_type), is_immovable(Corresponding<StoredT>::is_immovable), reference_count(1), attrs((AttrsT*)0), stored_value(std::move(construct_from)) */{
		type = AssociatedData<StoredT>::enum_type;
		reference_count = 1;
		is_immovable = AssociatedData<StoredT>::is_immovable;
		//attrs = (AttrsT*)0;
		stored_value = std::move(construct_from);
		/*
		If a large object like a long string is passed in, it should be
		moved, not copied.

		If we construct from a new dictionary, that can be moved when
		passed in to the function, or copied otherwise
		*/
	}

	template <typename StoredT>
	InternalObject<StoredT>::~InternalObject() {
		//if (attrs) delete attrs; // If dictionary exists, delete it
	};

	#include "objects/external_object.cpp"

	#include "cpp_function.cpp"
	#include "function_view.cpp"

	namespace Classes {
		ExternalObject object = Aliases::CustomT();
		ExternalObject string = Aliases::CustomT();
	} // Classes

} // UL

#endif