#ifndef LOOKUP_H
#define LOOKUP_H

//clog << "a is " << setw(2) << setfill('0') << hex << (int) a ; 

/*
Suppose we attempted access of an attribute for an arbitrary object.

For a custom object, it looks in its dictionary to find the attribute and
returns if it it's present. For a built-in object, there is no dictionary, so it
uses a separate lookup which uses the type to determine if an attribute is
present.

Built-in objects don't all have their own dictionary; this would waste
resources. For example, a string stores its length in the object used in the
template argument for InternalObject: std::string. This object stores the string
and other information about the string itself; the data is encapsulated within
the object used as a template. If we wanted to create a dictionary, the string's
attributes would be used to generate a dictionary on-the-fly.

When we do
	myStr = "this is a string"
	print(myStr.Len)
	print(myStr.GetAttr("Len"))
	print(myStr.Dict("Len"))
we are treating myStr as if it has its own dictionary, which it does not.
On 2, it recognises that myStr is a string so it looks in a map of maps to find
the method.
It can't use a constexpr hash function because if there was a collision, then it
would not know and the resulting jump table may end up with an inaccessible
label (and there is no guarantee that hash results are consistent across runs).
Instead, there is a 2D unordered_map used.

An object's type is stored in 2 different ways - an enum is used for built-in
types and does not refer to an existing object. Members are obtained from these
objects by using a lookup table. Built-in objects do not have associated meta
class. Most's meta classes would only require a Call method which returns a new
object with the associated type. The default Class object has a method which
returns a new object based on what is passed and there is an associated
constructor for all built-in types. When you call String.Call(num), you call
Class.Call(String, num). The function then switches to the corresponding branch,
then to the corresponding constructor.
*/

print("Empty");
enum_to_class = {
	{ Types::any, &Classes::object },
	{ Types::null_type, &Classes::null_type },
	{ Types::number, &Classes::integer },
	{ Types::string, &Classes::string },
	{ Types::boolean, &Classes::boolean },
	{ Types::cpp_function, &Classes::cpp_function },
	{ Types::cpp_function_view, &Classes::cpp_function_view },
	{ Types::pair, &Classes::pair },
	{ Types::array, &Classes::array },
	{ Types::dictionary, &Classes::dict },
	{ Types::base_exception, &Classes::base_exception },
	{ Types::code_block, &Classes::code_block },
	{ Types::if_chain, &Classes::if_chain },
	{ Types::generic_singleton, &Classes::generic_singleton },
	{ Types::custom, &Classes::custom }
};
// E.g. Types::string maps to &Classes::string

#define ADD_BUILTIN(name) all_builtins[name] = MKCSTM("Name", name##s);

ADD_BUILTIN("Object")
ADD_BUILTIN("Class")
ADD_BUILTIN("NullType")
ADD_BUILTIN("Integer")
ADD_BUILTIN("String")
ADD_BUILTIN("Boolean")
ADD_BUILTIN("Function")
ADD_BUILTIN("FunctionView")
ADD_BUILTIN("Pair")
ADD_BUILTIN("Array")
ADD_BUILTIN("Dictionary")
ADD_BUILTIN("CodeBlock")
ADD_BUILTIN("If")
ADD_BUILTIN("Singleton")
ADD_BUILTIN("ExcBase")

#define EMPLACE(name) class_ptr->emplace(name, std::move(o));

#define REASSIGN(type) \
	print("Me:", Classes::type); \
	class_ptr = &Classes::type.get<Aliases::CustomT>();\
	class_ptr->try_emplace("Type", Classes::cls, true);

#define ADD_BASIC_MRO(type) \
	class_ptr->try_emplace("MRO", MKRRY_W(Classes::type, Classes::object));

#define GET_EXC(name) all_builtins.at(name).get<Aliases::BaseExceptionT>()
//#define MAKE_EXC(name, message) []() { auto args = make_eo_vec(message); all_builtins.at(name)(make_eo_vec(message)); }()

print("Setting up members");

std::set<void const*> original_builtins;
// InternalObject pointers referenced to by builtin objects
for (const auto& pair : all_builtins) {
	original_builtins.insert(&pair.second);
}

