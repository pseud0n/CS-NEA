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

#define MIN_CACHED_INTEGER_VALUE -5
#define MAX_CACHED_INTEGER_VALUE 10

#include "object_pointer.h"

#define OPTR UL::ObjectPointer


namespace UL { std::array<OPTR, 1 + MAX_CACHED_INTEGER_VALUE - MIN_CACHED_INTEGER_VALUE> cached_numbers; }

#include "forward_decl.h" //Header file to forward-declare object
#include "exceptions.h"
#include "printer.h"

#define OBJ_RET(x) [](){ return OPTR(x) }
//If an object is optional, no need to construct it unless it is actually used
//Assumes that return type of lambda is a UL::Object, so this will be converted

#define DY_LMBD [](UL::CppFunction* argument_data, const std::vector<OPTR>& arguments)
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
            std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{ });
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
    namespace Tracker {
        //std::unordered_map<Object*, unsigned int> remaining_objects = { };

        std::vector<Object*> stored_objects = { };
        std::vector<unsigned int> object_counts = { };

        void add_pair(Object* object) {
            stored_objects.push_back(object);
            object_counts.push_back(1);
        }

        void remove_pair(Object* object) {
            stored_objects.pop_back();
            object_counts.pop_back();
        }

        void repr() {
            cout << "{";
            for (size_t i = 0; i < stored_objects.size(); ++i) {
                if (i != 0) cout << ", ";
                cout << stored_objects[i] << " (" << *stored_objects[i] << ", " << (stored_objects[i]->is_const ? "const" : "non-const") << ") : " << object_counts[i];
            }
            cout << "}\n";
        }
    }

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
        std::function<OPTR(CppFunction*, const std::vector<OPTR>&)> function; //The function object containing the code

        template <size_t MinArgCount, typename VariadicType, typename... TypesT>
        bool assign_args(const std::vector<OPTR>& inputs, std::vector<VariadicType>* variadic_var, TypesT&... outputs) {

        	//cout << "Assigning args:\n";
        	//cout << sizeof...(outputs) << "\n";

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

            #define CURRENT_TYPE std::tuple_element_t<MinArgCount, std::tuple<TypesT&...>>
            // I wouldn't want to make you read the macro's value over and over (it's for your own good, I promise :p )

            Utils::inplace_tuple_slice_apply<MinArgCount>(
				[&argument_traverser, &inputs](auto&... var_pointer) {
					((var_pointer = inputs[argument_traverser++].cast<CURRENT_TYPE>()), ...); // Assigns each variable the current (required) entered argument
				},//std::tuple_element_t<N, std::tuple<Ts&...>>
            	[&inputs, this](auto&... var_pointer) { // `this` is a pointer so it's captured by value
					// All optional arguments that remain are handled here
					// Takes 6 integer references
					size_t index = 0;
					((
						[&var_pointer, this, &index, &inputs]() { // Lambda in a lambda!
							//cout << "INDEX " << index << " " << (MinArgCount + optional_arguments.size() - inputs.size() - 1) << "\n";
							if (index < MinArgCount + optional_arguments.size() - inputs.size() - 1) { // If this optional argument was passed explicitly
								var_pointer = inputs[index + MinArgCount].cast<CURRENT_TYPE>(); // Set pointer to
							}
							else { // If this optional argument was not passed and instead is set to a default value
								var_pointer = optional_arguments[index].cast<CURRENT_TYPE>();
							}
							++index;
						}()
						//cout << (var_pointer = *(optional_arguments[index++ + MinArgCount + optional_arguments.size() - inputs.size() - 1]())) << "\n"
					), ...);
					//Number of optional arguments explicitly entered = `inputs.size() - min_arg_count`
				},
				std::tuple<TypesT&...>(outputs...)
			); // Note that the tuple container is able to store references

            #undef CURRENT_TYPE

            //cout << "Here " << variadic_var << "\n";

            *variadic_var = std::vector<VariadicType>(); // vector should be initialised regardless of arguments entered

            //cout << "And Here\n";
            //cout << is_variadic << " " << inputs.size() << " " << MinArgCount << " " << optional_arguments.size();
            if (is_variadic && inputs.size() > MinArgCount + optional_arguments.size())
                assign_variadic_args<VariadicType>(MinArgCount + optional_arguments.size(), inputs, variadic_var);

            return true;
        }

        template <typename>
        void assign_varidic_args(size_t, const std::vector<OPTR>&, void*) {
        }

        template <typename VariadicType>
        void assign_variadic_args(size_t non_variadic_count, const std::vector<OPTR>& inputs, std::vector<VariadicType>* variadic_var) {
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

        CppFunction(std::vector<OPTR> optional_arguments, bool is_variadic, std::function<OPTR(CppFunction*, const std::vector<OPTR>&)> func)
            : optional_arguments(optional_arguments), is_variadic(is_variadic), function(func) {

        }

        CppFunction(const CppFunction& from) //Copy constructor
            : optional_arguments(from.optional_arguments), is_variadic(from.is_variadic) {
        }

        ~CppFunction() {
            cout << "Destructing CppFunction " << this << "\n";
        }

        OPTR operator ()(std::vector<OPTR> args) {
            return function(this, args);
        }
        
        OPTR operator ()() {
            return function(this, { });
        }
    };

    /*
    template <typename... KeyValuePairT> // KeyValuePairT: std::pair<std::string, ObjectPointer>
    UserDefinedObject::UserDefinedObject(KeyValuePairT... names_and_values)
    	: attributes(names_and_values...) {
    }
    */

    ObjectPointer::ObjectPointer()
        : object_ptr(0), is_weak(false) {
        // Invalid in this state - do not use!
        // Default constructor required in some cases
    }

    ObjectPointer::ObjectPointer(Object* object_ptr, bool make_const)
        : object_ptr(object_ptr), is_weak(false) {
        /*
        make_const     object_ptr->is_const    result
        false           false                   false
        false           true                    true
        true            false                   true
        true            true                    true
        */
        //if (is_weak) object_ptr->reference_count = 0;
        object_ptr->is_const = make_const || object_ptr->is_const;
        cout << "Created ObjectPointer " << this << " (" << (is_weak? "weak" : "strong") << ", " << (object_ptr->is_const ? "const" : "non-const") <<  ")\n";
    }

    #define NUMERIC_CONSTRUCTOR(type) /* Input can be negative - underflow is not a problem */                                                  \
        ObjectPointer::ObjectPointer(type number, bool make_const) {                                                                           \
        cout << "Constructing OPTR from " << #type << " " << number << "\n";                                                                \
        /*this->is_weak = is_weak; */                                                                                                           \
        object_ptr->is_const = make_const || object_ptr->is_const; \
        if (number >= MIN_CACHED_INTEGER_VALUE && number <= MAX_CACHED_INTEGER_VALUE) { /* Has already been cached */                       \
            /* e.g. number = -5.0 & MIN_CACHED_INTEGER_VALUE = -5 then index = 0 */                                                         \
            object_ptr = cached_numbers[static_cast<size_t>(number) - MIN_CACHED_INTEGER_VALUE].object_ptr;                                              \
            /* size_t is unsigned so cast will cause an underflow if number < 0 (but it doesn't matter since it will then overflow) */      \
            ++*object_ptr; /* Increment number of references for cached integer being pointed to */                                         \
        } else                                                                                                                              \
            object_ptr = new Object(number);                                                                                                \
        }


    //ObjectPointer::ObjectPointer(double number, bool is_weak) { NUMERIC_CONSTRUCTOR(double) }
    //ObjectPointer::ObjectPointer(int number, bool is_weak) { NUMERIC_CONSTRUCTOR(int) }

    NUMERIC_CONSTRUCTOR(int)
    NUMERIC_CONSTRUCTOR(double)

    #undef NUMERIC_CONSTRUCTOR

    template <typename ConstructorT>
    ObjectPointer::ObjectPointer(ConstructorT construct_from, bool make_const)
        : is_weak(false) {
        object_ptr = new Object(construct_from, make_const);
        cout << "Constructing OPTR from arbitrary type (" << object_ptr << ")\n";   
        //if (is_weak) object_ptr->reference_count = 0;
    }

    ObjectPointer::~ObjectPointer() {
        cout << "Deleting (" << r_optr_strength(*this) << ") OPTR " << this << " wrapping " << object_ptr << " (" << *object_ptr << ") with refcount " << object_ptr->reference_count;
        if (!is_weak) {
            cout << " to " << object_ptr->reference_count - 1 << "\n";
            --*object_ptr;
        } else {
            cout << "\n";
        }
    }

    ObjectPointer::ObjectPointer(const ObjectPointer& from, bool force_strong) // Copy constructor
        : object_ptr(from.object_ptr), is_weak(force_strong || from.is_weak || from.object_ptr->is_const) {
        cout << "Constructing OPTR " << this << " (" << r_optr_strength(*this) << ") from OPTR " << &from << " (" << r_optr_strength(from) << ", " << r_optr_constness(from.object_ptr) << ")\n";
        /*
        force_strong    from.is_weak    from.object_ptr->is_const   is_weak
        false           false           false                       false
        false           false           true                        true
        false           true            false                       true
        true            false           false                       true
        true            false           true                        true
        true            true            false                       true
        true            true            true                        true
        */
        if (!is_weak) ++*object_ptr; // A new strong reference
        // If the underlying Object is
    }

    //ObjectPointer::ObjectPointer operator ()(std::vector<OPTR>&);

    OPTR ObjectPointer::operator ()(const std::vector<OPTR>& args) {
        return (*object_ptr->union_val.function_val)(args);
    }

    OPTR ObjectPointer::operator ()() {
        return (*object_ptr->union_val.function_val)();
    }

    template <typename ConstructFromT>
    void ObjectPointer::create_from_blank(ConstructFromT construct_from, bool is_weak) {
        object_ptr = new Object(construct_from);
        this->is_weak = is_weak;
    }

    template <typename CastT>
    typename std::remove_reference<CastT>::type ObjectPointer::cast() const {
        return static_cast<typename std::remove_reference<CastT>::type>(*object_ptr);
    }

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

    Object::Object(std::nullptr_t, bool make_const) // Initially weak: reference count starts at 1 still
    // Using (unsigned int)!is_const as the initial value for reference_count would not work since we require that is is finally decremented to zero and not overflowed
        : type(Types::null), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
        cout << "Constructed null object (" << this << ")\n";
        Tracker::add_pair(this);
    }

    Object::Object(double x, bool make_const) //default argument
        : type(Types::number), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
        union_val.numerical_val = new double(x);
        cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
        Tracker::add_pair(this);
    }

    Object::Object(int x, bool make_const) //default argument
		: type(Types::number), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
		union_val.numerical_val = new double(static_cast<double>(x));
		cout << "Constructed numerical object '" << *this << "' (" << this << ")\n";
        Tracker::add_pair(this);
    }

    Object::Object(const char* string, bool make_const) //Enter a string literal
        : type(Types::string), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
        union_val.string_val = new std::string(string);
        cout << "Constructed string object '" << *this << "' (" << this << ")\n";
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
    Object::Object(std::initializer_list<T> i_list, bool make_const)
        : type(Types::list), /*is_weak(is_weak),*/ is_const(make_const), reference_count(1) {
        union_val.list_val = new std::vector<T>();
        union_val.list_val->insert(union_val.list_val->end(), i_list.begin(), i_list.end());
        Tracker::add_pair(this);
    }

    /*
    template <typename ... T>
    Object::Object(std::tuple<T...>) {
    }
    */

    Object::Object(const Object* from /*bool force_weak*/, bool make_const)
        : type(from->type), /*is_weak(force_weak || from->is_weak),*/ is_const(make_const), reference_count(1) {
        // Copy made; should not share same location
        Tracker::add_pair(this);
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
        cout << "\tDestructed object " << this << " (" << *this << ")\n";
        Tracker::remove_pair(this);
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
        ++Tracker::object_counts[std::distance(Tracker::stored_objects.begin(), std::find(Tracker::stored_objects.begin(), Tracker::stored_objects.end(), this))];
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
        cout << "DECREF " << this << " (" << *this << ") to " << reference_count - 1 << "\n";
    	if (reference_count == 1) delete this; // If the last reference is being removed
    	else {
            --reference_count;
            --Tracker::object_counts[std::distance(Tracker::stored_objects.begin(), std::find(Tracker::stored_objects.begin(), Tracker::stored_objects.end(), this))];
        }
        
    }

    /*
    Object::operator int() const {
        return *union_val.numerical_val;
    }
    */

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

        //template </*size_t DirectBaseClassCount, typename... PairPlaceholderT*/>
        struct BuiltinClass {
            const std::vector<BuiltinClass*> direct_bases; // Can not change
            std::unordered_map<std::string, OPTR> attributes; // Should not shrink or grow by values may change

            BuiltinClass(std::vector<BuiltinClass*> direct_bases, std::unordered_map<std::string, OPTR> kv /*PairPlaceholderT... names_and_values*/)
                : direct_bases(direct_bases), attributes(kv) {
                //attributes = { names_and_values... };
            }

            BuiltinClass(const BuiltinClass&) = delete;

            ~BuiltinClass() { // Not done by language, but potentially done by compiler directly
                // `attributes` and `bases` cleaned up automatically

                // Commented out under assumption that keys are stored on the stack
                //for (auto& pair : attributes) delete pair.second;
                cout << "Called deleter\n";
                UL::Tracker::repr();
            }
        };

        /*
        A builtin type is interfaced through an OPTR and can be treated as one.
        When creating a user-defined type, a built-in class can be inherited from.
        It will have to be treated as a UL object in that instance.
        Built-in types inheriting from other types will inherit from other BuiltinClass objects
        */

        //*

        #define BLANK_RETURN_FN(return_val) OPTR(new UL::CppFunction({ }, false, DY_LMBD{ return OPTR(return_val); }), true)
        // The functions should be weak references since they have no reason to be tracked
        /*
        BuiltinClass object(
            { }, // No super classes - this is at the top of the hierarchy
            {
                //{ "init", BLANK_RETURN_FN(nullptr) },
                { "type", BLANK_RETURN_FN("Object") }
            }
        );
        
        

        BuiltinClass null(
            { &object },
            {
                { "type", BLANK_RETURN_FN("Number") }
            }
        );

        BuiltinClass number(
            { &object },
            {
                { "type", BLANK_RETURN_FN("NullType") }
            }
        );

        BuiltinClass string(
            { &object },
            {
                { "type", BLANK_RETURN_FN("String") }
            }
        );
    */
        #undef BLANK_RETURN_FN
    }

}

int main() {
	cout << "\n┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n┃ Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC ┃\n┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";
	
    {

//#include "pre_decl.h"
//#include "tests/cpp_function.h"

    UL::Tracker::repr();

    //OPTR f(new UL::CppFunction({ }, false, DY_LMBD{ return OPTR("It's a me"); }));

    //cout << f() << "\n";

    OPTR x("hello", true);
    OPTR y(x);
    OPTR z(x);


    UL::Tracker::repr();

    cout << "\nEXITED LOCAL SCOPE\n";
	}

    UL::Tracker::repr();

    //cout.clear();
    cout << "\nEXITED PROGRAM\n";
    return 0;
}
