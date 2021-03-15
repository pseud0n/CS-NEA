#ifndef CPP_FUNCTION_CPP
#define CPP_FUNCTION_CPP

#include "cpp_function.h"

CppFunction::CppFunction()
	: optional_arguments(empty_eobject_vec), required_types(empty_type_vec) {
}

CppFunction::CppFunction(
	std::vector<ExternalObject>&& optional_arguments,
	bool is_variadic, FuncT func,
	const std::vector<Types>& required_types, Types variadic_type)
	: optional_arguments(std::forward<std::vector<ExternalObject>>(optional_arguments)), is_variadic(is_variadic), function(func),
		required_types(required_types), variadic_type(variadic_type),
		has_type_requirement(!required_types.empty()) {
	clog << "Constructing CppFunction " << this << '\n';
	//print("CF CONSTRUCT", function ? "callable" : "not callable");
}

CppFunction::CppFunction(const CppFunction& from) // Copy constructor
	: optional_arguments(from.optional_arguments), is_variadic(from.is_variadic),
	  function(from.function), required_types(from.required_types),
	  has_type_requirement(from.has_type_requirement) {
	print("Copy-constructing", &from, "to", this);
}

CppFunction& CppFunction::operator =(const CppFunction& from) {
	print("Copy-assigning", &from, "to", this);
	optional_arguments = from.optional_arguments;
	is_variadic = from.is_variadic;
	function = from.function;
	required_types = from.required_types;
	has_type_requirement = from.has_type_requirement;
	return *this;
}

CppFunction::CppFunction(CppFunction&& from) noexcept {
	print("Move-constructing", &from, "to", this);
	optional_arguments = std::move(from.optional_arguments);
	is_variadic = from.is_variadic;
	function = std::move(from.function);
	required_types = std::move(from.required_types);
	has_type_requirement = from.has_type_requirement;
}

CppFunction& CppFunction::operator =(CppFunction&& from) noexcept {
	print("Move-constructing", &from, "to", this);
	optional_arguments = std::move(from.optional_arguments);
	is_variadic = from.is_variadic;
	function = std::move(from.function);
	required_types = std::move(from.required_types);
	has_type_requirement = from.has_type_requirement;
	return *this;
}

CppFunction::~CppFunction() {
	clog << "Destructing CppFunction " << this << '\n';
	function.~FuncT();
	print("Data:", optional_arguments, is_variadic, required_types, variadic_type, has_type_requirement);
	/*
	for (ExternalObject object : optional_arguments)
		delete object; // Optional arguments stored on heap and automatically deleted
	*/
}

ExternalObject CppFunction::operator ()(std::vector<ExternalObject>& args) {
	print("Calling with", args);
	return function(this, args);
}

ExternalObject CppFunction::operator ()() { // Simple function which takes no arguments
	return function(this, empty_eobject_vec);
}