all_builtins["True"] = true;
all_builtins["False"] = false;
all_builtins["Null"] = nullptr;
all_builtins["N"] = "\n"s;
all_builtins["S"] = " "s;
all_builtins["End"] = Singleton();

Aliases::CustomT *class_ptr;
ExternalObject o;

// OBJECT

REASSIGN(object)
class_ptr->try_emplace("MRO", MKRRY_W(Classes::object)); // Array of weak references
print("class_ptr:", Classes::cls, *class_ptr);

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr;
		argument_data->assign_args<1>(arguments, self);
		print("DEEP COPYING", *self);
		ExternalObject eo = ExternalObject::deep_copy_of(*self);
		return eo;
	}, CppFunction::empty_type_vec
); EMPLACE("Copy")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		Types self_type = self->type(), other_type = other->type();
		void *equals_method = self->get_attr("_Eq");
		std::vector<ExternalObject> args;
		if (self->type() == Types::custom) {
			if (equals_method) {
				return ExternalObject(equals_method)(args = make_eo_vec(*other));
			}
		} else if (other->type() == Types::custom) {
			equals_method = other->get_attr("_Eq");
			if (equals_method) {
				return ExternalObject(equals_method)(args = make_eo_vec(*self)); // Assumes commutability
			}
		}
		if (self->type() != other->type()) {
			ExternalObject remove_ptr = equals_method;
			THROW_ERROR(Exceptions::comparison_different_types(*self, *other))
		}
		// Default implementation requires that types are the same
		return ExternalObject(equals_method)(args = make_eo_vec(*other));
	}, CppFunction::empty_type_vec
); EMPLACE("Eq")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		return get_object_as_bool(*self) && get_object_as_bool(*other);
	}, CppFunction::empty_type_vec
); EMPLACE("And")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		return get_object_as_bool(*self) || get_object_as_bool(*other);
	}, CppFunction::empty_type_vec
); EMPLACE("Or")

o = make_monadic_method<Aliases::BoolT, Aliases::BoolT>(
	[](Aliases::BoolT* self) -> Aliases::BoolT {
		return !*self;
	}
); EMPLACE("Not")
// CLASS

