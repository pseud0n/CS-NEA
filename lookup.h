#ifndef LOOKUP_H
#define LOOKUP_H

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

std::unordered_map<Types, std::unordered_map<const char*, CppFunction>> builtin_lookup {
	{Types::number, {}},
	{Types::string, {}}
};


//builtin_lookup.emplace();

#endif