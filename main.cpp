/*
clang++-7 -pthread -std=c++2a -Wall -Wextra  -o main main.cpp && ./main

*/

#include <initializer_list>
#include <unordered_map>
#include <unordered_set>
#include <functional>
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

using std::cout;
using std::cerr;

#define TEST_REPR(str) cout << "--------TEST: " << str << "\n\n"

#include "forward_decl.h" //Header file to forward-declare object
#include "exceptions.h"
#include "printer.h"

#define OBJ_RET(x) [](){ return OPTR(x) }
//If an object is optional, no need to construct it unless it is actually used
//Assumes that return type of lambda is a UL::Object, so this will be converted

#define DY_LMBD [](UL::CppFunction* argument_data, std::vector<OPTR>& arguments)
//Macro for defining the lambda for a CppFunction as DY_LMBD{ /* stuff */ }

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
    constexpr void inplace_tuple_slice_apply(Lambda1Type&& lambda1, Lambda2Type&& lambda2, TupleType&& tuple) {
        apply_impl<N, Lambda1Type, Lambda2Type, TupleType>(
            std::forward<Lambda1Type>(lambda1),
            std::forward<Lambda2Type>(lambda2),
            // template<std::size_t... Ints> using index_sequence = std::integer_sequence<std::size_t, Ints...>;
            std::forward<TupleType>(tuple), // Perfect forwarding of tuple to account for lvalue reference or rvalue reference for tuple
            std::make_index_sequence<N>{}, // Compile-time size_t sequence up to `N`
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TupleType> > - N>{}); //Compile-time size_t sequence up to tuple length - `N`
    }

}

namespace UL {

    const std::unordered_map<UL::Types, const std::unordered_set<UL::Types> > conversion_table {
        {Types::null,               {Types::number, Types::string}},
        {Types::number,             {Types::string}},
        {Types::string,             {Types::number, Types::string}},
        {Types::cpp_function,       {Types::string}},
        {Types::bytecode_function,  {Types::string}},
        {Types::list,               {Types::string}}
    };

    namespace Conversions {
        bool can_convert(int, Types type) { return type == Types::number || type == Types::string; }
        bool can_convert(double, Types type) { return can_convert(0, type); }
        bool can_convert(float, Object* object) { return can_convert(0, object); }

        bool can_convert(std::string, Types) { return true; }
        bool can_convert(char*, Types) { return true; }
        bool can_convert(const char*, Types) { return true; }
        bool can_convert(std::string&, Types) { return true; }
    }


    struct ByteCodeFunction;
    struct CppFunction;

    struct ByteCodeFunction {
        //Functor class which represents a function written in bytecode
        size_t start_line, code_length;
    };

    struct CppFunction {
        // Functor class which represents a function written in C++
        const std::vector<OPTR> optional_arguments;
        //Vector of lambdas returning UL::Object (implicitly converts return value) to stop objects from being constructed if they're not actually used
        //All additional arguments go to default values and then to the variable storing variadic arguments
        bool is_variadic; //Whether the function accepts any number of arguments beyond required + optional
        std::function<OPTR(CppFunction*, std::vector<OPTR>&)> function; //The function object containing the code

        template <size_t MinArgCount, typename VariadicType, typename... TypesT>
        bool assign_args(std::vector<OPTR>& inputs, std::vector<VariadicType>* variadic_var, TypesT&... outputs) {

        	cout << "Assigning args:\n";
        	cout << sizeof...(outputs) << "\n";

            // outputs: tuple of pointers to give the inputs to
            // inputs: vector of Object pointers
            //cout << "?" << MinArgCount + optional_arguments.size() - inputs.size() - 1 << "\n";
            //cout << "is " << *optional_arguments[1]()->union_val.numerical_val << " " << *optional_arguments[2]()->union_val.numerical_val << "\n";
            if (inputs.size() < MinArgCount)
			    return false; //If the entered arguments are too small
            if (inputs.size() > MinArgCount + optional_arguments.size() && !is_variadic)
                return false; //Too many arguments for a non-variadic function
            //if (((void*)variadic_var == 0) == is_variadic) //If no pointer has been given for extra arguments, it must not be a variadic function and vice versa
                //return false; //Equivalent to `variadic_var != nulltptr XNOR arguments_data.is_variadic`


            // Sets var pointers to first `min_arg_count + optional_arguments.size()` values of `inputs`
            // Sets var pointers to remaining optional arguments
            size_t argument_traverser = 0;
            cout << __PRETTY_FUNCTION__ << "\n";
            //MinArgCount = 3

            //std::function<void(TypesT&...)>

            Utils::inplace_tuple_slice_apply<MinArgCount>(
				[&argument_traverser, &inputs](auto&... var_pointer) {
					((var_pointer = inputs[argument_traverser++].cast<decltype(var_pointer)>()), ...); // Assigns each variable the current (required) entered argument
				},
            	[&inputs, this](auto&... var_pointer) { // `this` is a pointer so it's captured by value
					// All optional arguments that remain are handled here
					// Takes 6 integer references
					size_t index = 0;
					((
						[&var_pointer, this, &index, &inputs]() { // Lambda in a lambda!
							//cout << "INDEX " << index << " " << (MinArgCount + optional_arguments.size() - inputs.size() - 1) << "\n";
							if (index < MinArgCount + optional_arguments.size() - inputs.size() - 1) { // If this optional argument was passed explicitly
								var_pointer = inputs[index + MinArgCount].cast<decltype(var_pointer)>(); // Set pointer to
							}
							else { // If this optional argument was not passed and instead is set to a default value
								var_pointer = optional_arguments[index].cast<decltype(var_pointer)>();
							}
							++index;
						}()
						//cout << (var_pointer = *(optional_arguments[index++ + MinArgCount + optional_arguments.size() - inputs.size() - 1]())) << "\n"
					), ...);
					//Number of optional arguments explicitly entered = `inputs.size() - min_arg_count`
				},
				std::tuple<TypesT&...>(outputs...)
			); // Note that the tuple container is able to store references


            //cout << "Here " << variadic_var << "\n";

            *variadic_var = std::vector<VariadicType>(); // vector should be initialised regardless of arguments entered

            //cout << "And Here\n";
            //cout << is_variadic << " " << inputs.size() << " " << MinArgCount << " " << optional_arguments.size();
            if (is_variadic && inputs.size() > MinArgCount + optional_arguments.size())
                assign_variadic_args<VariadicType>(MinArgCount + optional_arguments.size(), inputs, variadic_var);

            return true;
        }