REASSIGN(cls)
ADD_BASIC_MRO(cls)
//print("cls", Classes::cls, Classes::cls.get<Aliases::CustomT>(), "!!", Classes::cls.ref_count_to<false>(Classes::cls.io_ptr, 0), "!!", Classes::object.ref_count_to<false>(Classes::object.io_ptr, 0));
print(Classes::cls, Classes::object);
print(Classes::cls.refcount(), Classes::object.refcount());
o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		print("In constructor", arguments);
		Plumber::get();
        ExternalObject *cls;
		std::vector<ExternalObject*> ctor_args;
		argument_data->assign_variadic_args<1>(arguments, ctor_args, cls);
		print("HERE!");
		Aliases::CustomT& attrs = cls->attrs_of();
		auto mro_it = attrs.find("MRO");
		if (mro_it == attrs.end()) {
			THROW_ERROR("Could not call Class with non-class type"s)
			return nullptr;
		}
		Types arg_1_type = cls->find_custom_enum();
		/*
		Types arg_1_type = Types::custom;
		for (auto& pair : enum_to_class) {
			if (cls->io_ptr == pair.second->io_ptr) {
				arg_1_type = pair.first;
				break;
			}
		}
		*/
		switch(arg_1_type) {
			case Types::null_type:
			{
				if (ctor_args.size() == 0)
					return nullptr;
				goto constructor_failure;
			}
			case Types::string:
			{
				static std::array<Types, 2> case_string_2 {Types::string, Types::number};
				switch (ctor_args.size()) {
					case 0:
						return ""s;
					case 1:
					{
						std::ostringstream osstream;
						osstream << ctor_args[0];
						return osstream.str();
					}
					case 2:
						if (type_match(ctor_args, case_string_2)) {
							std::ostringstream osstream;
							Aliases::StringT& str = ctor_args[0]->get<Aliases::StringT>();
							Aliases::NumT&	count = ctor_args[1]->get<Aliases::NumT>();
							for (size_t i = 0; i < count; ++i)
								osstream << str;
							return osstream.str();
						 }
						print("NO TYPE MATCH :(");
        	            goto constructor_failure;
				    default:
						print("NO!");
					    goto constructor_failure;
						
				}
			}
			case Types::number:
			{
				switch (ctor_args.size()) {
					case 0:
						return 0;
					case 1:
					{

						print("arg 2 actual type:", ctor_args[0]->type());
						Types arg_2_type = ctor_args[0]->get_enum_type();
						print("arg_2_type:", arg_2_type);
						if (arg_2_type == Types::string) {
							try {
								print("Attempting to construct from string");
								Aliases::NumT int_from_str(ctor_args[0]->get<Aliases::StringT>());
								print("Got", int_from_str);
								return int_from_str;
							} catch (const std::runtime_error&) {
								std::ostringstream oss;
								oss << "Could not construct integer from " << *ctor_args[0];
								THROW_ERROR(oss.str());
							}
						} else {
							goto constructor_failure;
						}
					}
				}
			}
			case Types::boolean:
			{
				switch(ctor_args.size()) {
					case 0:
						return false;
					case 1:
					{
						ExternalObject& eo = *ctor_args[0];
						/*
						if (eo.type() = Types::custom) {
							Aliases::CustomT::iterator it;
							Aliases::CustomT& attrs = eo.get<Aliases::CustomT>();
							if ((it = attrs.find("Bool")) != attrs.end()) {
								return (*it)();
							}
						}
						*/
						void *boolean_func = eo.get_attr_no_throw("ToBool");
						if (boolean_func) {
							return ExternalObject(boolean_func)();
						}
						return true; // Other values are truthy by default
					}
				}
			}
			case Types::array:
			{
				switch(ctor_args.size()) {
					case 0:
						return Aliases::ArrayT();
					case 1:
						return ExternalObject(ctor_args[0]->get_attr("ToArray"))();
					default:
						goto constructor_failure;
				}
			}
			case Types::base_exception:
			{
				switch(ctor_args.size()) {
					case 0:
						return reinterpret_cast<void*>(new InternalObject<Aliases::BaseExceptionT>(""s, *cls));
					case 1:
						/*
						if (ctor_args[0].type() == Types::string) {
							return reinterpret_cast<void*>(new InternalObject<Aliases::BaseExceptionT>(ctor_args[0].get<Aliases::StringT>()));
						} else {
							goto constructor_failure;
						}
						*/
						return reinterpret_cast<void*>(new InternalObject<Aliases::BaseExceptionT>(*ctor_args[0], *cls));
					default:
						goto constructor_failure;
				}
        	}
			case Types::code_block:
			{
				if (ctor_args.empty()) {
					return Aliases::CodeBlockT(0, true);
				}
				break;
			}
			case Types::if_chain:
			{
				if (ctor_args.size() == 1) {
					return Aliases::IfT(get_object_as_bool(*ctor_args[0]));
				}
				break;
			}
			case Types::custom:
			{
				/*
				It's possible that this type is a subclass of another type.
				E.g. exceptions subclass ExcBase but are of type custom.
				To instantiate a type like this, look for the first Call in its
				MRO. Here, the first call is in Object.
				E.g. ExcBreak has:
				[ExcBreak, ExcControl, ExcRuntime, ExcBase, Object]
				Iterate through each item look for Call method
				*/
				print("CUSTOM");
				return 1234;
				/*
				for (auto& attr : attrs) {
					if (pair.first == "Call")
				}
				*/
			}
		}
		print("Sup!");
        constructor_failure:
		//std::ostringstream oss;
		//show_simple_arg_error(oss, arguments);
		ctor_args.clear();
		THROW_ERROR(str_simple_type_error(arguments))
		return nullptr;
	}, std::vector<Types>{Types::any}
); EMPLACE("Call")

// NULLTYPE

REASSIGN(null_type)
ADD_BASIC_MRO(null_type)

print("ADDING");
o = make_monadic_method<Aliases::NullT, Aliases::BoolT>(
	[](Aliases::NullT*) -> Aliases::BoolT {
		return false;
	}
); EMPLACE("ToBool")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		return true;
	}
); EMPLACE("Eq")

print("ADDED");

// BOOLEAN

REASSIGN(boolean)
ADD_BASIC_MRO(boolean)

o = make_monadic_method<Aliases::BoolT, Aliases::BoolT>(
	[](Aliases::BoolT* self) -> Aliases::BoolT {
		return *self; // Whether self is true or false, but already boolean
	}
); EMPLACE("ToBool")

