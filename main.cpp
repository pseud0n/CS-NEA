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

using std::cout;
using std::cerr;

#define TEST_REPR(str) cout << "--------TEST: " << str << "\n\n"

#include "forward_decl.h" //Header file to forward-declare object
#include "exceptions.h"
#include "printer.h"

#define OBJ_RET(x) [](){ return new UL::Object(x); }
//If an object is optional, no need to construct it unless it is actuall used
//Assumes that return type of lambda is a UL::Object, so this will be converted
 
#define DY_LMBD [](UL::CppFunction* argument_data, std::vector<UL::Object*>& arguments)
//Macro for defnining the lambda for a CppFunction as DY_LMBD{ /* stuff */ }

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
            std::invoke(std::forward<Lambda2Type>(lambda2), std::get<SecondPartIndices + N>(std::forward<TupleType>(tuple))...);
        }
    }
    
    template <size_t N, typename Lambda1Type, typename Lambda2Type, typename TupleType>
    constexpr void inplace_tuple_slice_apply(Lambda1Type&& lambda1, Lambda2Type&& lambda2, TupleType&& tuple) {
        apply_impl<N>(
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
        bool can_convert(int, Object* object) { return object->type == Types::number || object->type == Types::string; }
        bool can_convert(double, Object* object) { return can_convert(0, object); }
        bool can_convert(float, Object* object) { return can_convert(0, object); }

        bool can_convert(std::string, Object*) { return true; }
        bool can_convert(char*, Object*) { return true; }
        bool can_convert(const char*, Object*) { return true; }
    }


    struct ByteCodeFunction;
    struct CppFunction;

    struct ByteCodeFunction {
        //Functor class which represents a function written in bytecode
        size_t start_line, code_length;
    };

    struct CppFunction {
        //Functor class which represents a function written in C++
        //size_t min_arg_count; //Minimum possible arguments required to be entered
        const std::vector<std::function<Object()> > optional_arguments;
        //Vector of lambdas returning UL::Object (implicitly converts return value) to stop objects from being constructed if they're not actually used
        //All additional arguments go to default values and then to the variable storing variadic arguments
        bool is_variadic; //Whether the function accepts any number of arguments beyond required + optional
        std::function<Object*(CppFunction*, std::vector<Object*>&)> function; //The function object containing the code

        template <size_t MinArgCount, typename VariadicType, typename ...Types>
        bool assign_args(std::vector<Object*>& inputs, std::vector<VariadicType>* variadic_var, Types* ... outputs) {
            //outputs: tuple of pointers to give the inp, uts to
            //inputs: vector of Object pointers
            //cout << "?" << MinArgCount + optional_arguments.size() - inputs.size() - 1 << "\n";
            //cout << "is " << *optional_arguments[1]()->union_val.numerical_val << " " << *optional_arguments[2]()->union_val.numerical_val << "\n";
            if (inputs.size() < MinArgCount)
			    return false; //If the entered arguments are too small
            if (inputs.size() > MinArgCount + optional_arguments.size() && !is_variadic)
                return false; //Too many arguments for a non-variadic function
            //if (((void*)variadic_var == 0) == is_variadic) //If no pointer has been given for extra arguments, it must not be a variadic function and vice versa
                //return false; //Equivalent to `variadic_var != nulltptr XNOR arguments_data.is_variadic`
            

            //Sets var pointers to first `min_arg_count + optional_arguments.size()` values of `inputs`
            //Sets var pointers to remaining optional arguments
            size_t argument_traverser = 0;
            Utils::inplace_tuple_slice_apply<MinArgCount>(
                [&argument_traverser, &inputs](auto&&... var_pointer) {
                    ((*var_pointer = *(inputs)[argument_traverser++]), ...);
                },
                [&inputs, this](auto&&... var_pointer) { //`this` is a pointer so it's captured by value
                    //All optional arguments that remain are handled here
                    size_t index = 0;
                    Object* temp_pointer;
                    //cout << MinArgCount + optional_arguments.size() - inputs.size() - 1 << " " << 6 - MinArgCount << "\n";
                    ((
                        [&var_pointer, this, &index, &inputs]() {
                            //cout << "INDEX " << index << " " << (MinArgCount + optional_arguments.size() - inputs.size() - 1) << "\n";
                            if (index < MinArgCount + optional_arguments.size() - inputs.size() - 1) {
                                *var_pointer = *inputs[index + MinArgCount];
                            }
                            else {
                                *var_pointer = (optional_arguments[index]());
                            }
                            ++index;
                        }()
                        //cout << (var_pointer = *(optional_arguments[index++ + MinArgCount + optional_arguments.size() - inputs.size() - 1]())) << "\n"
                    ), ...);
                    //Number of optional arguments explicitly entered = `inputs.size() - min_arg_count`
                }, std::tuple<Types*...>(outputs...)
            );

            //cout << "Here " << variadic_var << "\n";

            *variadic_var = std::vector<VariadicType>(); // vector should be initialised regardless of arguments entered

            //cout << "And Here\n";
            if (is_variadic && inputs.size() > MinArgCount + optional_arguments.size())
                assign_variadic_args<VariadicType>(MinArgCount + optional_arguments.size(), inputs, variadic_var);

            return true;
            cout << "Completed arg assignment\n";
            return true;
        }

        template <typename>
        void assign_varidic_args(size_t, std::vector<Object*>&, void*) {
        }

        template <typename VariadicType>
        void assign_variadic_args(size_t non_variadic_count, std::vector<Object*>& inputs, std::vector<VariadicType>* variadic_var) {
            //cout << "VC: " << inputs.size() - non_variadic_count << "\n";
            for (size_t variadic_argument_traverser = non_variadic_count; variadic_argument_traverser < inputs.size(); ++variadic_argument_traverser) {
                //cout << "VAT: " << variadic_argument_traverser << " " << *inputs[variadic_argument_traverser] << " is now ";
                variadic_var->push_back(*inputs[variadic_argument_traverser]);
                //variadic_var->push_back("test");
                //cout << (variadic_var->back()) << "\n";
            }

            cout << variadic_var->size() << "\n";
        }

        CppFunction(std::vector<std::function<Object()> > optional_arguments, bool is_variadic, std::function<Object*(CppFunction*, std::vector<UL::Object*>&)> func)
            : optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {
        }

        CppFunction(const CppFunction& from) //Copy constructor
            : optional_arguments(from.optional_arguments), is_variadic(from.is_variadic) {
        }

        Object* operator ()(std::vector<Object*> args) {
            return function(this, args);
        }
    };
    

    template <typename ...KeyValuePairT> // Implicit
    UserDefinedObject::UserDefinedObject(KeyValuePairT... names_and_values) {
        attributes = {names_and_values...};
    }

    Object* UserDefinedObject::get_attribute(std::string name) const {
        return attributes.at(name);
    }

    //*
    template <typename AlternateReturnT>
    AlternateReturnT UserDefinedObject::get_attribute(std::string name, AlternateReturnT alternate_value) const {
        cout << "Looking for name: " << name << ", alternate value: " << alternate_value << "\n";
        //if (has_attribute(name)) return (AlternateReturnT)(*get_attribute(name));
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
        delete attributes[name]; // Delete object of heap pointer
        attributes.erase(name); // Delete string and pointer (not what it points to though, hence the previous line)
    }


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

    auto Location::instances = std::unordered_set<Location*>(); //Construct set of locations

    Object::Object(std::nullptr_t, bool is_weak)
        : type(Types::null), is_weak(is_weak), reference_location(new Location(this)) {
        cout << "Constructed null object (" << this << ")\n";
    }

    Object::Object(double x, bool is_weak) //default argument
        : type(Types::number), is_weak(is_weak), reference_location(new Location(this)) {
        union_val.numerical_val = new double(x);
        cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(const char* string, bool is_weak) //Enter a string literal
        : type(Types::string), is_weak(is_weak), reference_location(new Location(this)) {
        union_val.string_val = new std::string(string);
        cout << "Constructed string object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(CppFunction* cpp_function, bool is_weak) //Enter a CppFunction heap pointer
        : type(Types::cpp_function), is_weak(is_weak), reference_location(new Location(this)) {
        cout << "Constructed C++ function object '" << *this << "' (" << this << ")\n";
        union_val.function_val = cpp_function; //cpp_function should be a heap pointer
    }

    Object::Object(ByteCodeFunction* bc_function, bool is_weak)
        : type(Types::bytecode_function), is_weak(is_weak), reference_location(new Location(this)) {
        union_val.bytecode_val = bc_function;
        cout << "Constructed bytecode function object '" << *this << "' (" << this << ")\n";
    }

    Object::Object(UserDefinedObject* cls, bool is_weak)
        : type(Types::user_defined_object), is_weak(is_weak), reference_location(new Location(this)) {
        union_val.udo_val = cls;
    }

    template <typename T>
    Object::Object(std::initializer_list<T> i_list, bool is_weak)
        : type(Types::list), is_weak(is_weak), reference_location(new Location(this)) {
        union_val.list_val = new std::vector<T>();
        union_val.list_val->insert(union_val.list_val->end(), i_list.begin(), i_list.end());
    }

    /*
    template <typename ... T>
    Object::Object(std::tuple<T...>, bool is_weak) {
    }
    */

    Object::Object(const Object* from, bool force_weak)
        : type(from->type), is_weak(force_weak || from->is_weak), reference_location(from->reference_location) {
        *reference_location += this;
        switch (type) {
            case Types::null:
                break;
            case Types::number:
                union_val.numerical_val = new double(*from->union_val.numerical_val);
                break;
            case Types::string:
                *union_val.string_val = *from->union_val.string_val;
                break;
            case Types::cpp_function:
                union_val.function_val = new CppFunction(*from->union_val.function_val);
                break;
            case Types::bytecode_function:
                *union_val.bytecode_val = *from->union_val.bytecode_val;
                break;
            case Types::list:
                *union_val.list_val = *from->union_val.list_val;
                break;
            default:
                cout << "How did I get here?\n";
        }
        cout << "Copying (" << (from->is_weak ? "weak->" : "strong->") << (is_weak ? "weak) " : "strong) ") << from << " (" << *from << ") to " << this << " (" << *this << ")\n";
    }

    Object::~Object() {
        *reference_location -= this; //Overloaded dereference on UL::Location object on heap
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
        }
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

    Object* Object::operator ()(std::vector<Object*> inputs) {
        return (*union_val.function_val)(inputs); //Pass inputs by reference
    }

    /*
    void Object::push_back(Object *object) {
        if (type == Types::list) {

        } else
            cerr << "Oh no!\n";
            //CompileTimeExc("")();
    }
    */

    /*
    namespace Classes {

        template <size_t DirectBaseClassCount>
        struct BuiltinClass {
            const std::array<BuiltinClass, DirectBaseClassCount> direct_bases;
            std::unordered_map<std::string>

            BuiltinClass(const std::array<BuiltinClass, DirectBaseClassCount> direct_bases)
                : direct_bases(direct_bases) {

            }
        };

        //auto Class = new BuiltinClass();

        //auto Object = new BuiltinClass();
    }*/

}

int main() {
    cout << "Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC\n";
    //cout.setstate(std::ios_base::failbit);

    //#include "tests/cpp_function.h"

    cout.clear();
    cout << "\nFINISHED\n";
    return 0;
} 