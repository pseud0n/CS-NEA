#ifndef PRE_DECL_H_
#define PRE_DECL_H_

//auto blank_function = new UL::CppFunction({}, false, DY_LMBD { return new UL::Object(nullptr); });

#define MIN_CACHED_VALUE -5
#define MAX_CACHED_VALUE 10

std::array<OPTR, 1 + MAX_CACHED_VALUE - MIN_CACHED_VALUE> cached_numbers;

for (int i = MIN_CACHED_VALUE; i <= MAX_CACHED_VALUE; ++i)
	cached_numbers[i - MIN_CACHED_VALUE].create_from_blank(i);

/*

When an integer is created, it doesn't need to allocate a new object onto the heap.
If the number has already been cached, then the new integer simply becomes a pointer to that.
When the pointer is deleted, the number of references to the original object is decremented.

When an object is passed into a function, it can be passed by reference or by value.
If it's passed by value, a new heap object is created as a copy of the object.
The passed-in value has no internal relation with the passed-in object.
If it's passed by reference, no new object is created.
The variable inside still refers to the outside variable.
It is simply a pointer to the same object.
As a result, any modification to the local object will modify the outer object.
It can be guaranteed that the other object will not be implicitly deleted before the scope ends.

When passed by reference using `Object::make_reference()`, the resulting pointer preserves strength.
*/

#endif