// STRING

REASSIGN(string)
ADD_BASIC_MRO(string)
print(Classes::string.get<Aliases::CustomT>(),Classes::cls.get<Aliases::CustomT>(),Classes::object.get<Aliases::CustomT>());

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		return *self == *other;
	}
); EMPLACE("_Eq")

o = make_monadic_method<Aliases::StringT, Aliases::ArrayT>(
	[](Aliases::StringT* str) -> Aliases::ArrayT {
		Aliases::ArrayT array(str->size());
		size_t i = 0;
		std::for_each(
			str->begin(), str->end(),
			[&](char c) {
				print("Copying ", c);
				array[i++] = std::string(1, c);
			}
		);
		return array;
	}
); EMPLACE("ToArray")

o = make_monadic_method<Aliases::StringT, Aliases::NullT>(
	[](Aliases::StringT* str) -> Aliases::NullT {
		std::for_each(
			str->begin(), str->end(),
			[](char& c) { c = std::tolower(c); }
		);
		print("String:", *str);
		return nullptr;// *str;
	}
); EMPLACE("toLower")

o = make_monadic_method<Aliases::StringT, Aliases::NullT>(
	[](Aliases::StringT* str) -> Aliases::NullT {
		std::for_each(
			str->begin(), str->end(),
			[](char& c) { c = std::toupper(c); }
		);
		return nullptr;// *str;
	}
); EMPLACE("toUpper")

o = make_monadic_method<Aliases::StringT, size_t>(
	[](Aliases::StringT* str) -> size_t {
		return str->size();
	}
); EMPLACE("Length")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
	std::string *self = nullptr, *other = nullptr;
	if (!argument_data->assign_args<2>(arguments, self, other)) {
		return nullptr;
	}
	std::string str_sum = *self + *other;
	return str_sum;

	}, std::vector<Types>{Types::string, Types::string}
); EMPLACE("Add")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		std::string *self = nullptr;
		Aliases::NumT *index = nullptr;
		argument_data->assign_args<2>(arguments, self, index);
		if (*index >= self->size() || *index < 0 || *index >= std::numeric_limits<size_t>::max() - 1) {
			THROW_ERROR(Exceptions::out_of_range(*index))
		}
		return std::string(1, (*self)[static_cast<size_t>(*index)]);
	}, std::vector<Types>{Types::string, Types::number}
); EMPLACE("Call")

o = make_monadic_method<Aliases::StringT, Aliases::BoolT>(
	[](Aliases::StringT* self) -> Aliases::BoolT {
		return self->size() != 0;
	}
); EMPLACE("ToBool")

// INTEGER

REASSIGN(integer)
ADD_BASIC_MRO(integer)

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		ExternalObject *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		return *self == *other;
	}
); EMPLACE("_Eq")

o = make_monadic_method<Aliases::NumT, Aliases::ArrayT>(
	[](Aliases::NumT* num) -> Aliases::ArrayT {
		return Aliases::ArrayT(static_cast<size_t>(*num));
	}
); EMPLACE("ToArray")

o = make_monadic_method<Aliases::NumT, Aliases::BoolT>(
	[](Aliases::NumT* num) -> Aliases::BoolT {
		return *num != 0;
	}
); EMPLACE("ToBool")

o = make_monadic_method<Aliases::NumT, Aliases::NumT>(
	[](Aliases::NumT* num) -> Aliases::NumT {
		return -*num;
	}
); EMPLACE("Neg")

o = make_monadic_method<Aliases::NumT, Aliases::NumT>(
	[](Aliases::NumT* num) -> Aliases::NumT {
		return ++*num;
	}
); EMPLACE("PreInc")

o = make_monadic_method<Aliases::NumT, Aliases::NumT>(
	[](Aliases::NumT* num) -> Aliases::NumT {
		return (*num)++;
	}
); EMPLACE("PostInc")

