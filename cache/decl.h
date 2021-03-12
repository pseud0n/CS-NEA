////////////////////////////////////////////////////////////////////////////////

#ifdef DO_CACHE_DECL

#ifndef CACHE_DECL_H
#define CACHE_DECL_H

// In namespace UL

namespace Cache {

	std::array<InternalObject<Aliases::NumT>*, 1 + MAX_CACHED_INTEGER_VALUE - MIN_CACHED_INTEGER_VALUE> numbers;

	/*
	The numbers which are to be cached; references to integers within this range
	are all identical and do not modify reference counts. No new objects are
	constructed and only 8 bytes are used on the stack. Beyond the cached range,
	numbers are entirely new objects.
	*/

	Aliases::NumT
		min = MIN_CACHED_INTEGER_VALUE,
		max = MAX_CACHED_INTEGER_VALUE;

	void *bool_true = 	new InternalObject<Aliases::BoolT>(true);
	void *bool_false = 	new InternalObject<Aliases::BoolT>(false);
	/*
	*/

} // Cache

#endif
#endif