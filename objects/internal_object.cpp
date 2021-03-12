#ifndef INTERNAL_OBJECT_CPP
#define INTERNAL_OBJECT_CPP

template <typename StoredT>
template <typename... CtorTs>
void InternalObject<StoredT>::emplace(CtorTs&&... args) {
	/*
	auto io_ptr = new InternalObject<StoredT>;
	io_ptr.type = AssociatedData<StoredT>::enum_type
	reference_count = 1;
	io_ptr.is_immovable = AssociatedData<StoredT>::is_immovable
	*/

	stored_value.emplace(std::forward<CtorTs>(args)...);
}

/*
template <typename StoredT>
InternalObject::InternalObject(InternalObject<StoredT>&& from)
	: type(from.type), reference_count(1), is_immovable() {
 }
*/

template <typename StoredT>
template <typename T>
InternalObject<StoredT>* InternalObject<StoredT>::copy(const T& object) { // pass by value; copy
	print("Making copy of object", typeid(T).name());
	//T object_copy(object); // Copy assignment operator; new, independent object made
	//print("Made copy of object", &object_copy, &object);
	return new InternalObject<StoredT>(object);
}


template <typename StoredT>
InternalObject<StoredT>::InternalObject()
	: type(AssociatedData<StoredT>::enum_type), reference_count(1),
		is_immovable(AssociatedData<StoredT>::is_immovable),
		stored_value(std::nullopt) {}

template <typename StoredT>
template <typename... CtorTs>
InternalObject<StoredT>::InternalObject(CtorTs&&... args) // e.g. std::string
	: type(AssociatedData<StoredT>::enum_type), reference_count(1), is_immovable(AssociatedData<StoredT>::is_immovable) {
	stored_value.emplace(std::forward<CtorTs>(args)...);
	// didn't use emplace function since it uses and extra std::forward
	/*
	If a large object like a long string is passed in, it should be
	moved, not copied.

	If we construct from a new dictionary, that can be moved when
	passed in to the function, or copied otherwise
	*/
}



template <typename StoredT>
InternalObject<StoredT>::~InternalObject() {
	//if (attrs) delete attrs; // If dictionary exists, delete it
};

#endif