o = make_monadic_method<Aliases::NumT, Aliases::NumT>(
	[](Aliases::NumT* num) -> Aliases::NumT {
		print("FACT!!");
		if (*num < 0) {
			THROW_ERROR(Exceptions::out_of_range(*num))
		}
		print("Positive");
		unsigned int counter = static_cast<unsigned int>(*num);
		Aliases::NumT n = 1;
		while (counter) {
			n *= counter;
			--counter;
		}
		return n;
	}
); EMPLACE("Fact")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self = nullptr, *other = nullptr;
		argument_data->assign_args<2>(arguments, self, other);
		Aliases::NumT result = *self + *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Add")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self = nullptr, *other = nullptr;
		if (!argument_data->assign_args<2>(arguments, self, other)) {
			return nullptr;
		}
		bmp::cpp_int result = *self - *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Sub")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self = nullptr, *other = nullptr;
		if (!argument_data->assign_args<2>(arguments, self, other)) {
			return nullptr;
		}
		bmp::cpp_int result = *self * *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Mul")


o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self = nullptr, *other = nullptr;
		if (!argument_data->assign_args<2>(arguments, self, other)) {
			return nullptr;
		}
		bmp::cpp_int result = *self % *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Mod")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self, *other;
		argument_data->assign_args<2>(arguments, self, other);
		//cout <<"Pow:" << *self << " " << *other << " " << (*other < 0);
		if (*other == 0) {
			std::ostringstream oss;
			oss << "Divison by zero: " << *self << "/" << *other;
			//THROW_ERROR(GET_EXC("ExcZeroDiv")(oss.str()))
			THROW_ERROR(MKCSTM("Message",oss.str(), "Type",all_builtins["ExcZeroDiv"]))
			//THROW_ERROR(oss.str())
			return nullptr;
		}
		Aliases::NumT result = *self / *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Div")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self = nullptr, *other = nullptr;
		if (!argument_data->assign_args<2>(arguments, self, other)) {
			return nullptr;
		}
		bmp::cpp_int result = *self * *other;
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Call")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::NumT *self, *other;
		argument_data->assign_args<2>(arguments, self, other);
		//cout <<"Pow:" << *self << " " << *other << " " << (*other < 0);
		if (*other < 0) {
			std::ostringstream oss;
			oss << "Negative integer exponent currently unsupported: " << *self << "**" << *other;
			THROW_ERROR(oss.str())
			return nullptr;
		}
		unsigned exponent = static_cast<unsigned>(*other);
		Aliases::NumT result = bmp::cpp_int(bmp::pow(*self, exponent));
		return result;
	}, std::vector<Types>{Types::number, Types::number}
); EMPLACE("Pow")

// ARRAY

REASSIGN(array)
ADD_BASIC_MRO(array)

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::ArrayT *self = nullptr;
		ExternalObject *head = nullptr;
		std::vector<ExternalObject*> tail;
		argument_data->assign_variadic_args<2>(arguments, tail, self, head);
		// a.append(b,c,d,e): self = a, head = b, tail = [c,d,e]
		self->resize(self->size() + 1 + tail.size());
		self->push_back(*head);
		for (ExternalObject* el : tail) {
			self->push_back(*el);
		}
		print(*self, self->capacity(), self->size());
		return nullptr;
	}, std::vector<Types>{Types::array}
); EMPLACE("append")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
	Aliases::ArrayT *self = nullptr;
	Aliases::NumT *index = nullptr;
	argument_data->assign_args<2>(arguments, self, index);
	return (*self)[static_cast<size_t>(*index)];

	}, std::vector<Types>{Types::array, Types::number}
); EMPLACE("Call")

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::ArrayT *self, *other;
		argument_data->assign_args<2>(arguments, self, other);
		//cout <<"Pow:" << *self << " " << *other << " " << (*other < 0);
		Aliases::ArrayT result = *self;
		print("Copied result:", result); // Not a deep copy
		result += *other;
		print("List addition result:", result);
		return result;
	}, std::vector<Types>{Types::array, Types::array}
); EMPLACE("Add")

o = make_monadic_method<Aliases::ArrayT, Aliases::BoolT>(
	[](Aliases::ArrayT* self) -> Aliases::BoolT {
		return self->size() != 0;
	}
); EMPLACE("ToBool")

// BASEEXCEPTION

REASSIGN(base_exception)
ADD_BASIC_MRO(base_exception)

