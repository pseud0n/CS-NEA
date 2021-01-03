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
attributes would be used to generate a dictionary on--the-fly.

When we do
	myStr = "this is a string"
	$ myStr.Len
	$ myStr.GetAttr("Len")
	$ myStr.Dict("Len")
we are treating myStr as if it has its own dictionary, which it does not.
On 2, it recognises that myStr is a string so it looks in a map of maps to find
the method.
It can't use a constexpr hash function because if there was a collision, then it
would not know and the resulting jump table may end up with an in accessible
label. Instead, there is a 2D unordered_map used.
*/

#include <unordered_map>

const std::unordered_map<Types, std::unordered_map<const char*, CppFunction>>

#endif