#ifndef CPP_FUNCTION_H
#define CPP_FUNCTION_H

class CppFunction {
	/*
	This object has ownership of 
	*/
public:
	using FuncT = std::function<ExternalObject(CppFunction*, std::vector<ExternalObject>&)>;

	static inline std::vector<ExternalObject> empty_eobject_vec;
	static inline std::vector<Types> 			empty_type_vec;

	//static std::string repr_arg_type_error(const CppFunction& cpp_function, std::vector<ExternalObject>& arguments);
	
	/*
	Functor class which represents a function written in C++
	A CppFunction accepts any number of arguments and input variables of C++ types
	*/

	//using std::initializer_list<std::reference_wrapper<ExternalObject>>

	std::vector<ExternalObject> optional_arguments;
	// Vector of lambdas returning UL::Object (implicitly converts return value) to stop objects from being constructed if they're not actually used
	// All additional arguments go to default values and then to the variable storing variadic arguments
	bool is_variadic = false; // Whether the function accepts any number of arguments beyond required + optional
	FuncT function = FuncT(); //The function object containing the code
	std::vector<Types> required_types; // Either empty or full, use Types::any if irrelevant
	Types variadic_type = Types::any; // Single type for every variadic element
	bool has_type_requirement = false; // n/nt for variadic_type, but for required_type
	//std::optional<ExternalObject> applied_argument = std::nullopt;

	CppFunction();
	CppFunction(
		std::vector<ExternalObject>&&, bool,
		FuncT,
		const std::vector<Types>& = empty_type_vec, Types = Types::any);
	CppFunction(const CppFunction&);
	CppFunction& operator =(const CppFunction&);
	CppFunction(CppFunction&&) noexcept;
	CppFunction& operator =(CppFunction&&) noexcept;
	~CppFunction();

	ExternalObject operator ()(std::vector<ExternalObject>&);
	ExternalObject operator ()();

	template <size_t MinArgCount, typename... TypesT>
	bool assign_args(std::vector<ExternalObject>&, TypesT*&...);
	template <size_t MinArgCount, typename VariadicType, typename... TypesT> bool assign_variadic_args(std::vector<ExternalObject>&, std::vector<VariadicType*>&, TypesT*&...);
	template <typename VariadicType> void assign_variadic_args(size_t, std::vector<ExternalObject>&, std::vector<VariadicType*>&);
};

bool operator ==(const CppFunction& f1, const CppFunction& f2) {
	return &f1 == &f2; // Cannot know if functions are the same, so just check if the internal pointers are equal
}

#endif
