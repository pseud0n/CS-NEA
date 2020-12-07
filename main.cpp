////////////////////////////////////////////////////////////////////////////////

/*
Made by Alex Scorza, 2020

clear && make && ./main

https://www.programiz.com/cpp-programming/online-compiler/
*/


#include <boost/multiprecision/cpp_int.hpp>
#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <forward_list>
#include <functional>
#include <algorithm>
#include <iostream>
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


#include "standalone/decl_display.h"

#include "standalone/plumber.h"

Plumber plumber;

#define TEST_REPR(str) cout << "--------TEST: " << str << "\n\n"

#define MIN_CACHED_INTEGER_VALUE 0
#define MAX_CACHED_INTEGER_VALUE 5

#include "object_pointer.h"

#define OPTR UL::ObjectPointer

//#include "big_dec.h"
#include "hash.h"
#include "forward_decl.h" // Header file to forward-declare object

#include "external_object.h"

#include "exceptions.h"
#include "printer.h"

SAY(HERE)

#define _GENERATE_SWITCH 							\
	switch (type) {									\
		case Types::null:							\
			break;									\
		case Types::number:							\
			SWITCH_MACRO(union_val.numerical_val);	\
			break;									\
		case Types::string:							\
			SWITCH_MACRO(union_val.string_val);		\
			break;									\
		case Types::cpp_function:					\
			SWITCH_MACRO(union_val.function_val);	\
			break;									\
		case Types::bytecode_function:				\
			SWITCH_MACRO(union_val.bytecode_val);	\
			break;									\
		case Types::list:							\
			SWITCH_MACRO(union_val.vector_val);		\
			break;									\
		case Types::user_defined_object:			\
			SWITCH_MACRO(union_val.udo_val);		\
			break;									\
		default:									\
			break;									\
	}


#define GENERATE_SWITCH(self) 							\
	switch (type) {									\
		case Types::null:							\
			break;									\
		case Types::number:							\
			SWITCH_MACRO(self.get(TypeAliases::NumT));	\
			break;									\
		case Types::string:							\
			SWITCH_MACRO(self.get(TypeAliases::StringT));		\
			break;									\
		case Types::cpp_function:					\
			SWITCH_MACRO(self.get(TypeAliases::CppFunctionT));	\
			break;									\
		case Types::bytecode_function:				\
			SWITCH_MACRO(self.get(TypeAliases::ByteCodeFunctionT));	\
			break;									\
		case Types::list:							\
			SWITCH_MACRO(self.get(TypeAliases::ListT));		\
			break;									\
		case Types::user_defined_object:			\
			SWITCH_MACRO(self.get(TypeAliases::UserDefObjT));		\
			break;									\
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

		//std::unordered_map<Object*, unsigned int> remaining_objects = { };


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

	#ifdef DO_CACHE_DECL
	std::array<Object*, 1 + MAX_CACHED_INTEGER_VALUE - MIN_CACHED_INTEGER_VALUE> cached_numbers;
	#endif
	auto null_obj(new Object(nullptr, true));
	OPTR null_optr(nullptr); // Automatically deleted
} // namespace UL

#define OBJ_RET(x) [](){ return OPTR(x) }
//If an object is optional, no need to construct it unless it is actually used
//Assumes that return type of lambda is a UL::Object, so this will be converted