/*
o = make_monadic_method<Aliases::BaseExceptionT, ExternalObject>(
	[](Aliases::BaseExceptionT& exc) -> ExternalObject {
		print("GOT INSIDE METHOD");
		return exc.message;
	}
); EMPLACE("Message")
*/

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	make_eo_vec(), true, UL_LMBD {
		Aliases::BaseExceptionT *exc = nullptr;
		if (!argument_data->assign_args<1>(arguments, exc)) {
			return nullptr;
		}
		print("Exc:", exc);
		return exc->message;
	}, std::vector<Types>{Types::any}
); EMPLACE("Message")

// CODEBLOCK

REASSIGN(code_block)
ADD_BASIC_MRO(code_block)

// IF

REASSIGN(if_chain)
ADD_BASIC_MRO(if_chain)

print("Reassigned to If");

CppFunction temp(
	make_eo_vec(), false, UL_LMBD {
		Aliases::IfT *self = nullptr;
		ExternalObject *other = nullptr;
		argument_data->assign_args<1>(arguments, self, other);
		//print("self:", *self, "other:", *other);
		print(argument_data->optional_arguments);
		print("other:", other);
		if (*other == all_builtins["End"]) {
			self->evaluate();
			return nullptr;
		}
		Types arg_2_type = other->get_enum_type();
		print("Calling If", arg_2_type);
		if (arg_2_type == Types::code_block) {
			self->send_code_block(*other);
			// May need to preserve object for call later after original is destoyed
		}
		else if (arg_2_type == Types::if_chain) {
			self->send_if(other->get<Aliases::IfT>());
			// Don't need to preserve object; only need condition once
		}
		else {
			THROW_ERROR(Exceptions::arg_types_wrong(arguments))
		}
		print("Finished call");
		return *self;
	}, std::vector<Types>{Types::if_chain, Types::any}
);

temp.optional_arguments = std::vector<ExternalObject>{all_builtins["End"]};

print("temp", temp.optional_arguments);

o = std::move(temp);

print("o", o, o.get<Aliases::CppFunctionT>().optional_arguments);

EMPLACE("Call")

#undef ADD_BASIC_MRO
#undef EMPLACE
#undef REASSIGN


#define PRC print("REFCOUNT:", Classes::object.refcount());
print("START");

ExternalObject *current;

#define ADD_CLASS(name, parent_name) \
	current = &all_builtins[name];\
	*current = MKCSTM(\
		"MRO",ExternalObject::shallow_copy_of(all_builtins[parent_name].get<Aliases::CustomT>()["MRO"]),\
		"Type",Classes::cls,\
		"Name",name##s\
	);\
	current->get<Aliases::CustomT>()["MRO"].get<Aliases::ArrayT>().emplace(0, *current, true);

ADD_CLASS("ExcRuntime", "ExcBase")
	ADD_CLASS("ExcControl", "ExcRuntime")
		ADD_CLASS("ExcReturn", "ExcControl")
		ADD_CLASS("ExcBreak", "ExcControl")
		ADD_CLASS("ExcContinue", "ExcControl")
	ADD_CLASS("ExcLogic", "ExcRuntime")
		ADD_CLASS("ExcArgs", "ExcLogic")
		ADD_CLASS("ExcArithmetic", "ExcLogic")
			ADD_CLASS("ExcZeroDiv", "ExcArithmetic")
			ADD_CLASS("ExcNegPow", "ExcArithmetic")
		ADD_CLASS("ExcBounds", "ExcLogic")
		ADD_CLASS("ExcName", "ExcLogic")
	ADD_CLASS("ExcEnviron", "ExcLogic")
		ADD_CLASS("ExcFile", "ExcEnviron")
		ADD_CLASS("ExcMemory", "ExcEnviron")
		ADD_CLASS("ExcRecursion", "ExcEnviron")

print(all_builtins["ExcRuntime"].get<Aliases::CustomT>(), all_builtins["ExcBase"].get<Aliases::CustomT>());
print(all_builtins["ExcRuntime"].io_ptr, all_builtins["ExcBase"].io_ptr);


#define DEFINE_TYPE_DIFF(enum_name, class_name) \
	builtin_dicts[Types::enum_name].try_emplace("Type", Classes::class_name, true); \
	//builtin_dicts[Types::enum_name].try_emplace("MRO", ExternalObject(MKRRY_W(Classes::class_name, Classes::object)));
#define DEFINE_TYPE(name) DEFINE_TYPE_DIFF(name, name)