        template <typename>
        void assign_varidic_args(size_t, std::vector<OPTR>&, void*) {
        }

        template <typename VariadicType>
        void assign_variadic_args(size_t non_variadic_count, std::vector<OPTR>& inputs, std::vector<VariadicType>* variadic_var) {
            //cout << "VC: " << inputs.size() - non_variadic_count << "\n";
            for (size_t variadic_argument_traverser = non_variadic_count; variadic_argument_traverser < inputs.size(); ++variadic_argument_traverser) {
                //cout << "VAT: " << variadic_argument_traverser << " " << *inputs[variadic_argument_traverser] << " is now ";
                variadic_var->push_back(inputs[variadic_argument_traverser].cast<VariadicType>());
                //variadic_var->push_back("test");
                //cout << (variadic_var->back()) << "\n";
            }

            cout << variadic_var->size() << "\n";
        }

        /*
        CppFunction(std::vector<std::function<Object()> > optional_arguments, bool is_variadic, std::function<Object*(CppFunction*, std::vector<UL::Object*>&)> func)
            : optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {
        }
        */

        CppFunction(std::vector<OPTR> optional_arguments, bool is_variadic, std::function<OPTR(CppFunction*, std::vector<OPTR>&)> func)
            : optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {

        }

        CppFunction(const CppFunction& from) //Copy constructor
            : optional_arguments(from.optional_arguments), is_variadic(from.is_variadic) {
        }

        OPTR operator ()(std::vector<OPTR> args) {
            return function(this, args);
        }
    };

    ObjectPointer ObjectPointer::operator ()(std::vector<OPTR>& arguments) { //OPTR operator ()(std::vector<OPTR>&);
    	return (*object_ptr->union_val.function_val)(arguments);
    }

    /*
    template <typename... KeyValuePairT> // KeyValuePairT: std::pair<std::string, ObjectPointer>
    UserDefinedObject::UserDefinedObject(KeyValuePairT... names_and_values)
    	: attributes(names_and_values...) {
    }
    */

    UserDefinedObject::UserDefinedObject(std::unordered_map<std::string, OPTR> attributes)
    	: attributes(attributes) {
    }

    OPTR UserDefinedObject::get_attribute(std::string name) const {
        return attributes.at(name);
    }

    //*
    template <typename AlternateReturnT>
    AlternateReturnT UserDefinedObject::get_attribute(std::string name, AlternateReturnT alternate_value) const {
        //cout << "Looking for name: " << name << ", alternate value: " << alternate_value << "\n";
        if (has_attribute(name)) return get_attribute(name).cast<AlternateReturnT>();
        return alternate_value;
    }
    //*/

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

    Object::Object(std::nullptr_t/*, bool is_weak*/)
        : type(Types::null), /*is_weak(is_weak),*/ reference_count(1) {
        cout << "Constructed null object (" << this << ")\n";
    }