#define DY_LMBD [](UL::CppFunction* argument_data, const std::vector<UL::Object*>& arguments)
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
		bool can_convert(float, Object* object) { return can_convert(0, object); }

		bool can_convert(std::string, Types) { return true; }
		bool can_convert(char*, Types) { return true; }
		bool can_convert(const char*, Types) { return true; }
		bool can_convert(std::string&, Types) { return true; }
	}

	struct ByteCodeFunction {
		//Functor class which represents a function written in bytecode
		size_t start_line, code_length;
	};


	class Pair {
	public:
		OPTR key, value; 
		//Pair(Object* o1, Object* o2) : key(o1), value(o2)  {}
	};

	struct CppFunction {
		/*
		Functor class which represents a function written in C++
		A CppFunction accepts any number of arguments and input variables of C++ types
		*/
		const std::vector<Object*> optional_arguments;
		//Vector of lambdas returning UL::Object (implicitly converts return value) to stop objects from being constructed if they're not actually used
		//All additional arguments go to default values and then to the variable storing variadic arguments
		bool is_variadic; //Whether the function accepts any number of arguments beyond required + optional
		std::function<OPTR(CppFunction*, const std::vector<Object*>&)> function; //The function object containing the code


		/*
		CppFunction(std::vector<std::function<Object()> > optional_arguments, bool is_variadic, std::function<Object*(CppFunction*, std::vector<UL::Object*>&)> func)
			: optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {
		}
		*/

		CppFunction(std::vector<Object*> optional_arguments, bool is_variadic, std::function<OPTR(CppFunction*, const std::vector<Object*>&)> func)
			: optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {
			print("CF CONSTRUCT", function ? "callable" : "not callable");
		}

		CppFunction(const CppFunction& from) // Copy constructor
			: optional_arguments(from.optional_arguments), is_variadic(from.is_variadic) {
		}

		~CppFunction() {
			cout << "Destructing CppFunction " << this << '\n';
			for (Object* object : optional_arguments)
				delete object; // Optional arguments stored on heap and automatically deleted
		}

		OPTR operator ()(const std::vector<Object*>& args) {
			//[](UL::CppFunction* argument_data, const std::vector<UL::Object*>& arguments) { ... }
			print(args, (function ? "callable" : "not callable"));
			return function(this, args);
		}
		
		OPTR operator ()() { // Simple function which takes no arguments
			return function(this, { });
		}

		//OPTR operator ()(OPTR self, ) 
		template <size_t MinArgCount, typename... TypesT>
		bool assign_args(const std::vector<Object*>& inputs, TypesT&... outputs) {
			/*
			This function is used when assigning arguments in a non-variadic function.
			It takes the inputs and outputs and simply maps them (also taking optional arguments into account)
			The OPTRs passed in are treated as objects since we don't need to reference count here
			*/

			//print("Assigning args:", inputs.size(), MinArgCount);
			//print(sizeof...(outputs),  __PRETTY_FUNCTION__);

			std::string error = repr_arg_error(MinArgCount, optional_arguments.size(), is_variadic, inputs.size());
			//print("Error:", error, error.empty());

			if (!error.empty()) {
				print(error);
				return false;
			}

			// outputs: tuple of pointers to give the inputs to
			// inputs: vector of Object pointers
			//cout << "?" << MinArgCount + optional_arguments.size() - inputs.size() - 1 << "\n";
			//cout << "is " << *optional_arguments[1]()->union_val.numerical_val << " " << *optional_arguments[2]()->union_val.numerical_val << "\n";
			//if (((void*)variadic_var == 0) == is_variadic) //If no pointer has been given for extra arguments, it must not be a variadic function and vice versa
				//return false; //Equivalent to `variadic_var != nulltptr XNOR arguments_data.is_variadic`


			// Sets var pointers to first `min_arg_count + optional_arguments.size()` values of `inputs`
			// Sets var pointers to remaining optional arguments
			size_t argument_traverser = 0;

			//#define CURRENT_TYPE std::tuple_element_t<MinArgCount, std::tuple<TypesT&...>>
			#define CURRENT_TYPE decltype(var_pointer)
			// I wouldn't want to make you read the macro's value over and over (it's for your own good, I promise :p )

			//print("yo");

			Utils::inplace_tuple_slice_apply<MinArgCount>(
				[&argument_traverser, &inputs](auto&... var_pointer) {
					((var_pointer = inputs[argument_traverser++]->cast<CURRENT_TYPE>()), ...); // Assigns each variable the current (required) entered argument
				},//std::tuple_element_t<N, std::tuple<Ts&...>>
				[&inputs, this](auto&... var_pointer) { // `this` is a pointer so it's captured by value
					// All optional arguments that remain are handled here (arguments beyond minimum)
					size_t index = 0;
					print("vectors:", repr_deref(inputs), repr_deref(optional_arguments));
					/*
					e.g.	MinArgCount = 3, inputs = {13, 14, 15, 16} & optional_arguments = {5, 6}
							then the resulting values should be {13, 14, 15, 16, 6}
					*/
					((
						[&](){
							print("> ", index, inputs.size(), MinArgCount, MinArgCount + optional_arguments.size() - inputs.size());
							if (index < MinArgCount + optional_arguments.size() - inputs.size() ) { // If this optional argument was passed explicitly, i.e. not left as default
								//print(index, MinArgCount + optional_arguments.size(), inputs.size());
								print("Explicit:", index);
								var_pointer = inputs[index + MinArgCount]->cast<CURRENT_TYPE>(); // Set variable to explicitly passed argument
							}
							else { // If this optional argument was not passed and instead is set to a default value
								print("Implicit", index);
								var_pointer = optional_arguments[index]->cast<CURRENT_TYPE>();
							}
							++index;
						}()
					), ...);
					/*/
					((
						[this, &index, &inputs]() { // Lambda in a lambda!
							//cout << "INDEX " << index << " " << (MinArgCount + optional_arguments.size() - inputs.size() - 1) << "\n";
							if (index < MinArgCount + optional_arguments.size() - inputs.size() - 1) { // If this optional argument was passed explicitly, i.e. not left as default
								//print(index, MinArgCount + optional_arguments.size(), inputs.size());
								var_pointer = inputs[index]->cast<CURRENT_TYPE>(); // Set pointer to
							}
							else { // If this optional argument was not passed and instead is set to a default value
								//print("index", index);
								var_pointer = optional_arguments[index]->cast<CURRENT_TYPE>();
							}
							++index;
						}()
						//cout << (var_pointer = *(optional_arguments[index++ + MinArgCount + optional_arguments.size() - inputs.size() - 1]())) << "\n"
					), ...);
					//Number of optional arguments explicitly entered = `inputs.size() - min_arg_count`
					*/
				},
				std::tuple<TypesT&...>(outputs...)
			); // Note that the tuple container is able to store references

			#undef CURRENT_TYPE
			//cout << "Here!\n";

			return true;
		}

		template <size_t MinArgCount, typename VariadicType, typename... TypesT>
		bool assign_variadic_args(const std::vector<Object*>& inputs, std::vector<VariadicType>* variadic_var, TypesT&... outputs) {
			*variadic_var = std::vector<VariadicType>(); // vector should be initialised regardless of arguments entered

			//cout << "And Here\n";
			//cout << is_variadic << " " << inputs.size() << " " << MinArgCount << " " << optional_arguments.size();
			if (inputs.size() > MinArgCount + optional_arguments.size())
				assign_variadic_args<VariadicType>(MinArgCount + optional_arguments.size(), inputs, variadic_var);
			else
				return false;

			return assign_args<MinArgCount>(inputs, outputs...);
		}

		template <typename VariadicType>
		void assign_variadic_args(size_t non_variadic_count, const std::vector<Object*>& inputs, std::vector<VariadicType>* variadic_var) {
			//cout << "VC: " << inputs.size() - non_variadic_count << "\n";
			for (size_t variadic_argument_traverser = non_variadic_count; variadic_argument_traverser < inputs.size(); ++variadic_argument_traverser) {
				//cout << "VAT: " << variadic_argument_traverser << " " << *inputs[variadic_argument_traverser] << " is now ";
				variadic_var->push_back(inputs[variadic_argument_traverser]->cast<VariadicType>());
				//variadic_var->push_back("test");
				//cout << (variadic_var->back()) << "\n";
			}

			cout << variadic_var->size() << "\n";
		}
	};

	bool operator ==(const ObjectPointer& o1, const ObjectPointer& o2) {
		return o1.object_ptr == o2.object_ptr;
	}

	/*
	template <typename... KeyValuePairT> // KeyValuePairT: std::pair<std::string, ObjectPointer>
	UserDefinedObject::UserDefinedObject(KeyValuePairT... names_and_values)
		: attributes(names_and_values...) {
	}

	static bool ObjectPointer::operator ==(const ObjectPointer& o1, const ObjectPointer& o2) {
		return o1.object_ptr == o2.object_ptr;
	}
	*/

	ObjectPointer::ObjectPointer()
		: object_ptr(null_obj), is_weak(true) {
		// Set to null - return the already-cached null object
		cout << "Constructing blank OPTR " << this << "\n";
		// Invalid in this state - do not use!
		// Default constructor required in some cases
	}


	#define NUMERIC_CONSTRUCTOR(type) /* Input can be negative - underflow is not a problem */												  \
		ObjectPointer::ObjectPointer(type number, bool /*make_const*/)																			\
			: is_weak(false) {   																												\
			cout << "Constructing OPTR from " << #type << " " << number << "\n";																\
			/*this->is_weak = is_weak; */																									   \
			object_ptr->is_const = true;/*make_const || object_ptr->is_const;*/																 \
			if (number >= MIN_CACHED_INTEGER_VALUE && number <= MAX_CACHED_INTEGER_VALUE) { /* Has already been cached */					   \
				/* e.g. number = -5.0 & MIN_CACHED_INTEGER_VALUE = -5 then index = 0 */														 \
				object_ptr = cached_numbers[static_cast<size_t>(number) - MIN_CACHED_INTEGER_VALUE].object_ptr;								 \
				cout << "\tThis value is already cached (" << object_ptr << ")\n";															  \
				/* size_t is unsigned so cast will cause an underflow if number < 0 (but it doesn't matter since it will then overflow) */	  \
				++*object_ptr; /* Increment number of references for cached integer being pointed to */										 \
			} else																															  \
				object_ptr = new Object(number, true);																						  \
		}


	//ObjectPointer::ObjectPointer(double number, bool is_weak) { NUMERIC_CONSTRUCTOR(double) }
	//ObjectPointer::ObjectPointer(int number, bool is_weak) { NUMERIC_CONSTRUCTOR(int) }

	//NUMERIC_CONSTRUCTOR(int)
	//NUMERIC_CONSTRUCTOR(double)
	ObjectPointer::ObjectPointer(int number, bool make_const) {
		// If cached, should be weak
		// If not cached, if const, strong, if not const, strong

		/*this->is_weak = is_weak; */
		//object_ptr->is_const = true;/*make_const || object_ptr->is_const;*/
		cout << "Constructing OPTR from " << "int" << " " << number << "\n";
		#ifdef DO_CACHE_DECL 
		if (number >= MIN_CACHED_INTEGER_VALUE && number <= MAX_CACHED_INTEGER_VALUE) { /* Has already been cached */
			/* e.g. number = -5.0 & MIN_CACHED_INTEGER_VALUE = -5 then index = 0 */
			is_weak = true;
			object_ptr = cached_numbers[static_cast<size_t>(number) - MIN_CACHED_INTEGER_VALUE];
			cout << "\tThis value is already cached (" << object_ptr << ")\n";
			/* size_t is unsigned so cast will cause an underflow if number < 0 (but it doesn't matter since it will then overflow) */
			//++*object_ptr; /* Increment number of references for cached integer being pointed to */
		} else {
		#endif
			is_weak = false; // Strong reference to new object
			object_ptr = new Object(number, make_const);
		#ifdef DO_CACHE_DECL
		}
		#endif
	}

	#undef NUMERIC_CONSTRUCTOR

	ObjectPointer::ObjectPointer(const ObjectPointer& from, bool force_weak) // Copy constructor
		: object_ptr(from.object_ptr), is_weak(force_weak || from.is_weak || from.object_ptr->is_const) {
		cout << "Constructing OPTR " << this << " (" << r_optr_strength(*this) << ") from OPTR " << &from << " (" << r_optr_strength(from) << ", " << r_optr_constness(from.object_ptr) << ")\n";
		/*
		force_weak		from.is_weak	from.object_ptr->is_const	is_weak
		false			false		   	false					 	false
		false		  	false		   	true						true
		false		  	true			false						true
		true			false		   	false					   	true
		true			false		   	true						true
		true			true			false					   	true
		true			true			true						true
		*/
		if (!is_weak) ++*object_ptr; // A new strong reference
		// If the underlying Object is
	}

	ObjectPointer::ObjectPointer(ObjectPointer&& from) noexcept
		: object_ptr(std::exchange(from.object_ptr, (Object*)0)), is_weak(from.is_weak) {
		cout << "Moving " << &from << " to " << this << "\n";
		// Transfer ownership - move the pointer (8 bytes) without moving all of the data!
	}

	ObjectPointer::ObjectPointer(std::nullptr_t, bool)
	// Does not matter what bool value is since it has to be weak. First argument must be nullptr (only value with nullptr_t type)
		: object_ptr(null_obj), is_weak(true) {
		cout << "Constructing null OPTR (" << this << ")\n";
	}
	
	ObjectPointer::ObjectPointer(Object* object_ptr, bool make_const)
		: object_ptr(object_ptr), is_weak(false) {
		/*
		make_const	 object_ptr->is_const	result
		false		   false				   false
		false		   true					true
		true			false				   true
		true			true					true
		*/
		//if (is_weak) object_ptr->reference_count = 0;
		object_ptr->is_const = make_const || object_ptr->is_const;
		cout << "Constructing OPTR " << this << " (" << (is_weak? "weak" : "strong") << ", " << (object_ptr->is_const ? "const" : "non-const") <<  ")\n";
	}

	template <typename ConstructorT>
	ObjectPointer::ObjectPointer(ConstructorT construct_from, bool make_const)
		: is_weak(false) { // Must be strong since it is initially the only reference to the object
		cout << "Constructing OPTR " << this << " from arbitrary type (" << construct_from << ")\n";
		object_ptr = new Object(construct_from, make_const);
	}

	ObjectPointer::~ObjectPointer() {
		if (object_ptr == NULL) return;
		/*
		Ignore blank OPTR (since pointer will be NULL).
		As such, you can explicitly delete an OPTR by deleting what it points to
		and then knowing that when the OPTR is automatically deleted when it
		goes out of scope, it will not attempt an erroneous delete.
		*/
		cout << "Deleting (" << r_optr_strength(*this) << ") OPTR " << this << " wrapping " << object_ptr << " (" << *object_ptr << ") with refcount " << object_ptr->reference_count;
		if (is_weak) {
			cout << "\n";
		} else {
			cout << " to " << object_ptr->reference_count - 1 << "\n";
			--*object_ptr;
		}
	}

	OPTR& ObjectPointer::operator =(OPTR) {
		return *this;
	}

	//ObjectPointer::ObjectPointer operator ()(std::vector<Object*>&);

	OPTR ObjectPointer::operator ()(const std::vector<Object*>& args) {
		return (*object_ptr->union_val.function_val)(args);
	}

	OPTR ObjectPointer::operator ()() {
		return (*object_ptr->union_val.function_val)();
	}

	Object& ObjectPointer::operator *() const {
		return *object_ptr;
		// Allows OPTR to be printed in the same way as Object
	}

	Object* ObjectPointer::operator ->() const {
		return object_ptr;
	}

	bool ObjectPointer::operator ==(const ObjectPointer& against) {
		return object_ptr == against.object_ptr;
	}

	template <typename ConstructFromT>
	void ObjectPointer::create_from_blank(const ConstructFromT& construct_from, bool is_weak_or_const) {
		/*
		Suppose we want to fill a vector with elements (to create an in-language array):
		We reserve the correct amount of space and call all of the OPTR default constructors.
		Then, each blank OPTR is given data.

		Elements inside any container may be used to keep an object alive so they need to be OPTRs.
		
		If the item to add is a strong reference to an already-existing OPTR, we have to decide if it's weak.
		is_weak_or_const decides if the OPTR should be weak or not.

		If the item to add is another type, is_weak_or_const determines if the new OPTR is const.
		By definition, making it const also means that it cannot be strongly referenced so the object is weak.

		Creation may not allow for a strong reference, e.g. to a cached object so this will force weakness.
		*/

		if (object_ptr->type != Types::null) {
			std::cerr << "Object " << this << " is not blank\n";
			exit(0);
		}

		object_ptr = new Object(construct_from, true); // OPTR should be blank
		cout << "Turning OPTR " << this << " into object (" << construct_from << ") type " << object_ptr->type << "\n";
		this->is_weak = is_weak_or_const;
	}

	template <typename CastT>
	typename std::remove_reference<CastT>::type ObjectPointer::cast() const {
		/*
		Reference collapsing for lvalue references and rvalue references:
		T&  & 	-> T&
		T&  && 	-> T&
		T&& & 	-> T&
		T&& &&  -> T&&

		However, it should not accept rvalue references, only lvalue references and rvalues

		template< class T > struct remove_reference      { typedef T type; };
		template< class T > struct remove_reference<T&>  { typedef T type; };
		template< class T > struct remove_reference<T&&> { typedef T type; };

		template< class T > using remove_reference_t = typename remove_reference<T>::type;
		*/
		return static_cast<typename std::remove_reference_t<CastT>>(*object_ptr);
	}

	UserDefinedObject::UserDefinedObject(std::unordered_map<std::string, OPTR> attributes)
		: attributes(attributes) {
	}

	OPTR UserDefinedObject::get_attribute(std::string name) const {
		return attributes.at(name);
	}

	template <typename AlternateReturnT>
	AlternateReturnT UserDefinedObject::get_attribute(std::string name, AlternateReturnT alternate_value) const {
		//cout << "Looking for name: " << name << ", alternate value: " << alternate_value << "\n";
		if (has_attribute(name)) return get_attribute(name).cast<AlternateReturnT>();
		return alternate_value;
	}

	/*
	std::string UserDefinedObject::get_attribute(std::string name, std::string alternate_value) const {
		//cout << "Looking for name: " << name << ", alternate value: " << alternate_value << "\n";
		if (has_attribute(name)) return (AlternateReturnT)(*get_attribute(name));
		return alternate_value;
	}
	*/

	bool UserDefinedObject::has_attribute(std::string name) const {
		return attributes.find(name) != attributes.end();
		// If key not present, returns iterator at end value of map
		// Concept of an 'end' is there for consistency with STL containers
		// unordered_map does not have a conceptual 'end'
	}

	void UserDefinedObject::delete_attribute(std::string name) {
		//delete attributes[name]; // Delete object of heap pointer
		attributes.erase(name); // Delete string and pointer (not what it points to though, hence the previous line)
	}


	/*
	struct Location {
		unsigned int reference_count;
		static std::unordered_set<Location*> instances;
		std::unordered_set<Object*> bound_objects; // All strongly or weakly-referenced heap pointers bound to the Location object

		Location()
			: reference_count(0) {
			cout << "NEWREF0 " << this << "\n";
			instances.insert(this);
		}

		Location(Object* first_heap_pointer)
			: reference_count(!first_heap_pointer->is_weak) { // false -> 1, true -> 0 (convenient, right?)
			cout << "NEWREF1 (" << (first_heap_pointer->is_weak ? "weak)  " : "strong) ") << this << " (object @ " << first_heap_pointer << "')\n";
			bound_objects.insert(first_heap_pointer);
			instances.insert(this);
		}

		~Location() {
			cout << "DELREF " << this << " (remaining weakrefs: "  << bound_objects.size() << ")\n";
			for (auto it : bound_objects) delete it;
			//Dereference iterator and delete resulting heap Object pointer
			instances.erase(this); //Remove empty Location from set of Location instances
		}

		void operator +=(Object* heap_pointer) {
			++reference_count;
			if (!heap_pointer->is_weak) std::cout << "INCREF " << this << " to " << reference_count <<"\n";
			bound_objects.insert(heap_pointer);
		}

		void operator -=(Object* heap_pointer) {
			if (reference_count == 0)
				delete this; //Was decremented without giving it any initial values; immediately delete self
				//Note: This shouldn't happen!
			else {
				--reference_count;
				bound_objects.erase(heap_pointer); //Remove strong/weak reference
				if (!heap_pointer->is_weak) std::cout << "DECREF " << this << " to " << reference_count <<"\n";
				if (reference_count == 0) //If no strong references remaining
					delete this; //Can I do this (calls destructor)?
			}
		}

		static std::string show_instances() {
			std::stringstream stream;
			stream << "{";
			for (auto it = instances.begin(); it != instances.end(); ++it) {
				if (it != instances.begin()) stream << ", ";
				stream << *it << ":" << (**it).reference_count;
			}
			stream << "}";
			return stream.str();
		}
	};
	*/

	/*
	This class is responsible for tracking the individual ObjectPointer objects.
	Each ObjectPointer wraps a pointer to a shared object.
	An ObjectPointer additionally stores if the object is weak or strong.
	When a strong ObjectPointer is created, its shared Object's location's reference count is increased
	When a strong ObjectPointer is deleted, its shared Object's location's reference count is decreased
	If the reference count reaches zero,
	*/

	/*
	class Location {
	public:
		unsigned int reference_count;
		OPTR tracked_object;
		std::unordered_set<OPTR*> bound_weak_objects; // All object which do not contribute to reference count


		static std::unordered_set<Location*> instances;

		Location(OPTR* track)
			: reference_count(1), tracked_object(*track) {
			instances.insert(this);
			bound_weak_objects.insert(track);
		}

		void operator -=(OPTR* weak_object_pointer) {
			--reference_count;
			if (reference_count == 0)
			// Guaruntee that if reference count is 0, all remaining objects are weak references
				for (auto optr : bound_weak_objects)
					delete optr;
		}

	};

	auto Location::instances = std::unordered_set<Location*>(); //Construct set of locations

	*/

	/*
	decltype(auto) Object::get_current_value() const {
		switch (type) {
			case Types::null:
				return (void*)0;
			case Types::blank:
				return (void*)0;
			case Types::number:
				return union_val.numerical_val;
			case Types::string:
				return union_val.string_val;
			case Types::cpp_function:
				return union_val.function_val;
			case Types::bytecode_function:
				return union_val.bytecode_val;
			case Types::list:
				return union_val.vector_val;
			case Types::user_defined_object:
				return union_val.udo_val;
		}
	}
	*/

	Object::Object(bool)
		: type(Types::null), is_const(true), reference_count(1) {
		/*
		4 bytes: union type
		8 bytes: dictionary pointer if exists
		8 bytes: reference count to object
		*/
		cout << "Constructed stem object (" << this << ")\n";
		print(sizeof(type), sizeof(is_const), sizeof(reference_count), sizeof(attrs), sizeof(union_val));
		Tracker::add_pair(this);
	}

	Object::Object(std::nullptr_t, bool) // Initially weak: reference count starts at 1 still
		: type(Types::null), is_const(true), reference_count(1) {
		cout << "Constructed null object (" << this << ")\n";
		Tracker::add_pair(this);
	}

	/*
	Object::Object(TypeAliases::NumT x, bool make_const) //default argument
		: type(Types::number), is_const(make_const), reference_count(1) {
		union_val.numerical_val = new TypeAliases::NumT(x);
		cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
		Tracker::add_pair(this);
	}
	*/

	Object::Object(int x, bool make_const) //default argument
		: type(Types::number), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		union_val.numerical_val = new TypeAliases::NumT(x);
		cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
		Tracker::add_pair(this);
	}

	Object::Object(const char* string, bool make_const) //Enter a string literal
		: type(Types::string), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		union_val.string_val = new std::string(string);
		print(sizeof(type), sizeof(is_const), sizeof(union_val), sizeof(reference_count), sizeof(attrs), sizeof(*union_val.string_val));
		cout << "Constructed string object " << *this << " (" << this << ")\n";
		Tracker::add_pair(this);
	}

	Object::Object(CppFunction* cpp_function, bool make_const) //Enter a CppFunction heap pointer
		: type(Types::cpp_function), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		cout << "Constructed C++ function object '" << *this << "' (" << this << ")\n";
		union_val.function_val = cpp_function; //cpp_function should be a heap pointer
		Tracker::add_pair(this);
	}

	Object::Object(ByteCodeFunction* bc_function, bool make_const)
		: type(Types::bytecode_function), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		union_val.bytecode_val = bc_function;
		cout << "Constructed bytecode function object '" << *this << "' (" << this << ")\n";
		Tracker::add_pair(this);
	}

	Object::Object(UserDefinedObject* cls, bool make_const)
		: type(Types::user_defined_object), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		union_val.udo_val = cls;
		Tracker::add_pair(this);
	}

	template <typename T>
	Object::Object(const std::vector<T> *heap_vector, bool make_const) // Should not be able not modify the value at this pointer
		: type(Types::list), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		//union_val.vector_val = new std::vector<T> {};
		union_val.vector_val = heap_vector;
		//union_val.vector_val->insert(union_val.vector_val->end(), i_list.begin(), i_list.end());
		Tracker::add_pair(this);
	}

	/*
	template <typename ... T>
	Object::Object(std::tuple<T...>) {
	}
	*/

	Object::Object(const OPTR& from, bool)
		: type(from.object_ptr->type), union_val(from.object_ptr->union_val)  {


	}

	Object::Object(const Object* from /*bool force_weak*/, bool make_const)
		: type(from->type), /*is_weak(force_weak || from->is_weak),*/ is_const(make_const), reference_count(1) {
		// Copy made; should not share same location and should not move
		Tracker::add_pair(this);
		switch (type) {
			case Types::null:
				break;
			case Types::number:
				union_val.numerical_val = new TypeAliases::NumT(*from->union_val.numerical_val);
				break;
			case Types::string:
				union_val.string_val = new std::string(*from->union_val.string_val);
				break;
			case Types::cpp_function:
				union_val.function_val = new CppFunction(*from->union_val.function_val);
				break;
			case Types::bytecode_function:
				union_val.bytecode_val = new ByteCodeFunction(*from->union_val.bytecode_val);
				break;
			case Types::pair:
				union_val.pair_val = new TypeAliases::PairT(*from->union_val.pair_val);
				break;
			case Types::list:
				union_val.list_val = new TypeAliases::ListT(*from->union_val.list_val);
				break;
			case Types::array:
				union_val.vector_val = new TypeAliases::ArrayT(*from->union_val.vector_val);
				break;
			case Types::dictionary:
				union_val.dict_val = new TypeAliases::DictT(*from->union_val.dict_val);//(*from->union_val.dict_val);
				break;
		}
		//cout << "Copying (" << (from->is_weak ? "weak->" : "strong->") << (is_weak ? "weak) " : "strong) ") << from << " (" << *from << ") to " << this << " (" << *this << ")\n";
		cout << "Copying Object" << from << " to " << this << "\n";
	}


	/*
	Object* Object::make_reference() {
		reference_location->reference_count++;
		return this;

	}
	*/

	Object::~Object() {
		//*reference_location -= this; //Overloaded dereference on UL::Location object on heap
		cout << "\tDestructed object " << this << " (" << *this << ")\n";
		Tracker::remove_pair(this);
		/*
		switch (type) {
			case Types::null:
				break;
			case Types::number:
				delete union_val.numerical_val;
				break;
			case Types::string:
				delete union_val.string_val;
				break;
			case Types::cpp_function:
				delete union_val.function_val;
				break;
			case Types::bytecode_function:
				delete union_val.bytecode_val;
				break;
			case Types::list:
				delete union_val.vector_val;
				break;
			case Types::user_defined_object:
				delete union_val.udo_val;
				break;
			default:
				break;
		}
		*/
		#define SWITCH_MACRO(thing) delete thing
		_GENERATE_SWITCH
		#undef SWITCH_MACRO
	}

	void Object::operator ++() {
		cout << "Incref'ing " << this << " (" << *this << ") to " << reference_count + 1 << "\n";
		++reference_count;
		#ifdef TRACKER_DEBUG
		++Tracker::object_counts[std::distance(Tracker::stored_objects.begin(), std::find(Tracker::stored_objects.begin(), Tracker::stored_objects.end(), this))];
		#endif
	}

	void Object::operator --() {
		/*
		The OPTR type is relied on to correctly and appropriately decrement Object objects.
		We can guarantee that if it is decremented, it is because the original strong OPTR has been deleted

		if (is_const) {
			cout << "DECREF " << this << " ignored due to constness of object (" << *this << ")\n";
			return;
		}
		*/
		cout << "Decref'ing " << this << " (" << *this << ") to " << reference_count - 1 << "\n";
		if (reference_count == 1) delete this; // If the last reference is being removed
		else {
			--reference_count;
			#ifdef TRACKER_DEBUG
			--Tracker::object_counts[std::distance(Tracker::stored_objects.begin(), std::find(Tracker::stored_objects.begin(), Tracker::stored_objects.end(), this))];
			#endif
		}
		
	}

	OPTR Object::operator ()(const std::vector<Object*>& args) {
		return (*union_val.function_val)(args); //Pass inputs by reference
	}

	Object::operator int() const {
		return (int)*union_val.numerical_val;
	}

	Object::operator std::string() const {
		//cout << "OBJECT " << *this << ": " << this->type << " ";
		if (type == Types::string)
			return *union_val.string_val;
		std::stringstream stream;
		stream << *this;
		return stream.str();
	}

	/*
	Object::operator const Object* () const {
		return this;
	}
	*/

	Object::operator Object* () const {
		return (Object*)this;
	}

	template <typename CastT>
	typename std::remove_reference<CastT>::type Object::cast() const {
		return static_cast<typename std::remove_reference<CastT>::type>(*this);
	}

	OPTR& Object::get_attr(const std::string& name) {
		/*
		This is simply wrapper for access of the object's map.
		It does not provide error checking but is faster when presence is known.
		E.g. access of the 'type' attribute does not beed to be checked for presence.
		This is more efficient that getting an iterator and dereferencing
		*/
		return attrs->at(name); // Does not increase reference count!
	}

	Object::AttrsT::iterator Object::get_iterator(const std::string& name) {
		return attrs->find(name);
	}

	OPTR* Object::get_mro_optrptr_if_present(const std::string&) {
		/*
		Assumes
		*/
		return (OPTR*)0;
	}

	OPTR* Object::get_class_optrptr_if_present(const std::string& name) {
		/*
		Returns * because it may be null
		Under the assumption that this is a class.
		These functions are unsafe - they do not check that the assumptions made are
		accuate, but they are faster as a result.
		We assume that the MRO' attribute is present.
		The first element of an object's MRO is itself.
		MRO is a special attribute which is always stored as a std::vector
		*/
		Object::AttrsT::iterator iterator, it_at;
		if ((it_at = get_iterator("mro")) == attrs->end())
			return (OPTR*)0; // This was not a class, so we just ignore it.
		for (const OPTR& optr : *(get_attr("mro").object_ptr->union_val.vector_val)) {
			if ((iterator = optr.object_ptr->get_iterator(name)) != optr.object_ptr->attrs->end())
				return &(iterator->second); // Pointer to found OPTR
		}
		return (OPTR*)0; // Explicit cast = clarity?
		// Don't dereference and use! Just check if this value == 0
	}

	namespace Classes {
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

		//template </*size_t DirectBaseClassCount, typename... PairPlaceholderT*/>
		class BuiltinClass {
		public:
			const std::vector<BuiltinClass>& direct_bases; // Can not change
			std::unordered_map<std::string, OPTR> attributes; // Should not shrink or grow by values may change
			
			BuiltinClass(const std::vector<BuiltinClass>& bases)
				: direct_bases(bases), attributes({ }) {
			}

			void emplace(const std::string& key, Object* value) {
				cout << "In emplace\n";
				attributes.try_emplace(key, OPTR(value)); // Where OPTR is constructed in call an passed by reference into function
				cout << "Finishing emplace\n";
			}

			/*
			// Should emplace back pairs into blank map instead of constructing then moving temporary map
			BuiltinClass(const std::initializer_list<BuiltinClass*> direct_bases, std::unordered_map<std::string, OPTR>& kv)
				: direct_bases(direct_bases), attributes(kv) {
				//attributes = { names_and_values... };
			}
			*/

			BuiltinClass(const BuiltinClass&) = delete;

			~BuiltinClass() { // Not done by language, but potentially done by compiler directly
				// `attributes` and `bases` cleaned up automatically

				// Commented out under assumption that keys are stored on the stack
				//for (auto& pair : attributes) delete pair.second;
				cout << "Called deleter\n";
				UL::Tracker::repr();
			}

			OPTR operator [](const char* attr_name) {
				return attributes.at(attr_name);
			}
		};
		BuiltinClass object({ });
	}

	template <typename T>
	struct Corresponding;

	#define MAKE_TYPE_CONVERSION(from, to, enum_t, make_immovable)		\
	template <> struct Corresponding<from> {							\
		using CorrespondingT = to;										\
		static inline Types enum_type = enum_t;							\
		static inline bool is_immovable = make_immovable;				\
	};																	

	template <typename T> using GetCorresponding = typename Corresponding<std::remove_reference_t<T>>::CorrespondingT;

	MAKE_TYPE_CONVERSION(std::string, std::string, Types::string, true)
	MAKE_TYPE_CONVERSION(bool, bool, Types::boolean, true)
	MAKE_TYPE_CONVERSION(std::nullptr_t, std::nullptr_t, Types::null, true)
	MAKE_TYPE_CONVERSION(int, TypeAliases::NumT, Types::number, true)
	MAKE_TYPE_CONVERSION(TypeAliases::ArrayT, TypeAliases::ArrayT, Types::array, true)

	/*
	template <> struct Corresponding<const char*> {
		using CorrespondingT = std::string;
		static inline Types enum_type = Types::string;
		static inline bool is_immovable = true;
	};
	*/	

	template <typename ConstructionT> // e.g. const char*
	struct InternalObject {
		using AttrsT = std::unordered_map<std::string, OPTR>;

		using MyT = GetCorresponding<ConstructionT>;

		Types type;
		bool is_immovable;
		unsigned short reference_count;
		AttrsT *attrs;
		MyT stored_value; // unidentified underlying object
		
		InternalObject(MyT construct_from) // e.g. std::string
		/*
			: type(Corresponding<StoredT>::enum_type), is_immovable(Corresponding<StoredT>::is_immovable), reference_count(1), attrs((AttrsT*)0), stored_value(std::move(construct_from)) */{
			type = Corresponding<ConstructionT>::enum_type;
			is_immovable = Corresponding<ConstructionT>::is_immovable;
			reference_count = 1;
			attrs = (AttrsT*)0;
			stored_value = construct_from;
			/*
			If a large object like a long string is passed in, it should be
			moved, not copied.

			If we construct from a new dictionary, that can be moved when
			passed in to the function, or copied otherwise
			*/

		}

		/*
		InternalObject(const char* text)
			: type(Types::string), is_const(false), reference_count(1), attrs(NULL), unknown_type(std::string(text)) {
		}
		*/
	};

	template <typename T>
	ExternalObject::ExternalObject(T construct_from) {
		/*
		The 'type' of an InternalObject is the type it is constructed from.
		Object may be copied or moved into construct_from, but is never
		copied after that
		*/
		io_ptr = (void*)(new InternalObject<std::remove_reference_t<T>>((GetCorresponding<T>(construct_from))));
	}

	template <typename T>
	T& ExternalObject::get() const {
		return ((InternalObject<T>*)io_ptr)->stored_value;
	}

	Types ExternalObject::type() const {
		return *(UL::Types*)io_ptr;
		// Since this is the first 4 bytes of the object
	}



} // UL