DEFINE_TYPE_DIFF(number, integer)
DEFINE_TYPE_DIFF(dictionary, dict)

DEFINE_TYPE(null_type)
DEFINE_TYPE(string)
DEFINE_TYPE(boolean)
DEFINE_TYPE(cpp_function)
DEFINE_TYPE(cpp_function_view)
DEFINE_TYPE(pair)
DEFINE_TYPE(array)
DEFINE_TYPE(base_exception)
DEFINE_TYPE(code_block)
DEFINE_TYPE(if_chain)
DEFINE_TYPE(generic_singleton)
DEFINE_TYPE(custom)

#undef DEFINE_TYPE
#undef DEFINE_TYPE_DIFF

//builtin_dicts[Types::base_exception].try_emplace("Message", nullptr);

// Emplaces weak reference; stops 2 references stopping eachother from being deleted

/*
HashableArray<ExternalObject> v(1);
auto no_effect = Classes::object;
v[0] = Classes::object;
*/

all_builtins.emplace("SpPuts",
	CppFunction(make_eo_vec(), true, UL_LMBD {
		std::vector<ExternalObject*> objects;
		if (!argument_data->assign_variadic_args<0>(arguments, objects))
			return nullptr;
#ifdef SHOW_CLOG
		cout << "SPPUTS:";
		cout << FG_GENERIC;
#endif
		for (size_t i = 0; i < objects.size(); ++i) {
			if (i != 0) cout << " ";
			repr_for_print(cout, *objects[i]);
			//cout.flush();
		}
#ifdef SHOW_CLOG
		cout << FBG_DEFAULT;
#endif
		//cout << std::endl;
		return nullptr;
	})
);

all_builtins.emplace("Puts",
	CppFunction(make_eo_vec(), true, UL_LMBD {
		std::vector<ExternalObject*> objects;
		argument_data->assign_variadic_args<0>(arguments, objects);
#ifdef SHOW_CLOG
		cout << "PUTS:";
		cout << FG_GENERIC;
#endif
		for (ExternalObject const* item : objects) {
			repr_for_print(cout, *item);
			//cout.flush();
		}
#ifdef SHOW_CLOG
		cout << FBG_DEFAULT;
#endif
		return nullptr;
	})
);

all_builtins.emplace("PutNL",
	CppFunction(make_eo_vec(), false, UL_LMBD {
			argument_data->assign_args<0>(arguments);
			cout << "\n";
			return nullptr;
		}
	)
);

all_builtins.emplace("GetLine",
	CppFunction(make_eo_vec(""s), false, UL_LMBD {
			std::string *str, input_line;
			// input_line is not a pointer because C++ syntax is weird
			argument_data->assign_args<0>(arguments, str);
#ifdef SHOW_CLOG
			cout << FG_GENERIC;
#endif
			cout << *str; // Print message
#ifdef SHOW_CLOG
			cout << FBG_DEFAULT;
#endif
			std::getline(std::cin, input_line);
			return input_line;
		}
	)
);

all_builtins.emplace("Len",
	CppFunction(make_eo_vec(), false, UL_LMBD {
		argument_data->assign_args<0>(arguments);
		cout << "\n";
		return nullptr;
		}
	)
);

all_builtins.emplace("Rand",
	CppFunction(make_eo_vec(), false, UL_LMBD {
		Aliases::NumT *start = nullptr, *end = nullptr;
		argument_data->assign_args<2>(arguments, start, end);

		static boost::random::random_device generator;

		boost::random::uniform_int_distribution<Aliases::NumT> uniform(*start, *end);
		return uniform(generator);
	})
);

all_builtins.emplace("_CppEnum",
	CppFunction(make_eo_vec(), false, UL_LMBD {
			ExternalObject *obj;
			argument_data->assign_args<1>(arguments, obj);
			std::ostringstream oss;
			oss << obj->type();
			return oss.str();
		}
	)
);

all_builtins.emplace("_CppEnumDeep",
	CppFunction(make_eo_vec(), false, UL_LMBD {
			ExternalObject *obj;
			argument_data->assign_args<1>(arguments, obj);
			std::ostringstream oss;
			oss << obj->get_enum_type();
			return oss.str();
		}
	)
);

#endif
