#ifndef FORWARD_DECL_H
#define FORWARD_DECL_H
//INCLUDED(FORWARD_DECL)
//Included("forward_decl");

#define OPTR UL::ObjectPointer

namespace UL {

    enum class Types {
        blank,
        /*
        Instance of Object; no special attributes
        */
        null,
        /*
        The equivalent of NULL/None/whatever.
        This type is returned by default from functions
        */
		number,
		/*
		The default number type stores an int
		TODO Make it store big float
		Each integer or float literal is converted to this type
		*/
		string,
		/*
        Stores a sequence of (wide) characters
		The value is not constant and all operations are in-place
		 */
		cpp_function,
		/*
		Does not have an associated union type
		While it has a 'call' method, it is also directly callable
		(otherwise there would be infinite recursion)
		It doesn't need to know argument type but it needs to know number of arguments
		Takes a list of argument names and an optional name to store all extra arguments in list form
		TODO Add keyword arguments (not separate syntax)
		There are 2 types of functions, depending on the context
		Functions that you write in Unnamed Language
		 	They take an array of Object
		 	The code within is generated by the UL interpreter
		 	They are less efficient since the interpreter doesn't know what you'll do with the variables
		 	Likely less human-readable
		Functions that I write in C++
		 	They take an array of WrappedUnionObject
				This type is a non-reference-counted version of the standard Object
				It acts as an intermediate object between the types that its union can store and an Object
				It can be efficiently converted to any type that it stores
				It can also be converted to an Object by adding a Location object and classifying it as weak or strong
			They are more efficient since you can interpret arguments as a variety of other types for efficiency
			Function returns an Object
		*/
        bytecode_function,

		pair,
		/*
		Stores 2 objects - a key and value
		*/

		array,
		/*
		This type means that the object's union value is an array
        An array cannot be modified (although its value is not known a compile time)
        A vector has fast random access since it is stored contiguously
		*/

        list,
        /*
        Implemented as a doubly-linked list for iteration in both directions.
        Does not support fast random access but supports fast bidirectional iteration
        Iterating by incrementing a variable and indexing is not recommended
        */

        dictionary,
        /*
        Uses key-value pairs and hashes the keys
        Keys must be hashable types (all immutable types are hashable)
		Is constructed from a list or array of pairs
        */
        
        user_defined_object
	};

    class Object;
    struct ByteCodeFunction;
    struct CppFunction;
    struct UserDefinedObject;
	class Pair;

	namespace UnionAliases {
		typedef unsigned char					BlankT;
		typedef bmp::cpp_int					NumT;
		typedef const std::pair<OPTR, OPTR>		PairT;
		typedef const std::vector<OPTR>			ArrayT;
		typedef std::list<OPTR>					ListT;
		typedef std::unordered_map<OPTR, OPTR> 	DictT;
	}

    union ObjectUnion {
        // Note that there is no 'null type' since it's a constant
		UnionAliases::BlankT	*blank_val;		//
        UnionAliases::NumT		*numerical_val; // boost::multiprecision::cpp_int
        std::string 			*string_val; 	// std::string pointer
        CppFunction 			*function_val; 	// C++ function
        ByteCodeFunction 		*bytecode_val;	// Bytecode function, which stores a start val & number of lines
		UnionAliases::PairT 	*pair_val;		// 2 objects
        UnionAliases::ArrayT	*vector_val; 	// Vector of objects
		UnionAliases::ListT		*list_val;		// Linked list
		UnionAliases::DictT 	*dict_val;		// Dictionary
        UserDefinedObject		*udo_val;
    };

    #include "object.h"

    struct UserDefinedObject {
        std::unordered_map<std::string, OPTR> attributes;
        //template <typename... KeyValuePairT> UserDefinedObject(KeyValuePairT...);
        UserDefinedObject(std::unordered_map<std::string, OPTR>);
        OPTR get_attribute(std::string) const;
        template <typename AlternateReturnT> AlternateReturnT get_attribute(std::string name, AlternateReturnT alternate_value) const;
        //std::string get_attribute(std::string name, std::string alternate_value);
        bool has_attribute(std::string name) const;
        void delete_attribute(std::string name);
    };
}

//std::ostream& operator <<(std::ostream& stream, const UL::Object& object);
//std::ostream& operator <<(std::ostream&, const UL::Types);

#endif