int main() {
	cout << std::boolalpha; // print true & false not 1 & 0
	cout << SEP;
	//#include "classes.h"
	cout << SEP;
	#include "cache_decl.h"
	cout << SEP;
	cout << "\n"
			"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n"
			"┃ Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC ┃\n"
			"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";
	
	{
		cout << SEP;


		UL::Tracker::repr();
		
		//#include "tests/class_hier.h"
		//std::string s("dofijdoifjadofadfhioadfohfpiehfeifiodhpaofidhaopfihdoipfhdaoifhn");

		UL::ExternalObject obj(100000);

		cout << ((UL::InternalObject<int>*)obj.io_ptr)->stored_value << "\n";

		//print(*(UL::Types*)(obj.io_ptr));

		UL::Tracker::repr();
		cout << SEP "EXITED LOCAL SCOPE\n";
	}

	cout << SEP;

	UL::Tracker::repr();
	#ifdef DO_CACHE_DECL
	for (UL::Object * cached_heap_ptr : UL::cached_numbers)
		delete cached_heap_ptr;
	#endif
	delete UL::null_obj;
	/*
	OPTR is automatically  deleted at the end of the program since object_ptr is
	now deleted/invalid/NULL.
	*/
	UL::Tracker::repr();
	//cout.clear();
	cout << SEP "EXITED PROGRAM\n";
	return 0;
}