    Object::Object(double x/*, bool is_weak*/) //default argument
        : type(Types::number), /*is_weak(is_weak),*/ reference_count(1) {
        union_val.numerical_val = new double(x);
        cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(int x/*, bool is_weak*/) //default argument
		: type(Types::number), /*is_weak(is_weak),*/ reference_count(1) {
		union_val.numerical_val = new double(static_cast<double>(x));
		cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(const char* string/*, bool is_weak*/) //Enter a string literal
        : type(Types::string), /*is_weak(is_weak),*/ reference_count(1) {
        union_val.string_val = new std::string(string);
        cout << "Constructed string object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(CppFunction* cpp_function/*, bool is_weak*/) //Enter a CppFunction heap pointer
        : type(Types::cpp_function), /*is_weak(is_weak),*/ reference_count(1) {
        cout << "Constructed C++ function object '" << *this << "' (" << this << ")\n";
        union_val.function_val = cpp_function; //cpp_function should be a heap pointer
    }

    Object::Object(ByteCodeFunction* bc_function/*, bool is_weak*/)
        : type(Types::bytecode_function), /*is_weak(is_weak),*/ reference_count(1) {
        union_val.bytecode_val = bc_function;
        cout << "Constructed bytecode function object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(UserDefinedObject* cls/*, bool is_weak*/)
        : type(Types::user_defined_object), /*is_weak(is_weak),*/ reference_count(1) {
        union_val.udo_val = cls;
    }

    template <typename T>
    Object::Object(std::initializer_list<T> i_list/*, bool is_weak*/)
        : type(Types::list), /*is_weak(is_weak),*/ reference_count(1) {
        union_val.list_val = new std::vector<T>();
        union_val.list_val->insert(union_val.list_val->end(), i_list.begin(), i_list.end());
    }

    /*
    template <typename ... T>
    Object::Object(std::tuple<T...>) {
    }
    */

    Object::Object(const Object* from/*, bool force_weak*/)
        : type(from->type), /*is_weak(force_weak || from->is_weak),*/ reference_count(1) {
        // Copy made; should not share same location
        switch (type) {
            case Types::null:
                break;
            case Types::number:
                union_val.numerical_val = new double(*from->union_val.numerical_val);
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
            case Types::list:
                union_val.list_val = new std::vector(*from->union_val.list_val);
                break;
            default:
                cout << "How did I get here?\n";
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
        cout << "Destructed object '" << *this << "' (" << this << ")\n";
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
                delete union_val.list_val;
                break;
            case Types::user_defined_object:
                delete union_val.udo_val;
                break;
            default:
            	break;
        }
    }

    void Object::operator ++() {
    	++reference_count;
    }

    void Object::operator --() {
    	if (reference_count == 1) delete this; // If the last reference is being removed
    	else --reference_count;
    }

    Object::operator int() const {
        return *union_val.numerical_val;
    }

    Object::operator std::string() const {
        //cout << "OBJECT " << *this << ": " << this->type << " ";
        if (type == Types::string)
            return *union_val.string_val;
        std::stringstream stream;
        stream << *this;
        return stream.str();
    }

    OPTR Object::operator ()(std::vector<OPTR> args) {
        return (*union_val.function_val)(args); //Pass inputs by reference
    }

    /*
    void Object::push_back(Object *object) {
        if (type == Types::list) {

        } else
            cerr << "Oh no!\n";
            //CompileTimeExc("")();
    }
    */


    namespace Classes {

        template <size_t DirectBaseClassCount, typename... PairPlaceholderT>
        struct BuiltinClass {
            const std::array<BuiltinClass, DirectBaseClassCount> direct_bases; // Can not change
            std::unordered_map<std::string, OPTR> attributes; // Should not shrink or grow by values may change

            BuiltinClass(const std::array<BuiltinClass, DirectBaseClassCount> direct_bases, PairPlaceholderT... names_and_values)
                : direct_bases(direct_bases) {
                attributes = { names_and_values... };
            }

            ~BuiltinClass() { // Not done by language, but potentially done by compiler directly
                // `attributes` and `bases` cleaned up automatically

                // Commented out under assumption that keys are stored on the stack
                //for (auto& pair : attributes) delete pair.second;
            }
        };

        /*
        auto Object = new BuiltinClass(
            {}, // No super classes - this is at the top of the hierarchy
            {"init"}
        );
        */

        //auto Object = new BuiltinClass();
    }

}

int main() {
	cout << "Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC\n------------------------------------------\n\n";
	{
		#include "pre_decl.h"
		//cout.setstate(std::ios_base::failbit);

		//#include "tests/cpp_function.h"

		OPTR func = OPTR(new UL::CppFunction({10, 11, 33}, true, DY_LMBD {
			// arguments: std::vector<UL::Object*>
			int a, b, c, d, e, f; // These 6 have to be (convertible to) integers
			std::vector<std::string> extra_args;
			// arguments: std::vector<OPTR>&, extra_args: std::vector<std::string>
			if(!argument_data->assign_args<3>(arguments, &extra_args, a, b, c, d, e, f)) {
				UL::ArgExc("Function takes 6+ integrals")();
				return OPTR(nullptr);
			}
			cout << "a-f: " << a << " " << b << " " << c << " " << d << " " << e << " " << f << "\n";
			cout << extra_args << "\n";
			return OPTR(a + b + c + d + e + f);
		}));

		std::vector<OPTR> arguments = {8, 3, 2, 1, 11, 100, "hello", "there", 5};

		OPTR out = func(arguments);
		cout << out << "\n";

		}

    cout.clear();
    cout << "\nFINISHED\n";
    return 0;
}