//ExternalObject operator ()(ExternalObject self, ) 
template <size_t min_arg_count, typename... TypesT>
bool CppFunction::assign_args(std::vector<ExternalObject>& inputs, TypesT*&... outputs) {
	/*
	This function is used when assigning arguments in a non-variadic function.
	It takes the inputs and outputs and simply maps them (also taking optional arguments into account)
	The OPTRs passed in are treated as objects since we don't need to reference count here
	*/

	print("Assigning args:", inputs, inputs.size(), min_arg_count);
	//print(sizeof...(outputs),  __PRETTY_FUNCTION__);
    

	std::string error = repr_arg_count_error(min_arg_count, optional_arguments.size(), is_variadic, inputs.size());
	//print("Error:", error, error.empty());

	if (error.empty()) {
		if (has_type_requirement) {
			error = repr_arg_type_error(required_types, inputs);
			if (!error.empty()) {
				THROW_ERROR(error);
				return false;
			}
		}
	} else {
		THROW_ERROR(error);
		return false;
	}
	size_t argument_traverser = 0;
	
	//#define CURRENT_TYPE std::tuple_element_t<min_arg_count, std::tuple<TypesT*...>>
#define CURRENT_TYPE std::remove_pointer_t<std::remove_reference_t<decltype(var_pointer)>>
	// I wouldn't want to make you read the macro's value over and over (it's for your own good, I promise :p )

	//print("yo");

	Utils::inplace_tuple_slice_apply<min_arg_count>(
		[&argument_traverser, &inputs](auto*&... var_pointer) {
			((
			  [&](){
                print("Current input:", inputs[argument_traverser]);
				//if constexpr(std::is_same_v<ExternalObject, CURRENT_TYPE>)
				//	var_pointer = inputs[argument_traverser++];    
				// else
				var_pointer = &inputs[argument_traverser++].get_mut<CURRENT_TYPE>();
				print("Successful assignment");
              }()
            ), ...); // Assigns each variable the current (required) entered argument

		},//std::tuple_element_t<N, std::tuple<Ts&...>>
		[&inputs, this](auto*&... var_pointer) { // `this` is a pointer so it's captured by value
			// All optional arguments that remain are handled here (arguments beyond minimum)
			size_t index = 0;
			print("vectors:", (inputs), (optional_arguments));
			/*
			e.g.	min_arg_count = 3, inputs = {13, 14, 15, 16} & optional_arguments = {5, 6}
					then the resulting values should be {13, 14, 15, 16, 6}
			*/

			((
				[&](){
					//print("> ", index, inputs.size(), min_arg_count, min_arg_count + optional_arguments.size() - inputs.size());
					if (index >= min_arg_count + optional_arguments.size() - inputs.size() ) { // If this optional argument was passed explicitly, i.e. not left as default
						//print(index, min_arg_count + optional_arguments.size(), inputs.size());
						print("Explicit:", index, inputs[index + min_arg_count], &inputs[index + min_arg_count]);
						var_pointer = &inputs[index + min_arg_count].get_mut<CURRENT_TYPE>(); // Set variable to explicitly passed argument
					}
					else { // If this optional argument was not passed and instead is set to a default value
						print("Implicit", index, optional_arguments[index + min_arg_count]);
						var_pointer = &optional_arguments[index].get_mut<CURRENT_TYPE>();
					}
					++index;
				}()
			), ...);
		},
		std::tuple<TypesT*&...>(outputs...)
	); // Note that the tuple container is able to store references

#undef CURRENT_TYPE
	print("assign_args successful");

	return true;
}

template <size_t min_arg_count, typename VariadicType, typename... TypesT>
bool CppFunction::assign_variadic_args(std::vector<ExternalObject>& inputs, std::vector<VariadicType*>& variadic_var, TypesT*&... outputs) {
	//*variadic_var = std::vector<VariadicType>(); // vector should be initialised regardless of arguments entered

	//clog << "And Here\n";
	//clog << is_variadic << " " << inputs.size() << " " << min_arg_count << " " << optional_arguments.size();
	if (inputs.size() > min_arg_count + optional_arguments.size())
		// At least one variadic argument
		assign_variadic_args<VariadicType>(min_arg_count + optional_arguments.size(), inputs, variadic_var);
	//else
		
	return assign_args<min_arg_count>(inputs, outputs...);
	// Standard assigment
}

template <typename VariadicType>
void CppFunction::assign_variadic_args(size_t non_variadic_count, std::vector<ExternalObject>& inputs, std::vector<VariadicType*>& variadic_var) {
	clog << "VC: " << inputs.size() - non_variadic_count << "\n";
	bool has_variadic_type_requirement = variadic_type != Types::any;
	for (size_t variadic_argument_traverser = non_variadic_count; variadic_argument_traverser < inputs.size(); ++variadic_argument_traverser) {
		print(variadic_argument_traverser, inputs[variadic_argument_traverser]);
		//clog << "VAT: " << variadic_argument_traverser << " " << inputs[variadic_argument_traverser] << " is now ";
		//if constexpr (std::is_same_v<VariadicType, ExternalObject>)
		//	variadic_var->push_back(inputs[variadic_argument_traverser]);
		//else
		if (has_variadic_type_requirement && inputs[variadic_argument_traverser].type() != variadic_type) {
			std::ostringstream oss;
			oss << "Variadic arguments must have type '" << variadic_type << "' not '" << inputs[variadic_argument_traverser].type() << "'";
			THROW_ERROR(oss.str())
		} else {
			variadic_var.push_back(&inputs[variadic_argument_traverser].get_mut<VariadicType>());
			// Gets pointer to type stored internally but types must be
			// ExternalObject* if different
		}

		//variadic_var->push_back("test");
		//clog << (variadic_var->back()) << "\n";
	}
	clog << "Nice!\n";

	//clog << variadic_var->size() << "\n";
}

#define LAZY_OBJECT(obj) \
	CppFunction( \
		CppFunction::empty_eobject_vec, false, UL_LMBD { \
			argument_data->assign_args<0>(arguments); \
			return obj; \
		} \
	)

#endif
