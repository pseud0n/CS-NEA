#ifndef LOOKUP_H
#define LOOKUP_H

//cout << "a is " << setw(2) << setfill('0') << hex << (int) a ; 

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

using AttrsMapT = std::unordered_map<Types, std::unordered_map<const char*, ExternalObject>>;

print("Empty");

//builtin_objects[Types::blank].get<Aliases::CustomT>().emplace("Type", ExternalObject(builtin_objects[Types::blank], true));


/*
SET_TYPE_WEAK(Types::blank, "Type", builtin_objects[Types::blank])
SET_TYPE_WEAK(Types::string, "Type", builtin_objects[Types::blank])

SET_TYPE_STRONG(Types::string, "lower", UL::CppFunction(
	{}, false, UL_LMBD {
		std::string str_obj;
		if (!argument_data->assign_args<1>(arguments, str_obj)) {
			return nullptr;
		}
		std::for_each(str_obj.begin(), str_obj.end(), [](char& c) { c = std::tolower(c); });
		return str_obj;
	}, {UL::Types::string}
))

#undef SET_TYPE_HELPER
#undef SET_TYPE_WEAK
#undef SET_TYPE_STRONG
*/

#define EMPLACE(name) class_ptr->emplace(name, std::move(o));

#define REASSIGN(type) \
	class_ptr = &Classes::type.get<Aliases::CustomT>(); \
	class_ptr->try_emplace("Type", Classes::object, true);

#define ADD_BASIC_MRO(type) \
	class_ptr->try_emplace("MRO", MKRRY_W(Classes::type, Classes::object));

print("Setting up members");

Aliases::CustomT *class_ptr;
ExternalObject o;


REASSIGN(object)
class_ptr->try_emplace("MRO", MKRRY_W(Classes::object)); // Array of weak references
//print(*class_ptr, Classes::object.refcount());

o = ExternalObject::emplace<Aliases::CppFunctionT>(
	CppFunction::empty_eobject_vec, true, UL_LMBD {
		ExternalObject cls;
		std::vector<ExternalObject> ctor_args;
		print("In func", arguments, cls);
		if (!argument_data->assign_variadic_args<1>(arguments, &ctor_args, cls)) {
			print("!!");
			return nullptr;
		}
		print("args:", cls, ctor_args);
		switch(cls.type()) {
			case Types::string:
				print("In string switch!");
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
						if (ctor_args[0].type() == Types::string && ctor_args[1].type() == Types::number) {
							std::ostringstream osstream;
							Aliases::StringT& str = ctor_args[0].get<Aliases::StringT>();
							Aliases::NumT&	count = ctor_args[1].get<Aliases::NumT>();
							for (size_t i = 0; i < count; ++i)
								osstream << str;
							return osstream.str();
						}
						throw_error(0);
						return nullptr;
				}
				default:
					throw_error(0);
		}
		return nullptr;
	}, std::vector<Types>{Types::any}, Types::any
);

EMPLACE("Call")

REASSIGN(string)
ADD_BASIC_MRO(string)

o = make_monadic_method<Aliases::StringT, Aliases::StringT>(
	[](Aliases::StringT& str) -> Aliases::StringT {
		std::for_each(
			str.begin(), str.end(),
			[](char& c) { c = std::tolower(c); }
		);
		return str;
	}
); EMPLACE("lower")

o = make_monadic_method<Aliases::StringT, Aliases::StringT>(
	[](Aliases::StringT& str) -> Aliases::StringT {
		std::for_each(
			str.begin(), str.end(),
			[](char& c) { c = std::toupper(c); }
		);
		return str;
	}
); EMPLACE("upper")

o = make_monadic_method<Aliases::StringT, size_t>(
	[](Aliases::StringT& str) -> size_t {
		return str.size();
	}
); EMPLACE("Length")

REASSIGN(base_exception)
ADD_BASIC_MRO(base_exception)

#undef ADD_BASIC_MRO
#undef EMPLACE
#undef REASSIGN


//print("class_ref:", Classes::object.get<Aliases::CustomT>());

builtin_dicts[Types::string].try_emplace("Type", Classes::string, true);
// Emplaces weak reference; stops 2 references stopping eachother from being deleted

#endif