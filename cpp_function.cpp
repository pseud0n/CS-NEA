#ifndef CPP_FUNCTION_CPP
#define CPP_FUNCTION_CPP

std::string CppFunction::repr_arg_type_error(const CppFunction& cpp_function, const std::vector<ExternalObject>& arguments) {
	std::stringstream error;
	//print("here");
	if (cpp_function.has_type_requirement) {
		std::vector<Types> v(arguments.size());
		std::transform(
			arguments.begin(), arguments.end(), v.begin(),
			[](const ExternalObject& o) -> Types { return o.type(); }
		);
		//print("has type requirement", arguments, v, cpp_function.required_types, cpp_function.required_types.size());
		for (size_t i = 0; i < cpp_function.required_types.size(); ++i) {
			if (cpp_function.required_types[i] != Types::any && cpp_function.required_types[i] != arguments[i].type()) {
				//print("Error!");
				error
					<< "For argument " << i
					<< ": acceptable type: " << cpp_function.required_types[i] 
					<< ", got type: " << arguments[i].type();
				break;
			}
		}
	}
	//print("Got to end");
	return error.str();
}

CppFunction::CppFunction()
	: optional_arguments(empty_eobject_vec), required_types(empty_type_vec) {
}

CppFunction::CppFunction(
	std::vector<ExternalObject> optional_arguments, bool is_variadic,
	FuncT func,
	const std::vector<Types>& required_types, Types variadic_type)
	: optional_arguments(optional_arguments), is_variadic(is_variadic), function(func),
		required_types(required_types), variadic_type(variadic_type),
		has_type_requirement(!required_types.empty()) {
	//print("CF CONSTRUCT", function ? "callable" : "not callable");
}

CppFunction::CppFunction(const CppFunction& from) // Copy constructor
	: optional_arguments(from.optional_arguments), is_variadic(from.is_variadic),
	  function(from.function), required_types(from.required_types),
	  has_type_requirement(from.has_type_requirement) {
}

CppFunction& CppFunction::operator =(const CppFunction& from) {
	optional_arguments = from.optional_arguments;
	is_variadic = from.is_variadic;
	function = from.function;
	required_types = from.required_types;
	has_type_requirement = from.has_type_requirement;
	return *this;
}

CppFunction& CppFunction::operator =(CppFunction&& from) noexcept {
	optional_arguments = std::move(from.optional_arguments);
	is_variadic = from.is_variadic;
	function = std::move(from.function);
	required_types = std::move(from.required_types);
	has_type_requirement = std::move(from.has_type_requirement);
	return *this;
}

CppFunction::~CppFunction() {
	cout << "Destructing CppFunction " << this << '\n';
	/*
	for (ExternalObject object : optional_arguments)
		delete object; // Optional arguments stored on heap and automatically deleted
	*/
}

ExternalObject CppFunction::operator ()(const std::vector<ExternalObject>& args) const {
	return function(this, args);
}

ExternalObject CppFunction::operator ()() const { // Simple function which takes no arguments
	return function(this, empty_eobject_vec);
}

//ExternalObject operator ()(ExternalObject self, ) 
template <size_t MinArgCount, typename... TypesT>
bool CppFunction::assign_args(const std::vector<ExternalObject>& inputs, TypesT&... outputs)const {
	/*
	This function is used when assigning arguments in a non-variadic function.
	It takes the inputs and outputs and simply maps them (also taking optional arguments into account)
	The OPTRs passed in are treated as objects since we don't need to reference count here
	*/

	//print("Assigning args:", inputs.size(), MinArgCount);
	//print(sizeof...(outputs),  __PRETTY_FUNCTION__);

	std::string error = repr_arg_count_error(MinArgCount, optional_arguments.size(), is_variadic, inputs.size());
	//print("Error:", error, error.empty());

	if (error.empty()) {
		error = repr_arg_type_error(*this, inputs);
		//print("Set error");
		if (!error.empty()) {
			print(error);
			return false;
		}
	} else {
		print(error);
		return false;
	}
	size_t argument_traverser = 0;

	//#define CURRENT_TYPE std::tuple_element_t<MinArgCount, std::tuple<TypesT&...>>
	#define CURRENT_TYPE decltype(var_pointer)
	// I wouldn't want to make you read the macro's value over and over (it's for your own good, I promise :p )

	//print("yo");

	Utils::inplace_tuple_slice_apply<MinArgCount>(
		[&argument_traverser, &inputs](auto&... var_pointer) {
			((var_pointer = inputs[argument_traverser++].cast<CURRENT_TYPE>()), ...); // Assigns each variable the current (required) entered argument
		},//std::tuple_element_t<N, std::tuple<Ts&...>>
		[&inputs, this](auto&... var_pointer) { // `this` is a pointer so it's captured by value
			// All optional arguments that remain are handled here (arguments beyond minimum)
			size_t index = 0;
			//print("vectors:", (inputs), (optional_arguments));
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
						var_pointer = inputs[index + MinArgCount].cast<CURRENT_TYPE>(); // Set variable to explicitly passed argument
					}
					else { // If this optional argument was not passed and instead is set to a default value
						print("Implicit", index);
						var_pointer = optional_arguments[index].cast<CURRENT_TYPE>();
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
bool CppFunction::assign_variadic_args(const std::vector<ExternalObject>& inputs, std::vector<VariadicType>* variadic_var, TypesT&... outputs) const {
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
void CppFunction::assign_variadic_args(size_t non_variadic_count, const std::vector<ExternalObject>& inputs, std::vector<VariadicType>* variadic_var) const {
	//cout << "VC: " << inputs.size() - non_variadic_count << "\n";
	for (size_t variadic_argument_traverser = non_variadic_count; variadic_argument_traverser < inputs.size(); ++variadic_argument_traverser) {
		//cout << "VAT: " << variadic_argument_traverser << " " << *inputs[variadic_argument_traverser] << " is now ";
		variadic_var->push_back(inputs[variadic_argument_traverser].cast<VariadicType>());
		//variadic_var->push_back("test");
		//cout << (variadic_var->back()) << "\n";
	}

	cout << variadic_var->size() << "\n";
}

#endif